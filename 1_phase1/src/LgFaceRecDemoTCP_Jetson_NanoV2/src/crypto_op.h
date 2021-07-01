//------------------------------------------------------------------------------------------------
// File: crypto_op.h
// Project: LG Security Specialist Program
// Versions:
// 1.0 June 2021 - initial version
//------------------------------------------------------------------------------------------------
#ifndef __CRYPTOOPH__
#define __CRYPTOOPH__

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

//------------------------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------------------------
#define AES256_BLOCK_SIZE       (16)
#define AESCBC_IV_LENGTH        (16)
#define AES256_KEY_SIZE         (32)

#define PBKDF2_SALT_LEN         (16)
#define PBKDF2_ITERTATION       (5000)

//------------------------------------------------------------------------------------------------
//  Function Prototypes
//------------------------------------------------------------------------------------------------
bool encrypt_data(const unsigned char* data, const unsigned int data_size,
                        const char* dir_path, const unsigned int dir_path_len,
                        const char* filename, const unsigned int filename_len);

bool encrypt_file(const char* dir_path, const unsigned int dir_path_len,
                const char* filename, const unsigned int filename_len);
bool decrypt_file(const char* src_path, unsigned char* img_buf, unsigned int* img_size, char* name_buf);
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext);
int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext);

void test_pbkdf2(const char* pass, const unsigned int pass_len);
void pbkdf2_hmac_sha_512(const char* pass, int passlen,
                                    const unsigned char* salt, int saltlen, unsigned int iterations,
                                    unsigned int outputBytes, char* hexResult, uint8_t* binResult);
bool crypto_op_init(const char* passphrase, const unsigned int pass_len);
#endif //__CRYPTOOPH__
//------------------------------------------------------------------------------------------------
//END of Include
//------------------------------------------------------------------------------------------------

