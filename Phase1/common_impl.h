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
#define LENGTH_NAME_MAX 50
#define LENGTH_IP_ADDR 10

#define ERROR_EXIT(str) {perror(str);exit(EXIT_FAILURE);}

/* definition du type des infos */
/* de connexion des processus dsm */
struct dsm_proc_distant  {
   int rank;
   char name[LENGTH_NAME_MAX];
   char IP[LENGTH_IP_ADDR];
   int port_ecoute; // port de la socket d'écoute (connexion inter processus distants)
   int fd_sock_init; // file descriptor de la socket d'initialisation
   int bool_init; // pseudo-booleen permettant de savoir si les infos on été initialisées
};
typedef struct dsm_proc_distant dsm_proc_distant_t;

/* definition du type des infos */
/* d'identification des processus dsm */
struct info_init {
  char name[LENGTH_NAME_MAX];
  int port;
};
typedef struct info_init info_init_t;

struct info_init_dsmwrap {
  char IP[LENGTH_IP_ADDR];
  int port;
  int rank;
};
typedef struct info_init_dsmwrap info_init_dsmwrap_t;

void error(char* error_description);

void proc_infos_init(dsm_proc_distant_t *proc_infos[], int nb_proc);

void info_dsmwrap_init(info_init_dsmwrap_t *infos_init_dsmwrap[], int nb_procs);

int creer_socket_serv(int *serv_port,struct sockaddr_in *serv_addr);

int do_socket();

void init_serv_addr(struct sockaddr_in *serv_addr, int port );

void init_client_addr(struct sockaddr_in *serv_addr, char *ip, int port) ;

void do_bind(int socket, struct sockaddr_in addr_in);

void do_listen(int socket, int nb_max);

int do_accept(int socket, struct sockaddr *addr, socklen_t* addrlen);

void do_connect(int sock, struct sockaddr_in host_addr);

int find_rank_byname(dsm_proc_distant_t *proc_infos[], char *name, int nb_proc);
