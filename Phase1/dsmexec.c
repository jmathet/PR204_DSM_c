#include "common_impl.h"

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
     ssize_t read;
     fp = fopen(argv[1], "r");

     if (fp == NULL){
        perror("error fd");
        exit(EXIT_FAILURE);
      }

      int j = 0;
      while ((read = getline(&line, &len, fp)) != -1) {
        machines[j] = line;
        printf("%s",machines[j] );
        i++;
      }


      fclose(fp);
      if (line)
        free(line);


     /* creation de la socket d'ecoute */
     /* + ecoute effective */

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
      	if(pid == -1) ERROR_EXIT("fork");

      	if (pid == 0) { /* fils */

      	   /* redirection stdout */
           close(STDOUT_FILENO);
           dup(pipefd_stdout[1]);
           close(pipefd_stdout[0]);

      	   /* redirection stderr */
           close(STDERR_FILENO);
           dup(pipefd_stderr[1]);
           close(pipefd_stderr[0]);

      	   /* Creation du tableau d'arguments pour le ssh */

      	   /* jump to new prog : */
      	   /* execvp("ssh",newargv); */

      	} else  if(pid > 0) { /* pere */
      	   /* fermeture des extremites des tubes non utiles */
           close(pipefd_stderr[1]);
           close(pipefd_stdout[1]);
      	   num_procs_creat++;
      	}
     }


     for(i = 0; i < num_procs ; i++){

	/* on accepte les connexions des processus dsm */

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

     /* on ferme les descripteurs proprement */

     /* on ferme la socket d'ecoute */
  //}
   exit(EXIT_SUCCESS);
}
