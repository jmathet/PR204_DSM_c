#include "common_impl.h"
#include "errno.h"

/* variables globales */

/* un tableau gerant les infos d'identification */
/* des processus dsm */
dsm_proc_t *proc_array = NULL;

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
   /* on traite les fils qui se terminent */
   /* pour eviter les zombies */
}


int main(int argc, char *argv[])
{
  //if (argc < 3){
    //usage();
  //} else {
     pid_t pid;
     int num_procs = 3;
     int i = 0;
     char * machines[3];

     /* Mise en place d'un traitant pour recuperer les fils zombies*/
     /* XXX.sa_handler = sigchld_handler; */

     /* lecture du fichier de machines */
     /* 1- on recupere le nombre de processus a lancer */
     /* 2- on recupere les noms des machines : le nom de */
     /* la machine est un des elements d'identification */
     FILE * fp;
     char * line = NULL;
     size_t len = 0;
     ssize_t line_read;
     fp = fopen(argv[1], "r");

     if (fp == NULL){
        perror("error fd");
        exit(EXIT_FAILURE);
      }

      int j = 0;
      while ((line_read = getline(&line, &len, fp)) != -1) {
        machines[j] = line;
        printf("%s",machines[j] );
        i++;
      }


      fclose(fp);
      if (line)
        free(line);



     /* creation de la socket d'ecoute */
     // INITS
     int sock;
     struct sockaddr_in serv_addr;
     int serv_port = 8080;

     int enable=1; // used for setsockopt

     // SET UP
     sock = do_socket();
     if(-1 == setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)))
       perror("setsockopt");
     init_serv_addr(&serv_addr, serv_port);
     do_bind(sock, serv_addr);
     do_listen(sock, NB_MAX_PROC);
     /* + ecoute effective (=listen)*/

     struct pollfd poll_set[6];
     int nfds = 0;
     memset(poll_set, 0, sizeof(struct pollfd));

     /* creation des fils */
     for(i = 0; i < num_procs ; i++) {

      	/* creation du tube pour rediriger stdout */
        int pipefd_stdout[2];
        if (pipe(pipefd_stdout)==-1){
          perror("pipe stdout");
          exit(EXIT_FAILURE);
        }

      	/* creation du tube pour rediriger stderr */
        int pipefd_stderr[2];
        if (pipe(pipefd_stderr)==-1){
          perror("pipe stderr");
          exit(EXIT_FAILURE);
        }

      	pid = fork();
        nfds++;
      	if(pid == -1) ERROR_EXIT("fork");

      	if (pid == 0) { /* fils */
      	   /* redirection stdout */
           dup(STDOUT_FILENO);
           close(STDOUT_FILENO);
           int test_fils_stdout = dup(pipefd_stdout[1]);
           //printf("N° du descriptteur : %d\n", test_fils_stdout);
           fflush(stdout);

           close(pipefd_stdout[0]);

      	   /* redirection stderr */
           close(STDERR_FILENO);
           dup(pipefd_stderr[1]);
           close(pipefd_stderr[0]);

      	   /* Creation du tableau d'arguments pour le ssh */

      	   /* jump to new prog : */
      	   /* execvp("ssh",newargv); */
           int e2 = execlp("ssh", "ssh", "julien@localhost", "/home/julien/Projets/PR204/Phase1/bin/dsmwrap", NULL);

           if (e2 == -1) {
             perror("exec");
             exit(EXIT_FAILURE);

           }

      	} else  if(pid > 0) { /* pere */
          poll_set[i].fd =pipefd_stdout[0];
          poll_set[i].events =POLLIN;
          poll_set[i+1].fd =pipefd_stderr[0];
          poll_set[i+1].events =POLLIN;


      	   /* fermeture des extremites des tubes non utiles */
           close(pipefd_stderr[1]);
           close(pipefd_stdout[1]);
           char buffer[100];
           memset(buffer, 0, 100);
           read(pipefd_stdout[0], buffer, 100);
           printf("lu : %s\n", buffer);
      	   num_procs_creat++;
      	}
     }
    struct sockaddr_in client_addr;

    for(i = 0; i < num_procs ; i++){

	  /* on accepte les connexions des processus dsm */
    socklen_t addrlen = sizeof(struct sockaddr);
    int connection_fd = do_accept(sock, (struct sockaddr*)&client_addr, &addrlen);
    printf("connexion réussi\n");
	/*  On recupere le nom de la machine distante */
	/* 1- d'abord la taille de la chaine */
	/* 2- puis la chaine elle-meme */

	/* On recupere le pid du processus distant  */

	/* On recupere le numero de port de la socket */
	/* d'ecoute des processus distants */
     }

     /* envoi du nombre de processus aux processus dsm*/

     /* envoi des rangs aux processus dsm */

     /* envoi des infos de connexion aux processus */

     /* gestion des E/S : on recupere les caracteres */
     /* sur les tubes de redirection de stdout/stderr */
     /* while(1)
         {
            je recupere les infos sur les tubes de redirection
            jusqu'à ce qu'ils soient inactifs (ie fermes par les
            processus dsm ecrivains de l'autre cote ...)

         };
      */

     /* on attend les processus fils */
     int timeout = 3 * 60 * 1000;
     poll(poll_set, nfds, timeout);

     /* on ferme les descripteurs proprement */

     /* on ferme la socket d'ecoute */
  //}
   exit(EXIT_SUCCESS);
}
