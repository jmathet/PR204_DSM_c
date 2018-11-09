#include "common_impl.h"

void error(char* error_description){
  perror(error_description);
  exit(EXIT_FAILURE);
}

int creer_socket(int *port_num)
{
   int fd = 0;
   int port;
   struct sockaddr_in *serv_addr;

   /* fonction de creation et d'attachement */
   /* d'une nouvelle socket */
   fd = do_socket();
   init_serv_addr(serv_addr, 0); // init avec port choisi par la machine
   do_bind(fd, *serv_addr);

   // Get my ip address and port
	 bzero(serv_addr, sizeof(serv_addr));
   int len = sizeof(*serv_addr);
   getsockname(fd, (struct sockaddr *) serv_addr, &len);
   port = ntohs(serv_addr->sin_port);

   *port_num = &port;

   printf("Local port : %d\n", port_num);
   /* renvoie le numero de descripteur */
   /* et modifie le parametre port_num */

   return fd;
}

int do_socket(){
  int file_des;
  do {
    file_des = socket(AF_INET, SOCK_STREAM, 0);
  } while ((file_des == -1) && (errno == EAGAIN || errno == EINTR));

  if (file_des == -1)
    error("socket");

  return file_des;
}


void init_serv_addr(struct sockaddr_in *serv_addr, int port)
 {
   memset(serv_addr, 0, sizeof(*serv_addr)); // clean structure
   serv_addr->sin_family = AF_INET; // IP V4
   serv_addr->sin_port = port;
   serv_addr->sin_addr.s_addr = INADDR_ANY;
 }

 void do_bind(int socket, struct sockaddr_in addr_in)
 {
   int bind_result = bind(socket, (struct sockaddr *) &addr_in, sizeof(addr_in));
   if (-1 == bind_result)
     error("bind");
 }

 void do_listen(int socket, int nb_max)
 {
   int listen_result = listen(socket, nb_max);
   if (-1 == listen_result)
     error("listen");
 }

 int do_accept(int socket, struct sockaddr *addr, socklen_t* addrlen)
{
  int file_des_new = accept(socket, addr, addrlen);
  if(-1 == file_des_new)
    error("accept");
  return file_des_new;
}
