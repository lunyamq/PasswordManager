#ifndef AES_OPENSSL_H
#define AES_OPENSSL_H

#include "cipher_interface.h"
#include <memory>

class AESImpl : public Cipher {
private:
    struct Impl;
    std::unique_ptr<Impl> pimpl; 

public:
    AESImpl(CipherType cipher_type);
    ~AESImpl() override;

    // Запрещаем копирование
    AESImpl(const AESImpl&) = delete;
    AESImpl& operator=(const AESImpl&) = delete;

    // Разрешаем перемещение
    AESImpl(AESImpl&&) = default;
    AESImpl& operator=(AESImpl&&) = default;

    bool init(const uint8_t* key, size_t key_len, const uint8_t* iv, size_t iv_len) override;

    void encrypt(const uint8_t* plaintext, uint8_t* ciphertext, size_t length) override;

    void decrypt(const uint8_t* ciphertext, uint8_t* plaintext, size_t length) override;

    size_t get_block_size() const override;
    size_t get_key_size() const override;
    size_t get_iv_size() const override;

    std::string get_name() const override;
    CipherType get_type() const override;
};

#endif