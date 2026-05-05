cmake_minimum_required(VERSION 3.16)
project(Crypt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)

# ------------------------------------------------------------
# Оптимизации для Release сборки
# ------------------------------------------------------------
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    # GCC, Clang или AppleClang
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG")
    if (APPLE)
        # macOS: -march=native не поддерживается
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
    endif()
elseif (MSVC)
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
    cli.cpp
    test_algorithm.cpp
    aes_openssl.cpp
    chacha20.cpp
    salsa20.cpp
    password_manager.cpp
    cipher_factory.cpp
    utils.cpp
    vault_storage.cpp
)

target_include_directories(Crypt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Crypt PRIVATE OpenSSL::SSL OpenSSL::Crypto)