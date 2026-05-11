#ifndef CHACHA20_H
#define CHACHA20_H

#include "cipher_interface.h"
#include <vector>
#include <array>

class ChaCha20 : public Cipher {
private:
    static constexpr size_t KEY_SIZE = 32;     // 256 бит
    static constexpr size_t IV_SIZE = 12;      // 96 бит
    static constexpr size_t BLOCK_SIZE = 64;   // 512 бит

    std::array<uint32_t, 16> state;            // 16 слов по 32 бита
    std::array<uint8_t, 64> keystream;
    size_t keystream_pos;
    std::array<uint32_t, 16> initial_state;

    static uint32_t rotate_left(uint32_t x, int n);
    static void quarter_round(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d);
    void generate_keystream_block();

public:
    ChaCha20();
    ~ChaCha20() override = default;

    bool init(const uint8_t* key, size_t key_len, const uint8_t* iv, size_t iv_len) override;

    void encrypt(const uint8_t* plaintext, uint8_t* ciphertext, size_t length) override;

    void decrypt(const uint8_t* ciphertext, uint8_t* plaintext, size_t length) override;

    size_t get_block_size() const override { return BLOCK_SIZE; }
    size_t get_key_size() const override { return KEY_SIZE; }
    size_t get_iv_size() const override { return IV_SIZE; }

    std::string get_name() const override { return "ChaCha20 (256-bit)"; }
    CipherType get_type() const override { return CipherType::CHACHA20; }
};

#endif