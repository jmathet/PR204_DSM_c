#include "common_impl.h"

int main(int argc, char **argv)
{
  int sock;
  struct sockaddr_in host_addr;
  printf("coucou");
  int host_port;
  int host_ip;
  printf("Port serveur : %d\n",atoi(argv[1]) );

   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */

   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */
   /* SOCKET SET-UP */
  //  init_client_addr(&host_addr, host_ip, host_port);
    //sock = do_socket();
    //do_connect(sock, host_addr);

   /* Envoi du nom de machine au lanceur */

   /* Envoi du pid au lanceur */

   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm => utilisation de getname */

   /* on execute la bonne commande */
   return 0;
}
