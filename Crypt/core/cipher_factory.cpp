//cipher_factory.cpp
#include "cipher_interface.h"
#include "aes_openssl.h"
#include "chacha20.h"
#include "salsa20.h"
#include "trivium.h"

#include <memory>
#include <iostream>

std::unique_ptr<Cipher> create_cipher(CipherType type) {
    switch (type) {
    case CipherType::AES_128:
    case CipherType::AES_256:
        return std::make_unique<AESImpl>(type);

    case CipherType::CHACHA20:
        return std::make_unique<ChaCha20>();

    case CipherType::SALSA20:
        return std::make_unique<Salsa20>();

    case CipherType::TRIVIUM:
        return std::make_unique<Trivium>();

    default:
        std::cerr << "Error: Unknown cipher type\n";
        return nullptr;
    }
}

std::string cipher_type_to_string(CipherType type) {
    switch (type) {
    case CipherType::AES_256: return "AES-256";
    case CipherType::AES_128: return "AES-128";
    case CipherType::CHACHA20: return "ChaCha20";
    case CipherType::SALSA20: return "Salsa20"; 
    case CipherType::TRIVIUM: return "Trivium";
    default: return "Unknown";
    }
}

CipherType string_to_cipher_type(const std::string& str) {
    if (str == "AES-256" || str == "aes-256") return CipherType::AES_256;
    if (str == "AES-128" || str == "aes-128") return CipherType::AES_128;
    if (str == "ChaCha20" || str == "chacha20") return CipherType::CHACHA20;
    if (str == "Salsa20" || str == "salsa20") return CipherType::SALSA20;
    if (str == "Trivium" || str == "trivium") return CipherType::TRIVIUM;
    return CipherType::AES_256;
}