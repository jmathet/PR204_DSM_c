#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
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
#define PATH "/home/gregory/Documents/PR204/Phase2"
#define _GNU_SOURCE

#define TOP_ADDR    (0x40000000)
#define PAGE_NUMBER (10) // 100 plus tard
#define PAGE_SIZE   (sysconf(_SC_PAGE_SIZE))
#define BASE_ADDR   (TOP_ADDR - (PAGE_NUMBER * PAGE_SIZE))
#define NB_MAX_PROC 50
#define LENGTH_NAME_MAX 50
#define LENGTH_IP_ADDR 10

extern char **environ;

int SOCKET_ECOUTE_GLOBAL;
int SOCKET_INITIALISATION_GLOBAL;

pthread_t comm_daemon;
extern int DSM_NODE_ID;
extern int DSM_NODE_NUM;

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

struct info_init {
  char name[LENGTH_NAME_MAX];
  int port;
};
typedef struct info_init info_init_t;

struct infos_dsm {
  char IP[LENGTH_IP_ADDR];
  int port;
  int rank;
};
typedef struct infos_dsm infos_dsm_t;

typedef enum
{
   NO_ACCESS,
   READ_ACCESS,
   WRITE_ACCESS,
   UNKNOWN_ACCESS
} dsm_page_access_t;

typedef enum
{
   INVALID,
   READ_ONLY,
   WRITE,
   NO_CHANGE
} dsm_page_state_t;

typedef int dsm_page_owner_t;

typedef struct
{
   dsm_page_state_t status;
   dsm_page_owner_t owner;
} dsm_page_info_t;

dsm_page_info_t table_page[PAGE_NUMBER];

char *dsm_init( int argc, char **argv);

void dsm_finalize(void);

void error(char* error_description);

void proc_infos_init(dsm_proc_distant_t *proc_infos[], int nb_proc);

void proc_infos_clean(dsm_proc_distant_t *proc_infos[], int nb_proc);

void info_dsminit_init(infos_dsm_t *infos_init_dsmwrap[], int nb_procs);

void info_dsm_clean(infos_dsm_t *infos_init[], int nb_procs);

int creer_socket_serv(int *serv_port,struct sockaddr_in *serv_addr);

int do_socket();

void init_serv_addr(struct sockaddr_in *serv_addr, int port );

void init_client_addr(struct sockaddr_in *serv_addr, char *ip, int port) ;

void do_bind(int socket, struct sockaddr_in addr_in);

void do_listen(int socket, int nb_max);

int do_accept(int socket, struct sockaddr *addr, socklen_t* addrlen);

void do_connect(int sock, struct sockaddr_in host_addr);

int find_rank_byname(dsm_proc_distant_t *proc_infos[], char *name, int nb_proc);
