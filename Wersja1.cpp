#include "Wersja1.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <iomanip>

using namespace std;

// Globalna flaga sterujaca - zadeklarowana w main.cpp
extern atomic<bool> running;

Wersja1::Wersja1() : chopsticks(NUM_PHILOSOPHERS), mealsEaten(NUM_PHILOSOPHERS) {
    srand(static_cast<unsigned int>(time(NULL)));
    // Inicjalizacja licznikow posilkow
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        mealsEaten[i] = 0;
    }
    // Inicjalizacja nazw filozofow
    names = {"Yoda", "Gandalf", "ObiWan", "Anakin", "Leia"};
}

void Wersja1::printMessage(const string &msg) {
    lock_guard<mutex> lock(printMutex);
    cout << msg << endl;
}

int Wersja1::randomTime(int min, int max, int philosopherId) {
    int bias = philosopherId * 300;  // jednolity bias dla wszystkich filozofow
    return rand() % (max - min + 1) + min + bias;
}

void Wersja1::philosopher(int id) {
    int right = (id + 1) % NUM_PHILOSOPHERS;
    while (running.load()) {
        // Probujemy podniesc lewa paleczke (wlasna)
        while (running.load()) {
            if (chopsticks[id].mtx.try_lock()) {
                chopsticks[id].holder.store(id);
                {
                    lock_guard<mutex> lock(printMutex);
                    cout << "[" << names[id] << " (Filozof " << id << ")] podniosl lewa paleczke ("
                         << id << ")." << endl;
                }
                break;
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        if (!running.load())
            break;
        // Probujemy podniesc prawa paleczke (sasiada z lewej)
        while (running.load()) {
            if (chopsticks[right].mtx.try_lock()) {
                chopsticks[right].holder.store(id);
                {
                    lock_guard<mutex> lock(printMutex);
                    cout << "[" << names[id] << " (Filozof " << id << ")] podniosl prawa paleczke ("
                         << right << ")." << endl;
                }
                break;
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        if (!running.load()) {
            chopsticks[id].mtx.unlock();
            break;
        }

        // Jedzenie (czas losowany od 1 do 4 sekund, z opoznieniem zaleznym od id)
        {
            lock_guard<mutex> lock(printMutex);
            cout << "[" << names[id] << " (Filozof " << id << ")] je." << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(randomTime(1000, 4000, id)));

        // Inkrementacja licznika posilkow
        mealsEaten[id]++;

        // Okladanie paleczek
        chopsticks[right].holder.store(-1);
        chopsticks[right].mtx.unlock();
        chopsticks[id].holder.store(-1);
        chopsticks[id].mtx.unlock();

        // Myslenie (czas losowany od 2 do 5 sekund, z opoznieniem)
        {
            lock_guard<mutex> lock(printMutex);
            cout << "[" << names[id] << " (Filozof " << id << ")] mysli." << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(randomTime(2000, 5000, id)));
    }
}

void Wersja1::run() {
    vector<thread> threads;
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        threads.emplace_back(&Wersja1::philosopher, this, i);
    }

    // Watek monitorujacy - co 1 sekunde wyswietla stan obu paleczek (lewa i prawa) oraz licznik posilkow
    thread monitor([this]() {
        while (running.load()) {
            {
                lock_guard<mutex> lock(printMutex);
                cout << "\n==== TABLICA STATUSU ====" << endl;
                cout << left << setw(12) << "Filozof"
                     << setw(20) << "Lewa paleczka"
                     << setw(20) << "Prawa paleczka"
                     << setw(10) << "Posilki" << endl;
                cout << "--------------------------------------------------------------" << endl;
                for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
                    string leftStatus, rightStatus;
                    // Dla filozofa i: lewa paleczka ma indeks i
                    if (chopsticks[i].holder.load() == i)
                        leftStatus = "W rece";
                    else if (chopsticks[i].holder.load() == -1)
                        leftStatus = "Wolna";
                    else
                        leftStatus = "P" + to_string(chopsticks[i].holder.load());

                    // Prawa paleczka - indeks (i+1)%NUM_PHILOSOPHERS
                    int rIndex = (i + 1) % NUM_PHILOSOPHERS;
                    if (chopsticks[rIndex].holder.load() == i)
                        rightStatus = "W rece";
                    else if (chopsticks[rIndex].holder.load() == -1)
                        rightStatus = "Wolna";
                    else
                        rightStatus = "P" + to_string(chopsticks[rIndex].holder.load());

                    cout << left << setw(12) << names[i]
                         << setw(20) << leftStatus
                         << setw(20) << rightStatus
                         << setw(10) << mealsEaten[i].load() << endl;
                }
                cout << "===================================" << endl;
            }
            this_thread::sleep_for(chrono::seconds(1));
        }
    });

    for (auto &t : threads) {
        t.join();
    }
    monitor.join();

    printMessage("Symulacja zakonczona.");
}
