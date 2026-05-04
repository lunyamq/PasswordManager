#include "vault_storage.h"
#include "password_manager.h"       // для PasswordManager
#include "utils.h"                  // для Random, string_to_bytes и пр.
#include <openssl/evp.h>            // для PBKDF2
#include <openssl/rand.h>           // для RAND_bytes (не используется напрямую, но Random::generate_bytes может его использовать)
#include <fstream>
#include <sstream>
#include <cstring>
#include <stdexcept>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <iostream>

// ----------------------------------------------------------------------
// Сериализация записей в текст (формат: поле1|поле2|... с экранированием)
// ----------------------------------------------------------------------
static std::string escape(const std::string& s) {
    std::string result;
    for (char c : s) {
        switch (c) {
        case '|': result += "\\|"; break;
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n"; break;
        default: result += c; break;
        }
    }
    return result;
}

static std::string unescape(const std::string& s) {
    std::string result;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            char next = s[i + 1];
            if (next == '|') result += '|';
            else if (next == '\\') result += '\\';
            else if (next == 'n') result += '\n';
            else result += s[i]; // оставить обратный слеш
            ++i;
        }
        else {
            result += s[i];
        }
    }
    return result;
}

static std::string entry_to_string(const Entry& e) {
    return escape(e.id) + "|" + escape(e.title) + "|" + escape(e.username) + "|"
        + escape(e.password) + "|" + escape(e.url) + "|" + escape(e.notes) + "|"
        + escape(e.created) + "|" + escape(e.updated);
}

static Entry string_to_entry(const std::string& line) {
    std::vector<std::string> parts;
    std::string current;
    bool escaped = false;
    for (char ch : line) {
        if (!escaped && ch == '\\') {
            escaped = true;
            continue;
        }
        if (!escaped && ch == '|') {
            parts.push_back(current);
            current.clear();
            continue;
        }
        current += ch;
        escaped = false;
    }
    parts.push_back(current);
    if (parts.size() != 8)
        throw std::runtime_error("Invalid entry format");
    Entry e;
    e.id = unescape(parts[0]);
    e.title = unescape(parts[1]);
    e.username = unescape(parts[2]);
    e.password = unescape(parts[3]);
    e.url = unescape(parts[4]);
    e.notes = unescape(parts[5]);
    e.created = unescape(parts[6]);
    e.updated = unescape(parts[7]);
    return e;
}

// ----------------------------------------------------------------------
// Реализация VaultStorage
// ----------------------------------------------------------------------
VaultStorage::VaultStorage() = default;
VaultStorage::~VaultStorage() { close(); }

bool VaultStorage::derive_key(const std::string& password, std::vector<uint8_t>& key) {
    size_t key_len = crypto_->get_key_size();
    key.resize(key_len);
    int result = PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
        salt_.data(), static_cast<int>(salt_.size()),
        iterations_, EVP_sha256(),
        static_cast<int>(key_len), key.data());
    return result == 1;
}

bool VaultStorage::write_to_file(const std::vector<uint8_t>& ciphertext) {
    std::ofstream ofs(filename_, std::ios::binary);
    if (!ofs) return false;

    uint32_t magic = 0x01505700; // "PW\1" в little-endian
    ofs.write(reinterpret_cast<const char*>(&magic), 4);

    uint8_t algo_id = static_cast<uint8_t>(cipher_type_);
    ofs.write(reinterpret_cast<const char*>(&algo_id), 1);

    uint8_t salt_len = static_cast<uint8_t>(salt_.size());
    ofs.write(reinterpret_cast<const char*>(&salt_len), 1);
    ofs.write(reinterpret_cast<const char*>(salt_.data()), salt_.size());

    ofs.write(reinterpret_cast<const char*>(&iterations_), 4);

    uint8_t iv_len = static_cast<uint8_t>(iv_.size());
    ofs.write(reinterpret_cast<const char*>(&iv_len), 1);
    ofs.write(reinterpret_cast<const char*>(iv_.data()), iv_.size());

    uint32_t ct_len = static_cast<uint32_t>(ciphertext.size());
    ofs.write(reinterpret_cast<const char*>(&ct_len), 4);
    ofs.write(reinterpret_cast<const char*>(ciphertext.data()), ct_len);

    return ofs.good();
}

bool VaultStorage::read_from_file(std::vector<uint8_t>& ciphertext) {
    std::ifstream ifs(filename_, std::ios::binary);
    if (!ifs) return false;

    uint32_t magic;
    ifs.read(reinterpret_cast<char*>(&magic), 4);
    if (magic != 0x01505700) return false;

    uint8_t algo_id;
    ifs.read(reinterpret_cast<char*>(&algo_id), 1);
    cipher_type_ = static_cast<CipherType>(algo_id);

    // Создаём PasswordManager и выбираем шифр
    crypto_ = std::make_unique<PasswordManager>();
    if (!crypto_->select_cipher(cipher_type_)) return false;

    uint8_t salt_len;
    ifs.read(reinterpret_cast<char*>(&salt_len), 1);
    salt_.resize(salt_len);
    ifs.read(reinterpret_cast<char*>(salt_.data()), salt_len);

    ifs.read(reinterpret_cast<char*>(&iterations_), 4);

    uint8_t iv_len;
    ifs.read(reinterpret_cast<char*>(&iv_len), 1);
    iv_.resize(iv_len);
    ifs.read(reinterpret_cast<char*>(iv_.data()), iv_len);

    uint32_t ct_len;
    ifs.read(reinterpret_cast<char*>(&ct_len), 4);
    ciphertext.resize(ct_len);
    ifs.read(reinterpret_cast<char*>(ciphertext.data()), ct_len);

    return ifs.good();
}

bool VaultStorage::create(const std::string& filename, const std::string& master_password, CipherType cipher_type) {
    close();
    filename_ = filename;

    crypto_ = std::make_unique<PasswordManager>();
    if (!crypto_->select_cipher(cipher_type)) return false;

    salt_ = Random::generate_bytes(16);
    iv_ = Random::generate_bytes(crypto_->get_iv_size());
    iterations_ = 100000;

    std::vector<uint8_t> key;
    if (!derive_key(master_password, key)) return false;
    derived_key_ = key;

    // Передаём ключ и IV в шифр
    std::vector<uint8_t> iv_vec(iv_.begin(), iv_.end());
    if (!crypto_->set_key_and_iv(derived_key_, iv_vec)) return false;

    // Пустое хранилище – пустая строка
    std::string empty_data;
    std::vector<uint8_t> plaintext(empty_data.begin(), empty_data.end());
    std::vector<uint8_t> ciphertext = crypto_->encrypt(plaintext.data(), plaintext.size());

    cipher_type_ = cipher_type;
    entries_.clear();

    if (!write_to_file(ciphertext)) return false;
    is_open_ = true;
    return true;
}

bool VaultStorage::open(const std::string& filename, const std::string& master_password) {
    close();
    filename_ = filename;

    std::vector<uint8_t> ciphertext;
    if (!read_from_file(ciphertext)) return false;
    if (!crypto_) return false;

    std::vector<uint8_t> key;
    if (!derive_key(master_password, key)) return false;
    derived_key_ = key;

    std::vector<uint8_t> iv_vec(iv_.begin(), iv_.end());
    if (!crypto_->set_key_and_iv(derived_key_, iv_vec)) return false;

    std::vector<uint8_t> plaintext = crypto_->decrypt(ciphertext.data(), ciphertext.size());
    std::string data(plaintext.begin(), plaintext.end());
    std::istringstream iss(data);
    std::string line;
    entries_.clear();
    while (std::getline(iss, line)) {
        if (line.empty()) continue;
        try {
            entries_.push_back(string_to_entry(line));
        }
        catch (...) {
            return false;
        }
    }

    is_open_ = true;
    return true;
}

bool VaultStorage::save() {
    if (!is_open_ || !crypto_) return false;

    std::ostringstream oss;
    for (const auto& e : entries_) {
        oss << entry_to_string(e) << '\n';
    }
    std::string data_str = oss.str();
    std::vector<uint8_t> plaintext(data_str.begin(), data_str.end());

    // Генерируем новый IV при каждом сохранении
    iv_ = Random::generate_bytes(crypto_->get_iv_size());
    std::vector<uint8_t> iv_vec(iv_.begin(), iv_.end());
    if (!crypto_->set_key_and_iv(derived_key_, iv_vec)) return false;

    std::vector<uint8_t> ciphertext = crypto_->encrypt(plaintext.data(), plaintext.size());
    return write_to_file(ciphertext);
}

void VaultStorage::close() {
    if (!derived_key_.empty()) {
        OPENSSL_cleanse(derived_key_.data(), derived_key_.size());
        derived_key_.clear();
    }
    entries_.clear();
    is_open_ = false;
    crypto_.reset();
    filename_.clear();
}

std::vector<Entry> VaultStorage::get_all_entries() const {
    return entries_;
}

bool VaultStorage::add_entry(const Entry& e) {
    entries_.push_back(e);
    return true;
}

bool VaultStorage::update_entry(const std::string& id, const Entry& e) {
    for (size_t i = 0; i < entries_.size(); ++i) {
        if (entries_[i].id == id) {
            entries_[i] = e;
            return true;
        }
    }
    return false;
}

bool VaultStorage::delete_entry(const std::string& id) {
    auto it = std::remove_if(entries_.begin(), entries_.end(),
        [&id](const Entry& e) { return e.id == id; });
    if (it == entries_.end()) return false;
    entries_.erase(it, entries_.end());
    return true;
}

Entry* VaultStorage::find_entry(const std::string& id) {
    for (auto& e : entries_)
        if (e.id == id) return &e;
    return nullptr;
}

// ----------------------------------------------------------------------
// Глобальные утилиты
// ----------------------------------------------------------------------
std::string generate_uuid() {
    auto bytes = Random::generate_bytes(16);
    std::stringstream ss;
    for (uint8_t b : bytes)
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    return ss.str();
}

std::string get_current_time() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    struct tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif
    std::stringstream ss;
    ss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void print_entry(const Entry& e) {
    std::cout << "ID: " << e.id << "\n"
        << "Title: " << e.title << "\n"
        << "Username: " << e.username << "\n"
        << "Password: " << e.password << "\n"
        << "URL: " << e.url << "\n"
        << "Notes: " << e.notes << "\n"
        << "Created: " << e.created << "\n"
        << "Updated: " << e.updated << "\n";
}