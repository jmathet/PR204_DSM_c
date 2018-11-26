#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>



/* fin des includes */

#define TOP_ADDR    (0x40000000)
#define PAGE_NUMBER (10) // 100 plus tard
#define PAGE_SIZE   (sysconf(_SC_PAGE_SIZE))
#define BASE_ADDR   (TOP_ADDR - (PAGE_NUMBER * PAGE_SIZE))

typedef enum
{
   NO_ACCESS, 
   READ_ACCESS,
   WRITE_ACCESS, 
   UNKNOWN_ACCESS 
} dsm_page_access_t;

typedef enum
{   
   INVALID,
   READ_ONLY,
   WRITE,
   NO_CHANGE  
} dsm_page_state_t;

typedef int dsm_page_owner_t;

typedef struct
{ 
   dsm_page_state_t status;
   dsm_page_owner_t owner;
} dsm_page_info_t;

dsm_page_info_t table_page[PAGE_NUMBER];

pthread_t comm_daemon;
extern int DSM_NODE_ID;
extern int DSM_NODE_NUM;

char *dsm_init( int argc, char **argv);
void  dsm_finalize( void );
