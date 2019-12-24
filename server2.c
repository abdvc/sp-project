#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#define WHITE 1
#define BLACK 2
#define EMPTY 0

//board state
int state[8][8] = {
    {0, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 0},
	{0, 1, 0, 1, 0, 1, 0, 1},
    {0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
    {2, 0, 2, 0, 2, 0, 2, 0},
	{0, 2, 0, 2, 0, 2, 0, 2},
    {2, 0, 2, 0, 2, 0, 2, 0}
};
int conq, cx, cy;

int validateMove(int arr[], int piece);
int updateState(int arr[], int piece);

int main()
{
	int sock, length, newsock[2], pkg[2], turnstates[2] = {1,0}, arr[5];
	int opt = 1;
	struct sockaddr_in server, client[2];
	socklen_t addr_size;
    char* turn;

	//pipeParent is the pipe where writing end is in parents
	//pipeChild is the pipe where writing end is in child
	int pipeParent[2], pipeChild[2];

	/* Create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&server, sizeof(server));
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
	
    int counter = 0;
    
	while(counter < 2) {
		newsock[counter] = accept(sock, (struct sockaddr*)&client[counter], &addr_size);
		counter = counter + 1;
    }

    printf("Connection accepted from %s:%d\n", inet_ntoa(client[0].sin_addr), ntohs(client[0].sin_port));
    printf("Connection accepted from %s:%d\n", inet_ntoa(client[1].sin_addr), ntohs(client[1].sin_port));
    turn = inet_ntoa(client[0].sin_addr);
    
	//{player turn, piece} 0 - no, 1-client's turn.
	pkg[0] = turnstates[0]; pkg[1] = BLACK;
    send(newsock[0], pkg, sizeof(pkg), 0);
    pkg[1] = turnstates[1]; pkg[1] = WHITE;
    send(newsock[1], pkg, sizeof(pkg), 0);
    int c1, c2;
	while (1) 
	{
		memset(arr, 0, sizeof(arr));
		
		c1 = recv(newsock[0], arr, sizeof(arr), MSG_DONTWAIT);
		//arr[0,1,2,3,4] = [initx,inity,destx,desty,piece]
		if (c1 > -1 && turnstates[0] == 1 && arr[4] == BLACK){
			if(validateMove(arr, BLACK) == 1) {
				arr[4] = updateState(arr, BLACK);
				turnstates[0] = !turnstates[0];
				turnstates[1] = !turnstates[1];
			}
			else
				arr[0] = -1;
		}
		//printf("%d %d %d %d %d\n",arr[0],arr[1],arr[2],arr[3],arr[4]);
		
		memset(arr, 0, sizeof(arr));
		if (c1 < 0) {
		c2 = recv(newsock[1], arr, sizeof(arr), MSG_DONTWAIT);
		//arr[0,1,2,3,4] = [initx,inity,destx,desty,piece]
		if (c2 > -1 && turnstates[1] == 1 && arr[4] == WHITE){
			if(validateMove(arr, WHITE) == 1) {
				arr[4] = updateState(arr, WHITE);
				turnstates[1] = !turnstates[1];
				turnstates[0] = !turnstates[0];
			}
			else
				arr[0] = -1;
		}
		}
		//printf("%d %d %d %d %d\n",arr[0],arr[1],arr[2],arr[3],arr[4]);
		if (c1 > -1 || c2 > -1) {
			send(newsock[0], arr, sizeof(arr), 0);
			send(newsock[1], arr, sizeof(arr), 0);
		}
		c1, c2 = -1;
	}
	
	close(sock);
} 

int validateMove(int arr[], int piece) {
	if (state[arr[1]][arr[0]] != piece && state[arr[3]][arr[2]] != EMPTY) {
		//check if init position is of valid piece
		//and if dest position is empty
		return -1;
	}
	if (arr[0] == arr[2] || arr[1] == arr[3]) {
		//check if init x or y is equal to dest x or y
		return -1;
	}
	int x = abs(arr[0] - arr[2]);
	int y = abs(arr[1] - arr[3]);
	int mx = x/2;
	int my = y/2;
	if ((x + y) == 4) {
		//check if conquering has occured
		if (piece == BLACK) {
			if (arr[0] > arr[2]) {
				if (state[arr[1] - my][arr[0] - mx] == piece) {
					return -1;
				} else {
					conq = 1;
					cy = arr[1] - my;
					cx = arr[0] - mx;
				}
			} else if (arr[0] < arr[2]) {
				if (state[arr[1] - my][arr[0] + mx] == piece) {
					return -1;
				} else {
					conq = 1;
					cy = arr[1] - my;
					cx = arr[0] + mx;
				}
			}
		} else if (piece == WHITE) {
			if (arr[0] > arr[2]) {
				if (state[arr[1] + my][arr[0] - mx] == piece) {
					return -1;
				} else {
					conq = 1;
					cy = arr[1] + my;
					cx = arr[0] - mx;
				}
			} else if (arr[0] < arr[2]) {
				if (state[arr[1] + my][arr[0] + mx] == piece) {
					return -1;
				} else {
					conq = 1;
					cy = arr[1] + my;
					cx = arr[0] + mx;
				}
			}
		}
	}
	return 1;
}

int updateState(int arr[], int piece) {
	state[arr[3]][arr[2]] = piece;
	state[arr[1]][arr[0]] = EMPTY;
	if (conq == 1) {
		state[cy][cx] = EMPTY;
		return 1;
	}
	return 0;
}