#ifndef BLOWFISH_H
#define BLOWFISH_H

#include "cipher_interface.h"
#include <array>
#include <vector>

class Blowfish : public Cipher {
private:
    static constexpr size_t BLOCK_SIZE = 8;     // 64 бита
    static constexpr size_t KEY_SIZE = 16;      // 128 бит
    static constexpr size_t IV_SIZE = 8;        // 64 бита

    // P-массив на 18 элементов
    std::array<uint32_t, 18> P;

    // 4 S-блока по 256 элементов каждый
    std::array<std::array<uint32_t, 256>, 4> S;

    // Вектор инициализации
    std::vector<uint8_t> iv;

    uint32_t F(uint32_t x) const;
    void encrypt_block(uint8_t* block);
    void decrypt_block(uint8_t* block);
    void key_schedule(const uint8_t* key, size_t key_len);

public:
    Blowfish();
    ~Blowfish() override = default;

    bool init(const uint8_t* key, size_t key_len, const uint8_t* iv_data, size_t iv_len) override;

    void encrypt(const uint8_t* plaintext, uint8_t* ciphertext, size_t length) override;

    void decrypt(const uint8_t* ciphertext, uint8_t* plaintext, size_t length) override;

    size_t get_block_size() const override { return BLOCK_SIZE; }
    size_t get_key_size() const override { return KEY_SIZE; }
    size_t get_iv_size() const override { return IV_SIZE; }

    std::string get_name() const override { return "Blowfish (128-bit)"; }
    CipherType get_type() const override { return CipherType::BLOWFISH; }
};

#endif