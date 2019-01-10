#include "dsm.h"
#include "errno.h"
#include <arpa/inet.h>

/* variables globales */

/* le nombre de processus effectivement crees */
volatile sig_atomic_t num_procs_creat = 0;

void usage(void)
{
  fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
  fflush(stdout);
  exit(EXIT_FAILURE);
}

/* Traitant de signal permettant la bonne terminaison des fils lors de la réception d'un sigchld. */
void sigchld_handler(int sig)
{
  wait(NULL);
  printf("!!! Un fils vient de se terminer !!!\n");
  fflush(stdout);
  num_procs_creat--;
}

int get_nb_machines(FILE *fd){
  char * line = NULL;
  size_t len = 0;
  ssize_t line_read;
  int j = 0;
  while ((line_read = getline(&line, &len, fd)) != -1) {
    j++;
  }
  return j;
}

int main(int argc, char *argv[])
{
  if (argc < 3)
    usage();
  /* ETAPE 0 : Mise en place d'un traitant pour recuperer les fils zombies */
  struct sigaction act;  //déclaration et initialisation de la structure sigaction
  memset(&act, 0, sizeof(struct sigaction));
  act.sa_handler = sigchld_handler;
  sigemptyset(&(act.sa_mask));
  act.sa_flags = 0;
  sigaction(SIGCHLD, &act, NULL); // mise en place effective du traitant


  /* ETAPE 1 : Lecture du nombre de ligne dans le machine_file passé en argument pour connaître le nombre et le nom des machines */
  FILE * fd_machine_file;
  int nb_procs;


  fd_machine_file = fopen(argv[1], "r");
  if (fd_machine_file == NULL)
    error("error fd open");

  nb_procs= get_nb_machines(fd_machine_file);
  char * machines[nb_procs];
  dsm_proc_distant_t *proc_infos[nb_procs];
  proc_infos_init(proc_infos, nb_procs); // initialisation du tableau de structure dsm_proc_distant_ts

  int j = 0;
  char * line = NULL;
  size_t len = 0;

  fseek(fd_machine_file, SEEK_SET, 0); // Remise à 0 de la position courante
  while ( getline(&line, &len, fd_machine_file) != -1) {
    machines[j] = strtok(line,"\n");
    sprintf(proc_infos[j]->name,"%s", machines[j]);
    proc_infos[j]->rank = j;
    j++;
  }

  fclose(fd_machine_file);
  free(line);
  /*------------- FIN ETAPE 1 ----------------*/

  /* ETAPE 2 : creation de la socket d'ecoute */
  // Déclarations
  int sock;
  struct sockaddr_in *serv_addr=malloc(sizeof(struct sockaddr_in));
  int *serv_port = malloc(sizeof(int));
  char *arg_ssh[7];
  arg_ssh[0]="ssh";
  arg_ssh[1]=malloc(20*sizeof(char)); // Nom de la machine distance
  arg_ssh[2] = "/home/julien/Projets/PR204/Phase2/bin/dsmwrap";
  arg_ssh[3]=malloc(sizeof(int));
  arg_ssh[4]=malloc(20*sizeof(char));
  arg_ssh[5]=malloc(strlen(argv[2])*sizeof(char)+1);
  strcpy(arg_ssh[5], argv[2]);
  arg_ssh[6]=NULL;




  // Initialisations
  sock = creer_socket_serv(serv_port,serv_addr);
  // Remplissage du tableau d'argument pour le ssh
  sprintf(arg_ssh[4],"%s", inet_ntoa(serv_addr->sin_addr));
  sprintf(arg_ssh[3],"%d", *serv_port);

  printf("%s//%d\n", inet_ntoa(serv_addr->sin_addr), *serv_port);

  // Ecoute effective (=listen)
  do_listen(sock, NB_MAX_PROC);

  /* --------- FIN ETAPE 2 ------------- */

  /* ETAPE 3 : création des fils (fork) et  gestion des redirections */
  // Déclarations
  int pipefd_stderr[nb_procs][2];
  int pipefd_stdout[nb_procs][2];
  int timeout = -1;
  struct pollfd poll_set[2*nb_procs];

  /* creation des fils */
  for(int i = 0; i < nb_procs ; i++) {
    /* creation du tube pour rediriger stdout */
    if (pipe(pipefd_stdout[i])==-1) error("pipe stdout");

    /* creation du tube pour rediriger stderr */
    if (pipe(pipefd_stderr[i])==-1) error("pipe stderr");

    pid_t pid = fork();
    if(pid == -1) error("fork");

    if (pid == 0) { /* fils */
      /* redirection stdout */
      close(STDOUT_FILENO);
      dup(pipefd_stdout[i][1]);
      close(pipefd_stdout[i][0]);

      /* redirection stderr */
      close(STDERR_FILENO);
      dup(pipefd_stderr[i][1]);
      close(pipefd_stderr[i][0]);

      /* jump to new prog : */
      arg_ssh[2] = "/home/gregory/Documents/PR204/Phase2/bin/dsmwrap";
      sprintf(arg_ssh[1],"%s", proc_infos[i]->name);
      int exec_res = execvp(arg_ssh[0], arg_ssh);

      if (exec_res == -1) error("exec");

    } else  if(pid > 0) { /* pere */
      /* fermeture des extremites des tubes non utiles */
      close(pipefd_stderr[i][1]);
      close(pipefd_stdout[i][1]);
      /* sockage des fd pour lecture futur dans les pipes */
      poll_set[i].fd = pipefd_stdout[i][0];
      poll_set[i].events = POLLIN;
      poll_set[nb_procs+i].fd = pipefd_stderr[i][0];
      poll_set[nb_procs+i].events = POLLIN;
      num_procs_creat++;
    } // END else if /* père */
  }
  /* ------- FIN ETAPE 3 -------- */

  /* ETAPE 4 : le père accepte toutes les connexions de ses fils
  puis récupère les infos tranmises et les redistribues à tout le monde */
  // Déclarations
  struct sockaddr_in client_addr;
  int fd_sock_init;
  int sent;
  int to_send;
  infos_dsm_t *info_init_dsmwrap;
  socklen_t addrlen = sizeof(struct sockaddr);
  info_init_t* buf_read;
  int test_read;

  // Initialisations
  info_init_dsmwrap = malloc(sizeof(info_init_dsmwrap));
  buf_read= malloc(sizeof(info_init_t));

  for(int i = 0; i < nb_procs ; i++){
    /* on accepte les connexions des processus dsm */
    fd_sock_init = do_accept(sock, (struct sockaddr*)&client_addr, &addrlen);
    printf("[dsmexec] connexion réussi (descrpteur de fichier associé : %d)\n", fd_sock_init);

    /* Reception du nom de la machine et de son numéro de port */
    test_read = read(fd_sock_init, buf_read, sizeof(info_init_t));
    if (test_read < 0)
      error("read");
    printf("[dsmexec]  nom machine : %s // port socket ecoute : %d // @IP = %s\n",buf_read->name, buf_read->port, inet_ntoa(client_addr.sin_addr));
    fflush(stdout);
    proc_infos[i]->port_ecoute = buf_read->port;
    strcpy(proc_infos[i]->name, buf_read->name);
    strcpy(proc_infos[i]->IP, inet_ntoa(client_addr.sin_addr));
    proc_infos[i]->rank = find_rank_byname(proc_infos, buf_read->name, nb_procs);
    proc_infos[i]->fd_sock_init = fd_sock_init;
    proc_infos[i]->bool_init = 1;
  }

  for (int j = 0; j < nb_procs; j++) {
    /* envoi du nombre de processus aux processus dsm */
    sent = 0;
    to_send = sizeof(int);
    do {
      sent += write(proc_infos[j]->fd_sock_init, &nb_procs, sizeof(int));
    } while(sent != to_send);

    /* envoi des rangs aux processus dsm */
    sent = 0;
    to_send = sizeof(int);
    do {
      sent += write(proc_infos[j]->fd_sock_init, &(proc_infos[j]->rank), sizeof(int));
    } while(sent != to_send);

    /* envoi des infos de connexion aux processus (au travers d'une structure info_init_dsmwrap) */
    for (int k = 0; k < nb_procs; k++) {
      strcpy(info_init_dsmwrap->IP, proc_infos[k]->IP);
      info_init_dsmwrap->port = proc_infos[k]->port_ecoute;
      info_init_dsmwrap->rank = proc_infos[k]->rank;
      sent = 0;
      to_send = sizeof(infos_dsm_t);
      do {
        sent += write(proc_infos[j]->fd_sock_init, info_init_dsmwrap, sizeof(infos_dsm_t)); //je pense que l'erreur est qu'on ne peut pas envoyer de structure avec write
      } while(sent != to_send);
    }
  }
  free(info_init_dsmwrap);
  /* ------- FIN ETAPE 4 -------- */

  /* gestion des E/S : on recupere les caracteres */
  /* sur les tubes de redirection de stdout/stderr */
  /*je recupere les infos sur les tubes de redirection
  jusqu'à ce qu'ils soient inactifs (ie fermes par les
  processus dsm ecrivains de l'autre cote ...)*/
  /* lecture dans les pipes */
  char* buffer;
  int read_ok;
  buffer = malloc(1000);

  while (num_procs_creat>0){
    int polling=0;
    do {
      polling = poll(poll_set,nb_procs,timeout);
    } while ((polling == -1) && (errno == EINTR) && (num_procs_creat>0));
    if (polling<0 && num_procs_creat==0)
      polling=0;
    else if (polling<0)
      error("poll");
    else if (polling==0)
      printf(" poll() timed out. End program.\n");

    for (int i = 0 ; i < nb_procs ; i++){ // Pour chaque tube
      if(poll_set[i].revents==POLLHUP){ // En cas de fermeture d'un tube
        poll_set[i].fd = -1; // Fermeture du file descriptor correspondant à stdout
        poll_set[nb_procs+i].fd = -1; // Fermeture du file descriptor correspondant à stderr
      }
      else if (poll_set[i].revents==POLLIN){ // En cas d'activité sur un tube
        memset(buffer,0, 1000);
        do {
          read_ok = read(poll_set[i].fd, buffer, 1000);
          if (read_ok==-1)
            error("read error");
          printf(">>[Processus %d - sdtout] %s", i+1, buffer);
          fflush(stdout);
        } while(read_ok<1);
      }
      else if (poll_set[nb_procs+i].revents==POLLIN){
        memset(buffer, 0, 1000);
        do {
          read_ok = read(poll_set[nb_procs+i].fd, buffer, 1000);
          if (read_ok==-1)
            error("read error");
          printf(">>[Processus %d - stderr] %s", i+1, buffer);
          fflush(stdout);
        } while(read_ok<1);
      }
    }
  }


  /* on ferme les descripteurs proprement */
  for (int i = 0; i < nb_procs; i++) {
    close(poll_set[i].fd);
    close(poll_set[nb_procs+i].fd);
  free(buffer);
  proc_infos_clean(proc_infos, nb_procs);
  /* on ferme la socket d'ecoute */
  close(sock);
  exit(EXIT_SUCCESS);
}
}
