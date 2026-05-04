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
    bool is_open = false;
    std::string current_vault;

    // Лямбда для добавления расширения .pwm, если его нет
    auto add_extension = [](const std::string& name) -> std::string {
        if (name.size() >= 4 && name.substr(name.size() - 4) == ".pwm")
            return name;
        else
            return name + ".pwm";
        };

    std::cout << "\n=== Password Manager Vault ===\n";
    std::cout << "Commands: create, open, close, list, add, get, update, delete, save, status, back\n";

    while (true) {
        // Приглашение с текущим состоянием
        std::cout << "\nvault"
            << (is_open ? "[" + current_vault + "]" : "[closed]")
            << "> ";
        std::getline(std::cin, command);

        if (command == "back") {
            if (is_open) {
                std::cout << "Vault is still open. Use 'close' first.\n";
                continue;
            }
            break;
        }

        // --- CREATE -------------------------------------------------
        if (command == "create") {
            if (is_open) {
                std::cout << "Please close current vault first (close).\n";
                continue;
            }
            std::string filename, password;
            int algo_choice;
            std::cout << "Filename (without extension): ";
            std::getline(std::cin, filename);
            std::string fullname = add_extension(filename);
            std::cout << "Master password: ";
            std::getline(std::cin, password);
            std::cout << "Cipher (1=AES-256, 2=ChaCha20, 3=Salsa20): ";
            std::cin >> algo_choice; std::cin.ignore();
            CipherType type;
            if (algo_choice == 1) type = CipherType::AES_256;
            else if (algo_choice == 2) type = CipherType::CHACHA20;
            else if (algo_choice == 3) type = CipherType::SALSA20;
            else { std::cout << "Invalid cipher\n"; continue; }
            if (vault.create(fullname, password, type)) {
                std::cout << "Vault created successfully.\n";
                is_open = true;
                current_vault = fullname;
            }
            else {
                std::cout << "Creation failed (check permissions or disk space).\n";
            }
        }

        // --- OPEN ---------------------------------------------------
        else if (command == "open") {
            if (is_open) {
                std::cout << "A vault is already open. Close it first.\n";
                continue;
            }
            std::string filename, password;
            std::cout << "Filename (without extension): ";
            std::getline(std::cin, filename);
            std::string fullname = add_extension(filename);
            std::cout << "Master password: ";
            std::getline(std::cin, password);
            if (vault.open(fullname, password)) {
                std::cout << "Vault opened. " << vault.get_all_entries().size() << " entries loaded.\n";
                is_open = true;
                current_vault = fullname;
            }
            else {
                std::cout << "Open failed (wrong password or corrupt file).\n";
            }
        }

        // --- CLOSE --------------------------------------------------
        else if (command == "close") {
            if (!is_open) {
                std::cout << "No vault is open.\n";
                continue;
            }
            vault.close();
            is_open = false;
            current_vault.clear();
            std::cout << "Vault closed.\n";
        }

        // --- STATUS -------------------------------------------------
        else if (command == "status") {
            if (is_open) {
                std::cout << "Opened vault: " << current_vault
                    << ", entries: " << vault.get_all_entries().size() << "\n";
            }
            else {
                std::cout << "No vault is currently open.\n";
            }
        }

        // --- LIST ---------------------------------------------------
        else if (command == "list") {
            if (!is_open) {
                std::cout << "No vault open. Use 'open' first.\n";
                continue;
            }
            auto entries = vault.get_all_entries();
            if (entries.empty()) {
                std::cout << "No entries.\n";
            }
            else {
                for (size_t i = 0; i < entries.size(); ++i) {
                    std::cout << i + 1 << ". "
                        << entries[i].id << " : "
                        << entries[i].title << " (" << entries[i].username << ")\n";
                }
            }
        }

        // --- ADD ----------------------------------------------------
        else if (command == "add") {
            if (!is_open) {
                std::cout << "No vault open. Use 'open' first.\n";
                continue;
            }
            Entry e;
            e.id = generate_uuid();
            std::cout << "Title: ";      std::getline(std::cin, e.title);
            std::cout << "Username: ";   std::getline(std::cin, e.username);
            std::cout << "Password: ";   std::getline(std::cin, e.password);
            std::cout << "URL: ";        std::getline(std::cin, e.url);
            std::cout << "Notes: ";      std::getline(std::cin, e.notes);
            e.created = get_current_time();
            e.updated = e.created;
            vault.add_entry(e);
            std::cout << "Added with ID " << e.id << "\n";
        }

        // --- GET ----------------------------------------------------
        else if (command == "get") {
            if (!is_open) {
                std::cout << "No vault open. Use 'open' first.\n";
                continue;
            }
            std::string id;
            std::cout << "Entry ID: "; std::getline(std::cin, id);
            Entry* e = vault.find_entry(id);
            if (e) print_entry(*e);
            else std::cout << "Not found.\n";
        }

        // --- UPDATE -------------------------------------------------
        else if (command == "update") {
            if (!is_open) {
                std::cout << "No vault open. Use 'open' first.\n";
                continue;
            }
            std::string id;
            std::cout << "Entry ID: "; std::getline(std::cin, id);
            Entry* e = vault.find_entry(id);
            if (!e) {
                std::cout << "Not found.\n";
                continue;
            }
            std::string buf;
            std::cout << "Leave blank to keep old value.\n";
            std::cout << "Title [" << e->title << "]: ";      std::getline(std::cin, buf); if (!buf.empty()) e->title = buf;
            std::cout << "Username [" << e->username << "]: "; std::getline(std::cin, buf); if (!buf.empty()) e->username = buf;
            std::cout << "Password [" << e->password << "]: "; std::getline(std::cin, buf); if (!buf.empty()) e->password = buf;
            std::cout << "URL [" << e->url << "]: ";          std::getline(std::cin, buf); if (!buf.empty()) e->url = buf;
            std::cout << "Notes [" << e->notes << "]: ";      std::getline(std::cin, buf); if (!buf.empty()) e->notes = buf;
            e->updated = get_current_time();
            std::cout << "Updated.\n";
        }

        // --- DELETE -------------------------------------------------
        else if (command == "delete") {
            if (!is_open) {
                std::cout << "No vault open. Use 'open' first.\n";
                continue;
            }
            std::string id;
            std::cout << "Entry ID: "; std::getline(std::cin, id);
            if (vault.delete_entry(id)) std::cout << "Deleted.\n";
            else std::cout << "Not found.\n";
        }

        // --- SAVE ---------------------------------------------------
        else if (command == "save") {
            if (!is_open) {
                std::cout << "No vault open.\n";
                continue;
            }
            if (vault.save()) std::cout << "Saved.\n";
            else std::cout << "Save failed (check file permissions).\n";
        }

        // --- UNKNOWN ------------------------------------------------
        else {
            std::cout << "Unknown command. Available: create, open, close, list, add, get, update, delete, save, status, back\n";
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
        std::cout << "1. Password Manager\n";
        std::cout << "2. Run benchmarks\n";
        std::cout << "3. Test algorithm\n";
        std::cout << "0. Exit\n";
        std::cout << "Choice: ";

        int choice;
        std::cin >> choice;
        std::cin.ignore();  // очистить буфер после ввода числа

        switch (choice) {
        case 1:
            vault_menu();
            break;
        case 2:
            PasswordManager::run_benchmarks();
            break;
        case 3:
            test_algorithm();
            break;
        case 0:
            return 0;
        default:
            std::cout << "Invalid choice.\n";
        }

        std::cout << "\nPress any key to continue...";
        GETCH();
        std::cout << "\n";
    }

    return 0;
}