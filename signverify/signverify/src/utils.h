#ifndef VERIFY_H_
#define VERIFY_H_

#include <stddef.h>

extern int verifySignature(void* publicKey, char *plainText, size_t plainTextlength, char* signatureBase64);

#endif /* VERIFY_H_ */
