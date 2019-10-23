#if !defined(CONN_H)
#define CONN_H

#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>


#define UNIX_PATH_MAX 108
#define SIZE 24


#define SYSCALL(r,c,e) \
    if((r=c)==-1) { perror(e);exit(errno); }
#define CHECKNULL(r,c,e) \
    if ((r=c)==NULL) { perror(e);exit(errno); }


//Funzione per scegliere p server random tra i k disponibili;
int* random_server (int p, int k){
int i,j,r;

int* servers = malloc(p * sizeof(int));

	for(i = 0; i < p; i++){
		
		init:
		
		r = rand() % k;
		j = 0;
		while(j < i){
			if(servers[j] == r){
				goto init;
			}
		j++;
		}
		servers[i] = r;
	}

return servers;

}

static inline int readn(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
	if ((r=read((int)fd ,bufptr,left)) == -1) {
	    if (errno == EINTR) continue;
	    return -1;
	}
	if (r == 0) return 0;   // gestione chiusura socket
        left    -= r;
	bufptr  += r;
    }
    return size;
}

static inline int writen(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
	if ((r=write((int)fd ,bufptr,left)) == -1) {
	    if (errno == EINTR) continue;
	    return -1;
	}
	if (r == 0) return 0;  
        left    -= r;
	bufptr  += r;
    }
    return 1;
}


#endif /* CONN_H */

