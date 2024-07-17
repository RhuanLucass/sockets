#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

void main(void)
{
  int iSock;
  int iBytes;
  struct sockaddr_in dest_addr;
  char buffer[100];
  iSock = socket(AF_INET, SOCK_STREAM, 0); // Cria um novo socket com comunicação TCP/IP com IPv4 e tipo de comunicação TCP
  // Se o socket for criado corretamente retornará um descritor para o arquivo

  if (iSock == -1)
  {
    perror("socket:");
    exit(1);
  }
  else
  {
    printf("Socket criado com sucesso!");
  }

  // Criando estrutura de endereçamento padrão
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(3333);
  dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  bzero(&(dest_addr.sin_zero), 8);

  // Realizando conexão
  if(connect(iSock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) < 0){
    perror("Connect:");
    exit(1);
  }

  // Recebendo conteúdo do servidor
  if((iBytes = recv(iSock, buffer, 100, 0)) < 0){
    perror("Recv:");
    exit(1);
  }

  buffer[iBytes] = '\0'; // Adicionando final nulo
  printf("Recebido: %s", buffer);
  close(iSock);
}