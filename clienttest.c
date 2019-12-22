/*
 Copyright (c) 1986 Regents of the University of California.
 All rights reserved.  The Berkeley software License Agreement
 specifies the terms and conditions for redistribution.

	@(#)streamwrite.c	6.2 (Berkeley) 5/8/86
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define DATA "Half a league, half a league . . ."

/*
 * This program creates a socket and initiates a connection with the socket
 * given in the command line.  One message is sent over the connection and
 * then the socket is closed, ending the connection. The form of the command
 * line is streamwrite hostname portnumber 
 */

int main(int argc, char *argv[])
	{
	int sock;
	struct sockaddr_in server;
	char buf[1024], out[1024];
    

	/* Create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("opening stream socket");
		exit(1);
	}
	
	server.sin_family = AF_INET;
	server.sin_port = htons(9000);
	server.sin_addr.s_addr = inet_addr("127.0.0.1");

	int ret = connect(sock, (struct sockaddr*)&server, sizeof(server));
	if(ret < 0){
		perror("connect");
		exit(1);
	}

	while(1){
		if (write(STDOUT_FILENO, "Client: ", strlen("Client: ")) < 0) {
			perror("write");
			exit(1);
		}
		int r = read(STDIN_FILENO, buf, sizeof(buf));

		if (r < 0) {
			perror("read");
			exit(1);
		}

		buf[r-1] = '\0';
		
		if (send(sock, buf, strlen(buf), 0) < 0) {
			perror("send");
			exit(1);
		}

		if(strcmp(buf, ":exit") == 0){
			close(sock);
			if (write(STDOUT_FILENO, "[-]Disconnected from server.\n", strlen("[-]Disconnected from server.\n")) < 0) {
				perror("write");
				exit(1);
			}
			exit(1);
		}

		if(recv(sock, buf, 1024, 0) < 0){
			perror("recv");
		}else{
			sprintf(out, "Server: %s\n", buf);
			write(STDOUT_FILENO, out, strlen(out));
		}
	}
}