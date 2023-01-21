// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 8080
#define MAXLINE 1024

FILE *file;

int port = 0;
char CommunityString[32];
char oid[16];
uint8_t datagram[128];

void read_mib_conf()
{
	char buff[64];
	file = fopen("mib.txt", "r");
	fscanf(file, "%d %s %s", &port, CommunityString, oid);
	printf("%d %s %s\n", port, CommunityString, oid);
	fclose(file);
}

void snmpget(uint8_t buffer[64])
{
	memset(datagram, 0, 128);
	char oid1[16];
	int t_oid = buffer[(buffer[2]) + 3];
	for (int i = 0; i < t_oid; i++)
	{
		oid1[i] = buffer[(buffer[2]) + 3 + 1 + i];
	}
	char fbuff[64];
	int j = 1;
	int found = 0;
	char value[32];
	file = fopen("mib.txt", "r");
	while (fgets(fbuff, sizeof(fbuff), file) != NULL && found == 0)
	{
		char *token = NULL;
		j = 1;
		for (token = strtok(fbuff, " "); token != NULL; token = strtok(NULL, " "))
		{
			if (j == 1)
			{
				if (strcmp(oid1, token) == 0)
				{
					found = 1;
				}
			}
			if (j == 4 && found)
			{
				// valor encontrado criar datagram e enviar
				strcpy(value, token);
				datagram[0] = 1;
				datagram[1] = t_oid;
				for (int i = 0; i < t_oid; i++)
				{
					datagram[i + 2] = oid1[i];
				}
				datagram[t_oid + 2] = strlen(value);
				for (int i = 0; i < strlen(value); i++)
				{
					datagram[t_oid + 3 + i] = value[i];
				}
				printf("Sucesso\n");
			}
			j++;
		}
	}
	fclose(file);
	// value not found
	if (found == 0)
	{
		printf(" Insucesso\n");
		datagram[0] = 0;
		char erro[32] = "OID não encontrado";
		int size = strlen(erro);
		datagram[1] = size;
		sprintf(datagram + 2, "%s", erro);
	}
}

void snmpgetnext(uint8_t buffer[64])
{
	memset(datagram, 0, 128);
	char oid1[16];
	int t_oid = buffer[(buffer[2]) + 3];
	for (int i = 0; i < t_oid; i++)
	{
		oid1[i] = buffer[(buffer[2]) + 3 + 1 + i];
	}
	char fbuff[64];
	int j = 1;
	int found = 0;
	int next = 0;
	char valor[32];
	char oid_next[32];
	memset(valor, 0, 32);
	file = fopen("mib.txt", "r");
	while (fgets(fbuff, sizeof(fbuff), file) != NULL && next == 0)
	{
		char *token = NULL;
		j = 1;
		if (found == 1)
		{
			next = 1;
		}
		for (token = strtok(fbuff, " "); token != NULL; token = strtok(NULL, " "))
		{
			if (j == 1)
			{
				if (strcmp(oid1, token) == 0)
				{
					found = 1;
				}
			}
			if (j == 1 && next)
			{
				strcpy(oid_next, token);
			}
			if (j == 4 && next)
			{
				// valor encontrado criar
				strcpy(valor, token);
				datagram[0] = 1;
				datagram[1] = strlen(oid_next);
				for (int i = 0; i < t_oid; i++)
				{
					datagram[i + 2] = oid_next[i];
				}
				datagram[strlen(oid_next) + 2] = strlen(valor);
				for (int i = 0; i < strlen(valor); i++)
				{
					datagram[t_oid + 3 + i] = valor[i];
				}
				printf("Sucesso\n");
			}
			j++;
		}
	}

	fclose(file);
	if (strlen(valor) <= 1)
	{
		// nao existia next oid nesse grupo!
		// printf("NAO HAVIA VALOR ABAIXO DESTE\n");
		printf("Insucesso\n");
		datagram[0] = 0;
		char erro[32] = "Erro- OID seguinte inexistente";
		int size = strlen(erro);
		datagram[1] = size;
		sprintf(datagram + 2, "%s", erro);
	}
	if (found == 0)
	{
		// valor nao encontrado criar
		printf("Insucesso\n");
		datagram[0] = 0;
		char erro[32] = "OID não encontrado";
		int size = strlen(erro);
		datagram[1] = size;
		sprintf(datagram + 2, "%s", erro);
	}
}

void snmpset(uint8_t buffer[64])
{
	memset(datagram, 0, 128);
	FILE *temp;
	char oid1[16];
	int t_oid = buffer[(buffer[2]) + 3];
	for (int i = 0; i < t_oid; i++)
	{
		oid1[i] = buffer[(buffer[2]) + 3 + 1 + i];
	}
	int s_v = buffer[buffer[2] + 3 + t_oid + 1];
	char val[32];
	memset(val, 0, 32);
	for (int i = 0; i < s_v; i++)
	{
		val[i] = buffer[buffer[2] + 3 + t_oid + 1 + i + 1];
	}
	int count = 0;
	char int_ou_nao = 0;
	for (int i = 0; i < s_v; i++)
	{
		if (buffer[buffer[2] + 3 + t_oid + 1 + i + 1] <= 57 && buffer[buffer[2] + 3 + t_oid + 1 + i + 1] >= 48)
		{
			count++;
		}
	}
	if (count == s_v)
	{
		int_ou_nao = 1;
	}
	char fbuff[256];
	int j = 1;
	int found = 0;
	char valor[32];
	file = fopen("mib.txt", "r");
	temp = fopen("temp.txt", "w");
	int variavel = 0;
	char var[32];
	int read_write = 0;
	char permi[8];
	char aux[256];
	int flag = 0;
	while (fgets(fbuff, sizeof(fbuff), file) != NULL)
	{
		memset(aux, 0, 256);
		strcpy(aux, fbuff);
		char *token = NULL;
		j = 1;
		for (token = strtok(fbuff, " "); token != NULL; token = strtok(NULL, " "))
		{
			if (j == 1)
			{
				if (strcmp(oid1, token) == 0)
				{
					found = 1;
				}
			}
			if (j == 2 && found == 1 && flag == 0)
			{
				if ((strcmp(token, "Integer") == 0 && int_ou_nao == 1) || (strcmp(token, "Integer") != 0 && int_ou_nao != 1))
				{
					variavel = 1;
					strcpy(var, token);
				}
				else
				{
					flag = 1;
				}
			}
			if (j == 3 && variavel == 1 && flag == 0)
			{
				if (strcmp(token, "rw") == 0)
				{
					read_write = 1;
					strcpy(permi, token);
				}
				else
				{
					flag = 1;
				}
			}
			if (j == 4 && read_write == 1 && flag == 0)
			{
				// valor encontrado criar datagram e enviar
				flag = 1;
				strcpy(valor, token);
				datagram[0] = 1;
				datagram[1] = t_oid;
				for (int i = 0; i < t_oid; i++)
				{
					datagram[i + 2] = oid1[i];
				}
				datagram[t_oid + 2] = strlen(val);
				for (int i = 0; i < strlen(val); i++)
				{
					datagram[t_oid + 3 + i] = val[i];
				}
				printf("Sucess\n");
				sprintf(aux, "%s %s %s %s\n", oid1, var, permi, val);
			}
			j++;
		}
		fwrite(aux, strlen(aux), 1, temp);
	}
	fclose(file);
	fclose(temp);
	if (found)
	{
		char buff[128];
		file = fopen("mib.txt", "w");
		temp = fopen("temp.txt", "r");
		while (fgets(buff, sizeof(buff), temp) != NULL)
		{
			fwrite(buff, strlen(buff), 1, file);
		}
		fclose(file);
		fclose(temp);
	}
	else if (found == 0)
	{
		// nao encontrado
		printf("Fail\n");
		// valor nao encontrado criar datagram e enviar
		datagram[0] = 0;
		char erro[32] = "OID not found";
		int size = strlen(erro);
		datagram[1] = size;
		sprintf(datagram + 2, "%s", erro);
	}
	if (variavel == 0)
	{
		// variavel incompativeis
		printf("Fail\n");
		// valor nao encontrado criar datagram e enviar
		datagram[0] = 0;
		char erro[32] = "Unmatched variables";
		int size = strlen(erro);
		datagram[1] = size;
		sprintf(datagram + 2, "%s", erro);
	}
	else if (read_write == 0)
	{
		// sem permissao para escrever
		printf("Fail\n");
		// valor nao encontrado criar datagram e enviar
		datagram[0] = 0;
		char erro[32] = "No permission to write";
		int size = strlen(erro);
		datagram[1] = size;
		sprintf(datagram + 2, "%s", erro);
	}
}

void snmpgetbulk(uint8_t buffer[64])
{
	memset(datagram, 0, 128);
	char oid1[16];
	int t_oid = buffer[(buffer[2]) + 3];
	for (int i = 0; i < t_oid; i++)
	{
		oid1[i] = buffer[(buffer[2]) + 3 + 1 + i];
	}
	char oid2[16];
	int t_oid2 = buffer[(buffer[2] + 3) + buffer[buffer[2] + 3] + 1];
	for (int i = 0; i < t_oid2; i++)
	{
		oid2[i] = buffer[(buffer[2] + 3) + buffer[buffer[2] + 3] + i + 2];
	}
	// printf("%s->%s\n", oid1, oid2);
	char valores[16][32];
	char bulk_oids[16][32];
	char fbuff[64];
	int j = 1;
	int pos_x = 0;
	int found = 0;
	int next = 0;
	char valor[32];
	file = fopen("mib.txt", "r");
	while (fgets(fbuff, sizeof(fbuff), file) != NULL && next == 0)
	{
		char *token = NULL;
		j = 1;
		for (token = strtok(fbuff, " "); token != NULL; token = strtok(NULL, " "))
		{
			if (j == 1)
			{
				if (strcmp(oid1, token) == 0)
				{
					found = 1;
				}
				if (found)
				{
					strcpy(bulk_oids[pos_x], token);
					// printf("%s\n", bulk_oids[pos_x]);
				}
			}
			if (j == 4 && found)
			{
				// valor encontrado criar datagram e enviar
				strcpy(valores[pos_x], token);
				valores[pos_x][strlen(valores[pos_x]) - 1] = '\0';
				// printf("%s\n", valores[pos_x]);
				pos_x++;
			}
			if (strcmp(oid2, token) == 0)
			{
				next = 1;
				printf("Sucesso\n");
			}
			j++;
		}
	}
	fclose(file);
	if (found == 0)
	{
		printf(" Insucesso\n");
		// valor nao encontrado criar datagram e enviar
		datagram[0] = 0;
		char erro[32] = "OID não encontrado";
		int size = strlen(erro);
		datagram[1] = size;
		sprintf(datagram + 2, "%s", erro);
	}
	else
	{
		datagram[0] = 1;
		datagram[1] = pos_x;
		int pos = 0;
		pos = 2;
		for (int i = 0; i < pos_x; i++)
		{
			datagram[pos] = strlen(bulk_oids[i]);
			for (int j = 0; j < strlen(bulk_oids[i]); j++)
			{
				datagram[pos + 1 + j] = bulk_oids[i][j];
			}
			pos = pos + datagram[pos] + 1;
			datagram[pos] = strlen(valores[i]);
			for (int j = 0; j < strlen(valores[i]); j++)
			{
				datagram[pos + 1 + j] = valores[i][j];
			}
			pos = pos + datagram[pos] + 1;
		}
	}
}

int main()
{
	int sockfd, len, n;
	uint8_t buffer[MAXLINE];
	char *hello = "First message from server";
	struct sockaddr_in servaddr, cliaddr;
	read_mib_conf();

	// Creating socket file descriptor
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));

	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(port);

	// Bind the socket with the server address
	if (bind(sockfd, (const struct sockaddr *)&servaddr,
			 sizeof(servaddr)) < 0)
	{
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}
	len = sizeof(cliaddr);

	while (1)
	{
		n = recvfrom(sockfd, buffer, MAXLINE,
					 MSG_WAITALL, (struct sockaddr *)&cliaddr,
					 &len);

		int s_cmst = (int)(buffer[2]);

		char comun_string[16];
		memset(comun_string, 0, 16);
		for (int i = 0; i < s_cmst; i++)
		{
			comun_string[i] = (char)buffer[i + 3];
		}
		if ((buffer[1]) == 1 && strcmp(comun_string, CommunityString) == 0)
		{
			// get
			if (buffer[0] == 0)
			{
				// buscar valor do oid
				printf("SNMPGET\n");
				snmpget(buffer);
			}
			else if ((buffer[0]) == 1)
			{ // getnext
				// buscar valor do seguinte oid
				printf("SNMPGETNEXT\n");
				snmpgetnext(buffer);
			}
			else if ((buffer[0]) == 2)
			{ // set
				// alterar valor caso seja valido para se alterar
				printf("SNMPSET\n");
				snmpset(buffer);
			}
			else if ((buffer[0]) == 3)
			{ // getbulk
				// todos os valores compreendidos nos oid dados
				printf("SNMPGETBULK\n");
				snmpgetbulk(buffer);
			}
		}
		else
		{
			printf("Failed\n");
			// valor nao encontrado criar datagram e enviar
			datagram[0] = 0;
			char erro[64] = "Version or community string incorrect";
			int size = strlen(erro);
			datagram[1] = size;
			sprintf(datagram + 2, "%s", erro);
		}
		buffer[n] = '\0';
		sendto(sockfd, datagram, 128,
			   0, (const struct sockaddr *)&cliaddr,
			   len);
	}
	return 0;
}
