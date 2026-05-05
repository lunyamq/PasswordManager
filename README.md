# Password Manager
Кроссплатформенный менеджер паролей с поддержкой AES-256, ChaCha20 и Salsa20.

## Зависимости
- **C++17**
- **OpenSSL** (криптографическая библиотека)
- **CMake** (система сборки)

## Сборка на Windows (вручную)
1. **Установите Visual Studio** (с поддержкой C++) – [скачать](https://visualstudio.microsoft.com/)
2. **Установите CMake** – [скачать](https://cmake.org/download/)
3. **Установите OpenSSL** – [скачать](https://slproweb.com/products/Win32OpenSSL.html)

```cmd
cd PasswordManager/Crypt
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
.\Release\Crypt.exe
```

## Сборка на macOS (Homebrew)
```bash
brew install cmake openssl

cd PasswordManager/Crypt
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DOPENSSL_ROOT_DIR=$(brew --prefix openssl)
cmake --build . --config Release
./Crypt
```

## Сборка на Linux 
```bash
sudo apt install build-essential cmake libssl-dev

cd PasswordManager/Crypt
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
./Crypt
```

## Функционал
1. Менеджер паролей – создание/открытие зашифрованного хранилища, добавление/поиск/редактирование записей.
2. Общий тест производительности – сравнительный бенчмарк алгоритмов.
3. Тест конкретного алгоритма – шифрование/расшифрование произвольного текста.
0. Выйти

## Структура хранилища
- Файл базы сохраняется с расширением `.pwm`
- Используется PBKDF2-HMAC-SHA256 для получения ключа из мастер‑пароля.
- Алгоритм шифрования базы выбирается при создании файла.