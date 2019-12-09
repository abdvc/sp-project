/*
 Copyright (c) 1986 Regents of the University of California.
 All rights reserved.  The Berkeley software License Agreement
 specifies the terms and conditions for redistribution.

	@(#)streamread.c	6.2 (Berkeley) 5/8/86
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#define TRUE 1

/*
 * This program creates a socket and then begins an infinite loop. Each time
 * through the loop it accepts a connection and prints out messages from it. 
 * When the connection breaks, or a termination message comes through, the
 * program accepts a new connection. 
 */

int main()
{
	int sock, length;
	int opt = 1;
	struct sockaddr_in server, client;
	int msgsock;
	char buf[1024], out[1024];
	int rval;
	socklen_t addr_size;

	/* Create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
		perror("opening stream socket");
		exit(1);
	}

	//make port reuseable
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) { 
        perror("setsockopt"); 
        exit(1); 
    } 

	/* Name socket using wildcards */
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(9000);
	
	if (bind(sock, (struct sockaddr *) &server, sizeof(server))) {
		perror("binding stream socket");
		exit(1);
	}

	/* Find out assigned port number and print it out */
	length = sizeof(server);
	if (getsockname(sock, (struct sockaddr *) &server, (socklen_t*) &length)) {
		perror("getting socket name");
		exit(1);
	}
	
	printf("Socket has port #%d\n", ntohs(server.sin_port));
	fflush(stdout);

	/* Start accepting connections */
	if((listen(sock, 2)) < 0 ) {
		perror("listen");
		exit(1);
	}


	while(1) {
		msgsock = accept(sock, (struct sockaddr*)&client, &addr_size);

		if (msgsock < 0) {
			perror("accept");
			exit(1);
		}

		printf("Connection accepted from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

		int f = fork();
		if (f < 0) {
			perror("fork");
			exit(1);
		}
		else if (f == 0) {
			//child process
			close(sock);
			
			while (1) {
				memset(buf, 0, sizeof(buf));
				memset(out, 0, sizeof(out));

				if (recv(msgsock, buf, sizeof(buf), 0) < 0) {
					perror("recv");
					exit(1);
				}
				if (strcmp(buf, ":exit\0") == 0){
					sprintf(out, "Disconnected from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
					
					if (write(STDOUT_FILENO, out, strlen(out)) < 0) {
						perror("write");
						exit(1);
					}

					break;
				} else {
					sprintf(out, "Client: %s\n", buf);
					
					if (write(STDOUT_FILENO, out, strlen(out)) < 0) {
						perror("write");
						exit(1);
					}

					if (send(msgsock, buf, strlen(buf), 0) < 0) {
						perror("send");
						exit(1);
					}
				}
			}
		} else {
			//parent process
			close(msgsock);
		}
	}
	/*
	 * Since this program has an infinite loop, the socket "sock" is
	 * never explicitly closed.  However, all sockets will be closed
	 * automatically when a process is killed or terminates normally. 
	 */
}