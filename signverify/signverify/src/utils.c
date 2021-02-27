
#include "utils.h"

#include <string.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>



static RSA* createPublicRSA(void *key);
static int RSAVerifySignature(
						 RSA* rsa,
						 unsigned char* MsgHash,
						 unsigned int MsgHashLen,
						 const char* Msg,
						 unsigned int MsgLen,
						 int* Authentic);
static size_t calcDecodeLength(const char* b64input);
static void Base64Decode(char* b64message, unsigned char** buffer, unsigned int* length);



static RSA* createPublicRSA(void *key)
{
	RSA *rsa = NULL;
	BIO *keybio;
	keybio = BIO_new_mem_buf(key, -1);
	if (keybio==NULL) {
	  return 0;
	}
	rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa,NULL, NULL);
	return rsa;
}



static int RSAVerifySignature(
						 RSA* rsa,
						 unsigned char* MsgHash,
						 unsigned int MsgHashLen,
						 const char* Msg,
						 unsigned int MsgLen,
						 int* Authentic)
{
	*Authentic = 0;
	EVP_PKEY* pubKey  = EVP_PKEY_new();
	EVP_PKEY_assign_RSA(pubKey, rsa);
	EVP_MD_CTX* m_RSAVerifyCtx;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
	m_RSAVerifyCtx = EVP_MD_CTX_create();
#else
	m_RSAVerifyCtx = EVP_MD_CTX_new();
#endif

	if (EVP_DigestVerifyInit(m_RSAVerifyCtx,NULL, EVP_sha256(),NULL,pubKey)<=0) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
 		EVP_MD_CTX_cleanup(m_RSAVerifyCtx);
#else
		EVP_MD_CTX_free(m_RSAVerifyCtx);
#endif
		return 1;
	}
	if (EVP_DigestVerifyUpdate(m_RSAVerifyCtx, Msg, MsgLen) <= 0) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
 		EVP_MD_CTX_cleanup(m_RSAVerifyCtx);
#else
		EVP_MD_CTX_free(m_RSAVerifyCtx);
#endif
		return 2;
	}
	int AuthStatus = EVP_DigestVerifyFinal(m_RSAVerifyCtx, MsgHash, MsgHashLen);
	if (AuthStatus==1) {
		*Authentic = 1;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
 		EVP_MD_CTX_cleanup(m_RSAVerifyCtx);
#else
		EVP_MD_CTX_free(m_RSAVerifyCtx);
#endif 
		return 0;
	} else if(AuthStatus==0){
#if OPENSSL_VERSION_NUMBER < 0x10100000L
 		EVP_MD_CTX_cleanup(m_RSAVerifyCtx);
#else
		EVP_MD_CTX_free(m_RSAVerifyCtx);
#endif
		return 3;
	} else{
#if OPENSSL_VERSION_NUMBER < 0x10100000L
 		EVP_MD_CTX_cleanup(m_RSAVerifyCtx);
#else
		EVP_MD_CTX_free(m_RSAVerifyCtx);
#endif
		return 4;
	}
}



static size_t calcDecodeLength(const char* b64input)
{
	size_t len = strlen(b64input), padding = 0;

	if (b64input[len-1] == '=' && b64input[len-2] == '=') //last two chars are =
		padding = 2;
	else if (b64input[len-1] == '=') //last char is =
		padding = 1;

	return (len*3)/4 - padding;
}



static void Base64Decode(char* b64message, unsigned char** buffer, unsigned int* length)
{
	BIO *bio, *b64;

	int decodeLen = calcDecodeLength(b64message);
	*buffer = (unsigned char*)malloc(decodeLen + 1);
	(*buffer)[decodeLen] = '\0';

	bio = BIO_new_mem_buf(b64message, -1);
	b64 = BIO_new(BIO_f_base64());
	bio = BIO_push(b64, bio);

	*length = BIO_read(bio, *buffer, strlen(b64message));
	BIO_free_all(bio);
}



int verifySignature(void* publicKey, char *plainText, size_t plainTextlength, char* signatureBase64)
{
	RSA* publicRSA = createPublicRSA(publicKey);
	unsigned char* encMessage;
	unsigned int encMessageLength;
	int authentic;
	Base64Decode(signatureBase64, &encMessage, &encMessageLength);
	int result = RSAVerifySignature(publicRSA, encMessage, encMessageLength, plainText, plainTextlength, &authentic);
	free(encMessage);
	RSA_free(publicRSA);
	return result;
}
