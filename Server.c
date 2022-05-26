#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

struct user {
	int socketNumber;
	char ip[INET_ADDRSTRLEN];
};

int User[50];
int userCounter = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER; // Cria variavel responsavel pelos locks

void *MonitorGenerateReport(void *str); // Gera o arquivo do relatorio (Monitor)
void sendToUsers(char *text, int userSender); // Responsavel pelo envio das mensagem aos usuarios
void *receive_message(void *socketN); // Responsavel por receber a mensagem de um cliente

int main(int argc,char *argv[])
{
	pthread_t callMonitor;
	int port;
	int userSocket;
	int senderSocket;
	char text[1000];
	char textArq[500];
	char ip[INET_ADDRSTRLEN];
	
	struct sockaddr_in senAddr, recAddr;
	struct user usuario;	
	socklen_t recAddrSize;		

	printf("Digite a porta que sera escolhida?\n");
	scanf("%d", &port);

	userSocket = socket(AF_INET,SOCK_STREAM, 0);
	memset(senAddr.sin_zero,'\0',sizeof(senAddr.sin_zero));
	senAddr.sin_family = AF_INET;
	senAddr.sin_port = htons(port);
	senAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	recAddrSize = sizeof(recAddr);

	if(bind(userSocket,(struct sockaddr *)&senAddr,sizeof(senAddr)) != 0) { // Bind do servidor
		perror("Erro na tentativa de bind");
		exit(1);
	}

	printf("Bind na porta %d realizado com sucesso!\n", port);

	if(listen(userSocket, 5) != 0) { // Processo de listening do servidor
		perror("Erro durante o processo de listening");
		exit(1);
	}

	FILE *arq;

	arq = fopen("Report.txt", "w");

	fclose(arq);

	while(1) {
		if((senderSocket = accept(userSocket,(struct sockaddr *)&recAddr,&recAddrSize)) < 0) {
			perror("Erro no processo de accept");
			exit(1);
		}

		// Inicio da secao criica
		pthread_mutex_lock(&m);

		inet_ntop(AF_INET, (struct sockaddr *)&recAddr, ip, INET_ADDRSTRLEN);
		printf("Novo cliente com IP %s se conectou\n",ip);

		strcpy(textArq, "Novo cliente com IP ");
        	strcat(textArq, ip);
        	strcat(textArq, " se conectou\n");

		pthread_create(&callMonitor, NULL, MonitorGenerateReport, &textArq);

		usuario.socketNumber = senderSocket;
		strcpy(usuario.ip,ip);
		User[userCounter] = senderSocket;
		userCounter++;

		pthread_t thread;
		pthread_create(&thread,NULL,receive_message,&usuario);

		pthread_mutex_unlock(&m);
		// Fim da secao critica

	}
	return 0;
}

void *MonitorGenerateReport(void *str){

	char text[1000];

	strcpy(text, (char*)str);

	// Inicio da secao criica
	pthread_mutex_lock(&m);	

	FILE *arq;

	arq = fopen("Report.txt", "a");

	fprintf(arq, "%s", text);

	pthread_mutex_unlock(&m);
	// Fim da secao critica

	fclose(arq);
}

void sendToUsers(char *text, int userSender)
{
	int i;
	int confereEnvio=0;

	// Inicio da secao criica
	pthread_mutex_lock(&m);

	for(i = 0; i < userCounter; i++) {

		if(User[i] != userSender) {
			confereEnvio = send(User[i], text, strlen(text), 0);

			if(confereEnvio < 0) {
				perror("Falha no envio da mensagem!\n");
				continue;
			}
		}
	}
	pthread_mutex_unlock(&m);
	// Fim da secao critica
}

void *receive_message(void *socketN)
{
	pthread_t callMonitor;
	struct user usuario = *((struct user *)socketN);
	char text[1000];
	char textArq[500];
	int textSize;
	int i=0;
	int counter=0;

	while((textSize = recv(usuario.socketNumber, text, 1000, 0)) > 0) {		

		text[strlen(text)] = '\0';
		if(counter>0) sendToUsers(text,usuario.socketNumber);

		if(counter>0){
            		printf("MENSAGEM de %s %s\n", usuario.ip, text);

            		strcpy(textArq, "MENSAGEM DE ");
            		strcat(textArq, usuario.ip);
            		strcat(textArq, " ");
            		strcat(textArq, text);
			strcat(textArq, "\n");
            
            		pthread_create(&callMonitor, NULL, MonitorGenerateReport, &textArq);
		} 	

		counter++;
		memset(text,'\0',sizeof(text));
	}

	// Inicio da secao criica

	pthread_mutex_lock(&m);	

	printf("Cliente com o IP %s se desconectou\n", usuario.ip);

	strcpy(textArq, "Cliente com o IP ");
        strcat(textArq, usuario.ip);
        strcat(textArq, " se desconectou\n");

	pthread_create(&callMonitor, NULL, MonitorGenerateReport, &textArq);

	for(i = 0; i < userCounter; i++) {
		if(User[i] == usuario.socketNumber) {
			int aux;
			aux = i;
			while(aux < userCounter-1) {
				User[aux] = User[aux+1];
				aux++;
			}
		}
	}
	userCounter--;
	pthread_mutex_unlock(&m);
	// Fim da secao critica
}
