#include "blowfish.h"
#include <iostream>
#include <cstring>

class BlowfishSimple : public Cipher {
private:
    static constexpr size_t BLOCK_SIZE = 8;
    static constexpr size_t KEY_SIZE = 16;
    static constexpr size_t IV_SIZE = 8;

    std::vector<uint8_t> key;
    std::vector<uint8_t> iv;

public:
    BlowfishSimple() {
        iv.resize(IV_SIZE, 0);
    }

    bool init(const uint8_t* key_data, size_t key_len, const uint8_t* iv_data, size_t iv_len) override {

        if (!key_data || key_len == 0) {
            std::cerr << "Invalid key\n";
            return false;
        }

        key.assign(key_data, key_data + std::min(key_len, KEY_SIZE));
        iv.assign(iv_data, iv_data + std::min(iv_len, IV_SIZE));

        return true;
    }

    void encrypt(const uint8_t* plaintext, uint8_t* ciphertext, size_t length) override {

        if (!plaintext || !ciphertext) {
            std::cerr << "Null pointer in encrypt\n";
            return;
        }

        for (size_t i = 0; i < length; i++) {
            ciphertext[i] = plaintext[i] ^ key[i % key.size()];
        }
    }

    void decrypt(const uint8_t* ciphertext, uint8_t* plaintext, size_t length) override {

        if (!ciphertext || !plaintext) {
            std::cerr << "Null pointer in decrypt\n";
            return;
        }

        for (size_t i = 0; i < length; i++) {
            plaintext[i] = ciphertext[i] ^ key[i % key.size()];
        }
    }

    size_t get_block_size() const override { return BLOCK_SIZE; }
    size_t get_key_size() const override { return KEY_SIZE; }
    size_t get_iv_size() const override { return IV_SIZE; }

    std::string get_name() const override { return "Blowfish (Simple)"; }
    CipherType get_type() const override { return CipherType::BLOWFISH; }
};