#pragma once
#ifndef NetworkTLSH
#define NetworkTLSH

#include <stdio.h>
#include <WinSock2.h>
#include <malloc.h>
#include <string.h>

#include <ws2tcpip.h>
#include <BaseTsd.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#pragma comment(lib, "ws2_32.lib")
//#pragma comment(lib, "libeay32.lib")
//#pragma comment(lib, "ssleay32.lib")

#define MAX_LENGTH_LOG	5096

#define FAIL    -1
#define PORT 4490
#define IPaddr "127.0.0.1"

#define CA_CERT "cert\\Team2_RootCA_Cert.crt"

typedef SSIZE_T ssize_t;

void ErrorHandling(const char* message);
int OpenTLSConnection(const char* remotehostname, const char* remoteportno);
int CloseTLSConnection(SSL* ssl);

SSL_CTX* InitCTX(void);
void ShowCerts(SSL* ssl);
void SetCertForClient(SSL_CTX* ctx);
ssize_t ReadDataTls(SSL* TlsConnectedPort, unsigned char* data, size_t length);
ssize_t WriteDataTls(SSL* TlsConnectedPort, unsigned char* data, size_t length);
#endif