cmake_minimum_required(VERSION 3.16)
project(Crypt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)

# ------------------------------------------------------------
# Оптимизации для Release сборки (оставляем как было)
# ------------------------------------------------------------
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG")
    if (APPLE)
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
# Статическая библиотека с общей логикой (шифры, хранилище, утилиты)
# ------------------------------------------------------------
add_library(crypt_core STATIC
    core/aes_openssl.cpp
    core/chacha20.cpp
    core/salsa20.cpp
    core/trivium.cpp
    core/password_manager.cpp
    core/cipher_factory.cpp
    core/utils.cpp
    core/vault_storage.cpp
)

target_include_directories(crypt_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(crypt_core PRIVATE OpenSSL::SSL OpenSSL::Crypto)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/app.rc")

# ------------------------------------------------------------
# Консольная версия (CLI)
# ------------------------------------------------------------
add_executable(crypt-cli
    cli/cli.cpp
    cli/test_algorithm.cpp
    cli/main_cli.cpp
    ${app_icon_resource_windows}
)

target_include_directories(crypt-cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(crypt-cli PRIVATE crypt_core OpenSSL::SSL OpenSSL::Crypto)

# ------------------------------------------------------------
# Графическая версия (GUI) – требует Qt6
# ------------------------------------------------------------cmake --build . --config Release
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Включаем автоматическую обработку Qt макросов
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

qt_add_executable(crypt-gui
    gui/main_gui.cpp
    gui/mainwindow.cpp
    gui/mainwindow.h
    ${app_icon_resource_windows}
)

if(WIN32)
    set_target_properties(crypt-gui PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif()

target_include_directories(crypt-gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(crypt-gui PRIVATE
    crypt_core
    Qt6::Core
    Qt6::Widgets
    OpenSSL::SSL
    OpenSSL::Crypto
)