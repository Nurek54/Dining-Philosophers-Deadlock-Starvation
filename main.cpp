#include <iostream>
#include "Wersja1.h"
#include "Wersja2.h"
#include "Wersja3.h"

int main() {
    int version = 0;

    std::cout << "Wybierz wersje programu (1 - naive, 2 - bez zakleszczenia, 3 - sprawiedliwa): ";
    while (version < 1 || version > 3) {
        std::cin >> version;
        if (version < 1 || version > 3) {
            std::cout << "Niepoprawna wersja. Wprowadz 1, 2 lub 3: ";
        }
    }

    if (version == 1) {
        std::cout << "Uruchamianie wersji 1 (moze dojsc do zakleszczenia)..." << std::endl;
        Wersja1 w1;
        w1.run();
    } else if (version == 2) {
        std::cout << "Uruchamianie wersji 2 (bez zakleszczenia, ale nierownosc)..." << std::endl;
        Wersja2 w2;
        w2.run();
    } else if (version == 3) {
        std::cout << "Uruchamianie wersji 3 (brak zakleszczenia i sprawiedliwy podzial)..." << std::endl;
        Wersja3 w3;
        w3.run();
    }

    return 0;
}
