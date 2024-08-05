/*
  Compilar
    cc client.c -o client
  Iniciar cliente
    ./client localhost
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

// Definindo constantes de tamanho e a porta utilizada pelo servidor
#define MAXDATASIZE (1024 * 5)
#define SERVPORT 3333

void *connection_handler(void *);

int main(int argc, char *argv[])
{
	int socket_desc, client_sock, c;
	struct sockaddr_in server, client;

	// Criando socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
	{
		perror("Não foi possível criar o socket");
	}
	puts("Socket criado com sucesso!");

	// Criando ponto de conexão
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(SERVPORT);

	// Associando socket à porta local do SO
	if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("Falha ao realizar bind");
		return 1;
	}
	puts("Bind realizado com sucesso! ");

	// Habilitando solicitações de conexão
	listen(socket_desc, 3);
	puts("Aguardando conexões de entrada...");

	c = sizeof(struct sockaddr_in);
	pthread_t thread_id;

	// Aceitando conexões com os clientes
	while ((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c)))
	{
		puts("Conexão aceita");

		// Criando thread única para cada cliente
		if (pthread_create(&thread_id, NULL, connection_handler, (void *)&client_sock) < 0)
		{
			perror("cNão foi possível criar a thread");
			return 1;
		}

		puts("Manipulando atributos");
	}

	// Verificando se houve falha na conexão com o cliente
	if (client_sock < 0)
	{
		perror("Falha ao aceitar conexão");
		return 1;
	}

	return 0;
}

// Função responsável por controlar individualmente a conexão com cada cliente
void *connection_handler(void *socket_desc)
{
	// Pegando o descritor do socket
	int sock = *(int *)socket_desc;
	int read_size;
	char *message, file_buffer[MAXDATASIZE], filename[256];
	FILE *file;
	int bytes_read;

	// Recebendo mensagem do cliente
	while ((read_size = recv(sock, filename, MAXDATASIZE, 0)) > 0)
	{
		// Removendo nova linha, se existir
		filename[strcspn(filename, "\n")] = 0;

		// Abrir o arquivo para leitura
		file = fopen(filename, "rb");

		if (file == NULL)
		{
			// Enviando código de erro para o cliente em caso de falha
			int error_code = -404; // Código de erro para "arquivo não encontrado"
			send(sock, &error_code, sizeof(error_code), 0);

			// Encerrando conexão
			close(sock);
			return NULL;
		}

		// Lendo bytes do arquivo
		while ((bytes_read = fread(file_buffer, 1, MAXDATASIZE, file)) > 0)
		{
			// Enviando bytes do arquivo
			if (send(sock, file_buffer, bytes_read, 0) < 0)
			{
				perror("Erro ao enviar o arquivo!");
				// Fechando arquivo e conexão
				fclose(file);
				close(sock);
				return NULL;
			}
		}
		// Fechando arquivo e saindo do loop
		fclose(file);
		break;
	}

	// Verificando se o cliente se desconectou ou se houve falha no recebimento
	if (read_size == 0)
	{
		puts("Cliente desconectado!");
		fflush(stdout);
	}
	else if (read_size == -1)
	{
		perror("Falha ao receber mensagem do cliente");
	}

	// Fechando conexão
	close(sock);
	return 0;
}
