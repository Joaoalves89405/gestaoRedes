// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <ctype.h>
	
#define PORT	 55555
#define MAXLINE 1024

uint8_t pacote[128];


//tpm 0 snmpget
//tpm 1 snmpgetnext
//tpm 2 snmpset
//tpm 3 snmpbulk
void criar_pacote(char * comando){
	memset(pacote, 0, 128);
	char* token = NULL;
	int i = 1;
	char snmp[16];
	char version[8];
	char comstring[16];
	char oid[64];
	for(token = strtok(comando," ");token != NULL; token = strtok(NULL, " ")){
		if (i == 1){
			memset(pacote, 0,32);
			strcpy(snmp, token);
			int j = 0;
			while(snmp[j]){
				snmp[j] = tolower(snmp[j]);
				j++;
			}
			if(strcmp(snmp, "snmpget")==0){
				pacote[0] = 0;
			}
			else if(strcmp(snmp, "snmpgetnext")==0){
				pacote[0] = 1;
			}
			else if(strcmp(snmp, "snmpset")==0){
				pacote[0] = 2;
			}
			else if(strcmp(snmp, "snmpgetbulk")==0){
				pacote[0] = 3;
			}
		}
		//se for versao correta escrevo um 1 senao um 2
		if (i == 2){
			strcpy(version, token);
			int j = 0;
			while(version[j]){
				version[j] = tolower(version[j]);
				j++;
			}
			if (strcmp(version, "-v2c")==0){
				pacote[1] = 1;
			}else{
				pacote[1] = 2;
			}
		}
		if (i == 3){
			strcpy(comstring, token);
			int j = 0;
			while(comstring[j]){
				comstring[j] = tolower(comstring[j]);
				pacote[j+3] = comstring[j];
				j++;
			}
			//bytes community string 
			pacote[2] = j;
		}
		if (i == 4){
			strcpy(oid, token);
			int j = 0;
			int a = 0;
			while(oid[j]){
				pacote[pacote[2] + 3 + j + 1] = oid[j];
				j++;
			}
			//bytes de oid 
			pacote[pacote[2] + 3 ] = j;
		}
		if(i == 5){ 
			if (pacote[0] == 2){
				char value[32];
				strcpy(value, token);
				int j = 0;
				while(value[j]){
					pacote[(pacote[2]+3)+pacote[pacote[2]+3]+2+j] = value[j];
					j++;
				}
				pacote[(pacote[2]+3)+pacote[pacote[2]+3]+1] = j;
			}else if(pacote[0] == 3){
				char oid2[32];
				strcpy(oid2, token);
				int j = 0;
				int a = 0;
				while(oid2[j]){
					pacote[(pacote[2]+3)+pacote[pacote[2]+3] + j +2] = oid2[j];
					j++;
				}
				//bytes de oid 
				pacote[(pacote[2]+3)+pacote[pacote[2]+3] +1 ] = j;
			}
		}
		i++;
	}
	
}

void Menu(){
	printf("-----SNMPv2c-----\n");
	printf("Insira 0 para sair\n");
	printf("Insira 1 para proceder ao comando SNMP\n");
	printf("Insira 2 para help!\n");
}

// Driver code
int main() {
	int sockfd;
	uint8_t buffer[MAXLINE];
	struct sockaddr_in	 servaddr;
	
	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	
	memset(&servaddr, 0, sizeof(servaddr));
		
	// Filling server information
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = INADDR_ANY;
		
	int n, len;
	int flag = 1;
	int opcao = 0;
	while(flag){
		Menu();
		printf("Opção- ");
		scanf("%d", &opcao);
		fflush(stdin);
		getchar();
		if(opcao == 0){
			flag = 0;
		
		}else if(opcao == 2){
			printf("snmpget -v2c public [OID]\n");
			printf("snmpgetnext -v2c public [OID]\n");
			printf("snmpset -v2c public [OID] [VALOR]\n");
			printf("snmpgetbulk -v2c public [OID] [OID]\n" );
		}
		else if(opcao == 1){
			char comando[64];
			char c;
			int a = 0;
			printf("Insira o comando SNMP:\n");
			fflush(stdin);
			while((c=getchar()) != '\n')
				comando[a++] = c;
			comando[a] = '\0';
			criar_pacote(comando);
			sendto(sockfd, pacote, 64,
				MSG_CONFIRM, (const struct sockaddr *) &servaddr,
					sizeof(servaddr));
			
			memset(buffer, 0, MAXLINE);
			n = recvfrom(sockfd, buffer, 128,
						MSG_WAITALL, (struct sockaddr *) &servaddr,
						&len);
			if(buffer[0] == 0){
				printf("MENSAGEM DE ERRO:\n");
				int s = buffer[1];
				for(int i = 0; i<s;i++){
					printf("%c", buffer[i+2]);
				}
				printf("\n");
			}
			else{
				if (pacote[0] == 3){
					int qnt = buffer[1];
					int size = 0;
					int pos = 2;
					for(int i = 0; i< qnt ; i++){
						printf("OID: ");
						size = buffer[pos];		
						for(int j = 0; j<size; j++){
							printf("%c", buffer[pos+1+j]);
						}
						pos = pos + size + 1;
						size = buffer[pos];
						printf("  ");
						for(int j = 0; j< size; j++){
							printf("%c", buffer[pos+1+j]);
						}
						printf("\n");
						pos = pos + size + 1;
					}

				}else{
					printf("RESPOSTA AGENT:\n");
					printf("OID: ");
					int l = buffer[1];
					for(int j = 0; j< l; j++){
						printf("%c", buffer[2+j]);
					}
					int l_v = buffer[buffer[1] + 2 ];
					printf("  ");
					for (int i = 0; i<l_v; i++){
						printf("%c", buffer[buffer[1]+3+i]);
					}
					printf("\n");
				}
				
			}
			buffer[n] = '\0';
			//printf("Server : %s\n", buffer);
		}
		
	}
	close(sockfd);
	return 0;
}
