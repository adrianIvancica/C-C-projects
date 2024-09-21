#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported

int takeInput(char* str)
{
        char* buf;

        buf = fgets(str, MAXCOM, stdin);

        if (buf == NULL)
        return 0;
        else
        return 1;
}


void execArgs(char** parsed)
{
        // Forking a child
        pid_t pid = fork();

        if (pid == -1) {
                printf("\nFailed forking child..");
                return;
        } else if (pid == 0) {
                if (execvp(parsed[0], parsed) < 0) {
                        printf("\nCould not execute command..");
                }
                exit(0);
        } else {
                // waiting for child to terminate
                wait(NULL);
                return;
        }
}

void parseSpace(char* str, char** parsed)
{
        int i;

        for (i = 0; i < MAXLIST; i++) {
                parsed[i] = strsep(&str, " ");

                if (parsed[i] == NULL)
                        break;
                if (strlen(parsed[i]) == 0)
                        i--;
        }
}

int processString(char* str, char** parsed)
{
        parseSpace(str, parsed);
        if (strcmp(parsed[0], "exit") == 0 || strcmp(parsed[0],"exit\n") == 0)
        {
                printf("\nGoodbye\n");
                exit(0);
                return 0;
        }
        else if (parsed[0] == "echo")
        {
                for (int i = 1; i < MAXLIST; i++){
                        printf("%s ", parsed[i]);
                }
                return 0;
        }
        else
                return 1;
}

int main()
{
        char inputString[MAXCOM], *parsedArgs[MAXLIST];
        int execFlag = 0;

        while (1) {
                printf("\nEnter Command: " );

                if (takeInput(inputString))
                {
                        execFlag = processString(inputString, parsedArgs);
                        // execute flag returns zero if there is no command given
                        // or it is a builtin command,
                        // 1 if simple command

                        if (execFlag == 1)
                                execArgs(parsedArgs);
                }
        }
        return 0;
}                            
 
 
