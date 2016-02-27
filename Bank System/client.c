#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <strings.h>
#include <pthread.h>
#include <errno.h>

void* input(void* p){
	char buffer[500];
	int *socket = (int*)p;
	int n;

	while(read(*socket, buffer, sizeof(buffer)-1) > 0) {
		
		printf("%s\n", buffer);
		if(strcmp(buffer, "Good Bye!") == 0){
			close(*socket);
		}

	}
	if(n == 0){
		printf("Connection closed. Exiting program.\n");
	}
//	printf("Exiting in the first\n");
    	exit(0);

}

void* output(void* p){
	char buffer[500];
	int *socket = (int*)p;
	int n;

printf("Welcome to our bank!!\n   Please choose one of the following options:\n     open [username]\n     start [username]\n     credit [amount]\n     debit [amount]\n     balance\n     finish\n     exit\n");
	
	while(read(0, buffer, sizeof(buffer)-1) > 0) {
		
		write(*socket, buffer, sizeof(buffer)-1);
		//buffer[n] = '\0';
		sleep(2);
	printf("Welcome to our bank!!\n   Please choose one of the following options:\n     open [username]\n     start [username]\n     credit [amount]\n     debit [amount]\n     balance\n     finish\n     exit\n");
		//printf("Welcome to our bank!!\n Please choose one of the following option numbers:\n 1 (open)\n 2 (start)\n 3 (credit)\n 4 (debit)\n 5 (balance)\n 6 (finish)\n 7 (exit)\n");
	}
	if(n == 0){
		printf("Connection has been closed. Cannot read.\n");
	}
	printf("Exiting here\n");
	exit(0);
}

int main (int argc, char *argv[]){   

	 if (argc < 2) {
        	printf("Not correct arguments.\n");
		exit(EXIT_FAILURE);
    }


	const char* address = argv[1];
	const char* port = "4027"; //Larger than 3500
	struct addrinfo bank;
	struct addrinfo *result, *resultp;	
	int error;
	int SD;
	char* msg;

	bank.ai_flags = 0; //ai_passive (server side flag) and 0 is the client flag
	bank.ai_family = AF_INET; //all caps: means you want 4 byte addresses ipv4
	bank.ai_socktype = SOCK_STREAM; //wants a tcp/ip: connection based communication
	bank.ai_protocol = 0; //Any protocol 
	bank.ai_addrlen = 0; 
	bank.ai_addr = NULL;
	bank.ai_canonname = NULL; 
	bank.ai_next = NULL;

	error = getaddrinfo(address, port, &bank, &result);
	if (error != 0){
		printf("There was an issue connecting to the specified server.\n");
		exit(EXIT_FAILURE);
	} else {
		printf("Connection established.\n");
	}

	 for (resultp = result; resultp != NULL; resultp = resultp->ai_next) {
        SD = socket(resultp->ai_family, resultp->ai_socktype, resultp->ai_protocol);
        if (SD == -1)
            continue;

       if (connect(SD, resultp->ai_addr, resultp->ai_addrlen) == 0){
       		printf("Connected.\n");
            break; 
        }
       close(SD); //Only reaches here if cannot bind.
    }

   if (resultp == NULL) {               /* No address succeeded */
        printf("Could not bind.\n");
        exit(EXIT_FAILURE);
    }

	freeaddrinfo(result);           /* No longer needed */
	
	//char buffer[500];
	//memset(buffer, '0', sizeof(buffer));

	//int n = 0;

    
    pthread_t in, out;

	pthread_create(&in, NULL, input, &SD);
	pthread_create(&out, NULL, output,&SD);

    pthread_join(in, NULL);
    pthread_join(out, NULL);
    exit(0);

 
}
