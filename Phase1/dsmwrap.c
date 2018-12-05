#include "common_impl.h"

int main(int argc, char **argv)
{
  printf("Lancement de dsmwrap\n");
  printf("[dsmwrap] Port serveur reçu : %d\n",atoi(argv[1]) );
  printf("[dsmwrap] IP serveur reçu : %d\n",atoi(argv[2]) );
  fflush(stdout);
   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */

   /* SOCKET d'initialisation SET-UP declarations */
   struct sockaddr_in serv_addr;
   char * host_ip;
   host_ip = malloc(LENGTH_IP_ADDR);
   strncpy(host_ip, argv[2], 10);
   int host_port = atoi(argv[1]);
   int sock_initialisation;

   /* SOCKET d'initialisation SET-UP construction */
   sock_initialisation = do_socket();
   init_client_addr(&serv_addr, host_ip, host_port);
   do_connect(sock_initialisation, serv_addr); // connexion à dsmexec (socket d'initialisation)

   /* Récupration du nom de la machine pour l'envoyer au lanceur */
   char hostname[1024];
   gethostname(hostname, 1024);

   /* Creation de la socket d'ecoute (port et IP aléatoires) pour les */
   /* connexions avec les autres processus dsm */
   int sock_ecoute;
   struct sockaddr_in *serv_addr_ecoute=malloc(sizeof(struct sockaddr_in));
   socklen_t addrlen = sizeof(struct sockaddr);
   int *serv_port = malloc(sizeof(int));

   sock_ecoute = creer_socket_serv(serv_port,serv_addr_ecoute);
   do_listen(sock_ecoute, NB_MAX_PROC);

   /* Récupération du n° de port de la socket */
   int port;
   socklen_t len = sizeof(struct sockaddr_in);
   getsockname(sock_ecoute, (struct sockaddr *) serv_addr_ecoute, &len);
   port = ntohs(serv_addr_ecoute->sin_port);

   /* Envoi du numero de port et du nom au lanceur au travers de la structure info_init */
   info_init_t *info_init = malloc(sizeof(info_init_t));
   strcpy(info_init->name, hostname);
   info_init->port = port;

   int sent = 0;
   int to_send = sizeof(info_init_t);
   do {
     sent +=    write(sock_initialisation, info_init, sizeof(info_init_t));
   } while(sent != to_send);

   /* Lecture du nombre de processus dsm */
   printf("[dsmwrap] début lecture\n");
   fflush(stdout);
   int nb_procs;
   int test_read_nbprocs = read(sock_initialisation, &nb_procs, sizeof(int));
   if (test_read_nbprocs < 0) {
     error("read nbprocs");
   }
   printf("[dsmwrap] nbprocs = %d\n", nb_procs);
   fflush(stdout);

   /* Lecture du rand du processus */
   int myrank;
   int test_read_rank = read(sock_initialisation, &myrank, sizeof(int));
   if (test_read_rank < 0) {
     error("read rank");
   }
   printf("[dsmwrap] rank = %d\n", myrank);
   fflush(stdout);

   /* Lecture des infos (port + IP) nécessaires aux connexions aux tres processus dsm */
   int fd_procs[nb_procs];
   info_init_dsmwrap_t * infos_init_dsmwrap[nb_procs];
   info_dsmwrap_init(infos_init_dsmwrap, nb_procs);
   for (int i = 0; i < nb_procs; i++) {
     int test_info_init_dsmwrap = read(sock_initialisation, infos_init_dsmwrap[i], sizeof(info_init_dsmwrap_t));
     if (test_info_init_dsmwrap < 0) {
       error("read info_init_dsmwrap");
     }
     printf("%d\n",infos_init_dsmwrap[i]->port );
     printf("%d\n",infos_init_dsmwrap[i]->rank );
     printf("%s\n",infos_init_dsmwrap[i]->IP );
     fflush(stdout);
   }

   /* SOCKET de communication avec les autres processus DMS : SET-UP declarations */
   struct sockaddr_in serv_addr_connexion;
   int sock;
   for (int j = 0; j <nb_procs; j++) {
     printf("====>%d\n",infos_init_dsmwrap[j]->rank);
     if (infos_init_dsmwrap[j]->rank > myrank) {
       sock = do_socket();
       init_client_addr(&serv_addr_connexion, infos_init_dsmwrap[j]->IP, infos_init_dsmwrap[j]->port);
       do_connect(sock, serv_addr_connexion); // connexion à dsmexec (socket d'initialisation)
       printf(">>>>>>>[dsmwrap] connexion ok : %d\n", sock);
     }
     else if (infos_init_dsmwrap[j]->rank != myrank){
       printf(">>>>>>>[dsmwrap%d] accept début %d\n",myrank, infos_init_dsmwrap[j]->rank );
       fflush(stdout);
       sock = do_accept(sock_ecoute, (struct sockaddr*)serv_addr_ecoute, &addrlen);
       printf(">>>>>>>[dsmwrap] accept fin : %d\n", sock);
       fflush(stdout);
     }
   }

   /* on execute la bonne commande */
   return 0;
}
