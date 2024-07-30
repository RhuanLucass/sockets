/*
		C socket server example, handles multiple clients using threads
		Compile
		gcc server.c -lpthread -o server
*/

#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h> //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>		 //write
#include <pthread.h>	 //for threading , link with lpthread

#define MAXDATASIZE (1024 * 5)
#define SERVPORT 3333

// the thread function
void *connection_handler(void *);

int main(int argc, char *argv[])
{
	int socket_desc, client_sock, c;
	struct sockaddr_in server, client;

	// Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");

	// Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(SERVPORT);

	// Bind
	if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		// print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");

	// Listen
	listen(socket_desc, 3);

	// Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	pthread_t thread_id;

	while ((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c)))
	{
		puts("Connection accepted");

		if (pthread_create(&thread_id, NULL, connection_handler, (void *)&client_sock) < 0)
		{
			perror("could not create thread");
			return 1;
		}

		// Now join the thread , so that we dont terminate before the thread
		// pthread_join( thread_id , NULL);
		puts("Handler assigned");
	}

	if (client_sock < 0)
	{
		perror("accept failed");
		return 1;
	}

	return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
	// Get the socket descriptor
	int sock = *(int *)socket_desc;
	int read_size;
	char *message, file_buffer[MAXDATASIZE], filename[256];
	FILE *file;
	int bytes_read;

	// Receive a message from client
	while ((read_size = recv(sock, filename, MAXDATASIZE, 0)) > 0)
	{
		// Removendo nova linha, se existir
		filename[strcspn(filename, "\n")] = 0;

		// Abrir o arquivo para leitura
		file = fopen(filename, "rb");

		if (file == NULL)
		{
			// Send the message back to client
			int error_code = -404; // Código de erro para "arquivo não encontrado"
			send(sock, &error_code, sizeof(error_code), 0);
			close(sock);
			return NULL;
		}

		// Enviando arquivo
		while ((bytes_read = fread(file_buffer, 1, MAXDATASIZE, file)) > 0)
		{
			if (send(sock, file_buffer, bytes_read, 0) < 0)
			{
				perror("Erro ao enviar o arquivo!");
				fclose(file);
				close(sock);
				return NULL;
			}
		}
		fclose(file);
		break;
	}

	// clear the message buffer
	// memset(filename, 0, MAXDATASIZE);
	// }

	if (read_size == 0)
	{
		puts("Client disconnected");
		fflush(stdout);
	}
	else if (read_size == -1)
	{
		perror("recv failed");
	}

	close(sock);
	return 0;
}
