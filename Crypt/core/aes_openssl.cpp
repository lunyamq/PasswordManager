// aes_openssl.cpp
#include "aes_openssl.h"
#include <openssl/evp.h>
#include <openssl/err.h>
#include <cstring>
#include <stdexcept>
#include <iostream>

struct AESImpl::Impl {
    EVP_CIPHER_CTX* encrypt_ctx;
    EVP_CIPHER_CTX* decrypt_ctx;
    const EVP_CIPHER* cipher;
    CipherType type;
    uint8_t iv[16];

    Impl(CipherType cipher_type) : encrypt_ctx(nullptr),
        decrypt_ctx(nullptr),
        type(cipher_type) {
        // Инициализация OpenSSL
        memset(iv, 0, sizeof(iv));
        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();

        switch (cipher_type) {
        case CipherType::AES_128:
            cipher = EVP_aes_128_gcm();
            break;
        case CipherType::AES_256:
            cipher = EVP_aes_256_gcm();
            break;
        default:
            throw std::runtime_error("Unsupported AES type");
        }

        encrypt_ctx = EVP_CIPHER_CTX_new();
        decrypt_ctx = EVP_CIPHER_CTX_new();

        if (!encrypt_ctx || !decrypt_ctx) {
            throw std::runtime_error("Failed to create OpenSSL context");
        }

        EVP_CIPHER_CTX_init(encrypt_ctx);
        EVP_CIPHER_CTX_init(decrypt_ctx);
    }

    ~Impl() {
        if (encrypt_ctx) EVP_CIPHER_CTX_free(encrypt_ctx);
        if (decrypt_ctx) EVP_CIPHER_CTX_free(decrypt_ctx);
    }
};

AESImpl::AESImpl(CipherType cipher_type): pimpl(std::make_unique<Impl>(cipher_type)) { }

// Деструктор
AESImpl::~AESImpl() = default;

bool AESImpl::init(const uint8_t* key, size_t key_len, const uint8_t* iv_data, size_t iv_len) {
    if (!pimpl->encrypt_ctx || !pimpl->decrypt_ctx) return false;

    // Сохраняем IV
    std::memset(pimpl->iv, 0, sizeof(pimpl->iv));
    size_t copy_len = (iv_len < sizeof(pimpl->iv)) ? iv_len : sizeof(pimpl->iv);
    std::memcpy(pimpl->iv, iv_data, copy_len);

    if (EVP_EncryptInit_ex(pimpl->encrypt_ctx, pimpl->cipher, nullptr, key, pimpl->iv) != 1) {
        return false;
    }

    if (EVP_DecryptInit_ex(pimpl->decrypt_ctx, pimpl->cipher, nullptr, key, pimpl->iv) != 1) {
        return false;
    }

    return true;
}

void AESImpl::encrypt(const uint8_t* plaintext, uint8_t* ciphertext,
    size_t length) {
    int out_len = 0;
    if (EVP_EncryptUpdate(pimpl->encrypt_ctx, ciphertext, &out_len, plaintext, static_cast<int>(length)) != 1) {
        throw std::runtime_error("Encryption failed");
    }
}

void AESImpl::decrypt(const uint8_t* ciphertext, uint8_t* plaintext, size_t length) {
    int out_len = 0;
    if (EVP_DecryptUpdate(pimpl->decrypt_ctx, plaintext, &out_len,
        ciphertext, static_cast<int>(length)) != 1) {
        throw std::runtime_error("Decryption failed");
    }
}

size_t AESImpl::get_block_size() const {
    return 16;
}

size_t AESImpl::get_key_size() const {
    return (pimpl->type == CipherType::AES_128) ? 16 : 32;
}

size_t AESImpl::get_iv_size() const {
    return 16;
}

std::string AESImpl::get_name() const {
    return (pimpl->type == CipherType::AES_128) ? "AES-128 (OpenSSL)" : "AES-256 (OpenSSL)";
}

CipherType AESImpl::get_type() const {
    return pimpl->type;
}