#include "dsm.h"

int main(int argc, char **argv)
{
   char *pointeur =malloc(20*sizeof(char));
   pointeur =dsm_init(argc,argv);
   //int *coucou=dsm_init();
   return 1;
  }
