/* Author: Josh Ceciliani
 * Title: smallsh
 * Date: 7/30/2018
 * Description: This is a shell to run basic commands such as status, cd and exit. This always used fork() and exec..() to create children to run fg and bg processes
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
//Foreground only mode switch
int fgom = 0;
// Function call for the sigaction SIGINT
void catchSIGINT(int signo)
{
	char* message = "terminated by signal 2\n";
	write(STDOUT_FILENO, message, 23);	 

}
// Function call for the sigaction SIGTSTP
void catchSIGTSTP(int singo)
{
	if(fgom == 0)
	{
		char* message = "\nEntering foreground-only mode(& is now ignored)\n";
		write(STDOUT_FILENO, message, 49);
		fgom = 1;
	}		
	else 
	{
		char* message = "\nExiting foreground-only mode\n";
		write(STDOUT_FILENO, message, 30);
		fgom = 0;
	}

}

int main()
{
	int outFile = -1; // Outfile in the children process to open and write
	char* outputFile = NULL; // Get the outfile in the arguments	
	int inFile = -1; // Input file in the children process to open and read
	char* inputFile = NULL; // Get the input file in the arguments
	char userInput[2048]; // Get the user input 
	//char firstArg[100]; // Used for first $$ argument
	//char secondArg[100]; // User for second $$ argument	
	const char space[3] = " \n"; // space const for all of the strtok calls
	char* totalArguments[512]; // Arguments array to hold all arguments given 
	char* token; // Token to store the parsed data from strtok
	int status = 0; // Stored global status to 0, until otherwise changed in the code
	int childPid; // Children pid to use with fork()
	struct sigaction SIGINT_action = {0}; // sigaction to catch SIGINT signals
	struct sigaction SIGTSTP_action = {0}; // sigaction to catch SIGTSTP signals

	//For SIGINT signal handling creation 
	SIGINT_action.sa_handler = catchSIGINT;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0;
	//Catches signal call - Does not exit shell
	sigaction(SIGINT, &SIGINT_action, NULL);	
	//For SUGTSTP signal handling creation
	SIGTSTP_action.sa_handler = catchSIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = 0;
	//Catches signal call - Does not exit shell
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	//While user doesn't type exit
	while(1)
	{
		// All processes are fg due to the 0 flag - until changed to 1 by bg flag 1
		// 0 = fg | 1 = bg	
		int foreGroundProcess = 0;
		//Contains all arguments for line about to be read in - used with strtok
		int numOfArguments = 0;

		// Print the : for the command line 
		printf(": ");
		fflush(stdout);
		
		//Get line from the user
		fgets(userInput, 2048, stdin);
/*
		// For loop for $$ part of the program 
		int l;
		int k;
		int len = strlen(userInput);
		int newlen;
		char pid = getpid();
		// Get first part of string, up to $$
		for(l = 0; l < len; l++)
		{
			// String copy until $$
			strcpy(firstArg[l], userInput[l]);		
		
			if(userInput[l] == "$" && userInput[(l + 1)] == "$")
			{	
				newlen = (l + 1);	
				
				// Get second part of string, until user input string is '\0'
				for(k = newlen; k < len; k++)
				{
					strcpy(secondArg[k], userInput[k]);
				}
				break;
			}
		}	
		// Place the string location of three combined strings
		asprintf(&pidHandling, "%s%s%s", firstArg, pid, secondArg);
*/
		// Tokenizes till newline - Following strtok's will be NULL in userInput
		// If userInput has $$, use the create string
		//if(strstr(userInput, "$$") != NULL)
		//{
			//token = strtok(pidHandling, space);
		//}
		// Else, use the regular string recieved from input
		//else
		//{
			token = strtok(userInput, space);
		//}

		// while the string/token is not NULL	
		while(token != NULL)
		{	// If strcmp hits file output symbol, enter if
			if(strcmp(token, ">") == 0)
			{
				// Go to next argument, which should be the file
				token = strtok(NULL, space);
				// "duplicate" new string in token, into outputFile
				outputFile = strdup(token);
				// Move to next argument
				token = strtok(NULL, space);
			}
			// If strcmp hits file input symbol, enter if 	
			else if(strcmp(token, "<") == 0)
			{		
				token = strtok(NULL, space);
				inputFile = strdup(token);
				token = strtok(NULL, space);

			}
			// If strcmp hits &(bg process), switch flag to 1 and break - last argument
			else if(strcmp(token, "&") == 0)
			{
				foreGroundProcess = 1;
				break;
			}
			else 
			{
				// If it is not <, > or &, must be an argument
				totalArguments[numOfArguments] = strdup(token);
				// Go to next argument, aka space
				token = strtok(NULL, space);
				// Increment argument count
				numOfArguments++;
			}
		
		}		
		//Set final argument in the list to NULL, or if no arguments are selected 
		totalArguments[numOfArguments] = NULL;
		
		//printf("Total arguments are: %s\n", totalArguments[0]);
		//fflush(stdout);

		//If arguments, aka no input, or a comment(#), continue
		if(totalArguments[0] == NULL || *(totalArguments[0]) == '#')
		{		
			continue;
		}
		//If first argument is status, display the status - either exit or singal
		else if(strcmp(totalArguments[0], "status") == 0)
		{
			//if(totalArguments[1] == "&")
			//{
			//	printf("terminated by signal %d\n", status);
			//	fflush(stdout);
			//	exit(0);
			//}
			
			if(WIFEXITED(status))
			{
				printf("exit value %d\n", WEXITSTATUS(status));	
				fflush(stdout);
			}
			else
			{
				printf("terminated by signal %d\n", status);
				fflush(stdout);
			}
		}
		//If first argument is cd, and second argument is name, change dir, else go to "HOME" dir
		else if(strcmp(totalArguments[0], "cd") == 0)
		{	
			if(totalArguments[1] == NULL)
			{
				chdir(getenv("HOME"));
			}
			else
			{
				chdir(totalArguments[1]);
			}	

		}
		//If first argument is exit, exit the program
		else if(strcmp(totalArguments[0], "exit") == 0)
		{
			exit(0);
		}
		//It is a non-built in command, lets get it!
		else
		{
			// Create the child with fork()
			childPid = fork();
			// Create result varaible for error testing
			int result = -5; 
			// Create switch statement
			switch(childPid)
			{
				case -1: 
					// perror the message
					perror("fork ");
					// Change global status to 1, or error
					status = 1;
					// break out of the switch	
					break;
				case 0:
					// if process is reading out(>) and != NULL
					if(outputFile != NULL)
					{
						// Create output file to write or trunc	
						outFile = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644); 
						// Error cheak to see if outFile == -1
						if(outFile == -1)
						{
							// print error about cannot open file
							printf("cannot open %s for output\n", outputFile);
							fflush(stdout);
							// Exit calling process - Status = 1?
							exit(1);	
						}	 
						// Error check to see if dup2 worked
						// Check result of dup2
						result = dup2(outFile, 1);
						// if dup2 errored 
						if(result == -1)
						{
							perror("dup2 ");
							exit(1);
						}	
					}
					
					// if process is reading in(<) and != NULL
					if(inputFile != NULL)
					{
						// Create input file to read
						inFile = open(inputFile, O_RDONLY); 
						// Error check to see if inFile == -1
						if(inFile == -1)
						{
							printf("cannot open %s for input\n", inputFile);
							fflush(stdout);
							exit(1);
						} 
						// Error check to see if inFile dup2 worked
						result = dup2(inFile, 0);
						if(result == -1)
						{
							perror("dup2 ");
							exit(1);
						} 
					}
					
					// if process is a background process, redirect stdin and stdout to /dev/null
					if(foreGroundProcess == 1 && fgom == 0)
					{
						inFile = open("/dev/null", O_RDONLY); 
						outFile = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
						// Error check inFile == -1
						if(inFile == -1)
						{
							perror("open ");
							exit(1);
						} 
						// Error check inFile for dup2
						result = dup2(inFile, 0);
						if(result == -1)
						{
							perror("dup2 ");
							exit(1);
						} 
						// Error check outFile == -1
						if(outFile == -1)
						{
							perror("open ");
							exit(1);
						} 
						// Error check inFile for dup2
						result = dup2(outFile, 1);
						if(result == -1)
						{
							perror("dup2 ");
							exit(1);
						} 
					}
					// Exec process with arguments 			
					if(execvp(totalArguments[0], totalArguments) < 0)
					{	
						char* file = totalArguments[0];	
						perror(file);
						exit(1);

					}
					break;
				//Parent 
				default:
					// If it is a foreGroundProcess and is foreGround-only mode, wait for it to be done	
					if(foreGroundProcess == 0 && fgom == 1)
					{
						// Wait for process before command line is back
						waitpid(childPid, &status, 0);
					}
					// If it is a foreGroundProcess, wait for it to be done
					else if(foreGroundProcess == 0)
					{
						waitpid(childPid, &status, 0);
					}
					// If it is a background process, but it is foreGround-only mode, wait for it to be done
					else if(foreGroundProcess == 1 && fgom == 1)
					{
						waitpid(childPid, &status, 0);
					}
					// else process is a bg process and and will run in the background
					else		
					{
						printf("background pid is %d\n", childPid);
						fflush(stdout);
					}
					break;
			}
		}	


		// For background pids - check if it is done > 0
		childPid = waitpid(-1, &status, WNOHANG);
		// If there is a background child process done
		if(childPid > 0)
		{	
			// Print that the background pid # is done 
			printf("background pid %d is done: ", childPid);
			// Print the status, if it was exited or terminated by a signal
			if(WIFEXITED(status))
			{
				printf("exit value %d\n", WEXITSTATUS(status));	
				fflush(stdout);
			}
			else
			{
				printf("terminated by signal %d\n", status);
				fflush(stdout);
			}
		}			
		
		//Free and clean up all data in the arguments
		int f;
		for(f = 0; f < numOfArguments; f++)
		{
			free(totalArguments[f]);
		}
	
		// Restore defaults to the varibles for saftey
		inFile = -1;
		outFile = -1;

		if(outputFile != NULL)
		{	
			free(outputFile);
			outputFile = NULL;
		}
		if(inputFile != NULL)
		{
			free(inputFile);
			inputFile = NULL;		
		}

		// Free pidHandling string	
		//free(pidHandling);
	}

	return 0;
}


