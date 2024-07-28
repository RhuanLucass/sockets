/*
 * =====================================================================================
 *
 *       Filename:  client.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2013年08月19日 17时19分33秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <error.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>

#define SEVPORT 3333
#define MAXDATASIZE (1024 * 5)

// #define TIME_DIFF(t1, t2) (((t1).tv_sec - (t2).tv_sec) * 1000 + ((t1).tv_usec - (t2).tv_usec) / 1000)

int main(int argc, char *argv[])
{
	int sockfd, sendbytes, recvbytes;
	char buf[MAXDATASIZE], filepath[MAXDATASIZE], server_reply[MAXDATASIZE];
	struct hostent *host;
	struct sockaddr_in serv_addr;
	FILE *file;
	// struct timeval timestamp;
	// struct timeval timestamp_end;

	while (1)
	{
		printf("Digite um nome de arquivo que deseja copiar ou 'fim' para encerrar: ");
		fgets(buf, MAXDATASIZE, stdin);
		buf[strcspn(buf, "\n")] = 0; // Remove o caractere de nova linha

		if (strcmp(buf, "fim") == 0)
		{
			printf("Cliente encerrando...\n");
			exit(1);
			break;
		}

		// Abre uma conexão com o servidor usando netcat
		char command[MAXDATASIZE + 50];
		snprintf(command, sizeof(command), "echo '%s' | nc localhost 8888", buf);
		system(command);
	}

	// if(argc < 2){
	// 	fprintf(stderr,"Please enter the server's hostname!\n");
	// 	exit(1);
	// }

	// if((host=gethostbyname(argv[1])) == NULL){
	// 	perror("gethostbyname:");
	// 	exit(1);
	// }

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket:");
		exit(1);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SEVPORT);
	serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(serv_addr.sin_zero), 8);

	if (connect(sockfd, (struct sockaddr *)&serv_addr,
							sizeof(struct sockaddr)) == -1)
	{
		perror("connect:");
		exit(1);
	}
	memset(buf, 0x15, sizeof(buf));

	if ((sendbytes = send(sockfd, buf, sizeof(buf), 0)) == -1)
	{
		perror("send:");
		exit(1);
	}

	// Criar diretório arquivos se não existir
	struct stat st = {0};
	if (stat("arquivos", &st) == -1)
	{
		mkdir("arquivos", 0700);
	}

	// Definir caminho do arquivo
	snprintf(filepath, sizeof(filepath), "arquivos/%s", buf);

	// Abrir arquivo local para escrita
	file = fopen(filepath, "wb");
	if (file == NULL)
	{
		perror("Não foi possível criar o arquivo local!");
		close(sockfd);
		return 1;
	}

	// Receber dados do servidor
	while ((recvbytes = recv(sockfd, server_reply, sizeof(server_reply), 0)) > 0)
	{
		fwrite(server_reply, 1, recvbytes, file); // grava no arquivo local
	}

	if (recvbytes < 0)
	{
		printf("Falha ao receber dados do servidor\n");
		fclose(file);
		close(sockfd);
		return 1;
	}

	// printf("Arquivo recebido e salvo em '%s'\n", filepath);

	close(sockfd);
	return 0;
}
