#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define NUM_CHILDREN 3

//runs when the user press CRtl+C
void sigint_handler(int sig) {
	printf("\n[Parent] Interrupt detected (Ctrl+ C). Jangan kacau saya!\n");
}

//the child's job is kept in its own fuct
void childTask(int num, int pipefds[2]) {
	char buffer[100];

	signal(SIGINT, SIG_IGN); //child ignore Ctrl+C, so only the parent show the message
	close(pipefds[1]); //child only read [Lab 3.4]

	ssize_t n = read(pipefds[0], buffer, sizeof(buffer) -1); //read from pipe
	if (n < 0) {
		perror("read");
		exit(EXIT_FAILURE);
	}
	buffer[n] = '\0'; //end of text

	printf("Child %d (PID %d, PPID %d) got: %s", num, getpid(), getppid(), buffer);
	
	sleep(3); //short pause so there its time test Ctrl+C
	close(pipefds[0]);
}
int main(void) {
	char message[100]; //the box that holds the message the user types
	
	//Install the Ctrl+C handler with signal()
	if (signal(SIGINT, sigint_handler) == SIG_ERR) {
		perror("signal");
		exit(EXIT_FAILURE);
	}

	//The PARENT ask the user for a message, only ONCE
	printf("Enter a message for the children: ");
	if (fgets(message, sizeof(message), stdin) == NULL) {  
		printf("Parent: no message received.\n");
		return EXIT_SUCCESS;
	}

	//This loop runs 3 times, making one child each time
	for (int i = 1; i <= NUM_CHILDREN; i++) {
		int pipefds[2]; 

		if (pipe(pipefds) == -1) { //create pipe bfr fork
			perror("pipe");
			exit(EXIT_FAILURE);
		}

		pid_t pid = fork();

		if (pid == 0) {
			childTask(i, pipefds);
			exit(EXIT_SUCCESS); //child stop here, so it doest create children
		}
		else if (pid > 0) {
			close(pipefds[0]); //parent only write, so close the read end
			write(pipefds[1], message, strlen(message)); //parent send message to child
			close(pipefds[1]);
		}
		else {
			perror("fork"); //fork return -1
			exit(EXIT_FAILURE);
		}
	}

	//The parent waits for ALL the children to finish
	for (int i = 0; i < NUM_CHILDREN; i++) {
		wait(NULL);
	}
	
	printf("Parent: all the children finished.\n");
	return EXIT_SUCCESS;
}
