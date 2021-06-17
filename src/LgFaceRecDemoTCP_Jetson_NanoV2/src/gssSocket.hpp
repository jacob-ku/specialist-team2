#ifndef __GSS_SOCKET_H__
#define  __GSS_SOCKET_H__

#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>


class gssSocketClass {
    public:
        gssSocketClass();
        ~gssSocketClass();

    public:
        int     initListenSockets(int nListenPort, bool bSecured);
        void    closeSockets();
        int     waitListenSocketAccept();
        int     isReceived();
        int     receiveData(unsigned char *recvBuf, const int recvBufSize); // Returned: Length of received (Exception -> 0: Nothing to receive, -1: Network Error)
        int     sendData(const unsigned char *messageBuf, const int messageSize); // Returned: Length of received (Exception -> 0: Nothing to receive, -1: Network Error)

        void    bindSSL(SSL_CTX *sslContext);

    private:
        void    closeDataSocket();
        int     setNonBlocking(int socketfd);

    private:
        int     m_sfdListen;       // Listen socket for TCP connection (Non-Secured)
        int     m_sfdData;         // Child socket after TCP/TLS accept (Common for both of secured and non-secured)
        bool    m_bSecureMode;     // Flag about the secure or non-secure mode

        SSL_CTX *m_pSslContext;    // SSL Context handler for TLS connection
        SSL     *m_pSsl;           // SSL channel handler for TLS connection

};


#endif
