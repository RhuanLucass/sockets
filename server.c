#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

void main(void)
{
  // Criando socket

  int iSock;
  iSock = socket(AF_INET, SOCK_STREAM, 0); // Cria um novo socket com comunicação TCP/IP com IPv4 e tipo de comunicação TCP
  // Se o socket for criado corretamente retornará um descritor para o arquivo

  if (iSock == -1)
  {
    perror("socket:");
    exit(1);
  }
  else
  {
    printf("Socket criado com sucesso!\n");
  }

  // sockaddr
  // Estrutura padrão definida com os parâmetros que devem ser passados para as outras funções
  // Para TCP/IP utiliza-se sockaddr_in

  struct sockaddr_in my_addr;

  my_addr.sin_family = AF_INET;         // Definindo a família de endereços
  my_addr.sin_port = htons(3333);       // Definindo número da porta através da função htons (converte e ordena o endereço para um ordenamento em byte significativo)
  my_addr.sin_addr.s_addr = INADDR_ANY; // Preenche o endereço IP com endereço local
  bzero(&(my_addr.sin_zero), 8);        // Preenche com 0 os bytes não utilizados da estrutura

  // Realizar associação do socket com a porta
  if (bind(iSock, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
  {
    perror("Bind: ");
    exit(1);
  }

  // Habilitar conexões
  if (listen(iSock, 10) < 0)
  {
    perror("Listen: ");
    exit(1);
  }

  // Aceitar conexões
  // 1 significa true, logo, irá aceitar conexões o tempo todo
  while (1)
  {
    int iFd;                        // Receberá a conexão aceita
    struct sockaddr_in client_addr; // Estrutura com as informações do cliente
    socklen_t sin_size;             // Tamahno do endereço
    char szMensagem[100];
    sin_size = sizeof(struct sockaddr_in);

    if ((iFd = accept(iSock, (struct sockaddr *)&client_addr, &sin_size)) < 0)
    {
      perror("Accept: ");
      exit(1);
    }

    printf("\nServidor recebeu conexão de %s", inet_ntoa(client_addr.sin_addr));

    memset(szMensagem, 0, sizeof(szMensagem));
    strcpy(szMensagem, "Ola cliente\n");

    // Enviando mensagem
    if(send(iFd, szMensagem, strlen(szMensagem), 0) < 0){
      perror("Send: ");
      exit(1);
    }
    
    // Fechando socket
    close(iFd);
  }
}