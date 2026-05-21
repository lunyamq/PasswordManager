#include "utils.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iostream>
#include <random>
#include <openssl/rand.h>

std::vector<uint8_t> Random::generate_bytes(size_t count) {
    std::vector<uint8_t> result(count);
    if (RAND_bytes(result.data(), static_cast<int>(count)) != 1) {
        // Обработка ошибки: недостаточно энтропии
        throw std::runtime_error("RAND_bytes failed");
    }
    return result;
}

uint32_t Random::generate_uint32() {
    std::random_device rd;
    return rd();
}


void Benchmark::start() {
    start_time = std::chrono::high_resolution_clock::now();
}

double Benchmark::stop() {
    auto end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end_time - start_time).count();
}

double Benchmark::measure_throughput(size_t data_size_bytes, double time_ms) {
    if (time_ms <= 0) return 0;
    double data_size_mb = data_size_bytes / (1024.0 * 1024.0);
    double time_seconds = time_ms / 1000.0;
    return data_size_mb / time_seconds;
}

std::vector<uint8_t> string_to_bytes(const std::string& str) {
    return std::vector<uint8_t>(str.begin(), str.end());
}

std::string bytes_to_string(const std::vector<uint8_t>& bytes) {
    return std::string(bytes.begin(), bytes.end());
}

std::string bytes_to_hex(const std::vector<uint8_t>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint8_t byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

uint32_t load32_le(const uint8_t* p) {
    return (static_cast<uint32_t>(p[0]) |
        (static_cast<uint32_t>(p[1]) << 8) |
        (static_cast<uint32_t>(p[2]) << 16) |
        (static_cast<uint32_t>(p[3]) << 24));
}

void store32_le(uint8_t* p, uint32_t v) {
    p[0] = static_cast<uint8_t>(v);
    p[1] = static_cast<uint8_t>(v >> 8);
    p[2] = static_cast<uint8_t>(v >> 16);
    p[3] = static_cast<uint8_t>(v >> 24);
}

uint32_t rotate_left(uint32_t x, int n) {
    return (x << n) | (x >> (32 - n));
}

uint32_t rotate_right(uint32_t x, int n) {
    return (x >> n) | (x << (32 - n));
}
