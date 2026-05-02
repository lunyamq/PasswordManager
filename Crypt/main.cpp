#include "password_manager.h"
#include <iostream>
#include <string>
#include <conio.h>
#include "utils.h"

void test_algorithm() {
    std::cout << "\n=== Algorithm Test ===\n";

    std::cout << "Select algorithm:\n";
    std::cout << "1. AES-256\n";
    std::cout << "2. AES-128\n";
    std::cout << "3. ChaCha20\n";
    std::cout << "4. Salsa20\n";  
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
    default:
        std::cout << "Invalid choice\n";
        return;
    }

    PasswordManager pm;
    if (!pm.select_cipher(type)) {
        std::cout << "Failed to select cipher\n";
        return;
    }

    std::cout << "\n1. Use random key/IV (recommended)\n";
    std::cout << "2. Use default test key/IV\n";
    std::cout << "Choice: ";
    std::cin >> choice;
    std::cin.ignore();

    std::vector<uint8_t> key, iv;

    if (choice == 1) {
        key = Random::generate_bytes(pm.get_key_size());
        iv = Random::generate_bytes(pm.get_iv_size());
        std::cout << "\nGenerated random key and IV\n";
    }
    else {
        key = std::vector<uint8_t>(pm.get_key_size(), 0xAA);
        iv = std::vector<uint8_t>(pm.get_iv_size(), 0xBB);
        std::cout << "\nUsing test key and IV\n";
    }

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

    auto decrypted = pm.decrypt(encrypted.data(), encrypted.size());

    std::cout << "\n=== Results ===\n";
    std::cout << "Algorithm: " << pm.get_cipher_name() << "\n";
    std::cout << "Key size: " << pm.get_key_size() << " bytes\n";
    std::cout << "IV size: " << pm.get_iv_size() << " bytes\n";
    std::cout << "Original text: " << text << "\n";
    std::cout << "Text length: " << text.length() << " chars\n";
    std::cout << "Encrypted size: " << encrypted.size() << " bytes\n";
    std::cout << "Decrypted text: " << bytes_to_string(decrypted) << "\n";

    if (text != bytes_to_string(decrypted)) {
        std::cout << "\nFAIL: Decryption error!\n";
    }
}

int main() {
        while (true) {
        std::cout << "\n=== Main Menu ===\n";
        std::cout << "1. Test algorithm with input\n";
        std::cout << "2. Run benchmarks\n";
        std::cout << "3. Exit\n";
        std::cout << "Choice: ";

        int choice;
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
        case 1:
            test_algorithm();
            break;

        case 2:
            PasswordManager::run_benchmarks();
            break;

        case 3:
            std::cout << "Exiting...\n";
            return 0;

        default:
            std::cout << "Invalid choice!\n";
        }

        std::cout << "\nPress any key to continue...";
        std::cout << "\n";
    }

    return 0;
}