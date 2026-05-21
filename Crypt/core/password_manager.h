#ifndef PASSWORD_MANAGER_H
#define PASSWORD_MANAGER_H

#include "cipher_interface.h"
#include <memory>
#include <vector>
#include <string>

class PasswordManager {
private:
    std::unique_ptr<Cipher> cipher;

public:
    PasswordManager();
    ~PasswordManager();

    bool select_cipher(CipherType type);

    bool set_key_and_iv(const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv);

    std::vector<uint8_t> encrypt(const uint8_t* data, size_t size);
    std::vector<uint8_t> decrypt(const uint8_t* data, size_t size);

    size_t get_key_size() const;
    size_t get_iv_size() const;
    std::string get_cipher_name() const;

    static void run_benchmarks();
};

#endif