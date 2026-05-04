        #ifndef CIPHER_INTERFACE_H
        #define CIPHER_INTERFACE_H

        #include <cstddef>
        #include <cstdint>
        #include <vector>
        #include <memory>
        #include <string>

        enum class CipherType {
            AES_256,
            AES_128,
            CHACHA20,
            SALSA20
        };

        class Cipher {
        public:
            virtual ~Cipher() = default;

            virtual bool init(const uint8_t* key, size_t key_len, const uint8_t* iv, size_t iv_len) = 0;

            virtual void encrypt(const uint8_t* plaintext, uint8_t* ciphertext, size_t length) = 0;

            virtual void decrypt(const uint8_t* ciphertext, uint8_t* plaintext, size_t length) = 0;

            virtual size_t get_block_size() const = 0;
            virtual size_t get_key_size() const = 0;
            virtual size_t get_iv_size() const = 0;

            virtual std::string get_name() const = 0;
            virtual CipherType get_type() const = 0;
        };

        class AESImpl;
        class ChaCha20;
        class Salsa20;

        std::unique_ptr<Cipher> create_cipher(CipherType type);
        std::string cipher_type_to_string(CipherType type);
        CipherType string_to_cipher_type(const std::string& str);

        #endif