#include <http_server.h>

#include <dict.h>

#include <fcntl.h>
#include <io.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

struct Dict* rootRoute;

void serve(const char* ADDRESS, const char* PORT)
{
	initWSA();

	SOCKET serverFD;
	struct addrinfo* res;
	struct sockaddr_in serverAddr;
	rootRoute = dictEntry("/", "http/index.html");
	memset(&serverAddr, 0, sizeof(serverAddr));

	res = createSocketFD(ADDRESS, PORT, &serverFD);
	bindSocketToPort(res, serverFD);
	listenConnections(serverFD);

	const char IP[16];
	getIP(serverFD, serverAddr, IP, sizeof(IP));
	printf("Server running @ %s\n", IP);

	printf("Now listening to clients on port %s...\n", PORT);

	listenClients(serverFD);

	_close(serverFD);
}

void getIP(SOCKET socket, struct sockaddr_in socketAddr, char* ipString, int bufferSize)
{
	if (!ipString || bufferSize < 1) {
		return;
	}
	char* IP[16];
	socklen_t addrLen = sizeof(socketAddr);
	getsockname(socket, (struct sockaddr*)&socketAddr, &addrLen);
	inet_ntop(AF_INET, &socketAddr.sin_addr, IP, sizeof(IP));
	strcpy_s(ipString, bufferSize, IP);
	ipString[bufferSize - 1] = "\0";
}

void initWSA()
{
	WSADATA wsaData;

	if (WSAStartup(0x202, &wsaData) < 0) {
		perror("WSA failed");
		exit(EXIT_FAILURE);
	}
}

struct addrinfo* createSocketFD(const char* ADDRESS, const char* PORT, SOCKET* serverFD)
{
	struct addrinfo hints;
	struct addrinfo* res;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(ADDRESS, PORT, &hints, &res) != 0) {
		perror("getaddrinfo failed");
		exit(EXIT_FAILURE);
	}

	*serverFD = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if (*serverFD < 0) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	printf("Socket creation successful!\n");
	return res;
}

void bindSocketToPort(struct addrinfo* res, SOCKET serverFD)
{
	int bindRes = bind(serverFD, res->ai_addr, res->ai_addrlen);

	if (bindRes < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	printf("Binding successful!\n");
}

void listenConnections(SOCKET serverFD)
{
	int listener = listen(serverFD, BACKLOG) < 0;

	if (listener) {
		perror("listen failed");
		exit(EXIT_FAILURE);
	}

	printf("Listen successful!\n");
}

void listenClients(SOCKET serverFD)
{
	while (1) {
		struct sockaddr_in clientAddr;
		socklen_t clientAddrLen = sizeof(clientAddr);
		SOCKET* clientFD = malloc(sizeof(SOCKET));

		*clientFD = accept(serverFD, (struct sockaddr*)&clientAddr, &clientAddrLen);

		if (*clientFD < 0) {
			perror("accept failed");
			continue;
		}

		const char* IP[16];
		getIP(clientFD, clientAddr, IP, sizeof(IP));
		printf("Client acccepted! Client's IP: %s\n", IP);

		pthread_t threadID;
		pthread_create(&threadID, NULL, processClient, (void*)clientFD);
		pthread_detach(threadID);
	}
}

const char* getContentType(const char* fileExt)
{
	if (_stricmp(fileExt, "html") == 0 || _stricmp(fileExt, "htm") == 0) {
		return "text/html";
	}
	if (_stricmp(fileExt, "txt") == 0) {
		return "text/plain";
	}
	if (_stricmp(fileExt, "jpg") == 0 || _stricmp(fileExt, "jpeg") == 0) {
		return "text/jpeg";
	}
	if (_stricmp(fileExt, "png") == 0) {
		return "text/png";
	}
	return "application/octet-stream";
}

const char* getFileExt(const char* fileName)
{
	const char* fileExt = strrchr(fileName, '.');

	if (!fileName || fileExt == fileName)
	{
		return "";
	}

	return fileExt + 1;
}

void processGET(size_t* responseLen, char* response, const char* urlRoute)
{
	printf("Processing GET request...\n");

	struct DictEntry* dest = search(rootRoute, urlRoute);
	if (!dest) {
		notFound(response);
		*responseLen = strlen(response);
		return;
	}
	const char* destFile = (const char*)dest->value;
	const char* fileExt = getFileExt(destFile);
	const char* contentType = getContentType(fileExt);

	char* header = (char*)malloc(BUFFER_SIZE * sizeof(char));

	snprintf(header, BUFFER_SIZE, "HTTP/1.1 200 OK\r\n""Content-Type: %s\r\n""\r\n", contentType);

	int fileFD;
	_sopen_s(&fileFD, destFile, _O_RDONLY, _SH_DENYNO, 0);

	if (fileFD == -1) {
		notFound(response);
		*responseLen = strlen(response);
		return;
	}

	*responseLen = 0;
	memcpy(response, header, strlen(header));
	*responseLen += strlen(header);
	
	ssize_t bytesRead;
	while ((bytesRead = _read(fileFD, response + *responseLen, BUFFER_SIZE - *responseLen)) > 0) {
		*responseLen += bytesRead;
	}

	free(header);
	_close(fileFD);
}

// TODO: POST, DELETE, PUT, PATCH. Maybe when I'm not lazy.

void notFound(char* response)
{
	snprintf(response, BUFFER_SIZE, "HTTP/1.1 404 Not Found\r\n""Content-Type: text/plain\r\n""\r\n""404 Not Found");
}

void getUrlRoute(char* urlRoute, char* buffer)
{
	char* nextToken = NULL;
	char* clientHeader = strtok_s(buffer, "\n", &nextToken);
	
	nextToken = NULL;
	char* headerRoute = strtok_s(clientHeader, " ", &nextToken);

	int currentHeaderLocation = 0;
	const int routeLocation = 1;
	while (headerRoute) {
		if (currentHeaderLocation == routeLocation) {
			strcpy_s(urlRoute, sizeof(headerRoute), headerRoute);
			return;
;		}
		headerRoute = strtok_s(NULL, " ", &nextToken);
		currentHeaderLocation++;
	}

	return;
}

void processHTTPRequest(char* buffer, char* response, size_t* responseLen)
{
	if (strstr(buffer, "GET")) {
		char urlRoute[256];
		getUrlRoute(urlRoute, buffer);
		processGET(responseLen, response, urlRoute);
		return;
	}

	notFound(buffer, response);
}

void* processClient(void* arg)
{
	int clientFD = *((int*)arg);
	char* buffer = (char*)malloc(BUFFER_SIZE * sizeof(char));

	ssize_t clientRecv = recv(clientFD, buffer, BUFFER_SIZE, 0);

	if (clientRecv <= 0) {
		free(buffer);
		return NULL;
	}

	char* response = (char*)malloc(BUFFER_SIZE * sizeof(char));
	size_t response_len;
	processHTTPRequest(buffer, response, &response_len);
	send(clientFD, response, response_len, 0);

	free(response);
	closesocket(clientFD);
	free(buffer);
	free(arg);

	return NULL;
}

struct Dict* registerRoutes()
{
	struct Dict* rootRoute = dictEntry("/", "index.html");

	printf("Current Routes:\n");
	getDictInOrder(rootRoute);

	return rootRoute;
}