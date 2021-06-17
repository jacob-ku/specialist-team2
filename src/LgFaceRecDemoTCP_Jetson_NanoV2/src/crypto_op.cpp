#include "crypto_op.h"

/* A 256 bit key */
//unsigned char *key = (unsigned char *)"01234567890123456789012345678901";
unsigned char key[AES256_KEY_SIZE] = {0, };

int get_random_bytes(const int bytes, unsigned char* buf)
{
    int i = 0;
    if (buf == NULL)
    {
        printf("Invalid parameter....\n");
        return -1;
    }

    if (RAND_bytes(buf, bytes) != 1)
        return -1;

    return 0;
}

int hex_to_hexstr(unsigned char* ciphertext, int ciphertext_len, char* out)
{
    int i = 0;

    if (ciphertext == NULL || out == NULL)
    {
        printf("Invalid parameter....\n");
        return -1;
    }

    for (i = 0; i < ciphertext_len; i++)
    {
        snprintf(out + i*2, 3, "%02x", ciphertext[i]);
    }

    return 0;
}

int hexstr_to_array(char* str, int str_len, unsigned char* out)
{
    int i = 0;

    if (str == NULL || out == NULL)
    {
        printf("Invalid parameter....\n");
        return -1;
    }

    for (i = 0; i < (int)(str_len/2); i++)
    {
        char temp[3] = {0, };
        memcpy(temp, str + i*2, 2);
        out[i] = (unsigned char) strtoul(temp, NULL, 16);
    }

    return 0;
}

inline unsigned int get_expected_ciphertext_len(unsigned int input_size)
{
    return (input_size + (AES256_BLOCK_SIZE - (input_size % AES256_BLOCK_SIZE)));
}

bool encrypt_data(const unsigned char* data, const unsigned int data_size,
                        const char* dir_path, const unsigned int dir_path_len,
                        const char* filename, const unsigned int filename_len)
{
    unsigned int des_len = 0;
    FILE *des_fptr = NULL;

    unsigned int des_path_len = 0;
    char* des_path = NULL;

    unsigned int des_filename_len = 0;
    char* des_filename = NULL;

    unsigned int expected_ciphertext_len = 0;

    int encrypted_file_len = 0;
    unsigned char* encrypted_file_buf = NULL;

    int encrypted_filename_len = 0;
    unsigned char encrypted_filename_buf[128] = {0, };

    bool result = false;

    if (data == NULL || dir_path == NULL || filename == NULL
        || dir_path_len == 0 || filename_len == 0 || data_size == 0)
    {
        printf("Invalid parameter....\n");
        return false;
    }

    /* A 128 bit IV */
    unsigned char iv_rand[AESCBC_IV_LENGTH] = {0, };

    if (get_random_bytes(AESCBC_IV_LENGTH, iv_rand))
    {
        printf("Fail to get random numbers....\n");
        goto exit;
    }

    // printf("random iv(%d) is:\n", AESCBC_IV_LENGTH);
    // BIO_dump_fp (stdout, (const char *)iv_rand, AESCBC_IV_LENGTH);


    /* [Start] Encrypt file */
    // printf("Data(%u) is:\n", data_size);
    // BIO_dump_fp (stdout, (const char *)data, data_size);

    expected_ciphertext_len = get_expected_ciphertext_len(data_size);
    encrypted_file_buf = (unsigned char*) calloc(expected_ciphertext_len, sizeof(char));
    if (encrypted_file_buf == NULL)
    {
        printf("Fail to alloc memroy....\n");
        goto exit;
    }

    encrypted_file_len = encrypt((unsigned char*) data, data_size, key, iv_rand, encrypted_file_buf);
    if (encrypted_file_len <= 0 || expected_ciphertext_len != encrypted_file_len)
    {
        printf("Fail to encrypt data....\n");
        goto exit;
    }

    // printf("Ciphertext(%d) is:\n", encrypted_file_len);
    // BIO_dump_fp (stdout, (const char *)encrypted_file_buf, encrypted_file_len);
    /* [End] Encrypt file */


    /* [Start] Encrypt filename */
    // printf("strlen(filename) : %u, filename : %s\n", filename_len, filename);

    expected_ciphertext_len = get_expected_ciphertext_len(filename_len);
    encrypted_filename_len = encrypt((unsigned char*) filename, filename_len, key, iv_rand, encrypted_filename_buf);
    if (encrypted_filename_len <= 0 || expected_ciphertext_len != encrypted_filename_len)
    {
        printf("Fail to encrypt filename....\n");
        goto exit;
    }

    // printf("Ciphertext(%d) is:\n", encrypted_filename_len);
    // BIO_dump_fp (stdout, (const char *)encrypted_filename_buf, encrypted_filename_len);


    des_filename_len = encrypted_filename_len * 2;
    des_filename = (char*) calloc(des_filename_len + 1, sizeof(char));
    if (des_filename == NULL)
    {
        printf("Fail to alloc memroy....\n");
        goto exit;
    }

    if (hex_to_hexstr(encrypted_filename_buf, encrypted_filename_len, des_filename))
    {
        printf("Fail to convert....\n");
        goto exit;
    }

    // printf("des_filename : %s\n", des_filename);
    /* [End] Encrypt filename */

    /* [Start] Store ciphertext as file */
    des_path_len = dir_path_len + des_filename_len;
    des_path = (char*) calloc(des_path_len + 1, sizeof(char));
    if (des_path == NULL)
    {
        printf("Fail to alloc memroy....\n");
        goto exit;
    }

    strncpy(des_path, dir_path, dir_path_len);
    strncat(des_path, des_filename, des_filename_len);
    // printf("des_path : %s\n", des_path);

    des_fptr = fopen(des_path, "wb");

    if (des_fptr == NULL) {
        printf("Fail to open des file....\n");
        goto exit;
    }

    fwrite(iv_rand, sizeof(char), AESCBC_IV_LENGTH, des_fptr);
    fwrite(encrypted_file_buf, sizeof(char), encrypted_file_len, des_fptr);
    /* [End] Store as file */

    result = true;

exit:
    if (des_fptr != NULL)               fclose(des_fptr);
    if (des_path != NULL)               free(des_path);
    if (des_filename != NULL)           free(des_filename);
    if (encrypted_file_buf != NULL)     free(encrypted_file_buf);
    return result;
}


bool encrypt_file(const char* dir_path, const unsigned int dir_path_len,
                const char* filename, const unsigned int filename_len)
{
    FILE *src_fptr = NULL;
    FILE *des_fptr = NULL;

    int src_len = 0;
    int des_len = 0;

    unsigned int src_path_len = 0;
    char* src_path = NULL;

    unsigned int des_path_len = 0;
    char* des_path = NULL;

    unsigned int des_filename_len = 0;
    char* des_filename = NULL;

    unsigned int read_bytes = 0;
    // unsigned char file_buf[1024*1024] = {0, };
    unsigned char* file_buf = NULL;

    unsigned int expected_ciphertext_len = 0;

    int encrypted_file_len = 0;
    unsigned char* encrypted_file_buf = NULL;

    int encrypted_filename_len = 0;
    unsigned char encrypted_filename_buf[128] = {0, };

    bool result = false;

    if (dir_path == NULL || filename == NULL || dir_path_len == 0 || filename_len == 0)
    {
        printf("Invalid parameter....\n");
        return false;
    }

    /* A 128 bit IV */
    unsigned char iv_rand[AESCBC_IV_LENGTH] = {0, };

    if (get_random_bytes(AESCBC_IV_LENGTH, iv_rand))
    {
        printf("Fail to get random numbers....\n");
        goto exit;
    }

    // printf("random iv(%d) is:\n", AESCBC_IV_LENGTH);
    // BIO_dump_fp (stdout, (const char *)iv_rand, AESCBC_IV_LENGTH);

    src_path_len = dir_path_len + filename_len + 1;
    src_path = (char*) calloc(src_path_len, sizeof(char));
    if (src_path == NULL)
    {
        printf("Fail to alloc memroy....\n");
        goto exit;
    }

    strncpy(src_path, dir_path, dir_path_len);
    strncat(src_path, filename, filename_len);
    // printf("src_path : %s\n", src_path);

    /* [Start] Encrypt file */
    src_fptr = fopen(src_path, "rb");
    if (src_fptr == NULL)
    {
        printf("Fail to open source file....\n");
        goto exit;
    }

    fseek(src_fptr, 0, SEEK_END);
    src_len = ftell(src_fptr);
    fseek(src_fptr, 0, SEEK_SET);
    if (src_len >= 1024*1024 || src_len <= 0)
    {
        printf("The size of file isn't acceptable....\n");
        goto exit;
    }

    // printf("src_len : %d\n", src_len);
    file_buf = (unsigned char*) calloc(src_len, sizeof(char));
    if (file_buf == NULL)
    {
        printf("Fail to alloc memroy....\n");
        goto exit;
    }

    do {
        int bytes = 0;
        bytes = fread(file_buf + read_bytes, sizeof(char), src_len - read_bytes, src_fptr);
        read_bytes += bytes;
    } while (read_bytes < src_len);

    // printf("Read file(%d) is:\n", read_bytes);
    // BIO_dump_fp (stdout, (const char *)file_buf, read_bytes);

    expected_ciphertext_len = get_expected_ciphertext_len(read_bytes);
    encrypted_file_buf = (unsigned char*) calloc(expected_ciphertext_len, sizeof(char));
    if (encrypted_file_buf == NULL)
    {
        printf("Fail to alloc memroy....\n");
        goto exit;
    }

    encrypted_file_len = encrypt((unsigned char*) file_buf, read_bytes, key, iv_rand, encrypted_file_buf);
    if (encrypted_file_len <= 0 || expected_ciphertext_len != encrypted_file_len)
    {
        printf("Fail to encrypt data....\n");
        goto exit;
    }

    // printf("Ciphertext(%d) is:\n", encrypted_file_len);
    // BIO_dump_fp (stdout, (const char *)encrypted_file_buf, encrypted_file_len);
    /* [End] Encrypt file */


    /* [Start] Encrypt filename */
    // printf("filename_len : %d, filename : %s\n", filename_len, filename);
    expected_ciphertext_len = get_expected_ciphertext_len(filename_len);

    encrypted_filename_len = encrypt((unsigned char*) filename, filename_len, key, iv_rand, encrypted_filename_buf);
    if (encrypted_filename_len <= 0 || expected_ciphertext_len != encrypted_filename_len)
    {
        printf("Fail to encrypt data....\n");
        goto exit;
    }

    // printf("Ciphertext(%d) is:\n", encrypted_filename_len);
    // BIO_dump_fp (stdout, (const char *)encrypted_filename_buf, encrypted_filename_len);


    des_filename_len = encrypted_filename_len * 2;
    des_filename = (char*) calloc(des_filename_len + 1, sizeof(char));
    if (des_filename == NULL)
    {
        printf("Fail to alloc memroy....\n");
        goto exit;
    }

    if (hex_to_hexstr(encrypted_filename_buf, encrypted_filename_len, des_filename))
    {
        printf("Fail to convert....\n");
        goto exit;
    }

    // printf("des_filename : %s\n", des_filename);
    /* [End] Encrypt filename */

    /* [Start] Store ciphertext as file */
    des_path_len = dir_path_len + des_filename_len;
    des_path = (char*) calloc(des_path_len + 1, sizeof(char));
    if (des_path == NULL)
    {
        printf("Fail to alloc memroy....\n");
        goto exit;
    }

    strncpy(des_path, dir_path, dir_path_len);
    strncat(des_path, des_filename, des_filename_len);
    // printf("des_path : %s\n", des_path);

    des_fptr = fopen(des_path, "wb");

    if (des_fptr == NULL) {
        printf("Fail to open des file....\n");
        goto exit;
    }

    fwrite(iv_rand, sizeof(char), AESCBC_IV_LENGTH, des_fptr);
    fwrite(encrypted_file_buf, sizeof(char), encrypted_file_len, des_fptr);
    /* [End] Store as file */

    result = true;

exit:
    if (src_fptr != NULL)               fclose(src_fptr);
    if (des_fptr != NULL)               fclose(des_fptr);
    if (src_path != NULL)               free(src_path);
    if (des_path != NULL)               free(des_path);
    if (des_filename != NULL)           free(des_filename);
    if (file_buf != NULL)               free(file_buf);
    if (encrypted_file_buf != NULL)     free(encrypted_file_buf);
    return result;
}

/*
    img_buf : Must be delivered with proper space.
*/
bool decrypt_file(const char* src_path, unsigned char* img_buf, unsigned int* img_size, char* name_buf)
{
    int src_len = 0;
    FILE *src_fptr = NULL;

    unsigned int read_bytes = 0;
    // unsigned char file_buf[1024*1024] = {0, };
    unsigned char* file_buf = NULL;

    /* A 128 bit IV */
    unsigned char iv[AESCBC_IV_LENGTH] = {0, };

    int decryptedtext_len = 0;
    unsigned char* decrypted_buf = NULL;

    unsigned int filename_len = 0;
    char* filename = NULL;

    unsigned int encrypted_filename_len = 0;
    unsigned char encrypted_filename_buf[128] = {0, };

    int decrypted_filename_len = 0;
    unsigned char decrypted_filename_buf[128] = {0, };

    bool result = false;

    if (src_path == NULL || img_buf == NULL || img_size == NULL || name_buf == NULL)
    {
        printf("Invalid parameter....\n");
        return false;
    }

    // printf("src_path : %s\n", src_path);

    /* [Start] Decrypt file */
    src_fptr = fopen(src_path, "rb");
    if (src_fptr == NULL)
    {
        printf("Fail to open source file....\n");
        goto exit;
    }

    fseek(src_fptr, 0, SEEK_END);
    src_len = ftell(src_fptr);
    fseek(src_fptr, 0, SEEK_SET);

    if (src_len >= 1024*1024 || src_len <= 0)
    {
        printf("The size of file isn't acceptable....\n");
        goto exit;
    }

    // printf("src_len : %d, src_path : %s\n", src_len, src_path);
    file_buf = (unsigned char*) calloc(src_len, sizeof(char));
    if (file_buf == NULL)
    {
        printf("Fail to alloc memroy....\n");
        goto exit;
    }

    do {
        unsigned int bytes = 0;
        bytes = fread(file_buf + read_bytes, sizeof(char), src_len - read_bytes, src_fptr);
        read_bytes += bytes;
    } while (read_bytes < src_len);

    // printf("Read file(%d) is:\n", read_bytes);
    // BIO_dump_fp (stdout, (const char *)file_buf, read_bytes);

    memcpy(iv, file_buf, AESCBC_IV_LENGTH);
    // printf("iv(%d) is:\n", AESCBC_IV_LENGTH);
    // BIO_dump_fp (stdout, (const char *)iv, AESCBC_IV_LENGTH);

    decrypted_buf = (unsigned char*) calloc(read_bytes - AESCBC_IV_LENGTH, sizeof(char));
    if (decrypted_buf == NULL)
    {
        printf("Fail to alloc memroy....\n");
        goto exit;
    }

    /* Decrypt the ciphertext */
    decryptedtext_len = decrypt(file_buf + AESCBC_IV_LENGTH, read_bytes - AESCBC_IV_LENGTH, key, iv, decrypted_buf);
    if (decryptedtext_len <= 0)
    {
        printf("Fail to decrypt data ....\n");
        goto exit;
    }

    // printf("Decrypted(%d) is:\n", decryptedtext_len);
    //BIO_dump_fp (stdout, (const char *)decrypted_buf, decryptedtext_len);

    *img_size = decryptedtext_len;
    memcpy(img_buf, decrypted_buf, decryptedtext_len);

    /* [End] Decrypt file */

    /* [Start] Decrypt filename */
    filename = strrchr((char*)src_path, '/') + 1;
    filename_len = strlen(filename);

    // printf("filename_len : %d, filename : %s\n", filename_len, filename);

    encrypted_filename_len = (int)(filename_len / 2);
    if (hexstr_to_array(filename, filename_len, encrypted_filename_buf))
    {
        printf("Fail to convert encrypted filename....\n");
        goto exit;
    }

    // BIO_dump_fp (stdout, (const char *)encrypted_filename_buf, 128);

    decrypted_filename_len = decrypt(encrypted_filename_buf, encrypted_filename_len, key, iv, decrypted_filename_buf);
    if (decrypted_filename_len <= 0)
    {
        printf("Fail to decrypt data ....\n");
        goto exit;
    }

    decrypted_filename_buf[decrypted_filename_len] = '\0';

    // printf("Decrypted(%d) is : %s\n", decrypted_filename_len, decrypted_filename_buf);

    strncpy(name_buf, (char*)decrypted_filename_buf, decrypted_filename_len);
    // name_buf[decrypted_filename_len] = '\0';
    /* [End] Decrypt filename */

    result = true;

exit:
    if (file_buf != NULL)           free(file_buf);
    if (src_fptr != NULL)           fclose(src_fptr);
    if (decrypted_buf != NULL)      free(decrypted_buf);
    return result;
}

void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    // abort();
}

int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int ciphertext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
    {
        handleErrors();
        return -1;
    }

    /*
     * Initialise the encryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    {
        handleErrors();
        return -1;
    }

    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    {
        handleErrors();
        return -1;
    }
    ciphertext_len = len;

    /*
     * Finalise the encryption. Further ciphertext bytes may be written at
     * this stage.
     */
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
    {
        handleErrors();
        return -1;
    }
    ciphertext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int plaintext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
    {
        handleErrors();
        return -1;
    }

    /*
     * Initialise the decryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    {
        handleErrors();
        return -1;
    }

    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary.
     */
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
    {
        handleErrors();
        return -1;
    }
    plaintext_len = len;

    /*
     * Finalise the decryption. Further plaintext bytes may be written at
     * this stage.
     */
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
    {
        handleErrors();
        return -1;
    }
    plaintext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}

void pbkdf2_hmac_sha_512(const char* pass, int passlen,
                                    const unsigned char* salt, int saltlen, unsigned int iterations,
                                    unsigned int outputBytes, char* hexResult, uint8_t* binResult)
{
    unsigned int i;
    unsigned char digest[outputBytes];

    if (pass == NULL || salt == NULL || hexResult == NULL || binResult == NULL)
        return;

    PKCS5_PBKDF2_HMAC(pass, passlen, salt, saltlen, iterations, EVP_sha512(), outputBytes, digest);
    for (i = 0; i < sizeof(digest); i++)
    {
        sprintf(hexResult + (i * 2), "%02x", 255 & digest[i]);
        binResult[i] = digest[i];
    };
}

bool kdf_for_aes(const char* pass, const unsigned int pass_len)
{
    const unsigned char salt[PBKDF2_SALT_LEN] = {0xC0, 0x62, 0x9F, 0x6C, 0xA4, 0x69, 0x43, 0xDE,
                                                 0x2B, 0xA0, 0xBA, 0xE9, 0x39, 0xE8, 0x6D, 0x44};

    char out_hexstr[AES256_KEY_SIZE * 2 + 1] = {0, };

    if (pass == NULL || pass_len == 0)
    {
        printf("Invalid parameter....\n");
        return false;
    }

    pbkdf2_hmac_sha_512(pass, pass_len, salt, PBKDF2_SALT_LEN, PBKDF2_ITERTATION, AES256_KEY_SIZE, out_hexstr, key);
//    printf("key_buf(%d) is:\n", AES256_KEY_SIZE);
//    BIO_dump_fp (stdout, (const char *)key, AES256_KEY_SIZE);

//    printf("out_hexstr: %s\n", out_hexstr);

    return true;
}

bool crypto_op_init(const char* passphrase, const unsigned int pass_len)
{
    return kdf_for_aes(passphrase, pass_len);
}

