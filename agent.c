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
uint8_t datagram[128];
int port = 0;
char CommunityString[32];
char oid[16];

/**
 * It reads the mib.txt file and stores the port number, community string, and oid in the variables
 * port, CommunityString, and oid, respectively
 */
void readMIB()
{
	char buff[64];
	file = fopen("mib.txt", "r");
	if (file == NULL)
	{
		perror("Error opening file mib.txt");
		return;
	}
	fscanf(file, "%d %s %s", &port, CommunityString, oid);
	printf("%d %s %s\n", port, CommunityString, oid);
	fclose(file);
}

/**
 * It reads the OID from the received datagram, searches for it in the MIB file and if it finds it, it
 * creates a datagram with the value of the OID and sends it back to the client. If it doesn't find it,
 * it creates a datagram with an error message and sends it back to the client
 *
 * @param buffer the buffer that contains the SNMP request
 */
void snmpget(uint8_t buffer[64])
{
	memset(datagram, 0, 128);
	char oid1[16];
	int oid = buffer[(buffer[2]) + 3];
	char fbuff[64];
	int j = 1;
	int found = 0;
	char value[32];
	file = fopen("mib.txt", "r");
	if (file == NULL)
	{
		perror("Error opening file mib.txt");
		return;
	}

	for (int i = 0; i < oid; i++)
	{
		oid1[i] = buffer[(buffer[2]) + 3 + 1 + i];
	}

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
				strcpy(value, token);
				datagram[0] = 1;
				datagram[1] = oid;
				for (int i = 0; i < oid; i++)
				{
					datagram[i + 2] = oid1[i];
				}
				datagram[oid + 2] = strlen(value);
				for (int i = 0; i < strlen(value); i++)
				{
					datagram[oid + 3 + i] = value[i];
				}
				printf("Sucess\n");
			}
			j++;
		}
	}
	fclose(file);
	if (found == 0)
	{
		printf("Fail\n");
		datagram[0] = 0;
		char erro[32] = "OID not found";
		int size = strlen(erro);
		datagram[1] = size;
		sprintf(datagram + 2, "%s", erro);
	}
}

/**
 * It reads the file line by line, and when it finds the OID that was requested, it reads the next line
 * and returns the OID and value of that line
 *
 * @param buffer the buffer that contains the SNMP request
 */
void snmpgetnext(uint8_t buffer[64])
{
	memset(datagram, 0, 128);
	char oid1[16];
	int oid = buffer[(buffer[2]) + 3];
	char fbuff[64];
	int j = 1;
	int found = 0;
	int next = 0;
	char value[32];
	char oid_next[32];
	memset(value, 0, 32);
	file = fopen("mib.txt", "r");
	if (file == NULL)
	{
		perror("Error opening file mib.txt");
		return;
	}

	for (int i = 0; i < oid; i++)
	{
		oid1[i] = buffer[(buffer[2]) + 3 + 1 + i];
	}

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
				strcpy(value, token);
				datagram[0] = 1;
				datagram[1] = strlen(oid_next);
				for (int i = 0; i < oid; i++)
				{
					datagram[i + 2] = oid_next[i];
				}
				datagram[strlen(oid_next) + 2] = strlen(value);
				for (int i = 0; i < strlen(value); i++)
				{
					datagram[oid + 3 + i] = value[i];
				}
				printf("Sucess\n");
			}
			j++;
		}
	}

	fclose(file);
	if (strlen(value) <= 1)
	{
		printf("Fail\n");
		datagram[0] = 0;
		char erro[32] = "Error- next OID doesn't exists";
		int size = strlen(erro);
		datagram[1] = size;
		sprintf(datagram + 2, "%s", erro);
	}
	if (found == 0)
	{
		printf("Fail\n");
		datagram[0] = 0;
		char erro[32] = "OID not found";
		int size = strlen(erro);
		datagram[1] = size;
		sprintf(datagram + 2, "%s", erro);
	}
}

void snmpset(uint8_t buffer[64])
{
	memset(datagram, 0, 128);
	FILE *temp;
	int sizeOID = buffer[(buffer[2]) + 3];
	int sizeValue = buffer[buffer[2] + 3 + sizeOID + 1];
	int count = 0;
	int isInt = 0;
	int j = 1;
	int found = 0;
	int variavel = 0;
	int read_write = 0;
	int flag = 0;
	char oid1[16];
	char val[32];
	char fbuff[256];
	char value[32];
	char var[32];
	char permission[8];
	char aux[256];
	memset(val, 0, 32);

	file = fopen("mib.txt", "r");

	if (file == NULL)
	{
		perror("Error opening file mib.txt");
		return;
	}

	temp = fopen("temp.txt", "w");

	if (temp == NULL)
	{
		perror("Error opening file temp.txt");
		return;
	}

	for (int i = 0; i < sizeOID; i++)
	{
		oid1[i] = buffer[(buffer[2]) + 3 + 1 + i];
	}

	for (int i = 0; i < sizeValue; i++)
	{
		val[i] = buffer[buffer[2] + 3 + sizeOID + 1 + i + 1];
	}
	for (int i = 0; i < sizeValue; i++)
	{
		if (buffer[buffer[2] + 3 + sizeOID + 1 + i + 1] <= 57 && buffer[buffer[2] + 3 + sizeOID + 1 + i + 1] >= 48)
		{
			count++;
		}
	}
	if (count == sizeValue)
	{
		isInt = 1;
	}
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
				if ((strcmp(token, "Integer") == 0 && isInt == 1) || (strcmp(token, "Integer") != 0 && isInt != 1))
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
					strcpy(permission, token);
				}
				else
				{
					flag = 1;
				}
			}
			if (j == 4 && read_write == 1 && flag == 0)
			{
				flag = 1;
				strcpy(value, token);
				datagram[0] = 1;
				datagram[1] = sizeOID;
				for (int i = 0; i < sizeOID; i++)
				{
					datagram[i + 2] = oid1[i];
				}
				datagram[sizeOID + 2] = strlen(val);
				for (int i = 0; i < strlen(val); i++)
				{
					datagram[sizeOID + 3 + i] = val[i];
				}
				printf("Sucess\n");
				sprintf(aux, "%s %s %s %s\n", oid1, var, permission, val);
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
		if (file == NULL)
		{
			perror("Error opening file mib.txt");
			return;
		}
		temp = fopen("temp.txt", "r");
		if (temp == NULL)
		{
			perror("Error opening file temp.txt");
			return;
		}
		while (fgets(buff, sizeof(buff), temp) != NULL)
		{
			fwrite(buff, strlen(buff), 1, file);
		}
		fclose(file);
		fclose(temp);
	}
	else if (found == 0)
	{
		printf("Fail\n");
		datagram[0] = 0;
		char erro[32] = "OID not found";
		int size = strlen(erro);
		datagram[1] = size;
		sprintf(datagram + 2, "%s", erro);
	}
	if (variavel == 0)
	{
		printf("Fail\n");
		datagram[0] = 0;
		char erro[32] = "Unmatched variables";
		int size = strlen(erro);
		datagram[1] = size;
		sprintf(datagram + 2, "%s", erro);
	}
	else if (read_write == 0)
	{
		printf("Fail\n");
		datagram[0] = 0;
		char erro[32] = "No permission to write";
		int size = strlen(erro);
		datagram[1] = size;
		sprintf(datagram + 2, "%s", erro);
	}
}

int main()
{
	int sockfd, len, n;
	uint8_t buffer[MAXLINE];
	char *hello = "First message from server";
	struct sockaddr_in servaddr, cliaddr;
	readMIB();

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
		n = recvfrom(sockfd, buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);

		int CString = (int)(buffer[2]);
		char comun_string[16];
		memset(comun_string, 0, 16);

		for (int i = 0; i < CString; i++)
		{
			comun_string[i] = (char)buffer[i + 3];
		}
		if ((buffer[1]) == 1 && strcmp(comun_string, CommunityString) == 0)
		{
			printf("AQUI:%u\n", buffer[0]);
			if (buffer[0] == 0)
			{
				printf("SNMPGET\n");
				snmpget(buffer);
			}
			else if ((buffer[0]) == 1)
			{
				printf("SNMPGETNEXT\n");
				snmpgetnext(buffer);
			}
			else if ((buffer[0]) == 2)
			{
				printf("SNMPSET\n");
				snmpset(buffer);
			}
		}
		else
		{
			printf("Failed\n");
			datagram[0] = 0;
			char erro[64] = "Version or community string incorrect";
			int size = strlen(erro);
			datagram[1] = size;
			sprintf(datagram + 2, "%s", erro);
		}
		buffer[n] = '\0';
		sendto(sockfd, datagram, 128, 0, (const struct sockaddr *)&cliaddr, len);
	}
	return 0;
}
