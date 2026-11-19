#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/pem.h>
#include <stdexcept>

char* encryptSPK(const char* const AES, uint32_t AES_length, EVP_PKEY* PKEY, uint32_t* lenres){
	if (AES==nullptr)
		throw std::runtime_error("ERROR: AES NOT FOUND");
	if (AES_length==NULL)
		throw std::runtime_error("ERROR: AES_LENGTH NOT FOUND");
	if (PKEY==nullptr)
		throw std::runtime_error("ERROR: PKEY NOT FOUND");
	if (lenres==nullptr)
		throw std::runtime_error("ERROR: LENRES NOT FOUND");
	EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(PKEY, nullptr);
	if (ctx == nullptr || EVP_PKEY_encrypt_init(ctx)<=0){
		EVP_PKEY_CTX_free(ctx);
		throw std::runtime_error("ERROR: CTX FAILED");
	}
	size_t encrypted_length = 0;
	if (EVP_PKEY_encrypt(ctx, nullptr, &encrypted_length, AES, AES_length)<=0){
		EVP_PKEY_CTX_free(ctx);
		throw std::runtime_error("ERROR: ENCRYPTION INITIALIZATION FAILED");
	}
	unsigned char* encrypted_buffer = (unsigned char*) malloc(encrypted_length);
	if (encrypted_buffer==NULL){
		EVP_PKEY_CTX_free(ctx);
		throw std::runtime_error("ERROR: BUFFER FAILED");
	}
	if (EVP_PKEY_encrypt(ctx, encrypted_buffer, &encrypted_length, AES, AES_length)<=0){
		EVP_PKEY_CTX_free(ctx);
		free(encrypted_buffer);
		throw std::runtime_error("ERROR: ENCRYPTION FAILED");
	}
	EVP_PKEY_CTX_free(ctx);
	*lenres = encrypted_length;
	return (char*) encrypted_buffer;
}
char * unencryptSPK(const char* const data, uint32_t data_length, uint32_t* lenres){
	if (data==nullptr)
		throw std::runtime_error("ERROR: DATA NOT FOUND");
	if (data_length==NULL)
		throw std::runtime_error("ERROR DATA_LENGTH NOT FOUND");
	if (lenres==nullptr)
		throw std::runtime_error("ERROR: LENRES NOT FOUND");
	BIO* PEM = BIO_new_file(/*path*/, "r");
	if (PEM==NULL)
		throw std::runtime_error("ERROR: BIO NOT FOUND");
	EVP_PKEY* key = PEM_read_bio_PrivateKey(PEM, nullptr, nullptr, nullptr);
	if (key==NULL) {
		BIO_free(PEM);
		throw std::runtime_error("ERROR: KEY NOT FOUND");
	}
	BIO_free(PEM);
	EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(key, nullptr);
    if (ctx==NULL || EVP_PKEY_decrypt_init(ctx)<=0)
    {
		EVP_PKEY_CTX_free(ctx);
		EVP_PKEY_free(key);
        throw std::runtime_error("ERROR: CTX FAILED");
    }

	size_t decrypted_length = 0;
	if (EVP_PKEY_decrypt(ctx, nullptr, &decrypted_length, data, data_length)<=0){
		EVP_PKEY_free(key);
		EVP_PKEY_CTX_free(ctx);
		throw std::runtime_error("ERROR: DECRYPTION INITIALIZATION FAILED");
	}
	unsigned char* decrypted_buffer = (unsigned char*) malloc(decrypted_length);
	if (decrypted_buffer==NULL){
		EVP_PKEY_free(key);
		EVP_PKEY_CTX_free(ctx);
		throw std::runtime_error("ERROR: BUFFER FAILED");
	}
	if (EVP_PKEY_decrypt(ctx, decrypted_buffer, &decrypted_length, data, data_length)<=0){
		EVP_PKEY_free(key);
		EVP_PKEY_CTX_free(ctx);
		free(decrypted_buffer);
		throw std::runtime_error("ERROR: DECRYPTION FAILED");
	}
	EVP_PKEY_CTX_free(ctx);
	EVP_PKEY_free(key);
	*lenres = decrypted_length;
	return (char*) decrypted_buffer;
}
char * encryptSymmetric(const char * const data, uint32_t data_length, ENCVAL_SYMMETRIC_TEMP key, uint32_t * lenres){
	if (data==nullptr)
		throw std::runtime_error("ERROR: DATA NOT FOUND");
	if (data_length==NULL)
		throw std::runtime_error("ERROR: DATA_LENGTH NOT FOUND");
	if (key==NULL)
		throw std::runtime_error("ERROR: KEY NOT FOUND");
	if (lenres==nullptr)
		throw std::runtime_error("ERROR: LENRES NOT FOUND");
	unsigned char* iv = (unsigned char*)malloc(16);
	if (iv==nullptr){
		throw std::runtime_error("ERROR: IV MALLOC FAILED");
	}
	if (RAND_bytes(iv, 16)<=0){
		free(iv);
		throw std::runtime_error("ERROR: IV INITIALIZATION FAILED");
	}
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	if (ctx==nullptr){
		free(iv);
		throw std::runtime_error("ERROR: CTX FAILED");
	}
	if(EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)<=0){ //using AES-CBC algorithm for now but could bbenefit with GCM
		free(iv);
		EVP_CIPHER_CTX_free(ctx);
		throw std::runtime_error("ERROR: ENCRYPTION INITIALIZATION FAILED");
	}
	if (EVP_CIPHER_CTX_set_padding(ctx, 1)<=0) {
		free(iv);
		EVP_CIPHER_CTX_free(ctx);
		throw std::runtime_error("ERROR: PADDING INITIALIZATION FAILED");
	}
	int encrypted_length = 0;
	int final_length = 0;
	unsigned char* encrypted_buffer = (unsigned char*) malloc(16 + data_length + EVP_MAX_BLOCK_LENGTH);
	if (encrypted_buffer==NULL){
		free(iv);
		EVP_CIPHER_CTX_free(ctx);
		throw std::runtime_error("ERROR: BUFFFER FAILED");	
	}
	memcpy(encrypted_buffer, iv, 16);
	unsigned char* ciphertext = encrypted_buffer+16;
	if (EVP_EncryptUpdate(ctx, ciphertext, &encrypted_length, (const unsigned char*)data, data_length)<=0){
		free(iv);
		EVP_CIPHER_CTX_free(ctx);
		free(encrypted_buffer);
		throw std::runtime_error("ERROR: ENCRYPTION FAILED");
	}
	if (EVP_EncryptFinal_ex(ctx, ciphertext + encrypted_length, &final_length)<=0){
		free(iv);
		EVP_CIPHER_CTX_free(ctx);
		free(encrypted_buffer);
		throw std::runtime_error("ERROR: ENCRYPTION FINALIZATION FAILED");
	}
	*lenres = encrypted_length + final_length + 16;
	EVP_CIPHER_CTX_free(ctx);
	free(iv);
	return (char*) encrypted_buffer;
}
char* decryptSymmetric(const char* const encrypted_data, uint32_t encrypted_data_length, ENCVAL_SYMMETRIC_TEMP key, uint32_t* lenres){
	if (encrypted_data==nullptr)
		throw std::runtime_error("ERROR: ENCRYPTED_DATA NOT FOUND");
	if (encrypted_data_length==NULL)
		throw std::runtime_error("ERROR: ENCRYPTED_DATA_LENGTH NOT FOUND");
	if (key==NULL)
		throw std::runtime_error("ERROR: KEY NOT FOUND");
	if (lenres==nullptr)
		throw std::runtime_error("ERROR: LENRES NOT FOUND");
	unsigned char* iv = (unsigned char*) malloc(16);
	if (iv==nullptr)
		throw std::runtime_error("ERROR: IV MALLOC FAILED");
	memcpy(iv, encrypted_data, 16);
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	if (ctx==nullptr){
		free(iv);
		throw std::runtime_error("ERROR: CTX FAILED");
	}
	if(EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)<=0){
		free(iv);
		EVP_CIPHER_CTX_free(ctx);
		throw std::runtime_error("ERROR: DECRYPTION INITIALIZATION FAILED");
	}
	int decrypted_length = 0;
	int final_length = 0;
	unsigned char* decrypted_buffer = (unsigned char*) malloc(encrypted_data_length);
	if (decrypted_buffer==NULL){
		free(iv);
		EVP_CIPHER_CTX_free(ctx);
		throw std::runtime_error("ERROR: BUFFFER FAILED");
	}
	if (EVP_DecryptUpdate(ctx, decrypted_buffer, &decrypted_length, (const unsigned char*)encrypted_data+16, encrypted_data_length-16)<=0){
		free(iv);
		EVP_CIPHER_CTX_free(ctx);
		free(decrypted_buffer);
		throw std::runtime_error("ERROR: DECRYPTION FAILED");
	}
	if (EVP_DecryptFinal_ex(ctx, decrypted_buffer + decrypted_length, &final_length)<=0){
		free(iv);
		EVP_CIPHER_CTX_free(ctx);
		free(decrypted_buffer);
		throw std::runtime_error("ERROR: DECRYPTION FINALIZATION FAILED");
	}
	*lenres = decrypted_length + final_length;
	EVP_CIPHER_CTX_free(ctx);
	free(iv);
	return (char*) decrypted_buffer;
}