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

   /* SOCKET SET-UP declarations */
   struct sockaddr_in serv_addr;
   char * host_ip;
   host_ip = malloc(LENGTH_IP_ADDR);
   strncpy(host_ip, argv[2], 10);
   int host_port = atoi(argv[1]);
   int sock_initialisation;

   /* SOCKET SET-UP construction */
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

   /* on execute la bonne commande */
   return 0;
}
