#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include <atomic>
#include "Wersja1.h"
#include "Wersja2.h"
#include "Wersja3.h"
#include "TableView.h"

using namespace std;

atomic<bool> running(true);
atomic<int> racePauseMs(0);
atomic<bool> stopRequested(false);

template <class Simulation>
void runSimulation(int version) {
    unique_ptr<Simulation> sim = make_unique<Simulation>();
    thread simThread([s = sim.get()]() { s->run(); });

    cout << "Wcisnij ENTER w konsoli albo zamknij okno, aby zakonczyc symulacje..." << endl;
    thread([]() {
        cin.get();
        stopRequested = true;
        running = false;
    }).detach();

    auto snapshot = [&sim]() { return sim->snapshot(); };
    auto restart = [&]() {
        running = false;
        simThread.join();
        if (stopRequested.load()) return;
        cout << "\n==== NOWY START SYMULACJI ====" << endl;
        sim = make_unique<Simulation>();
        running = true;
        simThread = thread([s = sim.get()]() { s->run(); });
    };

    runTableView(version, snapshot, restart, running, racePauseMs);

    running = false;
    if (simThread.joinable()) simThread.join();
}

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

    cin.ignore();

    if (version == 1) {
        cout << "Uruchamianie wersji 1 (moze dojsc do zakleszczenia)..." << endl;
        runSimulation<Wersja1>(version);
    } else if (version == 2) {
        cout << "Uruchamianie wersji 2 (moze dojsc do zaglodzenia)..." << endl;
        runSimulation<Wersja2>(version);
    } else if (version == 3) {
        cout << "Uruchamianie wersji 3 (sprawiedliwa, brak zakleszczenia i zaglodzenia)..." << endl;
        runSimulation<Wersja3>(version);
    }

    cout << "Koniec symulacji." << endl;

    // Watek czekajacy na ENTER nadal blokuje stdin, wiec zwykle wyjscie z main by sie zawiesilo
    if (!stopRequested.load()) {
        cout.flush();
        _Exit(0);
    }
    return 0;
}
