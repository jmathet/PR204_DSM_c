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
   host_ip = malloc(10*sizeof(char));
   strncpy(host_ip, argv[2], 10);
   int host_port = atoi(argv[1]);
   int sock;

   /* SOCKET SET-UP construction */
   sock = do_socket();
   init_client_addr(&serv_addr, host_ip, host_port);
   do_connect(sock, serv_addr);


   /* Envoi du nom de machine au lanceur */
   //gethostname()

   /* Envoi du pid au lanceur */
   //getpid()

   /* Creation de la socket d'ecoute (port et IP aléatoires) pour les */
   /* connexions avec les autres processus dsm */

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm => utilisation de getname */

   /* on execute la bonne commande */
   return 0;
}
