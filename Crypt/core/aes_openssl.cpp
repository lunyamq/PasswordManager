#include "aes_openssl.h"
#include <openssl/evp.h>
#include <openssl/err.h>
#include <cstring>
#include <stdexcept>
#include <memory>

struct AESImpl::Impl {
    EVP_CIPHER_CTX* encrypt_ctx;
    EVP_CIPHER_CTX* decrypt_ctx;
    const EVP_CIPHER* cipher;
    CipherType type;
    uint8_t iv[16];

    Impl(CipherType cipher_type) : encrypt_ctx(nullptr), decrypt_ctx(nullptr), type(cipher_type) {
        memset(iv, 0, sizeof(iv));
        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();

        switch (cipher_type) {
        case CipherType::AES_128:
            cipher = EVP_aes_128_ctr();
            break;
        case CipherType::AES_256:
            cipher = EVP_aes_256_ctr();
            break;
        default:
            throw std::runtime_error("Unsupported AES type for CTR");
        }

        encrypt_ctx = EVP_CIPHER_CTX_new();
        decrypt_ctx = EVP_CIPHER_CTX_new();
        if (!encrypt_ctx || !decrypt_ctx)
            throw std::runtime_error("Failed to create OpenSSL context");
    }

    ~Impl() {
        if (encrypt_ctx) EVP_CIPHER_CTX_free(encrypt_ctx);
        if (decrypt_ctx) EVP_CIPHER_CTX_free(decrypt_ctx);
    }
};

AESImpl::AESImpl(CipherType cipher_type) : pimpl(std::make_unique<Impl>(cipher_type)) {}
AESImpl::~AESImpl() = default;

bool AESImpl::init(const uint8_t* key, size_t key_len, const uint8_t* iv_data, size_t iv_len) {
    if (!pimpl->encrypt_ctx || !pimpl->decrypt_ctx) return false;

    size_t expected_key = (pimpl->type == CipherType::AES_128) ? 16 : 32;
    if (key_len != expected_key) return false;
    if (iv_len != 16) return false;

    std::memcpy(pimpl->iv, iv_data, 16);

    // Инициализация контекстов
    EVP_CIPHER_CTX_reset(pimpl->encrypt_ctx);
    EVP_CIPHER_CTX_reset(pimpl->decrypt_ctx);

    if (1 != EVP_EncryptInit_ex(pimpl->encrypt_ctx, pimpl->cipher, nullptr, key, pimpl->iv))
        return false;
    if (1 != EVP_DecryptInit_ex(pimpl->decrypt_ctx, pimpl->cipher, nullptr, key, pimpl->iv))
        return false;

    // Для CTR отключаем padding 
    EVP_CIPHER_CTX_set_padding(pimpl->encrypt_ctx, 0);
    EVP_CIPHER_CTX_set_padding(pimpl->decrypt_ctx, 0);

    return true;
}

void AESImpl::encrypt(const uint8_t* plaintext, uint8_t* ciphertext, size_t length) {
    int out_len = 0;
    if (1 != EVP_EncryptUpdate(pimpl->encrypt_ctx, ciphertext, &out_len, plaintext, static_cast<int>(length))) {
        throw std::runtime_error("Encryption failed");
    }
}

void AESImpl::decrypt(const uint8_t* ciphertext, uint8_t* plaintext, size_t length) {
    int out_len = 0;
    if (1 != EVP_DecryptUpdate(pimpl->decrypt_ctx, plaintext, &out_len, ciphertext, static_cast<int>(length))) {
        throw std::runtime_error("Decryption failed");
    }
}

size_t AESImpl::get_block_size() const { return 16; }
size_t AESImpl::get_key_size() const { return (pimpl->type == CipherType::AES_128) ? 16 : 32; }
size_t AESImpl::get_iv_size() const { return 16; }
CipherType AESImpl::get_type() const { return pimpl->type; }
std::string AESImpl::get_name() const {
    return (pimpl->type == CipherType::AES_128) ? "AES-128-CTR" : "AES-256-CTR";
}