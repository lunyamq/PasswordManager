// salsa20.cpp
#include "salsa20.h"
#include "utils.h"
#include <cstring>

Salsa20::Salsa20() : keystream_pos(BLOCK_SIZE) {
    state.fill(0);
    initial_state.fill(0);
    keystream.fill(0);
}

uint32_t Salsa20::rotate_left(uint32_t x, int n) {
    return (x << n) | (x >> (32 - n));
}

void Salsa20::quarter_round(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d) {
    b ^= rotate_left(a + d, 7);
    c ^= rotate_left(b + a, 9);
    d ^= rotate_left(c + b, 13);
    a ^= rotate_left(d + c, 18);
}

void Salsa20::generate_keystream_block() {
    std::array<uint32_t, 16> x = state;

    for (int i = 0; i < 10; ++i) {
        // Quarter round столбцов
        quarter_round(x[0], x[4], x[8], x[12]);
        quarter_round(x[5], x[9], x[13], x[1]);
        quarter_round(x[10], x[14], x[2], x[6]);
        quarter_round(x[15], x[3], x[7], x[11]);

        // Quarter round строк
        quarter_round(x[0], x[1], x[2], x[3]);
        quarter_round(x[5], x[6], x[7], x[4]);
        quarter_round(x[10], x[11], x[8], x[9]);
        quarter_round(x[15], x[12], x[13], x[14]);
    }

    // Прибавляем исходное состояние
    for (int i = 0; i < 16; ++i)
        x[i] += state[i];

    // Упаковываем в keystream
    for (int i = 0; i < 16; ++i)
        store32_le(&keystream[i * 4], x[i]);

    keystream_pos = 0;
    state[8]++;
}

bool Salsa20::init(const uint8_t* key, size_t key_len, const uint8_t* iv, size_t iv_len) {
    if (key_len != KEY_SIZE || iv_len != IV_SIZE)
        return false;

    // Константы Salsa20
    state[0] = 0x61707865;
    state[1] = 0x3320646e;
    state[2] = 0x79622d32;
    state[3] = 0x6b206574;

    // Ключ
    for (int i = 0; i < 8; ++i)
        state[4 + i] = load32_le(key + i * 4);

    // Nonce
    state[12] = load32_le(iv);
    state[13] = load32_le(iv + 4);

    // Счётчик блока
    state[8] = 0;
    state[9] = 0;
    state[10] = 0;   
    state[11] = 0;   
    state[14] = 0;   
    state[15] = 0;   

    initial_state = state;
    keystream_pos = BLOCK_SIZE;
    return true;
}

void Salsa20::encrypt(const uint8_t* plaintext, uint8_t* ciphertext, size_t length) {
    state = initial_state;
    keystream_pos = BLOCK_SIZE;

    for (size_t i = 0; i < length; ++i) {
        if (keystream_pos >= BLOCK_SIZE)
            generate_keystream_block();
        ciphertext[i] = plaintext[i] ^ keystream[keystream_pos++];
    }
}

void Salsa20::decrypt(const uint8_t* ciphertext, uint8_t* plaintext, size_t length) {
    encrypt(ciphertext, plaintext, length);
}