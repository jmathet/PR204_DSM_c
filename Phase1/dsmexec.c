#include "common_impl.h"
#include "errno.h"
#include <arpa/inet.h>

/* variables globales */

/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;

void usage(void)
{
  fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
  fflush(stdout);
  exit(EXIT_FAILURE);
}

void sigchld_handler(int sig)
{
  wait(NULL);
  printf("!!! Un fils vient de se terminer !!!\n");
  fflush(stdout);
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
  //if (argc < 3)
  //usage();

  /* ETAPE 0 : Mise en place d'un traitant pour recuperer les fils zombies*/
  /* XXX.sa_handler = sigchld_handler; */
  struct sigaction *act = malloc(sizeof(struct sigaction));  //déclaration et initialisation de la structure sigaction
  act->sa_handler = &sigchld_handler;
  act->sa_flags = SA_SIGINFO;
  sigset_t *set;
  sigset_t *oldset;

  sigemptyset(set);
  if (sigaddset(set, SIGSEGV)==-1) {
    perror("erreur sigaddset");
    exit(0);
  }

  if (sigprocmask(SIG_BLOCK, set, oldset)==-1) {
    perror("erreur sigprocmask SIG_BLOCK");
    exit(0);
  }

  //appels fonctions sigactions
  sigaction(SIGCHLD, act, NULL);

  /* ETAPE 1 : Lecture du nombre de ligne dans le machine_file passé en argument pour connaître le nombre et le nom des machines */
  FILE * fd_machine_file;
  fd_machine_file = fopen(argv[1], "r");
  if (fd_machine_file == NULL)
  error("error fd open");

  int nb_procs = get_nb_machines(fd_machine_file);
  char * machines[nb_procs];
  dsm_proc_distant_t *proc_infos[nb_procs];
  proc_infos_init(proc_infos, nb_procs); // initialisation du tableau de structure dsm_proc_distant_t + boolen init ?

  int j = 0;
  ssize_t line_read;
  char * line = NULL;
  size_t len = 0;
  while ((line_read = getline(&line, &len, fd_machine_file)) != -1) {
    machines[j] = strtok(line,"\n");
    sprintf(proc_infos[j]->name,"%s", machines[j]);
    proc_infos[j]->rank = j;
    printf("%s // %d\n",proc_infos[j]->name, proc_infos[j]->rank );
    fflush(stdout);
    j++;
  }

  fclose(fd_machine_file);
  /*------------- FIN ETAPE 1 ----------------*/

  /* ETAPE 2 : creation de la socket d'ecoute */
  // INITS
  int sock;
  struct sockaddr_in *serv_addr=malloc(sizeof(struct sockaddr_in));
  int *serv_port = malloc(sizeof(int));
  char *arg_ssh[3];
  arg_ssh[1]=malloc(sizeof(int));
  arg_ssh[2]=malloc(20*sizeof(char));


  // SET UP
  sock = creer_socket_serv(serv_port,serv_addr);
  sprintf(arg_ssh[2],"%s\n", inet_ntoa(serv_addr->sin_addr));
  sprintf(arg_ssh[1],"%d", *serv_port);
  printf("%s//%d\n", inet_ntoa(serv_addr->sin_addr), *serv_port);

  // + ecoute effective (=listen)
  do_listen(sock, NB_MAX_PROC);

  /* --------- FIN ETAPE 2 ------------- */

  /* ETAPE 3 : création des fils (fork) et  gestion des redirections */
  int pipefd_stderr[nb_procs][2];
  int pipefd_stdout[nb_procs][2];

  int timeout=-1;
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

      /* Creation du tableau d'arguments pour le ssh */

      /* jump to new prog : */
      arg_ssh[0] = "/home/julien/Projets/PR204/Phase1/bin/dsmwrap";
      int exec_res = execlp("ssh", "ssh", "julien@localhost", arg_ssh[0], arg_ssh[1], arg_ssh[2], NULL);

      if (exec_res == -1) error("exec");

    } else  if(pid > 0) { /* pere */
      /* fermeture des extremites des tubes non utiles */
      close(pipefd_stderr[i][1]);
      close(pipefd_stdout[i][1]);
      /* sockage des fd pour lecture futur dans les pipes */
      poll_set[i].fd=pipefd_stdout[i][0];
      poll_set[i].events=POLLIN;
      poll_set[2*i].fd=pipefd_stderr[i][0];
      poll_set[2*i].events=POLLIN;
      num_procs_creat++;
    } // END else if /* père */
  }
  /* ------- FIN ETAPE 3 -------- */

  /* ETAPE 4 : le père accepte toutes les connexions de ses fils
  puis récupère les infos tranmises et les redistribues à tout le monde */
  struct sockaddr_in client_addr;
  for(int i = 0; i < nb_procs ; i++){

    /* on accepte les connexions des processus dsm */
    socklen_t addrlen = sizeof(struct sockaddr);
    int fd_sock_init = do_accept(sock, (struct sockaddr*)&client_addr, &addrlen);
    printf("[dsmexec] connexion réussi %d\n", fd_sock_init);

    /* Reception du nom de la machine et de son numéro de port */
    info_init_t* buf_read;
    buf_read= malloc(sizeof(info_init_t));
    int test_read = read(fd_sock_init, buf_read, sizeof(info_init_t));
    if (test_read < 0)
    error("read");
    printf("[dsmexec]  nom machine : %s // port socket ecoute : %d \n",buf_read->name, buf_read->port );
    printf("@IP = %s\n",inet_ntoa(client_addr.sin_addr));
    fflush(stdout);
    proc_infos[i]->port_ecoute = buf_read->port;
    strcpy(proc_infos[i]->name, buf_read->name);
    strcpy(proc_infos[i]->IP, inet_ntoa(client_addr.sin_addr));
    proc_infos[i]->rank = find_rank_byname(proc_infos, buf_read->name, nb_procs);
    proc_infos[i]->fd_sock_init = fd_sock_init;
    proc_infos[i]->bool_init = 1;
    printf("%d\n", proc_infos[i]->rank);
  }
  for (int j = 0; j < nb_procs; j++) {
    /* envoi du nombre de processus aux processus dsm*/
    int sent = 0;
    int to_send = sizeof(int);
    do {
      sent += write(proc_infos[j]->fd_sock_init, &nb_procs, sizeof(int));
    } while(sent != to_send);

    /* envoi des rangs aux processus dsm */
    sent = 0;
    to_send = sizeof(int);
    do {
      sent += write(proc_infos[j]->fd_sock_init, &(proc_infos[j]->rank), sizeof(int));
    } while(sent != to_send);

    /* envoi des infos de connexion aux processus */
    info_init_dsmwrap_t * info_init_dsmwrap;
    info_init_dsmwrap = malloc(sizeof(info_init_dsmwrap));
    for (int k = 0; k < nb_procs; k++) {

      strcpy(info_init_dsmwrap->IP, proc_infos[k]->IP);
      info_init_dsmwrap->port = proc_infos[k]->port_ecoute;
      info_init_dsmwrap->rank = proc_infos[k]->rank;
      int sent_info_init_dsmwrap = 0;
      to_send = sizeof(info_init_dsmwrap_t);
      do {
        sent_info_init_dsmwrap += write(proc_infos[j]->fd_sock_init, info_init_dsmwrap, sizeof(info_init_dsmwrap_t));
      } while(sent_info_init_dsmwrap != to_send);

    }
    printf(">>>>>>>>[dsmexec] envoi 3 OK\n");
    fflush(stdout);
  }
  /* gestion des E/S : on recupere les caracteres */
  /* sur les tubes de redirection de stdout/stderr */
  /*je recupere les infos sur les tubes de redirection
  jusqu'à ce qu'ils soient inactifs (ie fermes par les
  processus dsm ecrivains de l'autre cote ...)*/
  /* lecture dans les pipes */
  char* buffer;
  int read_ok;
  buffer = malloc(1000);
  int time_execution = 0;
  while (time_execution<1000){
    int polling=poll(poll_set,nb_procs,timeout);
    if (polling<0){
      perror("poll\n");
    }
    if (polling==0){
      printf(" poll() timed out. End program.\n");
    }
    for (int i = 0 ; i < nb_procs ; i++){
      if(poll_set[i].revents==POLLHUP){
        poll_set[i].fd = -1;
        poll_set[nb_procs+i].fd = -1;
      }
      else if (poll_set[i].revents==POLLIN){
        memset(buffer, 0, 1000);
        do {
          read_ok = read(poll_set[i].fd, buffer, 1000);
          if (read_ok==-1) error("read error");
          printf("%s", buffer);
          fflush(stdout);
        } while(read_ok<1);
      }
      else if (poll_set[nb_procs+i].revents==POLLIN){
        memset(buffer, 0, 1000);
        do {
          read_ok = read(poll_set[nb_procs+i].fd, buffer, 1000);
          if (read_ok==-1) error("read error");
          printf("%s", buffer);
          fflush(stdout);
        } while(read_ok<1);
      }
    }
    time_execution++;
  }

  /* on ferme les descripteurs proprement */

  /* on ferme la socket d'ecoute */
  //}
  exit(EXIT_SUCCESS);
}
