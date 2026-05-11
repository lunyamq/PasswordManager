#ifndef TRIVIUM_H
#define TRIVIUM_H

#include "cipher_interface.h"
#include <array>
#include <cstdint>
#include <bitset>

class Trivium : public Cipher {
public:
    static constexpr size_t KEY_SIZE = 10;      // 80 бит = 10 байт
    static constexpr size_t IV_SIZE = 10;       // 80 бит = 10 байт
    static constexpr size_t BLOCK_SIZE = 1;     // Побайтовая работа

    Trivium();
    ~Trivium() override = default;

    bool init(const uint8_t* key, size_t key_len, const uint8_t* iv, size_t iv_len) override;

    void encrypt(const uint8_t* plaintext, uint8_t* ciphertext, size_t length) override;
    void decrypt(const uint8_t* ciphertext, uint8_t* plaintext, size_t length) override;

    size_t get_block_size() const override { return BLOCK_SIZE; }
    size_t get_key_size() const override { return KEY_SIZE; }
    size_t get_iv_size() const override { return IV_SIZE; }
    std::string get_name() const override { return "Trivium (80-bit)"; }
    CipherType get_type() const override { return CipherType::TRIVIUM; }

private:
    static constexpr size_t STATE_SIZE = 288;
    static constexpr size_t INIT_ROUNDS = 1152;

    std::bitset<STATE_SIZE> state;  // 288 бит состояния

    void generate_keystream(uint8_t* output, size_t length);
};

#endif // TRIVIUM_H