#include "dsm.h"
extern char **environ;

int main(int argc, char **argv)
{
  printf("Lancement de dsmwrap\n");
  printf("[dsmwrap] Port serveur reçu : %d\n",atoi(argv[1]) );
  printf("[dsmwrap] IP serveur reçu : %d\n",atoi(argv[2]) );
  printf("[dsmwrap] exécutable : %s\n",argv[3] );
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
  char hostname[1024];
  

  /* SOCKET d'initialisation SET-UP construction */
  sock_initialisation = do_socket();
  init_client_addr(&serv_addr, host_ip, host_port);
  do_connect(sock_initialisation, serv_addr); // connexion à dsmexec (socket d'initialisation)

  free(host_ip); // Libération de ressources intiles

  /* Récupration du nom de la machine pour l'envoyer au lanceur */
  gethostname(hostname, 1024);

  /* Creation de la socket d'ecoute (port et IP aléatoires) pour les */
  /* connexions avec les autres processus dsm */
  int sock_ecoute;
  struct sockaddr_in *serv_addr_ecoute=malloc(sizeof(struct sockaddr_in));
  socklen_t addrlen = sizeof(struct sockaddr);
  int *serv_port = malloc(sizeof(int));
  int port;
  int sent = 0;
  int to_send = sizeof(info_init_t);

  sock_ecoute = creer_socket_serv(serv_port,serv_addr_ecoute);
  do_listen(sock_ecoute, NB_MAX_PROC);

  free(serv_port); // Libération de ressources intiles

  /* Récupération du n° de port de la socket */
  socklen_t len = sizeof(struct sockaddr_in);
  getsockname(sock_ecoute, (struct sockaddr *) serv_addr_ecoute, &len);
  port = ntohs(serv_addr_ecoute->sin_port);

  /* Envoi du numero de port et du nom au lanceur au travers de la structure info_init */
  info_init_t *info_init = malloc(sizeof(info_init_t));
  strcpy(info_init->name, hostname);
  info_init->port = port;
  do {
    sent += write(sock_initialisation, info_init, sizeof(info_init_t));
  } while(sent != to_send);

  free(info_init); // Libération de ressources intiles

  /* Libération des ressources */
  free(serv_addr_ecoute);
//  close(sock_initialisation);
//  close(sock_ecoute);
  //info_dsmwrap_clean(info_init, nb_procs);

  /* on execute la bonne commande */
  char *argsam[2];
  argsam[0] = malloc(50);
  sprintf(argsam[0], "%s%s",PATH, argv[3]);
  printf("%s",argsam[1]);
  argsam[1]=NULL;


  char val1[24];
  sprintf(val1,"SOCKET_ECOUTE=%d",sock_ecoute);
  putenv(val1);
  char val2[24];
  sprintf(val2,"SOCKET_INITIALISATION=%d",sock_initialisation);
  putenv(val2);

  int exec_res =execvpe(argsam[0], argsam ,environ);
  if (exec_res == -1) error("exec dsminit");

  return 0;
}
