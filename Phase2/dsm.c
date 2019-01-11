#include "dsm.h"

int DSM_NODE_ID;
int DSM_NODE_NUM;

void error(char* error_description){
  perror(error_description);
  exit(EXIT_FAILURE);
}

void proc_infos_init(dsm_proc_distant_t *proc_infos[], int nb_procs){
  for (int i = 0; i < nb_procs; i++) {
    proc_infos[i] = malloc(sizeof(dsm_proc_distant_t));
    proc_infos[i]->bool_init = 0;
  }
}

void info_dsmwrap_init(infos_dsm_t *infos_dsm[], int nb_procs){
  for (int i = 0; i < nb_procs; i++)
  infos_dsm[i] = malloc(sizeof(infos_dsm_t));
}

void proc_infos_clean(dsm_proc_distant_t *proc_infos[], int nb_procs){
  for (int i = 0; i < nb_procs; i++) {
    free(proc_infos[i]);
    close(proc_infos[i]->fd_sock_init);
  }
}

void info_dsmwrap_clean(infos_dsm_t *infos_init_dsmwrap[], int nb_procs){
  for (int i = 0; i < nb_procs; i++)
    free(infos_init_dsmwrap[i]);
}

int creer_socket_serv(int *serv_port,struct sockaddr_in *serv_addr)
{
  int fd;
  int port;

  /* fonction de creation et d'attachement d'une nouvelle socket */
  fd = do_socket();
  init_serv_addr(serv_addr, *serv_port); // init avec port choisi par la machine
  do_bind(fd, *serv_addr);

  // Récupération du n° de port de la socket
  socklen_t len = sizeof(struct sockaddr_in);
  getsockname(fd, (struct sockaddr *) serv_addr, &len);
  port = ntohs(serv_addr->sin_port);
  *serv_port = port;

  /* renvoie le numero de descripteur */
  /* et modifie le parametre serv_port */
  return fd;
}

int do_socket(){
  int file_des;
  do {
    file_des = socket(AF_INET, SOCK_STREAM, 0);
  } while ((file_des == -1) && (errno == EAGAIN || errno == EINTR));

  if (file_des == -1)
  error("socket");

  return file_des;
}

void init_serv_addr(struct sockaddr_in *serv_addr, int port)
{
  memset(serv_addr, 0, sizeof(struct sockaddr_in)); // clean structure
  serv_addr->sin_family = AF_INET; // IP V4
  serv_addr->sin_port = port;
  serv_addr->sin_addr.s_addr = INADDR_ANY;
}

void init_client_addr(struct sockaddr_in *serv_addr, char *ip, int port) {
  // clean structure
  memset(serv_addr, '\0', sizeof(*serv_addr));
  serv_addr->sin_family = AF_INET; // IP V4
  serv_addr->sin_port = htons(port); // specified port in args
  serv_addr->sin_addr.s_addr = inet_addr(ip); // specified server IP in args
}

void do_bind(int socket, struct sockaddr_in addr_in)
{
  int bind_result = bind(socket, (struct sockaddr *) &addr_in, sizeof(addr_in));
  if (-1 == bind_result)
  error("bind");
}

void do_listen(int socket, int nb_max)
{
  int listen_result = listen(socket, nb_max);
  if (-1 == listen_result)
  error("listen");
}

int do_accept(int socket, struct sockaddr *addr, socklen_t* addrlen)
{
  int file_des_new =-1;
  do {
     file_des_new = accept(socket, addr, addrlen);
  } while( (-1==file_des_new) && (errno == EAGAIN || errno==EINTR)) ;
  if(-1 == file_des_new)
    error("accept");
  return file_des_new;
}

void do_connect(int sock, struct sockaddr_in host_addr) {
  int connect_result;

  do {
    connect_result = connect(sock, (struct sockaddr *) &host_addr, sizeof(host_addr));
  } while ((connect_result == -1) && (errno == EAGAIN || errno == EINTR));

  if (connect_result == -1)
  error("connect");
}

int find_rank_byname(dsm_proc_distant_t *proc_infos[], char *name, int nb_proc){
  for (int i = 0; i < nb_proc; i++) {
    if (strcmp(proc_infos[i]->name, name)==0 && proc_infos[i]->bool_init==0) {
      proc_infos[i]->bool_init = 1;
      return i+1;
    }
  }
  return 0;
}

/*void info_dsmwrap_init(infos_dsm_t *infos_init[], int nb_procs){
  for (int i = 0; i < nb_procs; i++)
  infos_init[i] = malloc(sizeof(infos_dsm_t));
}*/




/* indique l'adresse de debut de la page de numero numpage */
static char *num2address( int numpage )
{
   char *pointer = (char *)(BASE_ADDR+(numpage*(PAGE_SIZE)));

   if( pointer >= (char *)TOP_ADDR ){
      fprintf(stderr,"[%i] Invalid address !\n", DSM_NODE_ID);
      return NULL;
   }
   else return pointer;
}

int address2num(char * addr){
    return (((long int)(addr-BASE_ADDR))/(PAGE_SIZE));
}

/* fonctions pouvant etre utiles */
static void dsm_change_info( int numpage, dsm_page_state_t state, dsm_page_owner_t owner)
{
   if ((numpage >= 0) && (numpage < PAGE_NUMBER)) {
	if (state != NO_CHANGE )
	table_page[numpage].status = state;
      if (owner >= 0 )
	table_page[numpage].owner = owner;
      return;
   }
   else {
	fprintf(stderr,"[%i] Invalid page number !\n", DSM_NODE_ID);
      return;
   }
}

static dsm_page_owner_t get_owner( int numpage)
{
   return table_page[numpage].owner;
}

static dsm_page_state_t get_status( int numpage)
{
   return table_page[numpage].status;
}

/* Allocation d'une nouvelle page */
static void dsm_alloc_page( int numpage )
{
   char *page_addr = num2address( numpage );
   mmap(page_addr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   return ;
}

/* Changement de la protection d'une page */
static void dsm_protect_page( int numpage , int prot)
{
   char *page_addr = num2address( numpage );
   mprotect(page_addr, PAGE_SIZE, prot);
   return;
}

static void dsm_free_page( int numpage )
{
   char *page_addr = num2address( numpage );
   munmap(page_addr, PAGE_SIZE);
   return;
}

static void *dsm_comm_daemon( void *arg)
{
  printf("[dsm_comm_daemon] Lancement \n");
  fflush(stdout);

  // Initialisation
  struct pollfd poll_set[DSM_NODE_NUM];
  int polling=0;
  char* buffer;
  int read_ok;
  int *sock;

  // Allocations
  buffer = malloc(1000);
  //sock = malloc(sizeof(int)*DSM_NODE_NUM);

  // Extraction des arguments
  sock = (int *)arg;

  for (int j = 0; j < DSM_NODE_NUM; j++) {
    poll_set[j].fd = sock[j];
    poll_set[j].events = POLLIN;
  }

  while(1){
    do {
      polling = poll(poll_set,DSM_NODE_NUM,-1);
    } while ((polling == -1) && (errno == EINTR));
    if (polling<0)
      error("poll");
    else if (polling==0)
      printf(" poll() timed out. End program.\n");

    for (int i = 0 ; i < DSM_NODE_NUM ; i++){
      if(poll_set[i].revents==POLLHUP){ // En cas de rupture de la socket
        poll_set[i].fd = -1;
      }
      else if (poll_set[i].revents==POLLIN){ // En cas d'activité sur la socket
        memset(buffer, 0, 1000);
        do {
          read_ok = read(poll_set[i].fd, buffer, 1000);
          if (read_ok==-1)
            error("read error");
        } while(read_ok<1);
      }
    }
  }
}


send_request(int owner,int numpage,int fd)
{
  char requete[100];
  memset(requete,'\0',46);
  sprintf(requete,"le processus %d demande d'acceder à la page : %d",DSM_NODE_ID,numpage);
  int wr=write(fd,requete,46);
  if (wr < 0) {
    error("error sending request\n");
  }
}

static int dsm_send(int dest,void *buf,size_t size)
{
   /* a completer */
}

static int dsm_recv(int from,void *buf,size_t size)
{
   /* a completer */
}

static void dsm_handler( void *page_addr)
{
   /* A modifier */
   printf("[%i] FAULTY  ACCESS !!! \n",DSM_NODE_ID);
   int numpage = address2num((char *)(page_addr));
   int owner = get_owner(numpage);
   int fd = SOCKET_ECOUTE_GLOBAL;
   //send_request(owner,nu);
   abort();
}

/* traitant de signal adequat */
static void segv_handler(int sig, siginfo_t *info, void *context)
{
   /* A completer*/
  if (info->si_code==SEGV_MAPERR){                                                                         //--> adresse sans objet
   /* adresse qui a provoque une erreur */
   void  *addr = info->si_addr;
  /* Si ceci ne fonctionne pas, utiliser a la place :*/
  /*
   #ifdef __x86_64__
   void *addr = (void *)(context->uc_mcontext.gregs[REG_CR2]);
   #elif __i386__
   void *addr = (void *)(context->uc_mcontext.cr2);
   #else
   void  addr = info->si_addr;
   #endif
   */
   /*
   pour plus tard (question ++):
   if (info->si_code==SEGV_ACCERR)--> permissions invalide
   dsm_access_t access  = (((ucontext_t *)context)->uc_mcontext.gregs[REG_ERR] & 2) ? WRITE_ACCESS : READ_ACCESS;
  */
   /* adresse de la page dont fait partie l'adresse qui a provoque la faute */
   void  *page_addr  = (void *)(((unsigned long) addr) & ~(PAGE_SIZE-1));

   if ((addr >= (void *)BASE_ADDR) && (addr < (void *)TOP_ADDR))
     {
	dsm_handler(page_addr);
     }
   else
     {
	/* SIGSEGV normal : ne rien faire*/
     }
    }
}

/* Seules ces deux dernieres fonctions sont visibles et utilisables */
/* dans les programmes utilisateurs de la DSM                       */
char *dsm_init(int argc, char **argv)
{
   struct sigaction act;
   int index;
   int nb_procs;

   char *val1=getenv("SOCKET_INITIALISATION");
   int SOCKET_INITIALISATION_GLOBAL=atoi(val1);
   char *val2=getenv("SOCKET_ECOUTE");
   int SOCKET_ECOUTE_GLOBAL=atoi(val2);

   /* Lecture du nombre de processus dsm */
   printf("[DSM init] début lecture\n");
   fflush(stdout);

   int test_read_nbprocs = read(SOCKET_INITIALISATION_GLOBAL, &DSM_NODE_NUM, sizeof(int));
   if (test_read_nbprocs < 0) {
     error("read nbprocs");
   }

   printf("[DSM init] DSM_NODE_NUM = %d\n", DSM_NODE_NUM);
   fflush(stdout);
   /* Lecture du rand du processus */
   int myrank;
   int test_read_rank = read(SOCKET_INITIALISATION_GLOBAL, &DSM_NODE_ID, sizeof(int));
   if (test_read_rank < 0) {
     error("read rank");
   }
   printf("[DSM init] rank = %d\n", DSM_NODE_ID);
   fflush(stdout);

   /* Lecture des infos (port + IP) nécessaires aux connexions aux tres processus dsm */
   infos_dsm_t *infos_init[DSM_NODE_NUM];
   info_dsmwrap_init(infos_init, DSM_NODE_NUM);
   for (int i = 0; i < DSM_NODE_NUM; i++) {
     int test_info_init = read(SOCKET_INITIALISATION_GLOBAL,infos_init[i],sizeof(infos_dsm_t));
     printf("########### lecture dsm init pour le rank %d\n", infos_init[i]->rank);
     fflush(stdout);
     if (test_info_init < 0)
       error("read info_init_dsminit");
   }

   /* initialisation des connexions */
   /* avec les autres processus : connect/accept */
   /*definition des variables nécessaires au poll*/

   struct sockaddr_in *serv_addr_ecoute=malloc(sizeof(struct sockaddr_in));
   socklen_t addrlen = sizeof(struct sockaddr);
   int *serv_port = malloc(sizeof(int));

   /* SOCKET de communication avec les autres processus DMS : SET-UP declarations */
   struct sockaddr_in serv_addr_connexion;
   int sock[DSM_NODE_NUM];
   for (int j = 0; j <DSM_NODE_NUM; j++) {
     sock[j] = -1;
     if (infos_init[j]->rank > DSM_NODE_ID) {
       sock[j] = do_socket();
       init_client_addr(&serv_addr_connexion, infos_init[j]->IP, infos_init[j]->port);
       do_connect(sock[j], serv_addr_connexion);
       printf(">>>>>>>[dsm] connexion ok : %d\n", sock[j]);
       fflush(stdout);
     }
     else if (infos_init[j]->rank != DSM_NODE_ID){
       printf(">>>>>>>[dsm%d] accept début %d\n",DSM_NODE_ID, infos_init[j]->rank );
       fflush(stdout);
       sock[j] = do_accept(SOCKET_ECOUTE_GLOBAL, (struct sockaddr*)serv_addr_ecoute, &addrlen);
       printf(">>>>>>>[dsm] accept fin de  la fin : %d\n", sock[j]);
       fflush(stdout);
     }
   }


   /* Allocation des pages en tourniquet */
   for(index = 0; index < PAGE_NUMBER; index ++){
     if ((index % DSM_NODE_NUM) == DSM_NODE_ID)
       dsm_alloc_page(index);
     dsm_change_info( index, WRITE, index % DSM_NODE_NUM);
   }

   /* mise en place du traitant de SIGSEGV */
   act.sa_flags = SA_SIGINFO; // traitant étendendu
   act.sa_sigaction = segv_handler;
   sigaction(SIGSEGV, &act, NULL);

   /* creation du thread de communication */
   /* ce thread va attendre et traiter les requetes */
   /* des autres processus */
   pthread_create(&comm_daemon, NULL, dsm_comm_daemon, (void*)&sock);

   /* Adresse de début de la zone de mémoire partagée */
   return ((char *)BASE_ADDR);
}

void dsm_finalize(void)
{
   /* fermer proprement les connexions avec les autres processus */
   /* Libération des ressources */


   sleep(2*DSM_NODE_NUM);
   /* terminer correctement le thread de communication */
   /* pour le moment, on peut faire : */
   pthread_cancel(comm_daemon);

  return;
}
