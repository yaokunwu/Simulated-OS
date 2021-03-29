#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#define MAXBUFLEN 1000000

void error(char *msg)
{
    perror(msg);
    exit(0);
}
void main(int argc, char *argv[]) {
    int sockfd, portno, ret;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    char cmd[256];
    char fname[256];
    

    if (argc < 3) {
      fprintf(stderr,"usage %s hostname port\n", argv[0]);
      exit(0);
    }
    portno = atoi(argv[2]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) error ("ERROR, no such host");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
          (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
      error("ERROR connecting");
   while(1) { 
        scanf ("%s", cmd);
        while (cmd[0] != 's' && cmd[0] != 'T') {
            
             printf("Error: Incorrect command!!!\n");
            scanf ("%s", cmd);
         }
    ret = write (sockfd, cmd, strlen(cmd));
    if (cmd[0] == 'T') {
        break;
    } 
    scanf ("%s", fname);
  ret = write (sockfd, fname, strlen(fname));
    if (ret < 0) error ("ERROR writing to socket");
    
    char source[MAXBUFLEN + 1];
    FILE *fp = fopen(fname, "r");                
    if (fp != NULL) {
        size_t newLen = fread(source, sizeof(char), MAXBUFLEN, fp);
        if ( ferror( fp ) != 0 ) {
            fputs("Error reading file", stderr);
        } else {
            source[newLen++] = '\0';
        }

        fclose(fp);
    }
     ret = write (sockfd, source, strlen(source));
        if (ret < 0) error ("ERROR writing to socket");
    while(1) {
            bzero (buffer, 256);
            ret = read (sockfd, buffer, 255);
            if (ret < 0) error ("ERROR reading from socket");
            printf ("%s",buffer);
            if (strstr(buffer, "successfully") != NULL || strstr(buffer, "!!!")!= NULL) {
                break;
            }
    }
    
 } 
    close (sockfd);
}

