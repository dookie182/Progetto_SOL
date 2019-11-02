#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <conn.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct estimate_t{

	uint32_t id;
	int secret;
	int estimate_number;

}estimate;

volatile sig_atomic_t sigint = 0;
volatile sig_atomic_t sigalrm = 0;
volatile sig_atomic_t sigquit = 0;

static void gestore (int signum) {


	if(signum == SIGINT){
		if(sigint == 0){
			sigint++;
			alarm(1);
		}
		else{
			sigquit = 1;
		}		
	}
	
	if(signum == SIGALRM){
		sigint = 0;
		sigalrm = 1;
	}
}


void print_all (estimate * estimated_list,int n_client,int flag){
	int n = 0;
	if(flag == 0){
		for(int i = 0; i < n_client; i++){

			SYSCALL(n,fprintf(stderr,"SUPERVISOR ESTIMATE %d FOR 0x%x BASED ON %d\n",estimated_list[i].secret,estimated_list[i].id,estimated_list[i].estimate_number),"fprintf stderr");
			}
	}else{
		for(int i = 0; i < n_client; i++){

				SYSCALL(n,fprintf(stdout,"SUPERVISOR ESTIMATE %d FOR 0x%x BASED ON %d\n",estimated_list[i].secret,estimated_list[i].id,estimated_list[i].estimate_number),"fprintf stdout");
				fflush(stdout);

		}
		printf("SUPERVISOR EXITING\n");
	}
}


int main(int argc,char*argv[]){

uint32_t id;
int n;
long int secret;
int k,i,j,size,pfd_dup,server_index,status;
char* args1,*args2,*msg,*svptr,*token = NULL;
int pfd[2];
bool notfound = true;
estimate *estimated_list = NULL;
int n_client = 0;
pid_t *pid_list;

if(argc != 2){
	SYSCALL(n,printf("Inserire il numero di Server da avviare!\n"),"printf argomenti");
	exit(EXIT_FAILURE);
}


struct sigaction s;
memset( &s, 0, sizeof(s) );
s.sa_handler = gestore;
SYSCALL(n,sigaction(SIGINT,&s,NULL),"Sigaction SIGINT"); 
SYSCALL(n,sigaction(SIGALRM,&s,NULL),"Sigaction SIGALRM"); 


k = atoi(argv[1]);

SYSCALL(n,printf("SUPERVISOR STARTING %d\n",k),"printf supervisor start");
fflush(stdout);


	//Creo la pipe;
	SYSCALL(n,pipe(pfd),"Creazione pipe");
	CHECKNULL(pid_list,(pid_t*)malloc(k*sizeof(pid_t)),"malloc pid_list");
	
	for(i = 0; i < k; i++){
		SYSCALL(pid_list[i],fork(),"fork supervisor");
		
		if(pid_list[i] == -1){
			perror("fork");
			exit(EXIT_FAILURE);
		}
		
		if(pid_list[i] == 0){
		
			//Duplico il descrittore della pipe per la scrittura;
			SYSCALL(pfd_dup,dup2(pfd[1],i+3),"Dup pfd");
			
			SYSCALL(size,snprintf(NULL,0,"%d",i)+1,"snprintf size args1");
			CHECKNULL(args1,(char*)malloc(size * sizeof(char)),"malloc args1");
			memset(args1,'\0',size);
			SYSCALL(n,snprintf(args1,size,"%d",i),"snprintf args1");
			
			SYSCALL(size,snprintf(NULL,0,"%d",pfd_dup)+1,"snprintf size args2");
			CHECKNULL(args2,(char*)malloc(size * sizeof(char)),"malloc pfd_dup");
			memset(args2,'\0',size);
			SYSCALL(n,snprintf(args2,size,"%d",pfd_dup),"snprintf args2");
			
			SYSCALL(n,execl("./server","server",args1,args2,NULL), "execl"); 
		}
	}
	
	SYSCALL(n,close(pfd[1]),"Chiusura Scrittura Pipe");



		
	while(1){
	CHECKNULL(msg,(char*)malloc(SIZE*sizeof(char)),"malloc msg");
	memset(msg,'\0',SIZE);
		while((n=read(pfd[0],msg,SIZE*sizeof(char))) <= 0){
		
			if(errno == EINTR){
				if(sigalrm == 1){
					print_all(estimated_list, n_client,0);
					sigalrm = 0;
				}
				
				if(sigquit == 1){
					SYSCALL(n,close(pfd[0]),"Chiusura Lettura Pipe");
					print_all(estimated_list, n_client,1);
					free(msg);
					if(estimated_list != NULL)
						free(estimated_list);
					for(i = 0; i < k; i++){
						kill(pid_list[i],SIGQUIT);
						wait(&status);
					}
					free(pid_list);
					exit(EXIT_SUCCESS);
				}
					
			continue;
			}
			
			if(n == -1){
			free(msg);
			free(pid_list);
			if(estimated_list != NULL)
				free(estimated_list);
			exit(EXIT_FAILURE);
			}
			
				
		}


		//Tokenizzo la stringa ricevuta;
		
		//Leggo l'indice del Server;
		CHECKNULL(token,strtok_r(msg," ",&svptr),"strtok_r 1");
		SYSCALL(server_index,strtol(token,NULL,10),"strtol server_index");
		
		//Leggo l'ID del Client;
		CHECKNULL(token,strtok_r(NULL," ",&svptr),"strtok_r 2");
		SYSCALL(id,strtol(token,NULL,16),"strtol id");
		
		//Leggo il Secret stimato;
		CHECKNULL(token,strtok_r(NULL," ",&svptr),"strtok_r 3");
		SYSCALL(secret,strtol(token,NULL,10),"strtol secret");

		
		//Cerco se esiste già un client con l'id ricevuto;
		notfound = true;
		for(j = 0; j < n_client; j++){
			if(id == estimated_list[j].id){
				estimated_list[j].estimate_number ++;
				notfound = false;
				if(secret < estimated_list[j].secret){
					estimated_list[j].secret = secret;
				}
			}
		}
		
		//Aggiungo un nuovo Client alla struttura dati;
		if(notfound){
			n_client++;
			if(n_client == 1){
				CHECKNULL(estimated_list,(estimate*)malloc(n_client * sizeof(estimate)),"realloc estimated_list");
			}
			else{
				CHECKNULL(estimated_list,(estimate*)realloc(estimated_list,n_client * sizeof(estimate)),"realloc estimated_list");
			}
			estimated_list[n_client-1].id = id;
			estimated_list[n_client-1].secret = secret;
			estimated_list[n_client-1].estimate_number = 1;
			
		}
		
		SYSCALL(n,printf("SUPERVISOR ESTIMATE %ld FOR 0x%x FROM %d\n", secret,id,server_index),"printf supervisor estimate");
		fflush(stdout);
		free(msg);
	}

}




