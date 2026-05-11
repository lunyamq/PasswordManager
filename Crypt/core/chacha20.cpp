// chacha20.cpp
#include "chacha20.h"
#include "utils.h"
#include <cstring>

ChaCha20::ChaCha20() : keystream_pos(BLOCK_SIZE) {
    state.fill(0);
    keystream.fill(0);
    initial_state.fill(0);
}

uint32_t ChaCha20::rotate_left(uint32_t x, int n) {
    return (x << n) | (x >> (32 - n));
}

void ChaCha20::quarter_round(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d) {
    a += b; d ^= a; d = rotate_left(d, 16);
    c += d; b ^= c; b = rotate_left(b, 12);
    a += b; d ^= a; d = rotate_left(d, 8);
    c += d; b ^= c; b = rotate_left(b, 7);
}

void ChaCha20::generate_keystream_block() {
    std::array<uint32_t, 16> x = state;

    for (int i = 0; i < 10; ++i) {
        quarter_round(x[0], x[4], x[8], x[12]);
        quarter_round(x[1], x[5], x[9], x[13]);
        quarter_round(x[2], x[6], x[10], x[14]);
        quarter_round(x[3], x[7], x[11], x[15]);

        quarter_round(x[0], x[5], x[10], x[15]);
        quarter_round(x[1], x[6], x[11], x[12]);
        quarter_round(x[2], x[7], x[8], x[13]);
        quarter_round(x[3], x[4], x[9], x[14]);
    }

    for (int i = 0; i < 16; ++i)
        x[i] += state[i];

    for (int i = 0; i < 16; ++i)
        store32_le(&keystream[i * 4], x[i]);

    keystream_pos = 0;
    state[12]++;
}

bool ChaCha20::init(const uint8_t* key, size_t key_len, const uint8_t* iv, size_t iv_len) {
    if (key_len != KEY_SIZE || iv_len != IV_SIZE) {
        return false;
    }

    state[0] = 0x61707865;
    state[1] = 0x3320646e;
    state[2] = 0x79622d32;
    state[3] = 0x6b206574;

    for (int i = 0; i < 8; i++) {
        state[4 + i] = load32_le(key + i * 4);
    }

    state[12] = 0;

    state[13] = load32_le(iv + 0);
    state[14] = load32_le(iv + 4);
    state[15] = load32_le(iv + 8);

    initial_state = state;
    keystream_pos = BLOCK_SIZE;

    return true;
}


void ChaCha20::encrypt(const uint8_t* plaintext, uint8_t* ciphertext, size_t length) {
    for (size_t i = 0; i < length; i++) {
        if (keystream_pos >= BLOCK_SIZE) {
            generate_keystream_block();   // использует текущий state и увеличивает счётчик
        }
        ciphertext[i] = plaintext[i] ^ keystream[keystream_pos++];
    }
}


void ChaCha20::decrypt(const uint8_t* ciphertext, uint8_t* plaintext, size_t length) {
    encrypt(ciphertext, plaintext, length);
}

