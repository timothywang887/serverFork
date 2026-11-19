#ifndef H_ENCRYPTION
#define H_ENCRYPTION
typedef ENCRYPTION_SYMMETRIC_KEY char[32];
typedef ENCRYPTION_ASYMMETRIC_KEY char[256];
typedef ENCRYPTION_UNIX_TIMESTAMP_IP_SIGNED char[256]; //8 bytes plus a signature. Length is temporary.
typedef ENCRYPTION_UNIX_TIMESTAMP_IP_ENCRYPTED_SYMMETRIC char[256]; //8 bytes, encrypted with symmetric key. Length is temporary.
typedef ENCRYPTION_USERNAME_PASSWORD_UNIX_TIMESTAMP_IP_SYMMETRIC_KEY_ENCRYPTED_ASYMMETRIC char[296]; //128 bytes + 128 bytes + 8 bytes + 32 bytes, encrypted with an asymmetric key. Length is temporary.
char * unencryptSPK(const char* const data, uint32_t data_length, uint32_t* lenres);

//char* encryptSPK(const char* const AES, uint32_t AES_length, EVP_PKEY* PKEY, uint32_t* lenres);

char * encryptSymmetric(const char * const data, uint32_t data_length, ENCVAL_SYMMETRIC_KEY key, uint32_t * lenres);
char* decryptSymmetric(const char* const encrypted_data, uint32_t encrypted_data_length, ENCVAL_SYMMETRIC_KEY key, uint32_t* lenres);

#endif
