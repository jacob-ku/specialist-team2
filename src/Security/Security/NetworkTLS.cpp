
/* ssl_client.c
*
* Copyright (c) 2000 Sean Walton and Macmillan Publishers.  Use may be in
* whole or in part in accordance to the General Public License (GPL).
*
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*/

/*****************************************************************************/
/*** ssl_client.c                                                          ***/
/***                                                                       ***/
/*** Demonstrate an SSL client.                                            ***/
/*****************************************************************************/

//#include "stdafx.h"

#include <iostream>
#include <new>
#include <stdio.h>
#include <string.h>
#include <afx.h>
#include "NetworkTLS.h"

extern void Log(int nType, unsigned int Size, const char* fmt, ...);

/*
#define FAIL    -1
#define PORT 4490
#define IPaddr "127.0.0.1"

WSADATA wsaData;
SOCKET hSocket;
SOCKADDR_IN addr;

void ErrorHandling(const char* message);
*/
WSADATA wsaData;
SOCKET hSocket;
SOCKADDR_IN addr;

ssize_t ReadDataTls(SSL* TlsConnectedPort, unsigned char* data, size_t length)
{
	ssize_t bytes = 0;

	if (!TlsConnectedPort) {
		return (-1);
	}

	for (size_t i = 0; i < length; i += bytes)
	{
		if ((bytes = SSL_read(TlsConnectedPort, (char*)(data + i), (int)(length - i))) <= 0)
		{
			return (-1);
		}
	}
	return(length);
}

ssize_t WriteDataTls(SSL* TlsConnectedPort, unsigned char* data, size_t length)
{
	ssize_t bytes = 0;

	if (!TlsConnectedPort) {
		return (-1);
	}

	for (size_t i = 0; i < length; i += bytes)
	{
		if ((bytes = SSL_write(TlsConnectedPort, (char*)(data + i), (int)(length - i))) <= 0)
		{
			return (-1);
		}
	}
	return(length);
}

//-----------------------------------------------------------------
// END ReadDataTls
//-----------------------------------------------------------------

#if 0
/*---------------------------------------------------------------------*/
/*--- OpenConnection - create socket and connect to server.         ---*/
/*---------------------------------------------------------------------*/
int OpenConnection()
{
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	hSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET)
		ErrorHandling("hSocket() error");

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	//addr.sin_addr.s_addr = inet_addr(IPaddr);
	InetPton(AF_INET, _T(IPaddr), &addr.sin_addr.s_addr);
	addr.sin_port = htons(PORT);

	if (connect(hSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		ErrorHandling("connect() Error");
	}

	return hSocket;
}
#else

/*---------------------------------------------------------------------*/
/*--- OpenTLSConnection - create socket and connect to server.         ---*/
/*---------------------------------------------------------------------*/
int OpenTLSConnection(const char* remotehostname, const char* remoteportno)
{
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		ErrorHandling("WSAStartup() error");
		return -1;
	}

	hSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET) {
		ErrorHandling("hSocket() error");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	//addr.sin_addr.s_addr = inet_addr(IPaddr);
#if 1
	//addr.sin_port = htons(PORT);
	int x;
	sscanf_s(remoteportno, "%d", &x);
	addr.sin_port = htons(x);
	InetPton(AF_INET,(PCSTR)remotehostname, &addr.sin_addr.s_addr);
#else
	addr.sin_port = htons(PORT);
	InetPton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);
#endif
	if (connect(hSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		ErrorHandling("connect() Error");
		return -1;
	}

	return (int)hSocket;
}
#endif


int CloseTLSConnection(SSL* ssl) {
	
	if (ssl == NULL) return -1;

	SSL_shutdown(ssl);

	return 0;
}

/*---------------------------------------------------------------------*/
/*--- InitCTX - initialize the SSL engine.                          ---*/
/*---------------------------------------------------------------------*/
SSL_CTX* InitCTX(void)
{
	//SSL_METHOD* method;
	SSL_CTX* ctx;

	SSL_library_init();
	OpenSSL_add_all_algorithms();        /* Load cryptos, et.al. */
	SSL_load_error_strings();            /* Bring in and register error messages */
	//method = SSLv23_client_method();			/* Create new client-method instance */
	//ctx = SSL_CTX_new(method);				/* Create new context */
	ctx = SSL_CTX_new(SSLv23_client_method());	/* Create new context */
	if (ctx == NULL)
	{
		ErrorHandling("ctx Error");
	}
	return ctx;
}

void SetCertForClient(SSL_CTX* ctx)
{
#if 0
	/*----- SSL_CTX for Loading Client Certificate -----*/
	if (SSL_CTX_use_certificate_file(ctx, CLIENT_CERT, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}

	/*----- SSL_CTX for Loading Private Key -----*/
	if (SSL_CTX_use_PrivateKey_file(ctx, CLIENT_KEY, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
#endif

	/* CA */
	if (!SSL_CTX_load_verify_locations(ctx, CA_CERT, NULL)) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
}

/*---------------------------------------------------------------------*/
/*--- ShowCerts - print out the certificates.                       ---*/
/*---------------------------------------------------------------------*/
void ShowCerts(SSL* ssl)
{
	X509* cert = NULL;
	char* line = NULL;

	cert = SSL_get_peer_certificate(ssl);    /* get the server's certificate */
	if (cert != NULL)
	{
		TRACE("Server certificates:\n");
		line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		TRACE("Subject: %s\n", line);
		ErrorHandling(line);
		free(line);                            /* free the malloc'ed string */
		line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
		TRACE("Issuer: %s\n", line);
		ErrorHandling(line); 
		free(line);                            /* free the malloc'ed string */
		X509_free(cert);						/* free the malloc'ed certificate copy */
	}
	else {
		TRACE("No certificates.\n");
		ErrorHandling("No certificates.\n");
	}
}

void ErrorHandling(const char* message)
{
	if (message != NULL) {
		//puts(message);
		unsigned int size = 0;
		size = (unsigned int)strnlen_s(message, MAX_LENGTH_LOG);
		if (size > 0) {
			Log(0, size, message);
		}
	}
}