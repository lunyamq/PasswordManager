# Password Manager

## Зависимости
- **C++17**
- **OpenSSL**
- **CMake** 3.16

### Установить инструменты
1. **Visual Studio** https://visualstudio.microsoft.com/
2. **OpenSSL** https://slproweb.com/products/Win32OpenSSL.html
3. **CMake** https://cmake.org/download/

## Сборка проекта
```bat
git clone https://github.com/lunyamq/PasswordManager.git
cd PasswordManager/Crypt

mkdir build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```
Готовый файл появится в директории `Debug`

## Функционал
1. Тест отдельного алгоритма
2. Общий тест производительности
3. Выйти
