#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <conn.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdbool.h>
#include <signal.h>


volatile sig_atomic_t sigquit = 0;

static void gestore (int signum) {

	if(signum == SIGQUIT)
		sigquit = 1;
			
}


void cleanup(char* sockname);
static void* worker(void* arg);
long gettime();


static int index;
int pfd;

int main(int argc, char* argv[]){

int size,n; 
long fd_c,fd_skt;
char* sockname;
pthread_t tid;
struct sockaddr_un sa;
sa.sun_family = AF_UNIX;

struct sigaction s,ign;
memset( &ign,0,sizeof(ign));
memset( &s, 0, sizeof(s) );
s.sa_handler = gestore;
ign.sa_handler = SIG_IGN;
SYSCALL(n,sigaction(SIGINT,&ign,NULL),"Sigaction SIGINT"); 
SYSCALL(n,sigaction(SIGALRM,&ign,NULL),"Sigaction SIGALRM");
SYSCALL(n,sigaction(SIGQUIT,&s,NULL),"Sigaction SIGQUIT");

	index = atoi(argv[1]);
	pfd = atoi(argv[2]);
	
	SYSCALL(size,snprintf(NULL,0,"./OOB-server-%d",index) + 1,"snprintf sockname size");
	CHECKNULL(sockname,(char*)malloc(size * sizeof(char)),"malloc sockname");
	memset(sockname,'\0',size);
    	SYSCALL(n,snprintf(sockname,size,"./OOB-server-%d",index),"snprintf sockname");
	strncpy(sa.sun_path,sockname,UNIX_PATH_MAX);
	cleanup(sockname);

	SYSCALL(fd_skt,socket(AF_UNIX,SOCK_STREAM,0),"creazione socket");
	SYSCALL(n,bind(fd_skt,(struct sockaddr *)&sa, sizeof(sa)),"bind");
	SYSCALL(n,listen(fd_skt,SOMAXCONN),"listen");
	SYSCALL(n,printf("SERVER %d ACTIVE\n",index),"server start");
	fflush(stdout);
	while(1){
	
		while((fd_c = accept(fd_skt,NULL,0)) <= 0){
			if(errno == EINTR){
				if(sigquit == 1){
					free(sockname);					
					exit(EXIT_SUCCESS);
				}
				continue;
			}
		}
	
		SYSCALL(n,printf("SERVER %d CONNECT FROM CLIENT\n",index),"printf connect");
		pthread_create(&tid,NULL,worker,(void*)fd_c);		
	}
}


static void* worker(void* arg){

long fd_c = (long)arg;
int n = -1;
uint32_t id;	
long t_last = 0,t_curr,t_tmp;
long int estimated = 3000;
char* buff;

	while(1){
	
		while((n = readn(fd_c, &id, sizeof(uint32_t))) <= 0){
			if(errno == EINTR){

				if(sigquit == 1){
					pthread_exit((void*)1);
				}
			}
			if(n == 0){
				SYSCALL(n,printf("SERVER %d CLOSING 0x%x ESTIMATE %ld\n",index,id,estimated),"printf server closing");
				fflush(stdout);
				close(fd_c);
				
				if(id != 0){
				CHECKNULL(buff,malloc(SIZE*sizeof(char)),"malloc buff");
				memset(buff,'\0',SIZE);
				SYSCALL(n,snprintf(buff,SIZE*sizeof(char),"%d %x %ld",index,id,estimated),"snprintf buff");
				SYSCALL(n,write(pfd,buff,SIZE*sizeof(char)),"pipe write");
				free(buff);
				}
				pthread_exit((void*)1);
			}
		}
	

		t_curr = gettime();
		
		//Converto l'ID in Host Byte Order;
		id = ntohl(id);
		t_tmp = t_curr - t_last;
		SYSCALL(n,printf("SERVER: %d INCOMING FROM CLIENT: 0x%x TIME: %ld\n",index,id,t_curr),"printf incoming estimate");
		fflush(stdout);
		if(t_tmp < estimated)
			estimated = t_tmp;
			
		t_last = t_curr;
	}
}


long gettime(){
struct timeval tv;
gettimeofday(&tv, NULL);
unsigned long ret = tv.tv_usec;
        ret /= 1000;
        ret += (tv.tv_sec * 1000);
return ret;
}


void cleanup(char* sockname){
unlink(sockname);
}
