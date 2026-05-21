#include "trivium.h"
#include "utils.h"
#include <cstring>

Trivium::Trivium() {
    state.reset(); 
}

bool Trivium::init(const uint8_t* key_data, size_t key_len, const uint8_t* iv_data, size_t iv_len) {
    if (key_len != KEY_SIZE || iv_len != IV_SIZE)
        return false;

    state.reset();

    // Загружаем 80-битный ключ в первые 93 бита
    for (size_t i = 0; i < key_len; ++i) {
        uint8_t byte = key_data[i];
        for (int bit = 0; bit < 8; ++bit) {
            if (byte & (1 << (7 - bit))) {
                state.set(i * 8 + bit);
            }
        }
    }

    // Загружаем 80-битный IV в следующие 84 бита
    for (size_t i = 0; i < iv_len; ++i) {
        uint8_t byte = iv_data[i];
        for (int bit = 0; bit < 8; ++bit) {
            if (byte & (1 << (7 - bit))) {
                state.set(93 + i * 8 + bit);
            }
        }
    }

    // Устанавливаем последние три бита состояния в 1
    state.set(285); // 
    state.set(286); // 
    state.set(287); 

    for (size_t round = 0; round < INIT_ROUNDS; ++round) {
        // Функции обратной связ
        bool t1 = state[65] ^ (state[90] & state[91]) ^ state[92] ^ state[170];
        bool t2 = state[161] ^ (state[174] & state[175]) ^ state[176] ^ state[263];
        bool t3 = state[242] ^ (state[285] & state[286]) ^ state[287] ^ state[68];

        state >>= 1; 
        state.set(287, t3); 
        state.set(176, t2);
        state.set(92, t1);
    }

    return true;
}

void Trivium::generate_keystream(uint8_t* output, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        uint8_t byte = 0;
        for (int bit = 0; bit < 8; ++bit) {
            bool out = state[65] ^ state[92] ^ state[161] ^ state[176] ^ state[242] ^ state[287];
            byte |= (out << (7 - bit));

            bool t1 = state[65] ^ (state[90] & state[91]) ^ state[92] ^ state[170];
            bool t2 = state[161] ^ (state[174] & state[175]) ^ state[176] ^ state[263];
            bool t3 = state[242] ^ (state[285] & state[286]) ^ state[287] ^ state[68];

            state >>= 1;
            state.set(287, t3);
            state.set(176, t2);
            state.set(92, t1);
        }

        output[i] = byte;
    }
}

void Trivium::encrypt(const uint8_t* plaintext, uint8_t* ciphertext, size_t length) {
    std::vector<uint8_t> keystream(length);
    generate_keystream(keystream.data(), length);

    for (size_t i = 0; i < length; ++i) {
        ciphertext[i] = plaintext[i] ^ keystream[i];
    }
}

void Trivium::decrypt(const uint8_t* ciphertext, uint8_t* plaintext, size_t length) {
    encrypt(ciphertext, plaintext, length);
}