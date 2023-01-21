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
#define PORT 8080
#define MAXLINE 1024
uint8_t datagram[128];

void menu()
{
	printf("\n");
	printf("---------SNMPv2c Managment App---------\n");
	printf("Press 1 to insert SNPM commands\n");
	printf("Press 0 to exit\n");
	printf("\n");
	printf("-----------Accepted Commands-----------\n");
	printf("snmpget -v2c public [OID]\n");
	printf("snmpgetnext -v2c public [OID]\n");
	printf("snmpset -v2c public [OID] [VALUE]\n");
	printf("snmpgetbulk -v2c public [OID] [OID]\n");
	printf("\n");
}

void create_datagram(char *snmpCommand)
{
	memset(datagram, 0, 128);
	char *token = NULL;
	int i = 1;
	char snmp[16];
	char version[8];
	char comstring[16];
	char oid[64];
	for (token = strtok(snmpCommand, " "); token != NULL; token = strtok(NULL, " "))
	{
		// first parameter of datagram
		if (i == 1)
		{
			memset(datagram, 0, 32);
			strcpy(snmp, token);
			int j = 0;
			while (snmp[j])
			{
				snmp[j] = tolower(snmp[j]);
				j++;
			}
			if (strcmp(snmp, "snmpget") == 0)
			{
				datagram[0] = 0;
			}
			else if (strcmp(snmp, "snmpgetnext") == 0)
			{
				datagram[0] = 1;
			}
			else if (strcmp(snmp, "snmpset") == 0)
			{
				datagram[0] = 2;
			}
			else if (strcmp(snmp, "snmpgetbulk") == 0)
			{
				datagram[0] = 3;
			}
		}
		// second parameter of the datagram (version)
		// if the version written in terminal is "v2c" writes "1" to the datagram otherwise it writes "2"
		if (i == 2)
		{
			strcpy(version, token);
			int j = 0;
			while (version[j])
			{
				version[j] = tolower(version[j]);
				j++;
			}
			if (strcmp(version, "-v2c") == 0)
			{
				datagram[1] = 1;
			}
			else
			{
				datagram[1] = 2;
			}
		}
		// community string bytes
		if (i == 3)
		{
			strcpy(comstring, token);
			int j = 0;
			while (comstring[j])
			{
				comstring[j] = tolower(comstring[j]);
				datagram[j + 3] = comstring[j];
				j++;
			}
			datagram[2] = j;
		}
		// OID bytes
		if (i == 4)
		{
			strcpy(oid, token);
			int j = 0;
			int a = 0;
			while (oid[j])
			{
				datagram[datagram[2] + 3 + j + 1] = oid[j];
				j++;
			}
			datagram[datagram[2] + 3] = j;
		}
		// for snmpset or snmpgetbulk commands
		if (i == 5)
		{
			if (datagram[0] == 2)
			{
				char value[32];
				strcpy(value, token);
				int j = 0;
				while (value[j])
				{
					datagram[(datagram[2] + 3) + datagram[datagram[2] + 3] + 2 + j] = value[j];
					j++;
				}
				datagram[(datagram[2] + 3) + datagram[datagram[2] + 3] + 1] = j;
			}
			else if (datagram[0] == 3)
			{
				char oid2[32];
				strcpy(oid2, token);
				int j = 0;
				int a = 0;
				while (oid2[j])
				{
					datagram[(datagram[2] + 3) + datagram[datagram[2] + 3] + j + 2] = oid2[j];
					j++;
				}
				datagram[(datagram[2] + 3) + datagram[datagram[2] + 3] + 1] = j;
			}
		}
		i++;
	}
}

int main()
{
	int sockfd, n, len;
	int flag = 1;
	int option = 0;
	uint8_t buffer[MAXLINE];
	struct sockaddr_in servaddr;

	// Creating socket fd
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Socket Creation Error");
		exit(EXIT_FAILURE);
	}

	// socket struct params configuration
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = INADDR_ANY;

	while (flag)
	{
		menu();
		printf("Option- ");
		scanf("%d", &option);
		fflush(stdin);
		getchar();
		if (option == 0)
		{
			flag = 0;
		}
		else if (option == 1)
		{
			char snmpCommand[64];
			char c;
			int a = 0;
			printf("Insert SNMP command:\n");
			fflush(stdin);
			while ((c = getchar()) != '\n')
				snmpCommand[a++] = c;
			snmpCommand[a] = '\0';
			create_datagram(snmpCommand);
			sendto(sockfd, datagram, 64,
				   0, (const struct sockaddr *)&servaddr,
				   sizeof(servaddr));

			memset(buffer, 0, MAXLINE);
			n = recvfrom(sockfd, buffer, 128,
						 MSG_WAITALL, (struct sockaddr *)&servaddr,
						 &len);
			if (buffer[0] == 0)
			{
				printf("Error:\n");
				int s = buffer[1];
				for (int i = 0; i < s; i++)
				{
					printf("%c", buffer[i + 2]);
				}
				printf("\n");
			}
			else
			{
				if (datagram[0] == 3)
				{
					int qnt = buffer[1];
					int size = 0;
					int pos = 2;
					for (int i = 0; i < qnt; i++)
					{
						printf("OID: ");
						size = buffer[pos];
						for (int j = 0; j < size; j++)
						{
							printf("%c", buffer[pos + 1 + j]);
						}
						pos = pos + size + 1;
						size = buffer[pos];
						printf("  ");
						for (int j = 0; j < size; j++)
						{
							printf("%c", buffer[pos + 1 + j]);
						}
						printf("\n");
						pos = pos + size + 1;
					}
				}
				else
				{
					printf("Agent Reply:\n");
					printf("OID: ");
					int l = buffer[1];
					for (int j = 0; j < l; j++)
					{
						printf("%c", buffer[2 + j]);
					}
					int l_v = buffer[buffer[1] + 2];
					printf("  ");
					for (int i = 0; i < l_v; i++)
					{
						printf("%c", buffer[buffer[1] + 3 + i]);
					}
					printf("\n");
				}
			}
			buffer[n] = '\0';
		}
	}
	close(sockfd);
	return 0;
}
