#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>
#include <string>

// Магическое число для файла хранилища (little-endian: "PW\1")
constexpr uint32_t MAGIC = 0x01505700;

// Количество итераций PBKDF2
constexpr uint32_t PBKDF2_ITERATIONS = 100000;

// Расширение файла хранилища
const std::string VAULT_EXTENSION = ".pwm";

// Длина соли в байтах
constexpr size_t SALT_SIZE = 16;

#endif // CONSTANTS_H