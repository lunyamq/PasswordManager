#ifndef CLI_H
#define CLI_H

#include "../core/vault_storage.h"
#include <string>

// Скрытый ввод пароля (без эха)
std::string read_password(const std::string& prompt);

// Функция тестирования алгоритма (с фиксированным ключом/IV для лавинности)
void test_algorithm();

// Меню менеджера паролей (хранилище)
void vault_menu();

#endif // CLI_H