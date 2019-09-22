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
void Variables(instruction * ptr);
void Builtins(instruction * ptr);
void pathResolution(instruction* i_ptr);
void shortcutRes(instruction* i_ptr);

//
// Variables used for Alias
//
instruction aliasName;
instruction aliasCommand[50];

//
// Global integer for exit
//
int outProcess;

//
// Main funcion that loops the process
//
int main()
{
  aliasName.tokens = NULL;
  aliasName.numTokens = 0;

  int i;
  for (i = 0; i < 50; i++)
  {
     aliasCommand[i].tokens = NULL;
     aliasCommand[i].numTokens = 0;
  }

  outProcess = 0;

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
  
  while(outProcess != 1)
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
        	if (token[i] == '|' || token[i] == '>' || token[i] == '<' || token[i] == '&' || token[i] == '$')
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

	Variables(&instr);
	Builtins(&instr);
	shortcutRes(&instr);
	pathResolution(&instr);
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
	int background = -1;				// counter for background processes

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
		}
		else	// Keeps index of second command
		{
			addToken(&pi, instr_ptr->tokens[i]);
		}

		if ((p == 0) && (in.numTokens == 0) && (out.numTokens == 0) && (background < 0))
		{
			addToken(&command, instr_ptr->tokens[i]);
		}
	}

	addNull(&command);	//Add a NULL to all instructions
	addNull(&in);
	addNull(&out);
	addNull(&pi);

	if (command.tokens[0] != NULL)
	{
		if ((background == instr_ptr->numTokens - 2) || (background <= 0))
		{
			if (p > 0)	// Executes Pipes or IO Redirect
			{
				execPipe(&command, &pi, background);
			}
			else
			{
				execCommand(&command, &in, &out, background);
			}
		}
		else
		{
			perror("Invalid background");
		}
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
		if (background <= 0)	// If statement for background processes
		{
			waitpid(pid, &status, 0);
		}
		else
		{
			printf("[1] %d%s\n",pid);	// Prints the PID position of function
			waitpid(pid, &status, WNOHANG);	// waitpid() function to continue while function is running
		}
	}
}

//
// Function to expand variables
//
// Creates new instance of instruction to copy expanded
// tokens to then copies back to original
//
void Variables(instruction * ptr)
{
	instruction temp;
	temp.numTokens = 0;
	temp.tokens = NULL;

	int i;
	for (i = 0; i < ptr->numTokens - 1; i++)
	{
		if (ptr->tokens[i][0] == '$')
		{
			if (getenv(ptr->tokens[i+1]) != NULL)
			{
				addToken(&temp, getenv(ptr->tokens[i+1]));
				i++;
			}
			else
			{
				perror("Environmental variable error");
			}
		}
		else
		{
			int bool = -1;
			int j;
			for (j = 0; j < aliasName.numTokens; j++)
			{
				if (strcmp(aliasName.tokens[j],ptr->tokens[0]) == 0)
				{
					bool = j;
					int k;
					for (k = 0; k < aliasCommand[j].numTokens; k++)
					{
						addToken(&temp, aliasCommand[j].tokens[k]);
					}
				}
			}
			if (bool == -1)
			{
				addToken(&temp, ptr->tokens[i]);
			}
		}
	}

	clearInstruction(ptr);
	for (i = 0; i < temp.numTokens; i++)
	{
		addToken(ptr, temp.tokens[i]);
	}
	addNull(ptr);
	clearInstruction(&temp);
}

//
// Function to execute builtins
//
void Builtins(instruction * ptr)
{
	instruction temp;
	temp.tokens = NULL;
	temp.numTokens = 0;

	int i;
	for (i = 0; i < ptr->numTokens - 1; i++)
	{
		if (strcmp(ptr->tokens[i], "exit") == 0)
		{
			// exit shell
			outProcess = 1;
		}
		else if (strcmp(ptr->tokens[i], "cd") == 0)
		{
			// change directory
			if (ptr->tokens[i+1] != NULL)
			{
				if (chdir(ptr->tokens[i+1]) == 0)
				{
					setenv("PWD",ptr->tokens[i+1], 1);
				}
				i++;
			}
		}
		else if (strcmp(ptr->tokens[i], "alias") == 0)
		{
			// store a name for a given command
			int j;
			int count = 0;
			int bool = -1;
			for (j = 0; j < aliasName.numTokens; j++)	// Checks if alias is already defined
			{
				if (strcmp(ptr->tokens[i+1],aliasName.tokens[j]) == 0)
				{
					bool = j;
				}
			}

			if (bool == -1)	// If alias is not defined create a new one
			{
				addToken(&aliasName, ptr->tokens[i+1]);
				count = count + 2;
				for (j = i + 3; j < ptr->numTokens - 1; j++)
				{
					addToken(&aliasCommand[aliasName.numTokens-1], ptr->tokens[j]);
					count++;
				}
			} 
			else	// Update alias command if alias name exists
			{
				count = count + 2;
				clearInstruction(&aliasCommand[bool]);
                                for (j = i + 3; j < ptr->numTokens - 1; j++)
                                {
                                        addToken(&aliasCommand[bool], ptr->tokens[j]);
					count++;
                                }
			}
			i = i + count;
		}
		else if (strcmp(ptr->tokens[i],"unalias") == 0)
		{
	 		// remove alias name entry from list of aliases

			instruction tempName, tempCommand[50];
			tempName.tokens = NULL;
			tempName.numTokens = 0;

			int j;
			for (j = 0; j < 50; j++)
			{
				tempCommand[j].tokens = NULL;
				tempCommand[j].numTokens = 0;
			}

			for (j = 0; j < aliasName.numTokens; j++)
			{
				if (strcmp(aliasName.tokens[j], ptr->tokens[i+1]) != 0)
				{
					addToken(&tempName, aliasName.tokens[j]);
					int k;
					for (k = 0; k < aliasCommand[j].numTokens; k++)
					{
						addToken(&tempCommand[j],aliasCommand[j].tokens[k]);
					}
				}
			}
			
			clearInstruction(&aliasName);
			for (j = 0; j < 50; j++)
			{
				clearInstruction(&aliasCommand[j]);
			}

			for (j = 0; j < tempName.numTokens; j++)
			{
				addToken(&aliasName,tempName.tokens[j]);
				int k;
				for (k = 0; k < tempCommand[j].numTokens; k++)
				{
					addToken(&aliasCommand[j],tempCommand[j].tokens[k]);
				}
			}
			i++;
		}
		else
		{
			addToken(&temp, ptr->tokens[i]);
		}
	}
	
	clearInstruction(ptr);
	for (i = 0; i < temp.numTokens; i++)
	{
		addToken(ptr, temp.tokens[i]);
	}
	addNull(ptr);
}

void shortcutRes(instruction* i_ptr)
{
	char* look = NULL;
	char* fin_dir = NULL;
	char* buf_dir = NULL;
	char* append = NULL;
	char* par_dir = NULL;
	int k = 0, j = 0, y = 0;
	int i = 0;
	size_t first = 1, home = 1, buff = 1;
	for (i; i < i_ptr->numTokens - 1; i++)
	{
		if (i_ptr->tokens[i][0] != '|' && i_ptr->tokens[i][0] != '>' && i_ptr->tokens[i][0] != '<' && i_ptr->tokens[i][0] != '&')
		{
		//buf_dir = NULL;
		look = strchr((i_ptr->tokens)[i], '/');

		//This if block runs if the token is a relative path

		if (look != NULL && i_ptr->tokens[i][0] != '/')
		{	
			//This for loop scans the entirety of i_ptr->tokens[i] by letter

			for (j; i_ptr->tokens[i][j] != '\0'; j++)
			{
				// If the path contains a . or .. shortcut, this if block runs
				if ((i_ptr->tokens)[i][j] == '.')
				{	
					//If there is a .. token in the path name, then this if block runs

					if(i_ptr->tokens[i][j+1] == '.')
					{
						j++;
						int copy = 0, slash = 0;
						append = NULL;					//append, par_dir, and int variables 
						par_dir = NULL;					//made null
						if (first == 1)					
							par_dir = getenv("PWD");		//if it's the first time through, the PWD
						else						//is stored in par_dir, if not then the current 
							par_dir = buf_dir;			//entry in buf_dir is stored

						append = (char*) malloc (sizeof(char));
						append[copy] = '\0';
						if (first == 1)
							k = j + 2;
						else
							k = j + 1;
						for (k; i_ptr->tokens[i][k] != '\0'; k++)
						{
							append = realloc(append, (copy + 2) * sizeof(char*));

												//this copies what hasn't yet been scanned 
							append[copy] = i_ptr->tokens[i][k];	//and processed from i_ptr->tokens and places
							copy++;					//it into the append char*
						}
						append[copy] = '\0';

						for (y = 0; par_dir[y] != '\0'; y++)
						{						//This finds the location of the final
							if (par_dir[y] == '/')			//forward-slash in the code so that any
								slash = y;			//data after is not copied over into
						}						//buf_dir
						k = 0;
						

						/* This if statement prepares buf_dir to reconfigure its contents. If the next
						directory in i_ptr->tokens[i] is also a .. shortcut, then the contents of par_dir
						are inserted into buf_dir as they are. */

						if (append[0] == '.' || append[1] == '.')
						{
							if(first == 1)
								buf_dir = (char*) malloc((slash + 1) * sizeof(char));
							else
								buf_dir = (char*) realloc(buf_dir, (slash + 1) * sizeof(char));
							memcpy(buf_dir, par_dir, slash);
							buf_dir[slash+1] = '\0';
						}
						
						/* If the next directory is not a . or .. shortcut, then the next directory from 
						i_ptr->tokens[i] is copied into buf_dir */

						else
						{
							/* If this is the first loop, the information from par_dir is
							copied into buf_dir until it reaches the final slash */
							if (first == 1)
							{
								buf_dir = (char*) malloc((slash + 1) * sizeof(char));
								memcpy(buf_dir, par_dir, slash + 1);
								buf_dir[++slash] = '\0';
							}
							copy = slash;
							for(k = 0; append[k] != '\0'; k++)
							{
								buf_dir = (char*) realloc(buf_dir, (copy + 3) * sizeof(char));
								buf_dir[copy] = append[k];
								buf_dir[++copy] = '\0';
								j++;
								if (append[k+1] == '/')
									break;
							}
						}

						/* If par_dir == PWD and not whatever is in buf_dir, then the first flag
						is set to signify that the parent directory does not need to be copied from
						par_dir */
						if (strcmp(par_dir, getenv("PWD")) == 0)
							first = 2;
						buff = 1;
					}
					/* This if block runs only if the . token is at the beginning of the relative path.
					This is because any directory followed by './' is still within that same directory.
					Thus, the only time this runs is at the beginning to copy the PWD. */

					else if(i_ptr->tokens[i][j+1] == '/')
					{
						//This runs when attaching a pwd shortcut to the extending buffer directory
						if (j==0)
						{
							par_dir = getenv("PWD");
							buf_dir = (char*) malloc(sizeof(char));
							for(y = 0; par_dir[y] != '\0'; y++)
							{
								buf_dir = (char*) realloc(buf_dir, (y + 2) * sizeof(char));
								buf_dir[y] = par_dir[y];
								buf_dir[y+1] = '\0';
							}		
						}
						buff = 1;		
					}
				}

				/* This else if runs when a normal directory name is encountered in the stored relative
				directory. The directory is copied from the tokens array into the buf_dir array. */

				else if (i_ptr->tokens[i][j] == '/' && i_ptr->tokens[i][j+1] != '.' && i_ptr->tokens[i][j+1] != '\0')
				{
					int copy = j;
					int end = 0;
					while (buf_dir[end] != '\0')
					{ end++; }
					
					/* This while loop continuously adds letters from the tokens array while it
					does not encounter a null character. When the loop encounters a second /, the 
					loop breaks. */
					while(i_ptr->tokens[i][copy] != '\0')	
					{
						buf_dir = (char*) realloc(buf_dir, (end + 1) * sizeof(char));
						buf_dir[end++] = i_ptr->tokens[i][copy];
						buf_dir[end] = '\0';
						copy++;
						if(i_ptr->tokens[i][copy+1] == '/')
							break;
					}
					buff = 1;
				}
				/* This else if runs when there is a ~ in the relative path as long as it's the first
				symbol in the argument. Once the argument is processed, a flag is set so that it doesn't
				run in the future. */

				else if((i_ptr->tokens)[i][0] == '~' && home == 1)
				{
					par_dir = getenv("HOME");
					buf_dir = (char*) malloc (sizeof(char));
					buf_dir[0] = '\0';
					for(y = 0; par_dir[y] != '\0'; y++)
					{
						buf_dir = (char*) realloc (buf_dir, (y + 2)*sizeof(char));
						buf_dir[y] = par_dir[y];
						buf_dir[y+1] = '\0';
					}
					if (home == 1)
						home = 2;
					buff = 1;
				}
				/* This else if runs when there is a ~ in the path that does not occur at the beginning
				and says an error message. */
				else if(i_ptr->tokens[i][j] == '~' && j != 0)
				{
					printf("%s: not a valid directory", (i_ptr->tokens)[i]);
					i_ptr->tokens[i] = NULL;	
					break;
				}
			}
		}
		/* This else if occurs when the token == ".." It stores the location of the parent directory. */
		else if (look == NULL && strcmp((i_ptr->tokens)[i], "..") == 0)
		{
			buff = 2;
			par_dir = getenv("PWD");
			int x = 0, last_sl = 0;
			for (x; x < strlen(par_dir); x++)
				if(par_dir[x] == '/')
					last_sl = x;
			(i_ptr->tokens)[i] = (char*) realloc ((i_ptr->tokens)[i], last_sl+1 * sizeof(char));
			memcpy(i_ptr->tokens[i], par_dir, last_sl+1);
		}
		/* This else if occurs when token == "." and stores the location of PWD */
		else if (look == NULL && strcmp((i_ptr->tokens)[i], ".") == 0)
		{
			buff = 2;
			par_dir = getenv("PWD");
			i_ptr->tokens[i] = (char*) realloc(i_ptr->tokens[i], (strlen(par_dir)+1)*sizeof(char));
			memcpy(i_ptr->tokens[i], par_dir, strlen(par_dir));
			i_ptr->tokens[i][strlen(par_dir)] = '\0';	
		}
		/* This else if occurs when token == "~" and stores the location of $HOME */
		else if (look == NULL && strcmp((i_ptr->tokens)[i], "~")  == 0)
		{
			buff = 2;
			par_dir = getenv("HOME");
			i_ptr->tokens[i] = (char*) realloc(i_ptr->tokens[i], (strlen(par_dir)+1) * sizeof(char));
			memcpy(i_ptr->tokens[i], par_dir, strlen(par_dir));
			i_ptr->tokens[i][strlen(par_dir)] = '\0';
		}

		/* This if statement copies the contents of buf_dir into the tokens array if buf_dir is used and
		if the for loop has reached the end of the token. */
		if (i_ptr->tokens[i][j+1] == '\0' && buff == 1)
		{
			i_ptr->tokens[i] = (char*) realloc (i_ptr->tokens[i], (strlen(buf_dir)+1 * sizeof(char)));
			memcpy(i_ptr->tokens[i], buf_dir, strlen(buf_dir));
			i_ptr->tokens[i][strlen(buf_dir)] = '\0';
		}
	}
	}
	free(append);
	free(buf_dir);
}			

/* This function tries to resolve any tokens that are placed in the tokens array that don't contain a
forward-slash by comparing them to the paths in $PATH. If there is a file that exists when $PATH/token 
has been formed, it is stored in the tokens array. If not, the token remains unchanged in case it is
a built-in command. */
void pathResolution(instruction *i_ptr)
{
	int i = 0, j = 0, x = 0, start = 0;
	char* look = NULL;
	char* path = getenv("PATH");
	char* buf_dir = NULL;
	char* temp = NULL;
	size_t first = 1, found = 1;
	for (i; i < i_ptr->numTokens - 1; i++)
	{
		if (i_ptr->tokens[i][0] != '|' && i_ptr->tokens[i][0] != '>' && i_ptr->tokens[i][0] != '<' && i_ptr->tokens[i][0] != '&' && i_ptr->tokens[i][0] != '-')
		{
		look = strchr(i_ptr->tokens[i], '/');
		// This if statement runs if / is not in a given token in the tokens array
		if (look == NULL)
		{
			buf_dir = NULL;
			buf_dir = (char*) malloc (sizeof(char));
			for(x; path[x] != '\0'; x++)
			{

				/* If a : is reached, the path that has been scanned and copied into buf_dir
				is concatenated with the token, after which the program determines if the file
				exists or not. If it doesn't exist, then the next path is copied into buf_dir. 
				If the file does exist, the loop breaks. */
				if(path[x] != ':' && path[x+1] != '\0')
				{
					buf_dir = (char*) realloc (buf_dir, (start + 2) * sizeof(char));
					buf_dir[start] = path[x];
					buf_dir[start+1] = '\0';
					start++;
				}
				else
				{
					if (path[x] == '.' && path[x+1] == '\0')
					{
						buf_dir = (char*) realloc (buf_dir, (strlen(getenv("PWD"))+1) * sizeof(char));
						memcpy(buf_dir, getenv("PWD"), strlen(getenv("PWD")));
						start = strlen(getenv("PWD"))-1;
					}
					buf_dir = (char*) realloc (buf_dir, (start + 2) * sizeof(char));
					buf_dir[start++] = '/';
					buf_dir[start] = '\0';
					for(j = 0; i_ptr->tokens[i][j] != '\0'; j++)
					{
						buf_dir = (char*) realloc(buf_dir, (start + 2) * sizeof(char));
						buf_dir[start] = i_ptr->tokens[i][j];
						buf_dir[start+1] = '\0';
						start++;
					}
					FILE* pTest = fopen(buf_dir, "r");
					if (pTest != NULL)
					{
						fclose(pTest);
						found = 2;
						break;
					}
					else
					{
						if (path[x+1] == '\0')
						{
							//printf("%s: no file or command exists\n", i_ptr->tokens[i]);
							//i_ptr->tokens[i] = NULL;
						}
					}
					buf_dir = NULL;
					start = 0;
				}
				
			}
		}
		/* This if statement copies the path stored in buf_dir into the tokens array so it can be
		processed by execution. */
		if (buf_dir != NULL && found == 2)
		{
			i_ptr->tokens[i] = (char*) realloc(i_ptr->tokens[i], (start + 1)* sizeof(char));
	 		memcpy(i_ptr->tokens[i], buf_dir, start + 1);
		}
	}
	}
	free(buf_dir);
}
