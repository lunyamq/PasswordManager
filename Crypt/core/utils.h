#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <vector>
#include <string>
#include <chrono>
#include <random>

class Random {
public:
    static std::vector<uint8_t> generate_bytes(size_t count);
    static uint32_t generate_uint32();
};

class Benchmark {
private:
    std::chrono::high_resolution_clock::time_point start_time;

public:
    void start();
    double stop();

    static double measure_throughput(size_t data_size_bytes, double time_ms);
};

std::vector<uint8_t> string_to_bytes(const std::string& str);
std::string bytes_to_string(const std::vector<uint8_t>& bytes);
std::string bytes_to_hex(const std::vector<uint8_t>& bytes);

// Функции для работы с 32-битными словами
uint32_t load32_le(const uint8_t* p);
void store32_le(uint8_t* p, uint32_t v);
uint32_t rotate_left(uint32_t x, int n);
uint32_t rotate_right(uint32_t x, int n);

#endif