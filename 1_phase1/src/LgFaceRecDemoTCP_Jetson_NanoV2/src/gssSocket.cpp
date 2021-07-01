#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <Winsock2.h>
#include <MSWSock.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
//#include <netdb.h>
#endif

#include "gssSocket.hpp"

#define MAX_LISTEN_BACKLOG     1     // Only one client connection allowed
#define SOCKET_BUF_RCV_SIZE    (50*1024)
#define SOCKET_BUF_SND_SIZE    (200*1024)

gssSocketClass::gssSocketClass()
{
    m_sfdListen    = 0;
    m_sfdData      = 0;
    m_bSecureMode  = false;
    m_pSslContext  = NULL;
}

gssSocketClass::~gssSocketClass()
{
    closeSockets();
}

void gssSocketClass::bindSSL(SSL_CTX *pSslContext)
{
    m_pSslContext = pSslContext;
}

int gssSocketClass::initListenSockets(int nListenPort, bool bSecured)
{
    struct sockaddr_in addr;
    int sfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sfd <= 0) {
        printf("Listen socket creation failure: %d\n", sfd);
        return -1;
    }

    /* Begin: This is development usage only */
    int option = 1;
    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option)) < 0) {
        close(sfd);
        printf("Socket Reuse configuration failure\n");
        return -1;
    }
    /* End: This is development usage only */    

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(nListenPort);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        printf("Listen socket(%d) can't bind\n", nListenPort);
        close(sfd);
        return -1;
    }

    if (listen(sfd, MAX_LISTEN_BACKLOG) != 0) {
        printf("Listen socket(%d) can't be listened\n", nListenPort);
        close(sfd);
        return -1;
    }

    m_sfdListen    = sfd;
    m_sfdData      = 0;
    m_bSecureMode  = bSecured;

    return sfd;
}


void gssSocketClass::closeSockets()
{
    if (m_sfdData > 0) {
        close(m_sfdData);
        m_sfdData = 0;
    }
    if (m_sfdListen > 0) {
        close(m_sfdListen);
        m_sfdListen = 0;
    }

    if(m_pSsl != NULL) {
        SSL_free(m_pSsl);
        m_pSsl = NULL;
    }

    m_bSecureMode = false;
}

int gssSocketClass::waitListenSocketAccept()
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int sfd;
    int sslRet, bufsize;

    if((sfd = accept(m_sfdListen, (struct sockaddr *)&client_addr, &client_len)) <= 0) {
        printf("DataSocket for TCP Error: %d\n", errno);
        return 0;
    }

    bufsize = SOCKET_BUF_RCV_SIZE;
    if(setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, (char *)&bufsize, sizeof(bufsize)) == -1) {
        close(sfd);
        return 0;
    }
    bufsize = SOCKET_BUF_SND_SIZE;
    if(setsockopt(sfd, SOL_SOCKET, SO_SNDBUF, (char *)&bufsize, sizeof(bufsize)) == -1) {
        close(sfd);
        return 0;
    }

    m_sfdData = sfd;

    if (m_bSecureMode == true) {
        m_pSsl = SSL_new(m_pSslContext);
        SSL_set_fd(m_pSsl, sfd);

        if((sslRet = SSL_accept(m_pSsl)) <= 0) {
            int sslErr = SSL_get_error(m_pSsl, sslRet);
            printf("TLS Connection failure: %d\n", sslErr);
            ERR_print_errors_fp(stderr);
            closeDataSocket();
            return 0;
        }
#if 0
        else {
            //printf("TLS connection ok\n");
            return(setNonBlocking(m_sfdData));
        }
    }
    else {
        //printf("TCP Connection ok\n");
        return(setNonBlocking(m_sfdData));
#endif
    }

    return 1;
}

int gssSocketClass::isReceived()
{
    if (m_sfdData == 0) return -1;

    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(m_sfdData, &read_set);

    struct timeval time_out;
    time_out.tv_sec = 0L;
    time_out.tv_usec = 10000L;

    int result = select(m_sfdData+1, &read_set, NULL, NULL, &time_out);
    if(result < 0) 
        return -1;
    else if(result == 0) 
        return 0;
    else 
        return(FD_ISSET(m_sfdData, &read_set));
}

int gssSocketClass::receiveData(unsigned char *recvBuf, const int nRecvLength)
{
    if(m_sfdData == 0) return (-1);
    if(nRecvLength < 0) return (-1);

    int recvBytes;

    if (m_bSecureMode == false) {
        for(int i = 0; i < nRecvLength; i += recvBytes) {
            if((recvBytes = recv(m_sfdData, (char *)(recvBuf+i), nRecvLength-i, 0)) <= 0) {
                closeDataSocket();
                return (-1);
            }
        }
    }
    else {
        for(int i = 0; i < nRecvLength; i += recvBytes) {
            if((recvBytes = SSL_read(m_pSsl, (char *)(recvBuf+i), nRecvLength-i)) <= 0) {
                closeDataSocket();
                return (-1);
            }
        }
    }

    return nRecvLength;

#if 0            
        int n = recv(m_sfdData, recvBuf, recvBufSize, 0);
        if(n <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return 0;
            }
            else {
                printf("TCP Reading Error: %d\n", errno);
                closeDataSocket();
                return -1;
            }
        }
        //printf("Received Length: %d\n", n);
    }
    else {
        int n = SSL_read(m_pSsl, recvBuf, recvBufSize);
        if (n <= 0) {
            int sslErr = SSL_get_error(m_pSsl, n);
            if(sslErr == SSL_ERROR_WANT_WRITE || sslErr == SSL_ERROR_WANT_READ) {
                return 0;
            }
            else {
                printf("TLS Reading Error: %d\n", sslErr);
                closeDataSocket();
                return -1;
            }
        }
        //printf("Recieved Length: %d\n", n);
    }

    return n;
#endif            
}

int gssSocketClass::sendData(const unsigned char *messageBuf, const int nSendLength)
{
    if(m_sfdData == 0) return (-1);
    if(nSendLength < 0) return (-1);

    int sendBytes;

    if (m_bSecureMode == false) {
        for(int i = 0; i < nSendLength; i += sendBytes) {
            if((sendBytes = send(m_sfdData, (char *)(messageBuf+i), nSendLength-i, 0)) <= 0) {
                closeDataSocket();
                return (-1);
            }
        }
    }
    else {
        for(int i = 0; i < nSendLength; i += sendBytes) {
            if((sendBytes = SSL_write(m_pSsl, (char *)(messageBuf+i), nSendLength-i)) <= 0) {
                closeDataSocket();
                return (-1);
            }
        }
    }

    return nSendLength;

#if 0
    int n;

    if (m_bSecureMode == false) {
        if((n = send(m_sfdData, (const void *)messageBuf, (size_t)messageSize, 0)) <= 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                return 0;
            }
            else {
                printf("TCP Sending Error: %d\n", errno);
                closeDataSocket();
                return -1;
            }
        }

        printf("TLS data sent: %d\n", n);
    }
    else {
        if((n = SSL_write(m_pSsl, messageBuf, messageSize)) <= 0) {
            int sslErr = SSL_get_error(m_pSsl, n);
            if(sslErr == SSL_ERROR_WANT_WRITE || sslErr == SSL_ERROR_WANT_READ) {
                return 0;
            }
            else {
                printf("TLS Sending Error: %d\n", sslErr);
                closeDataSocket();
                return -1;
            }
        }

        printf("TLS data sent: %d\n", n);
    }

    return n;
#endif    
}

void gssSocketClass::closeDataSocket()
{
    if(m_sfdData != 0) {
        //fcntl(m_sfdData, F_SETFL, 0);
        if (m_bSecureMode == false) {
            close(m_sfdData);
            m_sfdData = 0;
        }
        else {
            if(m_pSsl != NULL) {
                SSL_free(m_pSsl);
                m_pSsl = NULL;
            }
            close(m_sfdData);
            m_sfdData = 0;                
        }
    }
}

/*---------------------
 *  Usage of this code  (Non-blocking socket)
 *---------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gssSocket.hpp"
#include "sslManager.h"

#define LISTENPORT          5000
#define RECVBUF_SIZE        10000
#define SENDBUF_SIZE        10000

#define CERTIFICATE_FILENAME       "certificate/team2.server.crt"
#define PRIVKEY_FILENAME           "certificate/team2.server.key"



int main(void)
{
    gssSocketClass  gssSocket;
    sslManagerClass sslManager;
    std::string     sPrivateKeyFileName  = PRIVKEY_FILENAME;
    std::string     sCertificateFileName = CERTIFICATE_FILENAME;

    initOpenSSL();

    sslManager.setRole(true);       // Work as TLS server
    if(sslManager.loadKeyCertificate(sPrivateKeyFileName, sCertificateFileName) == 0) {
        printf("Security key and certificate loading failure\n");
        return 0;
    }
    else {
        printf("Security key and certificate loading Ok\n");
    }
    gssSocket.bindSSL(sslManager.getSslCtx());

    if(!gssSocket.initListenSockets(LISTENPORT, true)) {
    //if(!gssSocket.initListenSockets(LISTENPORT, false)) {
        printf("Listen socket initialization failure\n");
        return 0;
    }

    unsigned char recvBuf[RECVBUF_SIZE];
    unsigned char sendBuf[SENDBUF_SIZE];
    int recvBufLen = sizeof(recvBuf);
    int sendMessageLen;

    bool bTerminateLoop = false;
    while(!bTerminateLoop) {
        if(gssSocket.waitListenSocketAccept()) {
            printf("Connection accepted\n");

            bool bTerminateLoop2 = false;
            int sentCnt = 0;
            while(!bTerminateLoop2) {
                int n = gssSocket.receiveData(recvBuf, recvBufLen);
                switch (n) {
                case -1:
                    bTerminateLoop2 = true;
                    printf("Network Connection Failure\n");
                    break;
                case 0:
                    // This is not error. Something progressing now .. because it is non-blocking socket.
                    printf("Receive progressing...\n");
                    break;
                default:
                    // Get the message, and parse it.
                    printf("ReceivedMessage: %d(%s)\n", n, recvBuf);
                    strcpy((char *)sendBuf, "Nice to meet you\n");
                    gssSocket.sendData(sendBuf, strlen((char *)sendBuf));
                    sentCnt++;
                    break;
                }

                // After Authenticated is finished and Command type is decided, Send Stream data
                if(sentCnt > 0) {
                    sprintf((char*)sendBuf, "This is just stream:%d\n", sentCnt);
                    gssSocket.sendData(sendBuf, strlen((char *)sendBuf));
                }

                printf("Recv loop\n");
                sleep(1);
            }
        }

        printf("qweqweeq\n");
        sleep(1);
        //usleep(10);
    }

    return 1;
}
*---------------------*/

