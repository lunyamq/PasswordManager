#include "password_manager.h"
#include "vault_storage.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <limits>

// ----------------------------------------------------------------------
// Кроссплатформенный ввод одного символа без ожидания Enter
// ----------------------------------------------------------------------
#ifdef _WIN32
#include <conio.h>
#define GETCH() _getch()
#else
#include <termios.h>
#include <unistd.h>
static int getch_linux() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    int ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#define GETCH() getch_linux()
#endif

// ----------------------------------------------------------------------
// Функция тестирования отдельного алгоритма
// ----------------------------------------------------------------------
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

    // Всегда генерируем случайные ключ и IV
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
    auto decrypted = pm.decrypt(encrypted.data(), encrypted.size());

    std::cout << "\n=== Results ===\n";
    std::cout << "Algorithm: " << pm.get_cipher_name() << "\n";
    std::cout << "Key size: " << pm.get_key_size() << " bytes\n";
    std::cout << "IV size: " << pm.get_iv_size() << " bytes\n";
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

// ----------------------------------------------------------------------
// Меню менеджера паролей (работа с хранилищем)
// ----------------------------------------------------------------------
void vault_menu() {
    VaultStorage vault;
    std::string command;

    std::cout << "\n=== Password Manager Vault ===\n";
    std::cout << "Commands: create, open, list, add, get, update, delete, save, close, back\n";

    while (true) {
        std::cout << "\nvault> ";
        std::getline(std::cin, command);

        if (command == "back") break;

        if (command == "create") {
            std::string filename, password;
            int algo_choice;
            std::cout << "Filename: "; std::getline(std::cin, filename);
            std::cout << "Master password: "; std::getline(std::cin, password);
            std::cout << "Cipher (1=AES-256, 2=ChaCha20, 3=Salsa20): ";
            std::cin >> algo_choice; std::cin.ignore();
            CipherType type;
            if (algo_choice == 1) type = CipherType::AES_256;
            else if (algo_choice == 2) type = CipherType::CHACHA20;
            else if (algo_choice == 3) type = CipherType::SALSA20;
            else { std::cout << "Invalid cipher\n"; continue; }
            if (vault.create(filename, password, type))
                std::cout << "Vault created.\n";
            else
                std::cout << "Creation failed.\n";
        }
        else if (command == "open") {
            std::string filename, password;
            std::cout << "Filename: "; std::getline(std::cin, filename);
            std::cout << "Master password: "; std::getline(std::cin, password);
            if (vault.open(filename, password)) {
                std::cout << "Vault opened. " << vault.get_all_entries().size() << " entries loaded.\n";
            }
            else {
                std::cout << "Open failed (wrong password or corrupt file).\n";
            }
        }
        else if (command == "list") {
            auto entries = vault.get_all_entries();
            if (entries.empty()) {
                std::cout << "No entries.\n";
            }
            else {
                for (size_t i = 0; i < entries.size(); ++i)
                    std::cout << i + 1 << ". " << entries[i].title << " (" << entries[i].username << ")\n";
            }
        }
        else if (command == "add") {
            Entry e;
            e.id = generate_uuid();
            std::cout << "Title: "; std::getline(std::cin, e.title);
            std::cout << "Username: "; std::getline(std::cin, e.username);
            std::cout << "Password: "; std::getline(std::cin, e.password);
            std::cout << "URL: "; std::getline(std::cin, e.url);
            std::cout << "Notes: "; std::getline(std::cin, e.notes);
            e.created = get_current_time();
            e.updated = e.created;
            vault.add_entry(e);
            std::cout << "Added with ID " << e.id << "\n";
        }
        else if (command == "get") {
            std::string id;
            std::cout << "Entry ID: "; std::getline(std::cin, id);
            Entry* e = vault.find_entry(id);
            if (e) print_entry(*e);
            else std::cout << "Not found.\n";
        }
        else if (command == "update") {
            std::string id;
            std::cout << "Entry ID: "; std::getline(std::cin, id);
            Entry* e = vault.find_entry(id);
            if (!e) {
                std::cout << "Not found.\n";
                continue;
            }
            std::string buf;
            std::cout << "Leave blank to keep old value.\n";
            std::cout << "Title [" << e->title << "]: "; std::getline(std::cin, buf); if (!buf.empty()) e->title = buf;
            std::cout << "Username [" << e->username << "]: "; std::getline(std::cin, buf); if (!buf.empty()) e->username = buf;
            std::cout << "Password [" << e->password << "]: "; std::getline(std::cin, buf); if (!buf.empty()) e->password = buf;
            std::cout << "URL [" << e->url << "]: "; std::getline(std::cin, buf); if (!buf.empty()) e->url = buf;
            std::cout << "Notes [" << e->notes << "]: "; std::getline(std::cin, buf); if (!buf.empty()) e->notes = buf;
            e->updated = get_current_time();
            std::cout << "Updated.\n";
        }
        else if (command == "delete") {
            std::string id;
            std::cout << "Entry ID: "; std::getline(std::cin, id);
            if (vault.delete_entry(id)) std::cout << "Deleted.\n";
            else std::cout << "Not found.\n";
        }
        else if (command == "save") {
            if (vault.save()) std::cout << "Saved.\n";
            else std::cout << "Save failed.\n";
        }
        else if (command == "close") {
            vault.close();
            std::cout << "Vault closed.\n";
        }
        else {
            std::cout << "Unknown command. Available: create, open, list, add, get, update, delete, save, close, back\n";
        }
    }
}

// ----------------------------------------------------------------------
// Главное меню
// ----------------------------------------------------------------------
int main() {
    std::cout << "=== Cryptographic Password Manager ===\n";

    while (true) {
        std::cout << "\nMain Menu:\n";
        std::cout << "1. Test algorithm (encrypt/decrypt)\n";
        std::cout << "2. Run performance benchmarks\n";
        std::cout << "3. Password Manager (vault)\n";
        std::cout << "4. Exit\n";
        std::cout << "Choice: ";

        int choice;
        std::cin >> choice;
        std::cin.ignore();  // очистить буфер после ввода числа

        switch (choice) {
        case 1:
            test_algorithm();
            break;
        case 2:
            PasswordManager::run_benchmarks();
            break;
        case 3:
            vault_menu();
            break;
        case 4:
            std::cout << "Exiting. Goodbye!\n";
            return 0;
        default:
            std::cout << "Invalid choice. Please enter 1-4.\n";
        }

        std::cout << "\nPress any key to continue...";
        GETCH();
        std::cout << "\n";
    }

    return 0;
}