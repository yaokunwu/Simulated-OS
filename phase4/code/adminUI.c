#include<stdio.h> 
#include <string.h>
#include<stdlib.h>
#include <sys/types.h> 
#include <unistd.h> 
#include <signal.h>
#include "simos.h"

char exe;
int childr, childw, parentr, parentw;
char buffer[1024];
int main(int argc, char** argv) {
	int numR, portno;
	char action[10];
	char run[64];
	int fds1[2], fds2[2];
	if (argc < 2) {
		printf("need number of rounds as an argument\n");
		return 0;
	}
	if (argc < 3) {
		printf("need port number\n");
		return 0;
	}

	numR = atoi(argv[1]);
	portno = atoi(argv[2]);
	pipe (fds1);
	pipe (fds2);
	childr = fds1[0];
	childw = fds2[1];
	parentr = fds2[0];
	parentw = fds1[1];
	int pid = fork();
	if (pid > 0) {
		usleep(100000);
		close(childr); close(childw);
		while (1) {
				usleep(10000);
				printf ("command> ");
				scanf ("%s", &action);
				exe = action[0];
				kill(pid, SIGRTMIN);
				usleep(1000);
				write(parentw, action,sizeof(action));
				if (exe == 'T') {
					goto end;
				}
				read(parentr, buffer, sizeof(buffer)); 	
				fprintf(stdout,"%s", buffer);
				memset(buffer,0,strlen(buffer));
				usleep(10000);

		}
	} else if (pid < 0) {
		printf("fork not successful");
		fflush(stdout);
		return 0;	
	} else {
		close (parentr); close (parentw);
		systemActive = 1;
  		initialize_system (portno);
 		execute_process_iteratively(numR, childr, childw);
		fprintf (infF, "System exiting!!!\n");
 		system_exit ();
		return 0;
	} 
	
		end:
		usleep(100000);
                kill(pid, SIGKILL);
		wait();
	return 0;
}
