#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <winsock2.h>
#include <dict.h>

#define BACKLOG 10
#define BUFFER_SIZE 5242880 // 5 MiB

typedef int socklen_t;
typedef int ssize_t;

void serve(const char* ADDRESS, const char* PORT);
void getIP(SOCKET socket, struct sockaddr_in socketAddr, char* ipString, int bufferSize);
void initWSA();
struct addrinfo* createSocketFD(const char* ADDRESS, const char* PORT, SOCKET* serverFD);
void bindSocketToPort(struct addrinfo* res, SOCKET serverFD);
void listenConnections(SOCKET serverFD);
void listenClients(SOCKET serverFD);
const char* getContentType(const char* fileExt);
const char* getFileExt(const char* fileName);
void processGET(size_t* responseLen, char* response, const char urlRoute);
void notFound(char* response);
void getUrlRoute(char* urlRoute, char* buffer);
void processHTTPRequest(char* buffer, char* response, size_t* responseLen);
void* processClient(void* arg);
struct Dict* registerRoutes();

#endif // !HTTP_SERVER_H