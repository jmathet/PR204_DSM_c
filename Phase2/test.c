#include "dsm.h"

int main(int argc, char **argv)
{
   int sock_ecoute;
   int sock_initialisation;
   sock_ecoute =atoi(argv[1]);
   sock_initialisation=atoi(argv[2]);
   printf("la socket d'initialisation est %d",sock_initialisation);
   printf("la socket d'écoute test %d",sock_ecoute);
   fflush(stdout);
   int *coucou=dsm_init();
   return 1;
  }
