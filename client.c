#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <conn.h>

typedef struct server_info {
	int index;
	int fd;
	struct sockaddr_un sa;
} server_info; 


int main(int argc,char*argv[]){

int i,secret,p,k,w,random_s,n;
int fd_cs;
int* servers;
uint32_t id;
uint32_t net_id;
char* sockname;

struct timespec t;

server_info* server_list;

struct sockaddr_un sa;
sa.sun_family = AF_UNIX;

	//Controllo che il client riceva esattamente 3 argomenti in ingresso;
	if(argc != 4){
		SYSCALL(n,printf("Inserire correttamente i parametri p,k,w\n"),"printf parametri");
		exit(EXIT_FAILURE);
	}
	else{
		p = atoi(argv[1]);
		k = atoi(argv[2]);
		w = atoi(argv[3]);

		//Verifico che gli argomenti in ingresso rispettino le proprietà richieste;
		if(p < 1 || p >= k || w <= 3*p){
		SYSCALL(n,printf("Inserire correttamente i parametri p,k,w:\n  1 <= p < k && w > 3p\n"),"printf parametri");
		exit(EXIT_FAILURE);	
	}

}

	//Genero l'ID univoco (32 bit)

	srand((unsigned int)time(0));
	id = (uint32_t)rand()*getpid();

	//Genero il secret (1<secret<3000)

	do{
	srand((unsigned int)time(0));
	secret = (unsigned int) (rand()*getpid()) % 3000;
	}while(secret <= 0);
	
	SYSCALL(n,printf("CLIENT 0x%x SECRET %d\n",id,secret),"printf secret client");
	fflush(stdout);

	//Scelgo i p server distinti ai quali collegarmi

	server_list = (server_info*)malloc(p*sizeof(server_info));
	servers = random_server(p,k);


	int size = 0;
	for(i = 0; i < p; i++){
	
	SYSCALL(size,snprintf(NULL,0,"./OOB-server-%d",servers[i]) + 1,"snprintf size sockname");
	
    	CHECKNULL(sockname,malloc(size*sizeof(char)),"malloc sockname");
    	SYSCALL(n,snprintf(sockname,size,"./OOB-server-%d",servers[i]),"snprintf sockname");
	strncpy(sa.sun_path,sockname,UNIX_PATH_MAX);
	free(sockname);

	SYSCALL(fd_cs,socket(AF_UNIX,SOCK_STREAM,0),"creazione socket");
	
	//Salvo fd ed sa nella struttura dati server_list[];
	server_list[i].index = servers[i];
	server_list[i].fd = fd_cs;
	server_list[i].sa = sa;

		//Effettuo la connessione ai Server selezionati;
		while(connect(fd_cs,(struct sockaddr *)&sa, sizeof(sa)) == -1){
			if(errno == ENOENT)
		        	sleep(1);
		    	else exit(EXIT_FAILURE);
		}

	}
	
	for(i = 0; i < w; i++){
	
	//Scelgo un Server random tra quelli ai quali sono connesso;
	random_s = rand() % p;
	
	//Converto l'ID in Network Byte Order;
	net_id = htonl(id);
	
	//Imposto il tempo per la nanosleep;
	t.tv_sec = secret / 1000;
	t.tv_nsec = (secret % 1000) * 1000000;

	//Invio l'ID al server selezionato;
	SYSCALL(n, writen(server_list[random_s].fd,&net_id,sizeof(uint32_t)), "writen connection_1");
	
	//Attendo il tempo specificato prima di inviare un altro messaggio;
	nanosleep(&t,NULL);
	
	}
	
	for(i = 0; i < p; i++)
		close(server_list[i].fd);
		
	SYSCALL(n,printf("CLIENT 0x%x DONE\n",id),"client exiting");
	fflush(stdout);
	

	free(server_list);
	free(servers);
	
}


