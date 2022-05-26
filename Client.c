#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <locale.h>
#include <string.h>

#define MAXSIZE 1000

void * receive_message(void *socketNumber); // Pre-declaracao da funcao "receive_message"

int main(int argc, char *argv[]){
	
	setlocale(LC_ALL, "Portuguese");
	
	struct sockaddr_in sAddr; 
	memset(sAddr.sin_zero,'\0',sizeof(sAddr.sin_zero));
	
	int userSocket = socket(AF_INET, SOCK_STREAM, 0);
	int port;
	int tSize=0;
	char text[MAXSIZE];
	char username[50];
	char theip[50];
	char fullContent[1051];	
	int Length=0;
	char ip[INET_ADDRSTRLEN];
	
	printf("Qual seu nome de usuario? (Max: 50 caracteres) (Nao utilize espacos!)\n");
	scanf("%s", username);

	printf("A qual IP deseja se conectar?\n");
	scanf("%s", theip);
	
	printf("A qual porta deseja se conectar?\n");
	scanf("%d", &port);
	
	sAddr.sin_family = AF_INET;
	sAddr.sin_port = htons(port);
	sAddr.sin_addr.s_addr = inet_addr(theip);
	
	if(connect(userSocket,(struct sockaddr *)&sAddr,sizeof(sAddr)) == -1){ 
		printf("Erro durante a conexao, por favor tente novamente!\n");
		exit(1);
	}
	
	inet_ntop(AF_INET, (struct sockaddr *)&sAddr, ip, INET_ADDRSTRLEN);
	
	printf("CONECTADO ao servidor com endereco IP %s na porta %d\n", ip, port);
	
	pthread_t thread;
	pthread_create(&thread, NULL, receive_message, (void *) &userSocket);
	
	while(fgets(text, MAXSIZE, stdin) > 0) {
		text[strlen(text)-1] = '\0';
		if(strcmp(text, "SAIR")==0 || strcmp(text, "sair")==0){	
			printf("Voce foi desconectado com sucesso!\n");
			close(userSocket);	
			exit(1);		
		}
		
		strcpy(fullContent, username);
		strcat(fullContent, ": ");
		strcat(fullContent, text);

		tSize = strlen(text);
		
		Length = write(userSocket, fullContent, strlen(fullContent));
		
		if(Length < 0) {
			perror("A mensagem nao pode ser enviada\n");
			exit(1);
		}

		if(tSize>1) printf("%s\n", fullContent);
		
		memset(text,'\0', sizeof(text));
		memset(fullContent, '\0', sizeof(fullContent));
	}
	
	pthread_join(thread, NULL);
	close(userSocket);	
}

void * receive_message(void *socketNumber){
	
	int senderSocket = *((int *) socketNumber); // Criacao da variavel que contera o valor do socket do client que envia a mensagem
	
	while(1){
		
		int textLength=0; 
		char text[MAXSIZE]; // Criacao do campo mensagem, que abrangera até 1000 caracteres
		textLength = recv(senderSocket, text, MAXSIZE, 0); // Recebimento da mensagem por meio dos parametros passados em recv
		text[textLength] = '\0'; // Insercao do null character no final do texto
		printf("%s\n",text); // Print da mensagem
		
	}	
}
