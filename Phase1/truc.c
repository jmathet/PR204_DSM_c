#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
   int fd;
   int i;
   char str[1024];
   char exec_path[1024];   
   char *wd_ptr = NULL;
   
   wd_ptr = getcwd(str,1024);
   fprintf(stdout,"Working dir is %s\n",str);
   
   fprintf(stdout,"Number of args : %i\n", argc);
   for(i= 0; i < argc ; i++)
     fprintf(stderr,"arg[%i] : %s\n",i,argv[i]);
    
   sprintf(exec_path,"%s/%s",str,"titi");	      
   fd = open(exec_path,O_RDONLY);
   if(fd == -1) perror("open");
   fprintf(stdout,"================ Valeur du descripteur : %i\n",fd);

   fflush(stdout);
   fflush(stderr);
   return 0;
}
