#include "common_impl.h"

void error(char* error_description){
  perror(error_description);
  exit(EXIT_FAILURE);
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
