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
#define USER_ERROR -1
#define SERVER "Server: csr_http1.1\r\n"
#define LINE_LEN 1000

int file_not_found(SOCKET sAccept);
int send_success_res(SOCKET sAccept, long flen, char *contentType);
int send_file(SOCKET sAccept, FILE *resource);
int send_not_found(SOCKET sAccept);

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

	if (recv(sAccept, recv_buf, sizeof(recv_buf), 0) == SOCKET_ERROR)   //接收错误
	{
		printf("recv data failed:%d\n", WSAGetLastError());
		return USER_ERROR;
	}
	else {
		printf("recv data from client:\n%s", recv_buf);

	}
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

	// 如果想做的规范些可以返回浏览器一个501未实现的报头和页面

	printf("\nmethod: %s\n", method);

	// 提取出第二个单词(url文件路径，空格结束)，并把'/'改为windows下的路径分隔符'\'
	// 这里只考虑静态请求(比如url中出现'?'表示非静态，需要调用CGI脚本，'?'后面的字符串表示参数，多个参数用'+'隔开
	// 例如：www.csr.com/cgi_bin/cgi?arg1+arg2 该方法有时也叫查询，早期常用于搜索)
	i = 0;
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
	printf("url: %s\n", url);
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

		printf("path: %s\n", path);

		FILE *resource = fopen(path, "rb");

		if (resource == NULL)
		{
			file_not_found(sAccept);
			if (0 == _stricmp(method, "GET"))
				send_not_found(sAccept);
			closesocket(sAccept);
			printf("file not found.\nclose ok.\n");
			return USER_ERROR;
		}
		fseek(resource, 0, SEEK_END);

		long flen = ftell(resource);

		printf("file length: %ld\n", flen);

		fseek(resource, 0, SEEK_SET);

		send_success_res(sAccept, flen, contentType);

		if (0 == send_file(sAccept, resource))
			printf("file send ok.\n");
		else
			printf("file send fail.\n");

		fclose(resource);
	}
	else if (!_stricmp(method, "POST"))
	{
		//recv_buf[]
		char *s = strstr(recv_buf, "\r\n\r\n");
		//char *p = s + 4;
		s += 4;
		if (!*s) {
			printf("postData is null\n");
		}

		//char p[5];
		//strcpy(p, url);
		//*(p + 5) = 0;
		//if (!_stricmp(p, "\\srt\\")) {
			char *resStr = "{\"code\":10000}";
			FILE *srtIn = fopen("res\\V_for_Vendetta.srt", "rb");
			FILE *srtOut = fopen("res\\V_for_Vendetta_SIMP.srt", "wb");
			cJSON *root = cJSON_Parse(s);
			cJSON *object = cJSON_GetObjectItem(root, "lines");
			int size = cJSON_GetArraySize(object);
			cJSON *item;
			unsigned int *sqs = (unsigned int *)malloc(sizeof(int)*size);
			memset(sqs, 0, sizeof(sqs));
			for (int i = 0; i < size; i++) {
				item = cJSON_GetArrayItem(object, i);
				*sqs = atoi(item->child->valuestring);
				sqs++;
			}
			sqs -= size;
			CompressSrt(srtIn, srtOut, sqs);


			send_success_res(sAccept, strlen(resStr), "application/json");
			send(sAccept, resStr, strlen(resStr), 0);
		//}
		
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

int file_not_found(SOCKET sAccept)
{
	char send_buf[MIN_BUF];
	//  time_t timep;   
	//  time(&timep);
	sprintf(send_buf, "HTTP/1.1 404 NOT FOUND\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	//  sprintf(send_buf, "Date: %s\r\n", ctime(&timep));
	//  send(sAccept, send_buf, strlen(send_buf), 0);
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

int send_success_res(SOCKET sAccept, long flen, char *contentType)
{
	char send_buf[MIN_BUF];
	//  time_t timep;
	//  time(&timep);
	sprintf(send_buf, "HTTP/1.1 200 OK\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "Connection: keep-alive\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	//  sprintf(send_buf, "Date: %s\r\n", ctime(&timep));
	//  send(sAccept, send_buf, strlen(send_buf), 0);
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

//定义的file_not_found页面
int send_not_found(SOCKET sAccept)
{
	char send_buf[MIN_BUF];
	sprintf(send_buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "<BODY><h1 align='center'>404</h1><br/><h1 align='center'>file not found.</h1>\r\n");
	send(sAccept, send_buf, strlen(send_buf), 0);
	sprintf(send_buf, "</BODY></HTML>\r\n");
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
		if (SOCKET_ERROR == send(sAccept, send_buf, strlen(send_buf), 0))
		{
			printf("send() Failed:%d\n", WSAGetLastError());
			return USER_ERROR;
		}
		if (feof(resource))
			return 0;
	}
}

int main()
{
	WSADATA wsaData;
	SOCKET sListen, sAccept;        //服务器监听套接字，连接套接字
	int serverport = DEFAULT_PORT;   //服务器端口号
	struct sockaddr_in ser, cli;   //服务器地址，客户端地址
	int iLen;

	printf("Server waiting\n");

	//第一步：加载协议栈
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Failed to load Winsock.\n");
		return USER_ERROR;
	}

	//第二步：创建监听套接字，用于监听客户请求
	sListen = socket(AF_INET, SOCK_STREAM, 0);
	if (sListen == INVALID_SOCKET)
	{
		printf("socket() Failed:%d\n", WSAGetLastError());
		return USER_ERROR;
	}

	//创建服务器地址：IP+端口号
	ser.sin_family = AF_INET;
	ser.sin_port = htons(serverport);               //服务器端口号
	ser.sin_addr.s_addr = htonl(INADDR_ANY);   //服务器IP地址，默认使用本机IP

											   //第三步：绑定监听套接字和服务器地址
	if (bind(sListen, (LPSOCKADDR)&ser, sizeof(ser)) == SOCKET_ERROR)
	{
		printf("blind() Failed:%d\n", WSAGetLastError());
		return USER_ERROR;
	}

	//第五步：通过监听套接字进行监听
	if (listen(sListen, 5) == SOCKET_ERROR)
	{
		printf("listen() Failed:%d\n", WSAGetLastError());
		return USER_ERROR;
	}
	while (1)  //循环等待客户的请求
	{
		//第六步：接受客户端的连接请求，返回与该客户建立的连接套接字
		iLen = sizeof(cli);
		sAccept = accept(sListen, (struct sockaddr*)&cli, &iLen);
		if (sAccept == INVALID_SOCKET)
		{
			printf("accept() Failed:%d\n", WSAGetLastError());
			break;
		}
		//第七步，创建线程接受浏览器请求
		DWORD ThreadID;
		CreateThread(NULL, 0, SimpleHTTPServer, (LPVOID)sAccept, 0, &ThreadID);
	}
	closesocket(sListen);
	WSACleanup();
	return 0;
}