#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/* autres includes (eventuellement) */

#define NB_MAX_PROC 50

#define ERROR_EXIT(str) {perror(str);exit(EXIT_FAILURE);}

/* definition du type des infos */
/* de connexion des processus dsm */
struct dsm_proc_conn  {
   int rank;
   /* a completer */
};
typedef struct dsm_proc_conn dsm_proc_conn_t;

/* definition du type des infos */
/* d'identification des processus dsm */
struct dsm_proc {
  pid_t pid;
  dsm_proc_conn_t connect_info;
};
typedef struct dsm_proc dsm_proc_t;

int creer_socket_serv(int *serv_port,struct sockaddr_in *serv_addr);

int do_socket();

void init_serv_addr(struct sockaddr_in *serv_addr, int port );

void init_client_addr(struct sockaddr_in *serv_addr, char *ip, int port) ;
void do_bind(int socket, struct sockaddr_in addr_in);

void do_listen(int socket, int nb_max);

int do_accept(int socket, struct sockaddr *addr, socklen_t* addrlen);

void do_connect(int sock, struct sockaddr_in host_addr);
