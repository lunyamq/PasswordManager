cmake_minimum_required(VERSION 3.16)

project(Crypt
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)

add_executable(Crypt
    Source Files/main.cpp
    Source Files/aes_openssl.cpp
    Source Files/chacha20.cpp
    Source Files/blowfish_simple.cpp
    Source Files/cipher_factory.cpp
    Source Files/password_manager.cpp
    Source Files/utils.cpp
)

target_include_directories(Crypt PRIVATE
    Header Files
)

target_link_libraries(Crypt PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
)

