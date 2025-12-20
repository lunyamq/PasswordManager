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

    const size_t test_size = 1024 * 1024; // 1MB
    auto test_data = Random::generate_bytes(test_size);

    struct BenchmarkResult {
        double encrypt_mbps;
        double decrypt_mbps;
        bool works;
    };

    std::map<CipherType, BenchmarkResult> results;

    for (auto type : { CipherType::AES_128, CipherType::AES_256, CipherType::CHACHA20, CipherType::BLOWFISH }) {

        std::cout << "Testing " << cipher_type_to_string(type) << "...\n";

        PasswordManager pm;
        if (!pm.select_cipher(type)) {
            std::cout << "  Skipped (initialization error)\n\n";
            continue;
        }

        auto key = Random::generate_bytes(pm.get_key_size());
        auto iv = Random::generate_bytes(pm.get_iv_size());

        if (!pm.set_key_and_iv(key, iv)) {
            std::cout << "  Skipped (key/IV error)\n\n";
            continue;
        }

        Benchmark bench;
        BenchmarkResult result;

        std::string test_str = "Test123";
        auto test_bytes = string_to_bytes(test_str);
        auto encrypted = pm.encrypt(test_bytes.data(), test_bytes.size());
        auto decrypted = pm.decrypt(encrypted.data(), encrypted.size());
        result.works = (test_bytes == decrypted);

        bench.start();
        encrypted = pm.encrypt(test_data.data(), test_size);
        double encrypt_time = bench.stop();
        result.encrypt_mbps = Benchmark::measure_throughput(test_size, encrypt_time);

        bench.start();
        decrypted = pm.decrypt(encrypted.data(), encrypted.size());
        double decrypt_time = bench.stop();
        result.decrypt_mbps = Benchmark::measure_throughput(test_size, decrypt_time);

        results[type] = result;

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "  Encryption: " << result.encrypt_mbps << " MB/s\n";
        std::cout << "  Decryption: " << result.decrypt_mbps << " MB/s\n";
        std::cout << "  Works: " << (result.works ? "YES" : "NO") << "\n\n";
    }

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