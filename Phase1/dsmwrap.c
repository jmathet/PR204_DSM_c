#include "common_impl.h"

int main(int argc, char **argv)
{
  printf("Lancement de dsmwrap\n");
  int sock;
  printf("Port serveur : %d\n",atoi(argv[1]) );
  printf("IP serveur : %d\n",atoi(argv[2]) );
   /* processus intermediaire pour "nettoyer" */

   /* la liste des arguments qu'on va passer */

   /* a la commande a executer vraiment */

   /* SOCKET SET-UP declarations */
   struct sockaddr_in serv_addr;
   char * host_ip;
   host_ip = malloc(10*sizeof(char));
   strncpy(host_ip, argv[2], 10);
   int host_port = atoi(argv[1]);
   int fd;

   /* SOCKET SET-UP construction */
   fd = do_socket();
   init_client_addr(&serv_addr, host_ip, host_port);

   do_connect(fd, serv_addr);
   printf("[dsmwrap]connexion en cours\n");
   /* au lanceur et envoyer/recevoir les infos */

   /* necessaires pour la phase dsm_init */



   /* Envoi du nom de machine au lanceur */

   /* Envoi du pid au lanceur */

   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm => utilisation de getname */

   /* on execute la bonne commande */
   while(1);
   return 0;
}
