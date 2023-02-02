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

void create_datagram(char *snmpCommand);
void menu();

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
			printf("Insert SNMP command:");
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
				printf("Agent Reply: ");
				printf("OID : ");
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
			buffer[n] = '\0';
		}
	}
	close(sockfd);
	return 0;
}

/**
 * It takes the command written in the terminal and creates a datagram that will be sent to the server.
 *
 * @param snmpCommand The command that the user entered in the terminal
 */
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
		/* Checking if the first parameter of the command is snmpget, snmpgetnext or snmpset. If it is
		snmpget it writes 0 to the datagram, if it is snmpgetnext it writes 1 to the datagram and if it is
		snmpset it writes 2 to the datagram. */
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
		}
		/* Checking if the version is v2c or v1. If it is v2c it writes 1 to the datagram otherwise it writes
		2. */
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
		/* Copying the community string to the datagram. */
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
		/* Copying the OID to the datagram. */
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
		/* The part of the code that is responsible for the snmpset command. It takes the value that the user
		wants to set and writes it to the datagram. */
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
		}
		i++;
	}
}

/**
 * It prints the menu.
 */
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
	printf("\n");
}