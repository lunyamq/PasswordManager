#include "cli.h"
#include "password_manager.h"
#include <iostream>
#include <limits>

#ifdef _WIN32
#include <conio.h>
#define GETCH() _getch()
#else
#include <termios.h>
#include <unistd.h>
static int getch_linux() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    int ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#define GETCH() getch_linux()
#endif

int main() {
    std::cout << "=== Cryptographic Password Manager ===\n";

    while (true) {
        std::cout << "\nMain Menu:\n";
        std::cout << "1. Password Manager\n";
        std::cout << "2. Run benchmarks\n";
        std::cout << "3. Test algorithm\n";
        std::cout << "0. Exit\n";
        std::cout << "Choice: ";

        int choice;
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
        case 1:
            vault_menu();
            break;
        case 2:
            PasswordManager::run_benchmarks();
            break;
        case 3:
            test_algorithm();
            break;
        case 0:
            std::cout << "Exiting.\n";
            return 0;
        default:
            std::cout << "Invalid choice.\n";
        }

        std::cout << "\nPress any key to continue...";
        GETCH();
        std::cout << "\n";
    }
    return 0;
}