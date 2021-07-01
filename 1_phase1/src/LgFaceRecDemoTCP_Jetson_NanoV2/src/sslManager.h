#ifndef  __SSL_MANAGER_H__
#define __SSL_MANAGER_H__

#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/pem.h>

extern const unsigned int sslManager_buf_size;
extern char sslManager_buf[];

extern int initOpenSSL();
extern void terminateOpenSSL();

#define CERTIFICATE_FILENAME       ("../certificate/server_cert.crt")
#define PRIVKEY_FILENAME           ("../certificate/server_priv.pem")
//#define CERTIFICATE_FILENAME       "../certificate/server.crt"
//#define PRIVKEY_FILENAME           "../certificate/server_priv_2048.pem"

class sslManagerClass {
    public:
        sslManagerClass();
        ~sslManagerClass();

    public:
        int     setRole(int bServer);
        int     loadKeyCertificate();
        void    terminateOpenSSL();
        SSL_CTX *getSslCtx();

    private:
        SSL_CTX     *m_pSSLCtx;
        SSL         *m_pSSL;
        X509        *m_pCert;

        const std::string m_privateKeyFileName  = PRIVKEY_FILENAME;
        const std::string m_certificateFileName = CERTIFICATE_FILENAME;
};

#endif
