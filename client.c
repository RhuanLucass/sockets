/*
  Compilar
    cc server.c -o server -lpthread
  Iniciar servidor
    ./server
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>

// Definindo constantes de tamanho e a porta utilizada pelo servidor
#define SERVPORT 3333
#define MAXDATASIZE (1024 * 5)

int main(int argc, char *argv[])
{
  int sockfd, recvbytes;
  char buf[MAXDATASIZE], filepath[MAXDATASIZE + 50], server_reply[MAXDATASIZE];
  struct hostent *host;
  struct sockaddr_in serv_addr;
  FILE *file;

  // Verificando se o endereço do servidor foi passado como argumento ao rodar o arquivo
  if (argc < 2)
  {
    fprintf(stderr, "Por favor, informe o endereço do servidor!\n");
    exit(1);
  }

  // Verificando se o endereço existe
  if ((host = gethostbyname(argv[1])) == NULL)
  {
    perror("Erro ao localizar endereço");
    exit(1);
  }

  // Loop para o cliente entrar com o nome do arquivo ou sair do programa
  while (1)
  {
    printf("Digite o nome do arquivo para buscar ou 'fim' para encerrar: ");
    // Pegando o conteúdo digitado
    fgets(buf, MAXDATASIZE, stdin);
    buf[strcspn(buf, "\n")] = 0; // Removendo a nova linha, se existir

    // Verificando se foi digitado "fim", se sim, encerra o programa
    if (strcmp(buf, "fim") == 0)
    {
      printf("Saindo...\n");
      exit(0);
    }

    // Criando socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror("Erro ao criar socket");
      exit(1);
    }

    // Criando ponto de conexão
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVPORT);
    serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    // Realizando conexão com o servidor
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1)
    {
      perror("Erro ao realizar conexão");
      exit(1);
    }

    // Enviando nome do arquivo para o servidor
    if (send(sockfd, buf, strlen(buf), 0) == -1)
    {
      perror("Erro ao enviar mensagem para o servidor");
      // Encerrando socket
      close(sockfd);
      continue;
    }

    // Recebendo dados do servidor
    recvbytes = recv(sockfd, server_reply, sizeof(server_reply), 0);
    if (recvbytes > 0)
    {
      // Verificando se recebeu um código de erro
      if (recvbytes == sizeof(int))
      {
        int *error_code = (int *)server_reply;

        if (*error_code < 0)
        {
          if (*error_code == -404)
          {
            puts("Erro: Arquivo não encontrado no servidor!");
          }
          else
          {
            printf("Erro desconhecido do servidor: %d\n", *error_code);
          }
          // Fechadno conexão
          close(sockfd);
          continue;
        }
      }

      // Criando diretório 'arquivos' se não existir
      struct stat st = {0};
      if (stat("arquivos", &st) == -1)
      {
        mkdir("arquivos", 0700);
      }

      // Definindo caminho do arquivo local
      snprintf(filepath, sizeof(filepath), "arquivos/%s", buf);

      // Abrindo arquivo local para escrita
      file = fopen(filepath, "wb");

      // Verificando se o arquivo foi criado com sucesso
      if (file == NULL)
      {
        perror("Não foi possível criar o arquivo local");
        // Encerrando conexão
        close(sockfd);
        continue;
      }

      // Escrevendo dados no arquivo
      do
      {
        fwrite(server_reply, 1, recvbytes, file);
        // Receber mais dados, se houver
        recvbytes = recv(sockfd, server_reply, sizeof(server_reply), 0);
      } while (recvbytes > 0);

      // Verificando se houve um erro ao receber dados do arquivo
      if (recvbytes < 0)
      {
        perror("Erro ao receber dados do servidor");
      }
      else if (recvbytes == 0)
      {
        printf("Arquivo salvo no endereço '%s'.\n\n", filepath);
      }

      // Fechando arquivo
      fclose(file);
    }
    else
    {
      perror("Erro ao receber resposta do servidor.");
    }

    // Encerrando conexão
    close(sockfd);
  }

  return 0;
}
