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
    printf(":");
    printf(getenv("PWD"));
    printf(">");    
    do
    {
      scanf("%ms", &token);
      temp = (char*)malloc((strlen(token) + 1) * sizeof(char));
      
      int i;
      int start = 0;
      for (i = 0; i <strlen(token); i++)
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
      
      if (start <strlen(token))
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
//    printTokens(&instr);
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
	char * command[2];
	command[0] = "echo";
	command[1] = "cd";

	if (strcmp(instr_ptr->tokens[0], command[0]) == 0)	// Echo command
	{
		char * buffer;

		if (instr_ptr->tokens[1][0] == '$')	// Env Variable
		{
			buffer = malloc(strlen(instr_ptr->tokens[1]) - 1);
			int i;
			for (i = 0; i < (strlen(instr_ptr->tokens[1]) -1); i++)
			{
				buffer[i] = toupper(instr_ptr->tokens[1][i+1]);
			}
			
			char * var = getenv(buffer);
				if (var != NULL)
			{
				printf("%s\n",var);
			}
			else
			{
				printf("%s\n", "Variable error");
			}
		}
		else if (instr_ptr->tokens[1][0] == '"')	// Quoted string
		{						
			int i;
			for (i = 1; i < (instr_ptr->numTokens - 1); i++)
			{				
				if (instr_ptr->tokens[i][0] != '|')
				{
					printf(instr_ptr->tokens[i]);
					printf(" ");
				}
				else
				{
					break;
				}
			}
			printf("\n");
		}
		else	// direct/file display
		{
			FILE * file = fopen(instr_ptr->tokens[1], "r");

			if (file == NULL)
			{
				printf("%s\n", instr_ptr->tokens[1]);
			}
			else
			{
				buffer = malloc(255 * sizeof(char));
				fgets(buffer,255, (FILE*) file);
				printf(buffer);
			}
		}
	}
	else if (strcmp(instr_ptr->tokens[0],command[1]) == 0)	// Started to work on cd command
	{							// It doesn't crash when you use it
		if (chdir(instr_ptr->tokens[1]) != 0)		// But doesn't update the PWD visually
		{
			printf("%s\n", "chdir failed");
		}			
	}
	else
	{
		printf("%s\n", "not a valid command");
	}
}
