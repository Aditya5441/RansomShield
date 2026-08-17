/* crypto.h */
#ifndef INC_CRYPTO_H_
#define INC_CRYPTO_H_

#include <stdint.h>

void CRYPTO_AES_Decrypt(uint8_t *in, uint8_t *key, uint8_t *out);
void CRYPTO_SHA256(const uint8_t *data, uint32_t len, uint8_t *hash);

#endif /* INC_CRYPTO_H_ */

