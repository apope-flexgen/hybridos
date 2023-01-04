#ifndef _LIBAES_HPP_
#define _LIBAES_HPP_

void loadAesKey(const char *fname);

int aesEncrypt(unsigned char *plaintext, int plaintext_len,
	       unsigned char *key,
	       unsigned char *iv, unsigned char *ciphertext);

int aesDecrypt(unsigned char *ciphertext, int ciphertext_len,
	       unsigned char *key,
	       unsigned char *iv, unsigned char *plaintext);
#endif //_LIBAES_HPP_
