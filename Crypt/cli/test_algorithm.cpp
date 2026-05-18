#include "cli.h"
#include "../core/password_manager.h"
#include "../core/utils.h"
#include <iostream>

void test_algorithm() {
    std::cout << "\n=== Algorithm Test ===\n";

    std::cout << "Select algorithm:\n";
    std::cout << "1. AES-256\n";
    std::cout << "2. AES-128\n";
    std::cout << "3. ChaCha20\n";
    std::cout << "4. Salsa20\n";
    std::cout << "5. Trivium\n";
    std::cout << "Choice: ";

    int choice;
    std::cin >> choice;
    std::cin.ignore();

    CipherType type;
    switch (choice) {
    case 1: type = CipherType::AES_256; break;
    case 2: type = CipherType::AES_128; break;
    case 3: type = CipherType::CHACHA20; break;
    case 4: type = CipherType::SALSA20; break;
    case 5: type = CipherType::TRIVIUM; break;
    default:
        std::cout << "Invalid choice\n";
        return;
    }

    PasswordManager pm;
    if (!pm.select_cipher(type)) {
        std::cout << "Failed to select cipher\n";
        return;
    }

    // Случайные ключ и IV
    auto key = Random::generate_bytes(pm.get_key_size());
    auto iv = Random::generate_bytes(pm.get_iv_size());

    if (!pm.set_key_and_iv(key, iv)) {
        std::cout << "Failed to set key and IV\n";
        return;
    }

    std::cout << "\nEnter text to encrypt: ";
    std::string text;
    std::getline(std::cin, text);
    if (text.empty()) {
        text = "Hello, Cryptography!";
        std::cout << "Using default text: " << text << "\n";
    }

    auto plaintext = string_to_bytes(text);
    auto encrypted = pm.encrypt(plaintext.data(), plaintext.size());

    // Для расшифрования создаём НОВЫЙ объект
    PasswordManager pm2;
    if (!pm2.select_cipher(type)) {
        std::cout << "Failed to select cipher for decryption\n";
        return;
    }
    if (!pm2.set_key_and_iv(key, iv)) {
        std::cout << "Failed to set key and IV for decryption\n";
        return;
    }
    auto decrypted = pm2.decrypt(encrypted.data(), encrypted.size());

    std::cout << "\n=== Results ===\n";
    std::cout << "Algorithm: " << pm.get_cipher_name() << "\n";
    std::cout << "Key size: " << pm.get_key_size() << " bytes (random)\n";
    std::cout << "IV size: " << pm.get_iv_size() << " bytes (random)\n";
    std::cout << "Original text: " << text << "\n";
    std::cout << "Encrypted (hex): " << bytes_to_hex(encrypted) << "\n";
    std::cout << "Decrypted text: " << bytes_to_string(decrypted) << "\n";

    if (text != bytes_to_string(decrypted)) {
        std::cout << "\nFAIL: Decryption error!\n";
    }
    else {
        std::cout << "\nSUCCESS: Encryption/decryption works correctly.\n";
    }
}