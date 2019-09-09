// Robert Smith, Brian Thervil, Nick Watts
// Fall 2019
// COP4610 Operating Systems
// Project 1

#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Struct utilized for commands and parameters
typedef struct
{
  char** tokens;
  int numTokens;
} instruction;

// Declaring Functions
void loop();
void addToken(instruction* instr_ptr, char* tok);
void printTokens(instruction* instr_ptr);
void clearInstruction(instruction* instr_ptr);
void addNull(instruction* instr_ptr);
void execInstruction(instruction* instr_ptr);

// Main funcion that loops the process
int main()
{
  loop();
  
  return 0;
}

// Function that loops user input, parsing the input, then execute
// Code from parser_help.c
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

          	/*----------FOR NICK-------------
          	Shortcut Resolution Scheme
				5) If a relative path is used (doesn't start with root or home)
					then concatenate the argument string to the $PWD
				6) If a file occurs in an argument string, it must be placed at
					the end
				7) Return an error if the directory or file doesn't exist
			----------------------------------*/


          	else if (token[i] == '.')
          	{
          		char* cur_addr = getenv("PWD");

          		if (token[i + 1] == '.')
          		{
          			i++; //so the next loop of the for loop isn't stuck on the second '.' in ".."
          			int final_slash = 0, j = 0;

          			//This for loop finds the location of the final slash so the current directory
          			//will be chopped off, giving the abs. path for the parent directory.
          			for (j; j < strlen(cur_addr); j++)
          				if(cur_addr[j] == '/')
          					final_slash = j;
          			memcpy(temp, cur_addr, final_slash + 1);
          			free(cur_addr);
          			cur_addr = NULL;
          			//This captures the parent directory and stores it in temp, but it causes a seg fault when
          			//used with "cd" command.
          		}
          		else
          		{
          			//This should take PWD and place it in *token in place of '.'
          			memcpy(temp, cur_addr, strlen(cur_addr) - 1);
          			free(cur_addr);
          			cur_addr = NULL;
          		}
          	}
          	else if (token[i] == '~')
          	{
          		//This if block should capture the $HOME variable and replace the ~ in
          		// *token with that path string. I don't know if this will do that, though
          		// due to unfamiliarity with memcpy and Cstrings 
          		char* home_addr = getenv("HOME");
          		memcpy(temp, home_addr, strlen(home_addr) - 1);
          		free(home_addr);
          		home_addr = NULL;
          	}
      	  }
      
      	  //Because I don't yet understand how the temp cstring is being appended to the instr pointer, and what that changes,
      	  //this code may need to be updated by you guys. However, the steps above should be the logic for shortcut resolution,
      	  //barring relative paths
		
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

// Function Block that handles the execution of commands
void execInstruction(instruction* instr_ptr)
{
	char *action[instr_ptr->numTokens];
	char *bin="/bin/";
	int i=0;
	int j=0;
	pid_t pid;
	for (i=0; i<instr_ptr->numTokens; i++)
	{
		action[i]=NULL;
	}
	do
	{	
	
		//printf("%s\n", action[0]);
		for( j=0; instr_ptr->tokens[i] != "|"; j++)
		{	printf("%s\n",j);
			if(j==0)
			{	printf("%s\n", "hi");	
				action[0]=malloc((10+sizeof(instr_ptr->tokens[i]))*sizeof(char));
				
				
				action[0]=bin;;
				
				strcat(action[0],instr_ptr->tokens[i]);
				
				action[0][strlen(action[0])]='\0';
				printf("%s\n", action[0]);
			}
			else
			{
				memcpy(action[j], instr_ptr->tokens[i], strlen(instr_ptr->tokens[i]));
			}

			if(instr_ptr->tokens[i]==NULL)
			{
				action[j]=NULL;
				break;
			}
			printf("%s\n", action);
		i++;
		}

		i++;
		pid = fork();
		if (pid < 0)
		{
			printf("problem executing $s\n");
		}
		else if( pid==0)
		{
			if(execv(action[0], action)==-1)
			{
				perror("problem");

			}

		}
		else
		{
			waitpid(pid, NULL, 0);
		}


	}while(instr_ptr->tokens[i] != NULL);
}
