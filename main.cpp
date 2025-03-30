#include <iostream>
#include <thread>
#include <atomic>
#include "Wersja1.h"
#include "Wersja2.h"
#include "Wersja3.h"

using namespace std;

// Globalna flaga sterująca symulacją
atomic<bool> running(true);

int main() {
    int version = 0;
    cout << "1 - Moze dojsc do zakleszczenia" << endl;
    cout << "2 - Moze dojsc do zaglodzenia" << endl;
    cout << "3 - Sprawiedliwa (brak zakleszczenia i zaglodzenia)" << endl;
    cout << "Wybierz wersje programu: ";

    while (version < 1 || version > 3) {
        cin >> version;
        if (version < 1 || version > 3) {
            cout << "Niepoprawna wersja. Wprowadz 1, 2 lub 3: ";
        }
    }

    cin.ignore(); // Usunięcie znaku nowej linii po wczytaniu numeru wersji

    if (version == 1) {
        cout << "Uruchamianie wersji 1 (moze dojsc do zakleszczenia)..." << endl;
        Wersja1 w1;
        thread simThread([&w1]() { w1.run(); });
        cout << "Wcisnij ENTER, aby zakonczyc symulacje..." << endl;
        cin.get();
        running = false;
        simThread.join();
    } else if (version == 2) {
        cout << "Uruchamianie wersji 2 (moze dojsc do zaglodzenia)..." << endl;
        Wersja2 w2;
        thread simThread([&w2]() { w2.run(); });
        cout << "Wcisnij ENTER, aby zakonczyc symulacje..." << endl;
        cin.get();
        running = false;
        simThread.join();
    } else if (version == 3) {
        cout << "Uruchamianie wersji 3 (sprawiedliwa – brak zakleszczenia i zaglodzenia)..." << endl;
        Wersja3 w3;
        thread simThread([&w3]() { w3.run(); });
        cout << "Wcisnij ENTER, aby zakonczyc symulacje..." << endl;
        cin.get();
        running = false;
        simThread.join();
    }

    cout << "Koniec symulacji." << endl;
    return 0;
}
