cmake_minimum_required(VERSION 3.16)
project(Crypt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)

# ------------------------------------------------------------
# Оптимизации для Release сборки
# ------------------------------------------------------------
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # GCC или Clang
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
elseif (MSVC)
    # Visual Studio
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /ZI /DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
else()
    message(WARNING "Unknown compiler, using default flags")
endif()

# ------------------------------------------------------------
# Добавляем исполняемый файл
# ------------------------------------------------------------
add_executable(Crypt
    main.cpp
    aes_openssl.cpp
    chacha20.cpp
    salsa20.cpp
    cipher_factory.cpp
    password_manager.cpp
    utils.cpp
)

target_include_directories(Crypt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Crypt PRIVATE OpenSSL::SSL OpenSSL::Crypto)