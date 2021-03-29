#include "simos.h"
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <semaphore.h>

void *handle_connection(int newsockfd);
int  check(int exp, const char *msg);
int accept_new_connection(int server_socket);
int setup_server(int port, int backlog);
//===============================================================
// The interface to interact with clients for program submission
// --------------------------
// Should change to create server socket to accept client connections
// -- Best is to use the select function to get client inputs
// Should change term.c to direct the terminal output to client terminal
//===============================================================
sem_t mutex;
int portno;


typedef struct submitNodeStruct
{ char *name;
  FILE *clientF;
  struct submitNodeStruct *next;
} submitNode;

submitNode *submitHead = NULL;
submitNode *submitTail = NULL;

void error(char *msg)
{
    perror(msg);
    exit(1);
}
fd_set current_sockets, ready_sockets;

void *submitT ()
{
int i;
     int sockfd = setup_server(portno, 5);

//     fd_set current_sockets, ready_sockets;

     FD_ZERO(&current_sockets);
     FD_SET(sockfd, &current_sockets);

     while (1) {
	ready_sockets = current_sockets;
	select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL);
	for (i = 0; i < FD_SETSIZE; i++) {
		if (FD_ISSET(i, &ready_sockets)) {
			if (i == sockfd) {
				int newsockfd = accept_new_connection(sockfd);
				FD_SET(newsockfd, &current_sockets);
			} else {
				handle_connection(i);
//				FD_CLR(i, &current_sockets);
			}
		}
	}


     }
}

void closeSocket(int socketno) {
     close (socketno); 
}

int setup_server(int portno, int backlog) {
    int sockfd;
    struct sockaddr_in serv_addr;
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
       error("ERROR opening socket");
     bzero ((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
       error("ERROR binding");
     listen(sockfd,backlog);
   return sockfd; 
}

int accept_new_connection(int server_socket) {
     struct sockaddr_in cli_addr;
     int clilen = sizeof(cli_addr);
    int newsockfd;

     newsockfd = accept(server_socket, (struct sockaddr *) &cli_addr, &clilen);
     if (newsockfd < 0) 
       error ("ERROR accepting");
     else printf ("\nAccept client socket %d, %d\n",
                   newsockfd, (int)cli_addr.sin_port);
     return newsockfd;
	
}

char arrayDes[100];
void *handle_connection(int newsockfd) {
     submitNode *node;
     FILE *tmpF;
     int ret;
     FILE *fp;
     char instr[100];
//     char arrayDes[100]; // why can not arrayDes as a local variable????
     char buffer[256];
     memset(arrayDes,0,strlen(arrayDes));
     
     ret = read (newsockfd, instr, 100);
     if (instr[0] == 'T') {
     		
      FD_CLR(newsockfd, &current_sockets);
      close (newsockfd); 
     return;
     }
     ret = read (newsockfd, arrayDes, 255);
     if (ret < 0) error ("ERROR reading from socket");
     

     fp = fopen(arrayDes, "w");

     bzero(buffer,256);
     ret = read (newsockfd, buffer, 255);
     if (ret < 0) error ("ERROR reading from socket");
     
     fprintf(fp, buffer);
     fclose(fp);
     tmpF = fdopen(newsockfd, "w");
	sem_wait(&mutex);
	  node = (submitNode *) malloc (sizeof (submitNode));
	  node->name=arrayDes;
	  node->clientF=tmpF;
	  node->next = NULL;
	  if (submitTail == NULL) 
	    { submitTail = node; submitHead = node; }
	  else // insert to tail
	    { submitTail->next = node; submitTail = node; }
	sem_post(&mutex);
  	
     set_interrupt (submitInterrupt);
}

pthread_t submitThread;

void program_submission ()
{ 
  submitNode *rnode;
  sem_wait(&mutex); 
  while (submitHead != NULL) {
  	submit_process(submitHead->name, submitHead->clientF);
	rnode = submitHead;
	submitHead = submitHead->next;
	free(rnode);
	if (submitHead == NULL) submitTail = NULL;
 }
   sem_post(&mutex);
  
}


void start_submit (int port)
{ int ret;

  sem_init (&mutex, 0, 1);
  portno = port;
  ret = pthread_create(&submitThread, NULL, submitT , (void *) 0);
  if (ret){
  	fprintf(stdout, "ERROR; return code from pthread_create() is %d\n", ret);
  } else {
  	fprintf(stdout, "Submit thread has been created successfully\n");
  }
}

void end_submit ()
{ int ret;
  pthread_join(submitThread,NULL);
  fprintf (infF, "submit thread has terminated %d\n", ret);
}

