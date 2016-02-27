#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/shm.h>
#include <pthread.h>

#define BUF_SIZE 500



typedef struct account{ 
	char user[500];
	float balance;
	pthread_mutex_t aLock;
	int session;
} account;


typedef struct bank{
	pthread_mutex_t bLock;
	int accountNumber;
	struct account account[20];
} bank;

account *start(int a, char b []);
void main_menu(int* a);
void client_service_function(int a, account* b);
void credit(int a, account* b, float c);
void debit(int a, account* b, float c);

bank *vault; //global bank
pthread_t print;
pthread_t mywait;
pthread_mutexattr_t bankattr;
char errormsg[100]; //global error buffer

void* mywait_thread(){
	wait();
}

void client_service_function(int socket, account* accptr){

	int n;
	char buffer[500];
	char action[500];
	char amount[500];
	float famount;
	//int flag = 0;

	printf("You are now being serviced\n");
	while(n = read(socket, buffer, sizeof(buffer)-1) > 0) {
	//	printf("You are servicing yourself\n");
		sscanf(buffer, "%s %s", action, amount); //need to check the value of the second input (String or float?)
		famount = atof(amount);
		if(strcmp(action,"credit") == 0){
			printf("You want to add money\n");
			credit(socket, accptr, famount);
		}

		if(strcmp(action,"debit") == 0){
			printf("You want to remove money\n");
			debit(socket, accptr, famount);
		}	
	
		if(strcmp(action, "finish") == 0){
			printf("We are done with the current session.\n");
			sprintf(buffer,"You have chosen to finish your session.\n");
                          write(socket, buffer, sizeof(buffer)-1);
			accptr->session = 0;
			pthread_mutex_unlock(&accptr->aLock);
			return;	
		}

		if(strcmp(action, "balance") == 0){
			 sprintf(buffer, "You current balance is:$%.2f.\n", accptr->balance);
  			  write(socket, buffer, sizeof(buffer)-1);
		}

		if(strcmp(action, "open") == 0 || strcmp(action, "start") == 0 || strcmp(action, "exit") == 0) {
			  sprintf(buffer, "You should <finish> your current session before trying to %s.\n", action);
  			  write(socket, buffer, sizeof(buffer)-1);
		}
	}
	

}



void* print_thread(){ //not complete

    while (pthread_mutex_trylock(&vault->bLock) != 0) {

        printf("The vault is locked.\n");

        sleep(2);

    }

    while(1){
    	int i;
    	printf( "\x1b[2;32m<=== Welcome to the Vault! ===>\x1b[0m\n");
        for (i = 0; i < vault->accountNumber; i++) {
            printf("Account name: %s\n", vault->account[i].user);
            printf("Balance: $%.2f\n\n", vault->account[i].balance);
            if (vault->account[i].session == 1) {
                printf("Account \"%s\" is currently in use.\n", vault->account[i].user);
        }
        	    
	    }
	printf( "\x1b[2;32m<=== Bank Status Update ===>\x1b[0m\n\n");
		pthread_mutex_unlock(&vault->bLock); 
	sleep(20);
	}
}

account *start(int socket, char username[]) {

    char update[500];

    while (pthread_mutex_trylock(&vault->bLock) != 0) {       
        sprintf(update, "The bank is currently busy. We will get back to you momentarily\n\n");
		write(socket, update, sizeof(update)-1);
        sleep(2);

    }

    if (strlen(username) == 0) {
        sprintf(update, "You have not entered a name.\n");
        write(socket, username, sizeof(update)-1);
        pthread_mutex_unlock(&vault->bLock);
        return NULL;
    }
    
    int i;

    for (i = 0; i < vault->accountNumber; i++) {
        if (strcmp(vault->account[i].user, username) == 0) {
            break;
        }
    }

    pthread_mutex_unlock(&vault->bLock);

    if (vault->accountNumber == i) {
        sprintf(update, "No opened account matches that name.\n");
        write(socket, update, sizeof(update)-1);
        return NULL;
    } else {
        while (pthread_mutex_trylock(&vault->account[i].aLock) != 0) {       
        sprintf(update, "%s's account is currently in session.\n", username);
        write(socket, update, sizeof(update)-1);
        sleep(2);
    }

        vault->account[i].session = 1;

        sprintf(update, "You have opened a new session.\n");
        write(socket, update, sizeof(update)-1);

        client_service_function(socket, &vault->account[i]);
        return &vault->account[i];
    }
}



void main_menu(int *socket){

 	char buffer[500];
	char command[100];
	char name[100];
	account *accptr;
    
    while(read(*socket, buffer, sizeof(buffer) - 1) > 0) {

	sscanf(buffer, "%s %[^\n]", command, name);

	if (strcmp(command, "open") == 0){
		printf("Command: %s\n", command);
			printf("Username chosen was: %s\n", name);			
			int val = open(*socket, name);			
        } 
    else if (strcmp(command, "start") == 0){
    	if(strcmp(name, "") == 0){
    		sprintf(buffer, "You forgot to enter an account name.\n");
    	} else {
			accptr = start(*socket, name);
			strcpy(command, "");
		} 	
	} else if(strcmp(command, "exit") == 0){
		sprintf(buffer, "You have chosen to exit the bank. Have a great day! Good bye!\n");
  		write(*socket, buffer, sizeof(buffer)-1);
		close(*socket);  //Closes connection
	} else if(strcmp(command, "finish") == 0) {
		sprintf(buffer, "You must first start an account session before attempting to <finish>.\n");
  		write(*socket, buffer, sizeof(buffer)-1);
	} else if(strcmp(command, "balance") == 0 || strcmp(command, "credit") == 0 || strcmp(command, "debit") == 0){
		sprintf(buffer, "You must first start an account session before attempting to check your balance, credit, or debit.\n");
  		write(*socket, buffer, sizeof(buffer)-1);
	}
}

    exit(0); //Child exits
} 





int open(int socket, char username[]){
while(pthread_mutex_trylock(&vault->bLock) != 0){ 
	sprintf(errormsg,"The bank is currently busy. We will access your account momentarily.\n");
	write(socket, errormsg, sizeof(errormsg)-1);
	sleep(2); 
}

if(vault->accountNumber == 20){
	
	sprintf(errormsg, "We have reached our maximum limit of open accounts.\n");
	write(socket, errormsg, sizeof(errormsg)-1);
	pthread_mutex_unlock(&vault->bLock);
	return 1; 
}

//printf("am I here?\n");
int i = 0;

while(i < vault->accountNumber) {
	if(strcmp(vault->account[i].user, username) == 0){
		pthread_mutex_unlock(&vault->bLock);
		sprintf(errormsg, "This user already exists. Try to open a different account under a different name.\n");
		//printf("User exists.\n");
		write(socket, errormsg, sizeof(errormsg)-1);
		return 2;
	}
	i++;
}

vault->accountNumber++;


strcpy(vault->account[i].user, username); //segfault
vault->account[i].balance = 0;
//vault->account[i]->balance = 0;
sprintf(errormsg, "User created successfully.\n");
write(socket, errormsg, sizeof(errormsg)-1);
//printf("User succesfully registered. Name is: %s\n", vault->account[i].user);
pthread_mutex_unlock(&vault->bLock);
return 0;
} //change return values

void credit(int socket, account *user, float amount) {
	char buffer[100];	
	
	float round = roundf(amount*100)/100;
	if(round < 0){
		sprintf(buffer, "You cannot deposit a negative account.\n");
  		write(socket, buffer, sizeof(buffer)-1);
  	} else {
		user->balance += round;
		sprintf(buffer, "Your current balance after credit is: $%.2f.\n", user->balance);
		write(socket, buffer, sizeof(buffer)-1);
	}
}

void debit(int socket, account *user, float amount) {
	char buffer[100];
		
	float round = roundf(amount*100)/100;	
	if(round < 0){
		sprintf(buffer, "You cannot withdraw a negative amount.\n");
  		write(socket, buffer, sizeof(buffer)-1);
	} else if(round > user->balance){
		sprintf(buffer, "You cannot withdraw more than your balance.\n");
  			  write(socket, buffer, sizeof(buffer)-1);
	} else {
		user->balance -= round;
			sprintf(buffer, "Your current balance after debit is: $%.2f.\n", user->balance);
			  write(socket, buffer, sizeof(buffer)-1);
	}	
}







int main (int argc, char *argv[]){   

	const char* address = "127.0.0.1";
	const char* port = "4027"; //Larger than 3500
	struct addrinfo stuff;
	struct addrinfo *result, *resultp;	
	int error;
	int SD;
	socklen_t ic = sizeof(struct sockaddr_in);
	//char buffer[500];
	int sockopt = 1;
	struct sockaddr_in dest; 
    struct sockaddr_in serv; 
	int i = 0;

    memset(&serv, 0, sizeof(serv)); 
    //memset(buffer, '0', sizeof(buffer)); 


    //begin network stuff =====================================================================================

	stuff.ai_flags = AI_PASSIVE; //ai_passive (server side flag) and 0 is the client flag
	stuff.ai_family = AF_INET; //all caps: means you want 4 byte addresses ipv4
	stuff.ai_socktype = SOCK_STREAM; //wants a tcp/ip: connection based communication
	stuff.ai_protocol = 0; //Any protocol 
	stuff.ai_addrlen = 0; 
	stuff.ai_addr = NULL;
	stuff.ai_canonname = NULL; 
	stuff.ai_next = NULL;

	error = getaddrinfo(NULL	, port, &stuff, &result); //change address and port to be user input
	if (error != 0){
		printf("There was an issue finding the specified ip.\n");
		exit(EXIT_FAILURE);
	} else {
//		printf("Attempting to connect to address.\n"); 
	}

	 for (resultp = result; resultp != NULL; resultp = resultp->ai_next) {
        SD = socket(resultp->ai_family, resultp->ai_socktype, resultp->ai_protocol);
        if (SD == -1) {
            continue;
}
       if (bind(SD, resultp->ai_addr, resultp->ai_addrlen) == 0){
//       		printf("Socket binded.\n");
            break; 
        }
       close(SD); //Only reaches here if cannot bind.
    }

   if (resultp == NULL) {               /* No address succeeded */
        printf("Could not bind.\n");
        exit(EXIT_FAILURE);
    }
    setsockopt(SD, SOL_SOCKET, (SO_REUSEADDR | SO_REUSEADDR), &sockopt, sizeof(int));
	
	freeaddrinfo(result);           /* No longer needed */


	listen(SD,20); //listening / ready to accept

	//end network initialization


	//===========================================================================================================
	

	
	key_t key;
	int shmid;
	int size = 4096;
	char *p;
	char message[] = " ";

	if ( errno = 0, (key = ftok( ".", 42 )) == -1 ) {
		printf( "ftok() failed  errno :  %s\n", strerror( errno ) );
		exit( 1 );
	} else if (errno = 0, (shmid = shmget( key, size, 0666 | IPC_CREAT | IPC_EXCL )) != -1 ) {
		errno = 0;
		vault = (bank *)shmat( shmid, 0, 0 );
		pthread_mutexattr_init(&bankattr);       
		pthread_mutexattr_setpshared(&bankattr, PTHREAD_PROCESS_SHARED);
		pthread_mutex_init(&vault->bLock, &bankattr);


		if ( vault == (void *)-1 ) {
			printf("shmat() failed  errno :  %s\n", strerror(errno));
			exit(1);
		} else {
			vault->accountNumber = 0;
			//pthread_create(&print, NULL, (void *) print_thread, NULL);
			//pthread_join(print, NULL);
			
			printf("Shared memory created.\n");
			char buffer[400];

						
	    }
	
	} else if (errno = 0, (shmid = shmget( key, 0, 0666 )) != -1 )	{ //Shared memory already existed
		errno = 0;
		vault = (bank*)shmat( shmid, 0, 0 );
		pthread_mutexattr_init(&bankattr);       
		pthread_mutexattr_setpshared(&bankattr, PTHREAD_PROCESS_SHARED);
		pthread_mutex_init(&vault->bLock, &bankattr);


		if ( vault == (void *)-1 ) {
			printf("shmat() failed  errno :  %s\n", strerror(errno));
			exit(1);
		} else {	

				printf("Accessing Shared Memory!\n");
		}//end inner else statement	
	}//end else statement
	  else{										
        printf( "shmget() failed  errno :  %s\n", strerror( errno ) );
        exit(1);
    }

//					End Shared Memory Initialization
//======================================================================================
    int sfd;
			//pthread_create(&print, NULL, (void *) print_thread, NULL);
			//pthread_join(print, NULL);

	 		while((sfd = accept(SD, (struct sockaddr *)&dest, &ic)) > 0){

			

	 		int pid = fork();
		 		if(pid != 0){
		 		pthread_create(&mywait, NULL, (void*) mywait_thread, NULL);
		 		shmctl(shmid, IPC_RMID, NULL);
		 		close(sfd); 	
	 		
	 		} else {
	 			pthread_create(&print, NULL, (void *) print_thread, NULL);
			 	int *socket =  (int*)malloc(sizeof(int));
			 	*socket = sfd;
			 	main_menu(socket);	
					}//end child loop
				} //end while sfd	

    return 0;

}//end main
	
