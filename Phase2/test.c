#include "dsm.h"

int main(int argc, char **argv)
{
   char *pointeur =malloc(20*sizeof(char));
   pointeur =dsm_init(argc,argv);

   printf("JE SUIS LE PROGRAMME TEST\n");

   dsm_finalize();
   return 1;
  }
