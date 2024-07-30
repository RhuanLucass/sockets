#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>

#define SERVPORT 3333
#define MAXDATASIZE (1024 * 5)

int main(int argc, char *argv[])
{
    int sockfd, recvbytes;
    char buf[MAXDATASIZE], filepath[MAXDATASIZE + 50], server_reply[MAXDATASIZE];
    struct hostent *host;
    struct sockaddr_in serv_addr;
    FILE *file;

    if (argc < 2)
    {
        fprintf(stderr, "Por favor, informe o endereço do servidor!\n");
        exit(1);
    }

    if ((host = gethostbyname(argv[1])) == NULL)
    {
        perror("gethostbyname:");
        exit(1);
    }

    while (1)
    {
        printf("Digite o nome do arquivo para buscar ou 'fim' para encerrar: ");
        fgets(buf, MAXDATASIZE, stdin);
        buf[strcspn(buf, "\n")] = 0; // Remove a nova linha

        if (strcmp(buf, "fim") == 0)
        {
            printf("Encerrando cliente...\n");
            exit(0);
        }

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("socket:");
            exit(1);
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(SERVPORT);
        serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
        bzero(&(serv_addr.sin_zero), 8);

        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1)
        {
            perror("connect:");
            exit(1);
        }

        // Enviar nome do arquivo para o servidor
        if (send(sockfd, buf, strlen(buf), 0) == -1)
        {
            perror("send:");
            close(sockfd);
            continue;
        }

        // Receber dados do servidor
        recvbytes = recv(sockfd, server_reply, sizeof(server_reply), 0);
        if (recvbytes > 0)
        {
            // Verificar se recebemos um código de erro
            if (recvbytes == sizeof(int))
            {
                int *error_code = (int *)server_reply;

                if (*error_code < 0)
                {
                    if (*error_code == -404)
                    {
                        printf("Erro: Arquivo não encontrado no servidor.\n");
                    }
                    else
                    {
                        printf("Erro desconhecido do servidor: %d\n", *error_code);
                    }
                    close(sockfd);
                    continue;
                }
            }

            // Criar diretório 'arquivos' se não existir
            struct stat st = {0};
            if (stat("arquivos", &st) == -1)
            {
                mkdir("arquivos", 0700);
            }

            // Definir caminho do arquivo local
            snprintf(filepath, sizeof(filepath), "arquivos/%s", buf);

            // Abrir arquivo local para escrita
            file = fopen(filepath, "wb");
            if (file == NULL)
            {
                perror("Não foi possível criar o arquivo local!");
                close(sockfd);
                continue;
            }

            // Escrever dados no arquivo
            do
            {
                fwrite(server_reply, 1, recvbytes, file);
                // Receber mais dados, se houver
                recvbytes = recv(sockfd, server_reply, sizeof(server_reply), 0);
            } while (recvbytes > 0);

            if (recvbytes < 0)
            {
                perror("Erro ao receber dados do servidor.");
            }
            else if (recvbytes == 0)
            {
                printf("Arquivo '%s' recebido com sucesso.\n", filepath);
            }

            fclose(file);
        }
        else
        {
            perror("Erro ao receber resposta do servidor.");
        }

        close(sockfd);
    }

    return 0;
}
