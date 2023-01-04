#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <stdarg.h>
#include "gen_time.h"

int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext);

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext);

int main (int argc, char *argv[])
{
    int bsize = 512;
    if (argc > 1) {
      bsize = atoi(argv[1]);
      if ((bsize < 64) || (bsize > 2048))
          bsize = 512;
    }
    /*
     * Set up the key and iv. Do I need to say to not hard code these in a
     * real application? :-)
     */

    /* A 256 bit key */
    unsigned char *key = (unsigned char *)"01234567890123456789012345678901";

    /* A 128 bit IV */
    unsigned char *iv = (unsigned char *)"0123456789012345";

    /* Message to be encrypted */
    unsigned char *plaintext = (unsigned char *)malloc(bsize);
    unsigned char *ciphertext = (unsigned char *)malloc(bsize*2);
    unsigned char *decryptedtext = (unsigned char *)malloc(bsize*2);


    //    (unsigned char *)"The quick brown fox jumps over the lazy dog";

    /*
     * Buffer for ciphertext. Ensure the buffer is long enough for the
     * ciphertext which may be longer than the plaintext, depending on the
     * algorithm and mode.
     */
    //unsigned char ciphertext[128];

    /* Buffer for the decrypted text */
    //unsigned char decryptedtext[128];
    for ( int i = 0 ; i < bsize; i++) {
      plaintext[i] = i % 254;
    }
    int decryptedtext_len, ciphertext_len;
    int rep;
    double stime=gen_ltime(1);
    for (rep = 0 ; rep < 1024; rep++) {

    /* Encrypt the plaintext */
    ciphertext_len = 
	encrypt (plaintext, bsize, key, iv,
                              ciphertext);

    /* Do something useful with the ciphertext here */
    //printf("Ciphertext is:\n");
    //BIO_dump_fp (stdout, (const char *)ciphertext, ciphertext_len);

    /* Decrypt the ciphertext */
    decryptedtext_len = decrypt(ciphertext, ciphertext_len, key, iv,
                                decryptedtext);

    /* Add a NULL terminator. We are expecting printable text */
    decryptedtext[decryptedtext_len] = '\0';

    /* Show the decrypted text */
    //printf("Decrypted text is:\n");

    //printf("%s\n", decryptedtext);
   }
    stime=(gen_ltime(1) - stime);
    printf("%s %2.3f mS length %d\n", " 1024 total cycles time ",
     stime*1000.0, bsize);
    printf("ciphertext_len %d decryptedtext_len %d\n", 
	ciphertext_len, decryptedtext_len);


    return 0;
}

void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int ciphertext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    /*
     * Initialise the encryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();

    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handleErrors();
    ciphertext_len = len;

    /*
     * Finalise the encryption. Further ciphertext bytes may be written at
     * this stage.
     */
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        handleErrors();
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
        handleErrors();

    /*
     * Initialise the decryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();

    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary.
     */
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handleErrors();
    plaintext_len = len;

    /*
     * Finalise the decryption. Further plaintext bytes may be written at
     * this stage.
     */
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        handleErrors();
    plaintext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}
/*
Ciphertext is:
0000 - e0 6f 63 a7 11 e8 b7 aa-9f 94 40 10 7d 46 80 a1   .oc.......@.}F..
0010 - 17 99 43 80 ea 31 d2 a2-99 b9 53 02 d4 39 b9 70   ..C..1....S..9.p
0020 - 2c 8e 65 a9 92 36 ec 92-07 04 91 5c f1 a9 8a 44   ,.e..6.....\...D
Decrypted text is:
The quick brown fox jumps over the lazy dog
*/

