#include "cli.h"
#include "../core/vault_storage.h"
#include "../core/password_manager.h"
#include "../core/utils.h"
#include "../core/constants.h"
#include <openssl/crypto.h>
#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
#include <stdexcept>

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

// Скрытый ввод пароля
std::string read_password(const std::string& prompt) {
    std::cout << prompt;
    std::string password;
#ifdef _WIN32
    char ch;
    while ((ch = _getch()) != '\r') {
        if (ch == '\b') {
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b";
            }
        }
        else if (ch != '\n') {
            password.push_back(ch);
            std::cout << '*';
        }
    }
    std::cout << std::endl;
#else
    // Linux: отключаем эхо
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    std::getline(std::cin, password);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << std::endl;
#endif
    return password;
}

// Поиск записей по ключевому слову
static void search_entries(const VaultStorage& vault, const std::string& keyword) {
    auto entries = vault.get_all_entries();
    if (entries.empty()) {
        std::cout << "No entries.\n";
        return;
    }
    std::string kw = keyword;
    std::transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
    bool found = false;
    for (const auto& e : entries) {
        std::string title_low = e.title;
        std::string username_low = e.username;
        std::string url_low = e.url;
        std::transform(title_low.begin(), title_low.end(), title_low.begin(), ::tolower);
        std::transform(username_low.begin(), username_low.end(), username_low.begin(), ::tolower);
        std::transform(url_low.begin(), url_low.end(), url_low.begin(), ::tolower);
        if (title_low.find(kw) != std::string::npos ||
            username_low.find(kw) != std::string::npos ||
            url_low.find(kw) != std::string::npos) {
            std::cout << e.id << " : " << e.title << " (" << e.username << ")\n";
            found = true;
        }
    }
    if (!found) std::cout << "No matching entries.\n";
}

void vault_menu() {
    VaultStorage vault;
    std::string command;
    bool is_open = false;
    std::string current_vault;

    auto add_extension = [](const std::string& name) -> std::string {
        if (name.size() >= 4 && name.substr(name.size() - 4) == VAULT_EXTENSION)
            return name;
        else
            return name + VAULT_EXTENSION;
        };

    std::cout << "\n=== Password Manager Vault ===\n";
    std::cout << "Type 'help' for commands list.\n";

    while (true) {
        std::cout << "\nvault"
            << (is_open ? "[" + current_vault + "]" : "[closed]")
            << "> ";
        std::getline(std::cin, command);

        if (command.empty()) continue;

        // Обработка команд с аргументами (например, search keyword)
        std::string cmd = command;
        std::string arg;
        size_t space = command.find(' ');
        if (space != std::string::npos) {
            cmd = command.substr(0, space);
            arg = command.substr(space + 1);
        }

        try {
            if (cmd == "back") {
                if (is_open) {
                    std::cout << "Vault is still open. Saving changes (if any) and closing...\n";
                    if (vault.save()) std::cout << "Saved.\n";
                    else std::cout << "Save failed, but closing anyway.\n";
                    vault.close();
                    is_open = false;
                    current_vault.clear();
                }
                break;
            }
            else if (cmd == "help") {
                std::cout << "Available commands:\n";
                std::cout << "  create              - create new vault\n";
                std::cout << "  open                - open existing vault\n";
                std::cout << "  close               - close current vault\n";
                std::cout << "  status              - show current vault status\n";
                std::cout << "  list                - list all entries\n";
                std::cout << "  add                 - add new entry\n";
                std::cout << "  get <id>            - show entry details\n";
                std::cout << "  update <id>         - update entry\n";
                std::cout << "  delete <id>         - delete entry\n";
                std::cout << "  search <keyword>    - search entries by title, username, or URL\n";
                std::cout << "  save                - save changes to disk\n";
                std::cout << "  back                - close vault and return to main menu\n";
                std::cout << "  help                - this help\n";
            }
            else if (cmd == "create") {
                if (is_open) {
                    std::cout << "Please close current vault first (close).\n";
                    continue;
                }
                std::string filename, password, password2;
                int algo_choice;
                std::cout << "Filename: ";
                std::getline(std::cin, filename);
                std::string fullname = add_extension(filename);
                password = read_password("Master password: ");
                password2 = read_password("Confirm master password: ");
                if (password != password2) {
                    std::cout << "Passwords do not match.\n";
                    continue;
                }
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
                // Затираем пароли из памяти
                OPENSSL_cleanse(&password[0], password.size());
                OPENSSL_cleanse(&password2[0], password2.size());
            }
            else if (cmd == "open") {
                if (is_open) {
                    std::cout << "A vault is already open. Close it first.\n";
                    continue;
                }
                std::string filename, password;
                std::cout << "Filename (without extension): ";
                std::getline(std::cin, filename);
                std::string fullname = add_extension(filename);
                password = read_password("Master password: ");
                if (vault.open(fullname, password)) {
                    std::cout << "Vault opened. " << vault.get_all_entries().size() << " entries loaded.\n";
                    is_open = true;
                    current_vault = fullname;
                }
                else {
                    std::cout << "Open failed (wrong password or corrupt file).\n";
                }
                OPENSSL_cleanse(&password[0], password.size());
            }
            else if (cmd == "close") {
                if (!is_open) {
                    std::cout << "No vault is open.\n";
                    continue;
                }
                vault.close();
                is_open = false;
                current_vault.clear();
                std::cout << "Vault closed.\n";
            }
            else if (cmd == "status") {
                if (is_open) {
                    std::cout << "Opened vault: " << current_vault
                        << ", entries: " << vault.get_all_entries().size() << "\n";
                }
                else {
                    std::cout << "No vault is currently open.\n";
                }
            }
            else if (cmd == "list") {
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
            else if (cmd == "add") {
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
            else if (cmd == "get") {
                if (!is_open) {
                    std::cout << "No vault open.\n";
                    continue;
                }
                if (arg.empty()) {
                    std::cout << "Usage: get <id>\n";
                    continue;
                }
                Entry* e = vault.find_entry(arg);
                if (e) print_entry(*e);
                else std::cout << "Not found.\n";
            }
            else if (cmd == "update") {
                if (!is_open) {
                    std::cout << "No vault open.\n";
                    continue;
                }
                if (arg.empty()) {
                    std::cout << "Usage: update <id>\n";
                    continue;
                }
                Entry* e = vault.find_entry(arg);
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
            else if (cmd == "delete") {
                if (!is_open) {
                    std::cout << "No vault open.\n";
                    continue;
                }
                if (arg.empty()) {
                    std::cout << "Usage: delete <id>\n";
                    continue;
                }
                if (vault.delete_entry(arg)) std::cout << "Deleted.\n";
                else std::cout << "Not found.\n";
            }
            else if (cmd == "search") {
                if (!is_open) {
                    std::cout << "No vault open.\n";
                    continue;
                }
                if (arg.empty()) {
                    std::cout << "Usage: search <keyword>\n";
                    continue;
                }
                search_entries(vault, arg);
            }
            else if (cmd == "save") {
                if (!is_open) {
                    std::cout << "No vault open.\n";
                    continue;
                }
                if (vault.save()) std::cout << "Saved.\n";
                else std::cout << "Save failed (check file permissions).\n";
            }
            else {
                std::cout << "Unknown command. Type 'help' for list.\n";
            }
        }
        catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }
}