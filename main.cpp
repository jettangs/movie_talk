#include <Winsock2.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <direct.h>     // 目录头文件

#include "lib/tools/tools.h"
#include "lib/cjson/cJSON.h"
#include "lib/srt/srt.h"

#pragma comment(lib,"Ws2_32.lib")

#define DEFAULT_PORT 8090
#define BUF_LENGTH 1024
#define MIN_BUF 128
#define LINE_LEN 1000
#define USER_ERROR -1
#define SERVER "Server: csr_http1.1\r\n"


int send_404_header(SOCKET sAccept);
int send_200_header(SOCKET sAccept, long flen, char *contentType);
int send_file(SOCKET sAccept, FILE *resource);

DWORD WINAPI SimpleHTTPServer(LPVOID lparam)
{
	SOCKET sAccept = (SOCKET)(LPVOID)lparam;
	char recv_buf[BUF_LENGTH];
	memset(recv_buf, 0, BUF_LENGTH);

	char method[MIN_BUF];
	memset(method, 0, MIN_BUF);

	char url[MIN_BUF];
	memset(url, 0, MIN_BUF);

	char path[_MAX_PATH];
	memset(path, 0, _MAX_PATH);

	recv(sAccept, recv_buf, sizeof(recv_buf), 0);

	printf("Recv data from client:\n%s", recv_buf);

	if (recv_buf[0] == '\0') {
		closesocket(sAccept);
		printf("Nothing.\n");
		return USER_ERROR;
	}

	int i = 0, j = 0;

	while (!(' ' == recv_buf[j]) && (i < sizeof(method) - 1))
	{
		method[i] = recv_buf[j];
		i++; j++;
	}
	method[i] = '\0';  
	i = 0;
	printf("\nMethod: %s\n", method);

	// 提取出第二个单词(url文件路径，空格结束)，并把'/'改为windows下的路径分隔符'\'
	while ((' ' == recv_buf[j]) && (j < sizeof(recv_buf)))
		j++;
	while (!(' ' == recv_buf[j]) && (i < sizeof(recv_buf) - 1) && (j < sizeof(recv_buf)))
	{
		if (recv_buf[j] == '/')
			url[i] = '\\';
		else if (recv_buf[j] == ' ')
			break;
		else
			url[i] = recv_buf[j];
		i++; j++;
	}
	url[i] = '\0';
	printf("URL: %s\n", url);
	_getcwd(path, _MAX_PATH);
	if (!_stricmp(method, "GET") || !_stricmp(method, "HEAD"))
	{
		strcat_s(path, "\\www");
		char *suffix = GetSuffix(url);
		if (!suffix) {
			suffix = "json";
			strcat_s(url, ".json");
		}
		char *contentType = GetContentType(suffix);
		strcat_s(path, url);
		printf("Path: %s\n", path);
		FILE *resource = fopen(path, "rb");
		if (resource == NULL)
		{
			send_404_header(sAccept);
			closesocket(sAccept);
			printf("File not found.\nclose ok.\n");
			return USER_ERROR;
		}
		fseek(resource, 0, SEEK_END);
		long flen = ftell(resource);
		printf("file length: %ld\n", flen);
		fseek(resource, 0, SEEK_SET);
		send_200_header(sAccept, flen, contentType);
		if (0 == send_file(sAccept, resource))
			printf("file send ok.\n");
		else
			printf("file send fail.\n");
		fclose(resource);
	}
	else if (!_stricmp(method, "POST"))
	{
		char *s = strstr(recv_buf, "\r\n\r\n");
		s += 4;
		if (!*s) {
			printf("PostData is null\n");
			closesocket(sAccept);
			return USER_ERROR;
		}
		char *resStr = "{\"code\":10000}";
		FILE *srtIn = fopen("res\\V_for_Vendetta.srt", "rb");
		FILE *srtOut = fopen("res\\V_for_Vendetta_SIMP.srt", "wb");
		cJSON *root = cJSON_Parse(s);
		cJSON *object = cJSON_GetObjectItem(root, "lines");
		int size = cJSON_GetArraySize(object);
		int *sqs = (int *)malloc(sizeof(int)*size);
		memset(sqs, 0, sizeof(sqs));
		cJSON *item;
		for (int i = 0; i < size; i++) {
			item = cJSON_GetArrayItem(object, i);
			*sqs = atoi(item->child->valuestring);
			sqs++;
		}
		sqs -= size;
		CompressSrt(srtIn, srtOut, sqs);
		send_200_header(sAccept, strlen(resStr), "application/json");
		send(sAccept, resStr, strlen(resStr), 0);
		cJSON_Delete(root);
	}
	else {
		closesocket(sAccept);
		printf("Not GET, HEAD, POST method.\nclose ok.\n");
		return USER_ERROR;
	}

	closesocket(sAccept); 
	printf("close ok.\n");
	return 0;
}

int send_404_header(SOCKET sAccept)
{
	char send_buf[MIN_BUF];
	sprintf(send_buf, "HTTP/1.1 404 NOT FOUND\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "Connection: keep-alive\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, SERVER);
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "Content-Type: text/html\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	return 0;
}

int send_200_header(SOCKET sAccept, long flen, char *contentType)
{
	char send_buf[MIN_BUF];
	sprintf(send_buf, "HTTP/1.1 200 OK\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "Connection: keep-alive\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, SERVER);
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "Content-Length: %ld\r\n", flen);
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "Content-Type: ");
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, contentType);
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	return 0;
}

int send_file(SOCKET sAccept, FILE *resource)
{
	char send_buf[BUF_LENGTH];
	while (1)
	{
		memset(send_buf, 0, sizeof(send_buf));       
		fgets(send_buf, sizeof(send_buf), resource);
		send(sAccept, send_buf, strlen(send_buf), 0);
		if (feof(resource))
			return 0;
	}
}

int main()
{
	WSADATA wsaData;
	SOCKET sAccept;       
	struct sockaddr_in ser, cli;  
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, 0);
	ser.sin_family = AF_INET;
	ser.sin_port = htons(DEFAULT_PORT);               
	ser.sin_addr.s_addr = htonl(INADDR_ANY);  
	bind(sListen, (LPSOCKADDR)&ser, sizeof(ser));
	listen(sListen, 5);
	while (1) 
	{
		int iLen = sizeof(cli);
		sAccept = accept(sListen, (struct sockaddr*)&cli, &iLen);
		DWORD ThreadID;
		CreateThread(NULL, 0, SimpleHTTPServer, (LPVOID)sAccept, 0, &ThreadID);
	}
	closesocket(sListen);
	WSACleanup();
	return 0;
}