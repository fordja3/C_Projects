/**************************************************
 File Name: client.c 
 Author: Jason Ford
 Date: 12/8/18
 Description: Requests a file from the server and saves it to the 
 		client directory. The user specifies the port number 
 		of the server, connects to the server, saves the file
 		requested in the directory and exits gracefully. Meant
 		to be used in conjunction with server.c file.   
 		
 Input: port number of server
	ip address of server
	filename requested
 	  
 Output: requested file 

**************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT "8500" 
#define MAXDATASIZE 100

/***************************************************
 Socket is created here
 get sockaddr, IPv4 or IPv6:
***************************************************/

void *get_in_addr(struct sockaddr *sa){

    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]){

    int sockfd;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN], *ip, *port, *filename;
    char filenamecmd[100];


/*****************************************************
Three arguments entered here, 
ip
port 
filename
******************************************************/

    ip = argv[1];
    port = argv[2];
    filename = argv[3];
    strcpy(filenamecmd, "get ");
    strcat(filenamecmd, filename);

/****************************************************
Checks to see if the user has entered 4 arguments and
returns an error message if they have not.
****************************************************/

    if (argc != 4) {
        fprintf(stderr,"put one address, port number and filename:\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;



    if ((rv = getaddrinfo(argv[1], port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }


/******************************************************
loop through all the results and connect to the first we can
as long as server is checking for new clients. 
*******************************************************/

    for(p = servinfo; p != NULL; p = p->ai_next) {
   
/* creating socket file descriptor */     
	if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

/* connecting to client socket */
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("client: connect");
            close(sockfd);
            continue;
        }
 
        break;
    }

/* if pointer is NULL, returns "no connection" error message  */
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); 


    if (send(sockfd, filenamecmd, strlen(filenamecmd), 0) == -1)
        perror("send");

	int file_name;

/* creates a new file for client to write-in requested file */
	if((file_name = open(filename, O_WRONLY | O_CREAT, 0644)) == -1){
	perror("open");
	close(sockfd);
	}
	
int receive;
char *out = "/cmsc257";
char *kill="/kill";
int write_to;

/********************************************************
Requests file from server and writes it to the client directory. 
*********************************************************/
	
	while(1){
		if((receive=recv(sockfd, buf, MAXDATASIZE, 0)) < -1){
			perror("recieve");
		}

		if(strstr(buf, out)){
			break;	
		}

		if(strstr( buf, kill )){
			printf( "server stopped.");

			close(sockfd);
			exit(0);
		}
/*writes file to the client directory */
		if((write_to=write(file_name, buf, receive))==-1){
		perror("write\n");
		close(sockfd);
		}	
	}	

/* writes last of the file before closing */
	if((write_to=write(file_name, buf, receive-8))==-1){
		perror("write\n");
		close(sockfd);
	}

/*after file is written, socket is closed  */
printf("File was written!-\n");
close(file_name);
close(sockfd);

    return 0;

}


