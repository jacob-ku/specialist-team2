#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sslManager.h"

#define SSL_BUF_SIZE		128
const unsigned int sslManager_buf_size  = SSL_BUF_SIZE;
char sslManager_buf[SSL_BUF_SIZE]       = {0, };

int initOpenSSL()
{
    SSL_library_init();
    SSL_load_error_strings();
    //SSLeay_add_ssl_algorithms();
    //ERR_load_BIO_strings();
    return 0;
}

int password_callback(char *buf, int size, int rwflag, void *userdata)
{
    strncpy(buf, sslManager_buf, size);
    buf[size - 1] = '\0';
    memset(sslManager_buf, 0x00, sslManager_buf_size);
    return(strlen(buf));
}

sslManagerClass::sslManagerClass()
{
    m_pSSLCtx = NULL;
    m_pCert   = NULL;
}

sslManagerClass::~sslManagerClass()
{
    terminateOpenSSL();
}

int sslManagerClass::setRole(int bServer)
{
    m_pSSLCtx = (bServer)
        ? m_pSSLCtx = SSL_CTX_new(TLS_server_method())
        : m_pSSLCtx = SSL_CTX_new(TLS_client_method());

    if (!m_pSSLCtx) {
        ERR_print_errors_fp(stderr);
        printf("SSL_CTX allocation error\n");
        return 0;
    }

    pem_password_cb* callback = &password_callback;
    SSL_CTX_set_default_passwd_cb(m_pSSLCtx, callback);
    return 1;
}

int sslManagerClass::loadKeyCertificate()
{
    const char *pCertName = m_certificateFileName.c_str();
    const char *pKeyName  = m_privateKeyFileName.c_str();

    if (SSL_CTX_use_certificate_file(m_pSSLCtx, pCertName, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        printf("certificate loading error\n");
        return 0;
    }

    //SSL_CTX_use_PrivateKey_file(m_pSSLCtx, pCertName, SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(m_pSSLCtx, pKeyName, SSL_FILETYPE_PEM);
    if(!SSL_CTX_check_private_key(m_pSSLCtx)) {
        ERR_print_errors_fp(stderr);
        printf("private key doesn't match with certificate\n");
        return 0;
    }
    //printf("private key is ok\n");

    return 1;
}

SSL_CTX *sslManagerClass::getSslCtx()
{
    return m_pSSLCtx;
}

void sslManagerClass::terminateOpenSSL()
{
    //X509_free(g_pCert);
    if (m_pSSLCtx != NULL) SSL_CTX_free(m_pSSLCtx);
}
