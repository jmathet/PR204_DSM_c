#include "common_impl.h"

void error(char* error_description){
  perror(error_description);
  fflush(stdout);
  exit(EXIT_FAILURE);
}

void proc_infos_init(dsm_proc_distant_t *proc_infos[], int nb_procs){
  for (int i = 0; i < nb_procs; i++) {
    proc_infos[i] = malloc(sizeof(dsm_proc_distant_t));
    proc_infos[i]->bool_init = 0;
  }
}

void proc_infos_clean(dsm_proc_distant_t *proc_infos[], int nb_procs){
  for (int i = 0; i < nb_procs; i++) {
    free(proc_infos[i]);
    close(proc_infos[i]->fd_sock_init);
  }
}

void info_dsmwrap_init(info_init_dsmwrap_t *infos_init_dsmwrap[], int nb_procs){
  for (int i = 0; i < nb_procs; i++)
    infos_init_dsmwrap[i] = malloc(sizeof(info_init_dsmwrap_t));
}

void info_dsmwrap_clean(info_init_dsmwrap_t *infos_init_dsmwrap[], int nb_procs){
  for (int i = 0; i < nb_procs; i++)
    free(infos_init_dsmwrap[i]);
}

int creer_socket_serv(int *serv_port,struct sockaddr_in *serv_addr)
{
  int fd;
  int port;

  /* fonction de creation et d'attachement d'une nouvelle socket */
  fd = do_socket();
  init_serv_addr(serv_addr, *serv_port); // init avec port choisi par la machine
  do_bind(fd, *serv_addr);

  // Récupération du n° de port de la socket
  socklen_t len = sizeof(struct sockaddr_in);
  getsockname(fd, (struct sockaddr *) serv_addr, &len);
  port = ntohs(serv_addr->sin_port);
  *serv_port = port;

  /* renvoie le numero de descripteur */
  /* et modifie le parametre serv_port */
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
  memset(serv_addr, 0, sizeof(struct sockaddr_in)); // clean structure
  serv_addr->sin_family = AF_INET; // IP V4
  serv_addr->sin_port = port;
  serv_addr->sin_addr.s_addr = INADDR_ANY;
}

void init_client_addr(struct sockaddr_in *serv_addr, char *ip, int port) {
  // clean structure
  memset(serv_addr, 0, sizeof(*serv_addr));
  serv_addr->sin_family = AF_INET; // IP V4
  serv_addr->sin_port = htons(port); // specified port in args
  serv_addr->sin_addr.s_addr = inet_addr(ip); // specified server IP in args
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
  printf("[do_accept] début\n");
  int file_des_new = accept(socket, addr, addrlen);
  if(-1 == file_des_new)
    error("accept");
  return file_des_new;
}

void do_connect(int sock, struct sockaddr_in host_addr) {
  int connect_result;
  do {
    connect_result = connect(sock, (struct sockaddr *) &host_addr, sizeof(host_addr));
  } while ((connect_result == -1) && (errno == EAGAIN || errno == EINTR));
  if (connect_result == -1)
    error("connect");
}

int find_rank_byname(dsm_proc_distant_t *proc_infos[], char *name, int nb_proc){
  for (int i = 0; i < nb_proc; i++) {
    if (strcmp(proc_infos[i]->name, name)==0 && proc_infos[i]->bool_init==0) {
      proc_infos[i]->bool_init = 1;
      return i+1;
    }
  }
  return 0;
}
