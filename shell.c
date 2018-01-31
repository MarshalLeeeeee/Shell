#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#define MAXLINE 80
#define HISTORY 10
#define RX_LEN 10
char buffer[HISTORY][MAXLINE];
char RX[RX_LEN];
char inputBuffer[MAXLINE];
int background;
char *args[MAXLINE / 2 + 1];
int fpid, exitstatus;
int head = 0; int tail = -1;

void handler_SIGINT()
{
	write(STDOUT_FILENO, "\n", 1);
	if(tail == -1) {write(STDOUT_FILENO, "No instruction\n", 15); fflush(stdin); return ;}
	else
	{
		for(int index = head; index <= tail; index++)
		{
			write(STDOUT_FILENO, buffer[index % HISTORY], strlen(buffer[index % HISTORY]));
			write(STDOUT_FILENO, "\n", 1);
		}
	}
	write(STDOUT_FILENO, "\n", 1);
	gets(RX);
	if(strlen(RX) == 1 && RX[0] == 'r'){exe(tail % HISTORY); fflush(stdin); return ;}
	else if(strlen(RX) == 3 && RX[0] == 'r')
	{
		for(int index = tail; index >= head; index--)
		{
			if(buffer[index%HISTORY][0] == RX[2]) {exe(index % HISTORY); fflush(stdin); return ;}
		}
		//exe(buffer[tail % HISTORY]); return ;
	}
	fflush(stdin);
}

void exe(int i)
{
	char inputBuffer[MAXLINE];
	int background;
	char *args[MAXLINE / 2 + 1];
	strcpy(inputBuffer, buffer[i]);
	strcpy(buffer[(++tail) % HISTORY], inputBuffer);
	if(tail - head == HISTORY) head++;
	int fpid, exitstatus;
	int index, aindex = 0;
	int flag = 0;
	for (index = 0; inputBuffer[index]; index++)
	{
		if (!flag)
		{
			if (inputBuffer[index] != ' ' && inputBuffer[index] != '\t') flag = 2;
			else continue;
		}
		if (flag)
		{
			if (inputBuffer[index] != ' ' && inputBuffer[index] != '\t')
			// this one isnot a blank
			{
				if (flag == 2)
				// the former char is a blank character, and this one is not.
				{
					args[aindex++] = inputBuffer + index;
					flag = 1;
					continue;
				}
			}
			else// this one is a blank
			{
				if (flag == 1)
				{
					inputBuffer[index] = 0;
					flag = 2;
				}
			}
		}
	}
	if (args[aindex - 1][0] == '&' && args[aindex - 1][1] == 0) 
	{background = 0; args[aindex-1] = NULL;}
	else {background = 1; args[aindex] = NULL;}
	fpid = fork();
	switch(fpid)
	{
		case -1: perror("fork failed"); exit(1);
		case 0 : execvp(args[0], args); perror("execvp failed"); exit(1);
		default: 
			if(background) while(wait(&exitstatus)!= fpid);
	}
	
}


void setup(char inputBuffer[], char *args[], int *background)
{
	//puts("Instruction: ");puts(inputBuffer);
	gets(inputBuffer);
	//puts("Instruction: ");puts(inputBuffer);
	if(!strlen(inputBuffer)) return ;
	strcpy(buffer[(++tail) % HISTORY], inputBuffer);
	if(tail - head == HISTORY) head++;
	int index, aindex = 0;
	int flag = 0;
	for (index = 0; inputBuffer[index]; index++)
	{
		if (!flag)
		{
			if (inputBuffer[index] != ' ' && inputBuffer[index] != '\t') flag = 2;
			else continue;
		}
		if (flag)
		{
			if (inputBuffer[index] != ' ' && inputBuffer[index] != '\t')
			// this one isnot a blank
			{
				if (flag == 2)
				// the former char is a blank character, and this one is not.
				{
					args[aindex++] = inputBuffer + index;
					flag = 1;
					continue;
				}
			}
			else// this one is a blank
			{
				if (flag == 1)
				{
					inputBuffer[index] = 0;
					flag = 2;
				}
			}
		}
	}
	if (args[aindex - 1][0] == '&' && args[aindex - 1][1] == 0) 
	{*background = 0; args[aindex-1] = NULL;}
	else {*background = 1; args[aindex] = NULL;}
}

int main(void)
{
	struct sigaction handler;
	handler.sa_handler = handler_SIGINT;
	sigaction(SIGINT, &handler, NULL);
	//strcpy(buffer, "\nCaught Control C\n");

	
	while(1)
	{
		printf("COMMAND->");
		setup(inputBuffer, args, &background);
		if(!strlen(inputBuffer)) continue;
		fpid = fork();
		switch(fpid)
		{
			case -1: perror("fork failed"); exit(1);
			case 0 : execvp(args[0], args); perror("execvp failed"); exit(1);
			default: 
				if(background) while(wait(&exitstatus)!= fpid);
		}
		inputBuffer[0] = 0;
	}
	return 0;
}
