#ifndef SALSA20_H
#define SALSA20_H

#include "cipher_interface.h"
#include <array>
#include <cstdint>

class Salsa20 : public Cipher {
public:
    static constexpr size_t KEY_SIZE = 32;     // 256 бит
    static constexpr size_t IV_SIZE = 8;       // 64 бит (Salsa20 использует 8-байтовый nonce)
    static constexpr size_t BLOCK_SIZE = 64;   // 64 байта на блок

    Salsa20();
    ~Salsa20() override = default;

    bool init(const uint8_t* key, size_t key_len, const uint8_t* iv, size_t iv_len) override;

    void encrypt(const uint8_t* plaintext, uint8_t* ciphertext, size_t length) override;
    void decrypt(const uint8_t* ciphertext, uint8_t* plaintext, size_t length) override;

    size_t get_block_size() const override { return BLOCK_SIZE; }
    size_t get_key_size() const override { return KEY_SIZE; }
    size_t get_iv_size() const override { return IV_SIZE; }
    std::string get_name() const override { return "Salsa20 (256-bit)"; }
    CipherType get_type() const override { return CipherType::SALSA20; }

private:
    std::array<uint32_t, 16> state;            // текущее состояние
    std::array<uint32_t, 16> initial_state;    // копия для сброса
    std::array<uint8_t, BLOCK_SIZE> keystream;
    size_t keystream_pos;

    static uint32_t rotate_left(uint32_t x, int n);
    static void quarter_round(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d);
    void generate_keystream_block();
};

#endif // SALSA20_H