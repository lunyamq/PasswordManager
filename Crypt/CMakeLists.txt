cmake_minimum_required(VERSION 3.16)

project(Crypt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)

add_executable(Crypt
    main.cpp
    aes_openssl.cpp
    chacha20.cpp
    blowfish_simple.cpp
    cipher_factory.cpp
    password_manager.cpp
    utils.cpp
)

target_include_directories(Crypt PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Crypt PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
)
