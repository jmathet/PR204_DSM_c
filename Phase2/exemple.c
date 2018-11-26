#include "dsm.h"

int main(int argc, char **argv)
{
   char *pointer; 
   char *current;
   int value;

   pointer = dsm_init(argc,argv);
   current = pointer;

   printf("[%i] Coucou, mon adresse de base est : %p\n", DSM_NODE_ID, pointer);
   
   if (DSM_NODE_ID == 0)
     {
       current += 4*sizeof(int);
       value = *((int *)current);
       printf("[%i] valeur de l'entier : %i\n", DSM_NODE_ID, value);
     } 
   else if (DSM_NODE_ID == 1)
     {
       current += PAGE_SIZE;
       current += 16*sizeof(int);

       value = *((int *)current);
       printf("[%i] valeur de l'entier : %i\n", DSM_NODE_ID, value);
     }
   dsm_finalize();
   return 1;
}
