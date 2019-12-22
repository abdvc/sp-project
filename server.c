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

int main()
{
	int sock, length;
	int opt = 1;
	struct sockaddr_in server, client;
	int msgsock;
	char buf[1024], out[1024], pipebuf1[1024], pipebuf2[1024];
	int rval;
	socklen_t addr_size;
	//pipeParent is the pipe where writing end is in parents
	//pipeChild is the pipe where writing end is in child
	int pipeParent[2], pipeChild[2];

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

	printf("IP Address: %s\n", inet_ntoa(server.sin_addr));
	
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

		if (pipe(pipeChild) < 0) {
			perror("pipeChild");
			exit(1);
		}

		if (pipe(pipeParent) < 0) {
			perror("pipeParent");
			exit(1);
		}

		int f = fork();
		if (f < 0) {
			perror("fork");
			exit(1);
		}
		else if (f == 0) {
			//child process
			close(sock);

			//close writing end of parent pipe
			//close(pipeParent[1]);
			//close reading end of child pipe
			//close(pipeChild[0]);
			
			while (1) {
				memset(buf, 0, sizeof(buf));
				memset(out, 0, sizeof(out));
				memset(pipebuf2, 0, sizeof(pipebuf2));

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

					if (write(pipeChild[1], out, strlen(out)) < 0) {
						perror("write to pipe: Child");
						exit(1);
					}
					
					if (read(pipeParent[0], pipebuf2, 1024) < 0) {
						perror("read from pipe: Child");
						exit(1);
					}

					if (send(msgsock, pipebuf2, strlen(pipebuf2), 0) < 0) {
						perror("send");
						exit(1);
					}
				}
			}
		} else {
			//parent process
			close(msgsock);
			while (1) {
				memset(pipebuf1, 0, 1024);

				//close writing end of child pipe
				//close(pipeChild[1]);

				if (read(pipeChild[0], pipebuf1, 1024) < 0) {
					perror("read from pipe: Parent");
					exit(1);
				}

				//close reading end of child pipe
				//close(pipeChild[1]);

				//close reading end of parent pipe
				//close(pipeParent[0]);

				if (write(pipeParent[1], "Message received\n", strlen("Message received\n")) < 0) {
					perror("write to pipe: Parent");
					exit(1);
				}

				/*if (write(STDOUT_FILENO, "Message received\n", strlen("Message received\n")) < 0) {
					perror("write to pipe: Parent");
					exit(1);
				}*/

				//close writing end of parent pipe
				//close(pipeParent[1]);
			}
		}
	}
}