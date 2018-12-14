#include "dsm.h"

int DSM_NODE_NUM; /* nombre de processus dsm */
int DSM_NODE_ID;  /* rang (= numero) du processus */

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
   while(1)
     {
	/* a modifier */
	printf("[%i] Waiting for incoming reqs \n", DSM_NODE_ID);
	sleep(2);
     }
   //return;
}
send_request(int owner,int numpage,int fd)
{
  char requete[46];
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

static void dsm_handler( void *page_addr,infos_dsm_t *infos_dsm[])
{
   /* A modifier */
   printf("[%i] FAULTY  ACCESS !!! \n",DSM_NODE_ID);
   int numpage = address2num((char *)(page_addr));
   int owner = get_owner(numpage);
   int fd= infos_dsm[owner]->sock;
   send_request(owner,numpage,fd);
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
   int sock_initialisation=atoi(argv[1]);
   int sock_ecoute=atoi(argv[2]);
   /* reception du nombre de processus dsm envoye */
   /* par le lanceur de programmes (DSM_NODE_NUM)*/
   /* Lecture du nombre de processus dsm */
   printf("[dsmwrap] début lecture\n");
   fflush(stdout);
   int test_read_nbprocs = read(sock_initialisation, &DSM_NODE_NUM, sizeof(int));
   if (test_read_nbprocs < 0) {
     error("read nbprocs");
   }
   printf("[dsmwrap] nbprocs = %d\n", DSM_NODE_NUM);
   fflush(stdout);
   /* reception de mon numero de processus dsm envoye */
   /* par le lanceur de programmes (DSM_NODE_ID)*/
   /* Lecture du rand du processus */
   int test_read_rank = read(sock_initialisation, &DSM_NODE_ID, sizeof(int));
   if (test_read_rank < 0) {
     error("read rank");
   }
   printf("[dsmwrap] rank = %d\n", DSM_NODE_ID);
   fflush(stdout);
   /* reception des informations de connexion des autres */
   /* processus envoyees par le lanceur : */
   /* nom de machine, numero de port, etc. */
   /* Lecture des infos (port + IP) nécessaires aux connexions aux tres processus dsm */
   int fd_procs[DSM_NODE_NUM];
   infos_dsm_t *infos_dsm[DSM_NODE_NUM];
   info_dsmwrap_init(infos_dsm, DSM_NODE_NUM);
   for (int i = 0; i < DSM_NODE_NUM; i++) {
     int test_info_init_dsm = read(sock_initialisation, infos_dsm[i], sizeof(infos_dsm_t));
     if (test_info_init_dsm < 0) {
       error("read info_init_dsmwrap");
     }

   }
   /* initialisation des connexions */
   /* avec les autres processus : connect/accept */
   /*definition des variables nécessaires au poll*/
   char *message = malloc(20*sizeof(char));
   strcpy(message,"coucou");
   /* SOCKET de communication avec les autres processus DMS : SET-UP declarations */
   struct sockaddr_in serv_addr_connexion;
   int sock;
   for (int j = 0; j <DSM_NODE_NUM; j++) {
     printf("====>%d\n",infos_dsm[j]->rank);
     if (infos_dsm[j]->rank > DSM_NODE_ID) {
       sock = do_socket();
       init_client_addr(&serv_addr_connexion, infos_dsm[j]->IP, infos_dsm[j]->port);
       do_connect(sock, serv_addr_connexion);
       printf(">>>>>>>[dsm] connexion ok : %d\n", sock);
       fflush(stdout);
     }
     else if (infos_dsm[j]->rank != DSM_NODE_ID){
       printf(">>>>>>>[dsm%d] accept début %d\n",DSM_NODE_ID, infos_dsm[j]->rank );
       fflush(stdout);
       sock = do_accept(sock_ecoute, (struct sockaddr*)serv_addr_ecoute, &addrlen);
       printf(">>>>>>>[dsm] accept fin : %d\n", sock);
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
   pthread_create(&comm_daemon, NULL, dsm_comm_daemon, NULL);

   /* Adresse de début de la zone de mémoire partagée */
   return ((char *)BASE_ADDR);
}

void dsm_finalize( void )
{
   /* fermer proprement les connexions avec les autres processus */

   /* terminer correctement le thread de communication */
   /* pour le moment, on peut faire : */
   pthread_cancel(comm_daemon);

  return;
}
