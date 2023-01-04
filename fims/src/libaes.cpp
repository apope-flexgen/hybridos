/*
 * libAES.cpp
 *
 *  Created on: Apr 27, 2020
 *      Author: pwilshire
 */

/* OS Includes */
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
/* C Standard Library Dependencies */
#include <cerrno>
#include <cstdio>
/* C++ Standard Library Dependencies */
#include <string>
#include <iostream>
#include <fstream>

/* External Dependencies */
// #include <cjson/cJSON.h>

/* openssl Dependencies */
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include "fims.h"
#include "libfims.h"
#include "libaes.h"

using namespace std;

uint8_t* g_aesKey;

// TODO(WALKER): get phil to change this so we don't have to use cjson to get this
// just make key a basic text file or something (with permissions of course)

//////////////////////////////////////
void loadAesKey(const char* fname)
{
    char* key = NULL;
    /** Read file into string. */
    ifstream ifs ;
    size_t size = 0;

    // get pointer to associated buffer object
    filebuf *pbuf = ifs.rdbuf();
    pbuf->open(fname, ios::in|ios::binary);

    if (pbuf->is_open()) {
        // get file size using buffer's members
        size = pbuf->pubseekoff (0,ifs.end,ifs.in);
        pbuf->pubseekpos (0,ifs.in);

        char* buffer=(char*)malloc(size);

        // get file data
        pbuf->sgetn(buffer,size);

        // TODO(WALKER/PHIL): this should probably just be a string itself
        // the fact that this is a "json" string is dumb and pointless
        // and just introduces dependencies. This is insanely lazy and should never have been the case.
        // for now I'm going to change this, but if for some reason this has to remain this way
        // then I'm going to be really really mad at Phil for doing this for something so simple
        // Pray to God there is no AES encryption being used on any site anywhere.
        pbuf->close();
        if (buffer)
        {
            key = (char*)malloc(AESSIZE);
            memset(key,0,AESSIZE);
            snprintf(key , AESSIZE, "%s" , (char*) buffer);
            free(buffer);
        }
    }
    // if (key == NULL) {
    //     cout <<" Note: No Key in File : " << fname << endl;
    // }
    g_aesKey = (uint8_t*)key;
    return;
}

void aesHandleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}

int aesEncrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;

    int len;
    int outlen;
    int ciphertext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        aesHandleErrors();

    /*
     * Initialise the encryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        aesHandleErrors();

    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &outlen, plaintext, plaintext_len))
        aesHandleErrors();

    ciphertext_len = outlen;

    /*
     * Finalise the encryption. Further ciphertext bytes may be written at
     * this stage.
     */
    if(!EVP_EncryptFinal_ex(ctx, ciphertext + outlen, &len))
    {
        if(0)printf(" encrypt error len %d, outlen %d\n",len,outlen);
        aesHandleErrors();
    }
    if(0)printf(" encrypt len %d, outlen %d\n",len, outlen);

    ciphertext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

int aesDecrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx;

    int len;
    int outlen;
    int plaintext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        aesHandleErrors();

    /*
     * Initialise the decryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        aesHandleErrors();

    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary.
     */
    if(!EVP_DecryptUpdate(ctx, plaintext, &outlen, ciphertext, ciphertext_len))
        aesHandleErrors();

    plaintext_len = outlen;

    /*
     * Finalise the decryption. Further plaintext bytes may be written at
     * this stage.
     */
    if(!EVP_DecryptFinal_ex(ctx, plaintext + outlen, &len))
    {
        if(0)printf(" decrypt error len %d, outlen %d\n",len,outlen);
        aesHandleErrors();
    }
        
    plaintext_len += len;
    if(0)printf(" decrypt  final len %d, plaintext_len %d\n",len, plaintext_len);

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}
