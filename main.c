// Robert Smith, Brian Thervil, Nick Watts
// Fall 2019
// COP4610 Operating Systems
// Project 1

#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

//
// Struct utilized for commands and parameters
//
typedef struct
{
  char** tokens;
  int numTokens;
} instruction;

//
// Declaring Functions
//
void loop();
void addToken(instruction* instr_ptr, char* tok);
void printTokens(instruction* instr_ptr);
void clearInstruction(instruction* instr_ptr);
void addNull(instruction* instr_ptr);
void execInstruction(instruction* instr_ptr);
void execCommand(instruction * ptr, instruction * in, instruction * out, int background);
void execPipe(instruction * ptr, instruction * p, int background);

//
// Main funcion that loops the process
//
int main()
{
  loop();
  
  return 0;
}

//
// Function that loops user input, parsing the input, 
// expanding variables, executing builtins then executing the rest
//
// Code from parser_help.c
//
void loop()
{
  char* token = NULL;
  char* temp = NULL;
  
  instruction instr;
  instr.tokens = NULL;
  instr.numTokens = 0;
  
  while(1)
  {
    printf(getenv("USER"));
    printf("@");
    printf(getenv("MACHINE"));
    printf(" : ");
    printf(getenv("PWD"));
    printf(" > ");

    	do
    	{
    	  scanf("%ms", &token);
    	  temp = (char*)malloc((strlen(token) + 1) * sizeof(char));
      
    	  int i;
     	  int start = 0;
      	  for (i = 0; i < strlen(token); i++)
      	  {
        	if (token[i] == '|' || token[i] == '>' || token[i] == '<' || token[i] == '&')
        	{
          		if (i-start > 0)
          		{
            		memcpy(temp, token + start, i - start);
            		temp[i - start] = '\0';
            		addToken(&instr, temp);
          		}
          
          		char specialChar[2];
          		specialChar[0] = token[i];
          		specialChar[1] = '\0';
          		addToken(&instr, specialChar);
          
          		start = i+1;
          	}
	}	
      	if (start < strlen(token))
      	{
        	memcpy(temp, token + start, strlen(token) - start);
        	temp[i - start] = '\0';
        	addToken(&instr, temp);
      	}
      
      	free(token);
      	free(temp);
      
      	token = NULL;
      	temp = NULL;
    	} while ('\n' != getchar());

  	addNull(&instr);
//	printTokens(&instr);
   	execInstruction(&instr);	// Our code to execute commands after entry
    	clearInstruction(&instr);
    }
}

void addToken(instruction* instr_ptr, char* tok)
{
	//extend token array to accomodate an additional token
	if (instr_ptr->numTokens == 0)
		instr_ptr->tokens = (char**) malloc(sizeof(char*));
	else
		instr_ptr->tokens = (char**) realloc(instr_ptr->tokens, (instr_ptr->numTokens+1) * sizeof(char*));

	//allocate char array for new token in new slot
	instr_ptr->tokens[instr_ptr->numTokens] = (char *)malloc((strlen(tok)+1) * sizeof(char));
	strcpy(instr_ptr->tokens[instr_ptr->numTokens], tok);

	instr_ptr->numTokens++;
}

void addNull(instruction* instr_ptr)
{
	//extend token array to accomodate an additional token
	if (instr_ptr->numTokens == 0)
		instr_ptr->tokens = (char**)malloc(sizeof(char*));
	else
		instr_ptr->tokens = (char**)realloc(instr_ptr->tokens, (instr_ptr->numTokens+1) * sizeof(char*));

	instr_ptr->tokens[instr_ptr->numTokens] = (char*) NULL;
	instr_ptr->numTokens++;
}

void printTokens(instruction* instr_ptr)
{
	int i;
	printf("Tokens:\n");
	for (i = 0; i < instr_ptr->numTokens; i++) {
		if ((instr_ptr->tokens)[i] != NULL)
			printf("%s\n", (instr_ptr->tokens)[i]);
	}
}

void clearInstruction(instruction* instr_ptr)
{
	int i;
	for (i = 0; i < instr_ptr->numTokens; i++)
		free(instr_ptr->tokens[i]);

	free(instr_ptr->tokens);

	instr_ptr->tokens = NULL;
	instr_ptr->numTokens = 0;
}

//
// Function Block that handles the execution of commands
// Parses commands into single events separated by pipes or IO redirect
//
void execInstruction(instruction* instr_ptr)
{
	instruction command, in, out, pi;	// Created an incstruction object
	command.tokens = in.tokens = out.tokens = pi.tokens = NULL;	// which will hold individual commands
	command.numTokens = in.numTokens = out.numTokens = pi.numTokens = 0;	// before being cleared every | or NULL
	int p = 0;
	int i = 0;					// i for for loop
	int background = 0;				// counter for background processes

	for(i = 0; i < instr_ptr->numTokens - 1; i++)	// For loop to go through the whole line of commands
	{
		if (p == 0)	// If individual instruction is empty, then adds address of commands before being added as token
		{
			if ((char)instr_ptr->tokens[i][0] == '&') // Keeps index of background processes
                        {
                                background = i;
                        }
                        else if ((char)instr_ptr->tokens[i][0] == (char)'<')    // Keeps index of input redirection
                        {
                                addToken(&in, instr_ptr->tokens[i + 1]);
                        }
                        else if ((char)instr_ptr->tokens[i][0] == (char)'>')    // Keeps index of output redirection
                        {
                                addToken(&out, instr_ptr->tokens[i + 1]);
                        }
			else if ((char)instr_ptr->tokens[i][0] == (char)'|')
			{
				p++;
			}
                        else if ((in.numTokens == 0) && (out.numTokens == 0))
                        {
                                addToken(&command, instr_ptr->tokens[i]);
                        }
		}
		else	// Keeps index of second command
		{
			addToken(&pi, instr_ptr->tokens[i]);
		}
	}

	addNull(&command);	//Add a NULL to all instructions
	addNull(&in);
	addNull(&out);
	addNull(&pi);

	if (p > 0)	// Executes Pipes or IO Redirect
	{
		execPipe(&command, &pi, background);
	}
	else
	{
		execCommand(&command, &in, &out, background);
	}

	clearInstruction(&command);	// Clear memory
        clearInstruction(&out);
        clearInstruction(&in);
        clearInstruction(&pi);
	background = 0;
	p = 0;
}

//
// Function to execute piping
// Code used from lecture slides
//
void execPipe(instruction * command, instruction * p, int back)
{
	int status, status2;
	int fd[2];
	int pid2;
	int pid = fork();
	if (pid == 0)
	{
		pipe(fd);
		pid2 = fork();
		if (pid2 == 0)
		{
			close(STDOUT_FILENO);
			dup(fd[1]);
			close(fd[0]);
			close(fd[1]);
			if (execv(command->tokens[0], command->tokens) == -1)
			{
				perror("Invalid Command:");
			}
		}
		else if (pid2 == -1)
		{
			perror("Piping error");
		}
		else
		{
			close(STDIN_FILENO);
			dup(fd[0]);
			close(fd[0]);
			close(fd[1]);
			if (execv(p->tokens[0],p->tokens) == -1)
			{
				perror("Invalid Command:");
			}
		}
	}
	else if (pid == -1)
	{
		perror("Pipe error");
	}
	else
	{
		waitpid(pid, &status, 0);
		waitpid(pid2, &status2, 0);
	}
}

//
// Function to execute normal or IO commands
//
void execCommand(instruction * command, instruction * in, instruction * out, int background)
{
	pid_t pid;
	int status;
	int fd_in;
	int fd_out;

	pid = fork();
	
	if (pid == 0)
	{
	        if (in->tokens[0] != NULL)           
	        {
	                int fd_in = open(in->tokens[0], O_RDONLY);
	                if (fd_in > -1)         // Replace stdin with file '<'
        	        {
        	                close(STDIN_FILENO);
        	                dup(fd_in);
        	                close(fd_in);
        	        }
	        }        

	        if (out->tokens[0] != NULL)
	        {
	                int fd_out = open(out->tokens[0], O_RDWR | O_CREAT | O_TRUNC, 0600);
	                if (fd_out > -1)        // Replace stdout with file '>'
	                {
	                        close(STDOUT_FILENO);
	                        dup(fd_out);
		                close(fd_out);
                	}
	        }
		
		if (execv(command->tokens[0], command->tokens) == -1)
		{
			perror("Invalid Command!!!!!");
		}
	}
	else if (pid < 0)
	{
		perror("Error");
	}
	else
	{
		waitpid(pid, &status, 0);
	}
}
