#include "password_manager.h"
#include "utils.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <map>
#include <algorithm>
#include <stdexcept>

PasswordManager::PasswordManager() { }

PasswordManager::~PasswordManager() { }

bool PasswordManager::select_cipher(CipherType type) {
    cipher = create_cipher(type);
    if (!cipher) {
        std::cerr << "Error: Failed to create cipher\n";
        return false;
    }

    std::cout << "Selected: " << cipher->get_name() << "\n";
    return true;
}

bool PasswordManager::set_key_and_iv(const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv) {
    if (!cipher) {
        std::cerr << "Error: Cipher not selected\n";
        return false;
    }

    if (key.size() != cipher->get_key_size()) {
        std::cerr << "Error: Invalid key size. Expected " << cipher->get_key_size() << " bytes, got " << key.size() << " bytes\n";
        return false;
    }

    if (iv.size() != cipher->get_iv_size()) {
        std::cerr << "Error: Invalid IV size. Expected " << cipher->get_iv_size() << " bytes, got " << iv.size() << " bytes\n";
        return false;
    }

    return cipher->init(key.data(), key.size(), iv.data(), iv.size());
}

std::vector<uint8_t> PasswordManager::encrypt(const uint8_t* data, size_t size) {
    if (!cipher) {
        throw std::runtime_error("Cipher not selected");
    }

    std::vector<uint8_t> ciphertext(size);
    cipher->encrypt(data, ciphertext.data(), size);
    return ciphertext;
}

std::vector<uint8_t> PasswordManager::decrypt(const uint8_t* data, size_t size) {
    if (!cipher) {
        throw std::runtime_error("Cipher not selected");
    }

    std::vector<uint8_t> plaintext(size);
    cipher->decrypt(data, plaintext.data(), size);
    return plaintext;
}

size_t PasswordManager::get_key_size() const {
    if (!cipher) return 0;
    return cipher->get_key_size();
}

size_t PasswordManager::get_iv_size() const {
    if (!cipher) return 0;
    return cipher->get_iv_size();
}

std::string PasswordManager::get_cipher_name() const {
    if (!cipher) return "None";
    return cipher->get_name();
}

void PasswordManager::run_benchmarks() {
    std::cout << "\n=== Encryption Algorithms Benchmark ===\n\n";

    const size_t test_size = 1024 * 1024; // 1 MB
    auto test_data = Random::generate_bytes(test_size);

    struct BenchmarkResult {
        double encrypt_mbps;
        double decrypt_mbps;
        bool works;
    };

    std::map<CipherType, BenchmarkResult> results;

    // Перебираем алгоритмы
    for (auto type : { CipherType::AES_128, CipherType::AES_256, CipherType::CHACHA20, CipherType::SALSA20 }) {

        std::cout << "Testing " << cipher_type_to_string(type) << "...\n";

        // --- Сначала проверяем работоспособность (шифрование/расшифрование маленького блока) ---
        PasswordManager pm_test;
        if (!pm_test.select_cipher(type)) {
            std::cout << "  Skipped (cipher creation error)\n\n";
            continue;
        }

        auto key = Random::generate_bytes(pm_test.get_key_size());
        auto iv = Random::generate_bytes(pm_test.get_iv_size());

        if (!pm_test.set_key_and_iv(key, iv)) {
            std::cout << "  Skipped (key/IV error)\n\n";
            continue;
        }

        std::string test_str = "Test123";
        auto test_bytes = string_to_bytes(test_str);
        auto encrypted_test = pm_test.encrypt(test_bytes.data(), test_bytes.size());

        // Для расшифрования создаём НОВЫЙ менеджер, чтобы избежать проблем с состоянием
        PasswordManager pm_test_dec;
        pm_test_dec.select_cipher(type);
        pm_test_dec.set_key_and_iv(key, iv);
        auto decrypted_test = pm_test_dec.decrypt(encrypted_test.data(), encrypted_test.size());

        bool works = (test_bytes == decrypted_test);
        if (!works) {
            std::cout << "  Functionality test FAILED, skipping benchmark\n\n";
            results[type] = { 0.0, 0.0, false };
            continue;
        }

        // --- Измерение производительности шифрования (отдельный менеджер) ---
        PasswordManager pm_enc;
        pm_enc.select_cipher(type);
        pm_enc.set_key_and_iv(key, iv);

        Benchmark bench;
        bench.start();
        auto encrypted_data = pm_enc.encrypt(test_data.data(), test_size);
        double encrypt_time = bench.stop();
        double encrypt_mbps = Benchmark::measure_throughput(test_size, encrypt_time);

        // --- Измерение производительности расшифрования (ещё один менеджер) ---
        PasswordManager pm_dec;
        pm_dec.select_cipher(type);
        pm_dec.set_key_and_iv(key, iv);

        bench.start();
        auto decrypted_data = pm_dec.decrypt(encrypted_data.data(), encrypted_data.size());
        double decrypt_time = bench.stop();
        double decrypt_mbps = Benchmark::measure_throughput(test_size, decrypt_time);

        // Дополнительная проверка: расшифрованные данные должны совпадать с исходными
        bool data_ok = (test_data == decrypted_data);
        if (!data_ok) {
            std::cout << "  WARNING: Decrypted 1MB data does not match original!\n";
        }

        results[type] = { encrypt_mbps, decrypt_mbps, works && data_ok };

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "  Encryption: " << encrypt_mbps << " MB/s\n";
        std::cout << "  Decryption: " << decrypt_mbps << " MB/s\n";
        std::cout << "  Works: " << (works ? "YES" : "NO") << "\n\n";
    }

    // --- Вывод сводной таблицы ---
    std::cout << "=== Performance Summary ===\n";
    std::cout << std::left << std::setw(12) << "Algorithm"
        << std::setw(18) << "Encrypt (MB/s)"
        << std::setw(18) << "Decrypt (MB/s)"
        << std::setw(10) << "Status" << "\n";
    std::cout << std::string(58, '-') << "\n";

    for (const auto& [type, result] : results) {
        std::cout << std::left << std::setw(12) << cipher_type_to_string(type)
            << std::setw(18) << result.encrypt_mbps
            << std::setw(18) << result.decrypt_mbps
            << std::setw(10) << (result.works ? "OK" : "FAIL") << "\n";
    }
}