#ifndef VAULT_STORAGE_H
#define VAULT_STORAGE_H

#include "cipher_interface.h"
#include "password_manager.h"
#include "constants.h"
#include <vector>
#include <string>
#include <memory>
#include <cstdint>

// Структура одной записи (логин/пароль)
struct Entry {
    std::string id;
    std::string title;
    std::string username;
    std::string password;
    std::string url;
    std::string notes;
    std::string created;
    std::string updated;
};

// Класс для работы с зашифрованным хранилищем
class VaultStorage {
public:
    VaultStorage();
    ~VaultStorage();

    // Создать новое хранилище (файл)
    bool create(const std::string& filename,
        const std::string& master_password,
        CipherType cipher_type);

    // Открыть существующее хранилище (расшифровать и загрузить записи)
    bool open(const std::string& filename,
        const std::string& master_password);

    // Сохранить текущие записи обратно в файл (перешифровать)
    bool save();

    // Закрыть хранилище (очистить ключи и записи из памяти)
    void close();

    // Работа с записями
    std::vector<Entry> get_all_entries() const;
    bool add_entry(const Entry& e);
    bool update_entry(const std::string& id, const Entry& e);
    bool delete_entry(const std::string& id);
    Entry* find_entry(const std::string& id);

    bool is_open() const { return is_open_; }

private:
    bool is_open_ = false;
    std::string filename_;
    CipherType cipher_type_;
    std::vector<Entry> entries_;
    std::vector<uint8_t> salt_;            // соль для PBKDF2
    std::vector<uint8_t> iv_;              // вектор инициализации
    uint32_t iterations_ = 100000;         // число итераций KDF
    std::unique_ptr<PasswordManager> crypto_; // менеджер шифрования
    std::vector<uint8_t> derived_key_;     // ключ, полученный из мастер-пароля

    bool derive_key(const std::string& password, std::vector<uint8_t>& key);
    bool write_to_file(const std::vector<uint8_t>& ciphertext);
    bool read_from_file(std::vector<uint8_t>& ciphertext);
};

// Вспомогательные функции (реализованы в .cpp)
std::string generate_uuid();
std::string get_current_time();
void print_entry(const Entry& e);

#endif // VAULT_STORAGE_H