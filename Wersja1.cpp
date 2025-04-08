#include "Wersja1.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <iomanip>

using namespace std;

extern atomic<bool> running;

Wersja1::Wersja1() : chopsticks(NUM_PHILOSOPHERS), mealsEaten(NUM_PHILOSOPHERS) {
    srand(static_cast<unsigned int>(time(NULL)));
    // Inicjalizacja licznikow posilkow
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        mealsEaten[i] = 0;
    }
    names = {"Yoda", "Gandalf", "ObiWan", "Anakin", "Leia"};
}

void Wersja1::printMessage(const string &msg) {
    lock_guard<mutex> lock(printMutex);
    cout << msg << endl;
}

int Wersja1::randomTime(int min, int max, int philosopherId) {
    int bias = philosopherId * 300;  // jednolity bias dla wszystkich filozofów
    return rand() % (max - min + 1) + min + bias;
}

void Wersja1::philosopher(int id) {
    int right = id;
    int left = (id - 1 + NUM_PHILOSOPHERS) % NUM_PHILOSOPHERS;

    while (running.load()) {
        // Najpierw pobiera prawą pałeczkę (własną)
        while (running.load()) {
            if (chopsticks[right].mtx.try_lock()) {
                chopsticks[right].holder.store(id);
                {
                    lock_guard<mutex> lock(printMutex);
                    cout << "[" << names[id] << " (Filozof " << id
                         << ")] podniosl prawa paleczke (" << right << ")." << endl;
                }
                break;
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        if (!running.load())
            break;
        // Następnie pobieramy lewą pałeczkę, czyli prawą pałeczkę lewego sąsiada
        while (running.load()) {
            if (chopsticks[left].mtx.try_lock()) {
                chopsticks[left].holder.store(id);
                {
                    lock_guard<mutex> lock(printMutex);
                    cout << "[" << names[id] << " (Filozof " << id
                         << ")] podniosl lewa paleczke (" << left << ")." << endl;
                }
                break;
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        if (!running.load()) {
            chopsticks[right].mtx.unlock();
            break;
        }

        // Jedzenie (czas losowany od 1 do 4 sekund, z opóźnieniem zależnym od id)
        {
            lock_guard<mutex> lock(printMutex);
            cout << "[" << names[id] << " (Filozof " << id << ")] je." << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(randomTime(1000, 4000, id)));

        // Inkrementacja licznika posiłków
        mealsEaten[id]++;

        // Odkładanie pałeczek (najpierw lewej, potem prawej)
        chopsticks[left].holder.store(-1);
        chopsticks[left].mtx.unlock();
        chopsticks[right].holder.store(-1);
        chopsticks[right].mtx.unlock();

        // Myślenie (czas losowany od 2 do 5 sekund, z opóźnieniem)
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
                    string rightStatus;
                    int holderRight = chopsticks[i].holder.load();
                    if (holderRight == -1) {
                        rightStatus = "wolna";
                    } else {
                        rightStatus = "F" + to_string(holderRight);
                    }

                    int lIndex = (i - 1 + NUM_PHILOSOPHERS) % NUM_PHILOSOPHERS;
                    string leftStatus;
                    int holderLeft = chopsticks[lIndex].holder.load();
                    if (holderLeft == -1) {
                        leftStatus = "wolna";
                    } else {
                        leftStatus = "F" + to_string(holderLeft);
                    }

                    cout << left << setw(12) << names[i]
                         << setw(20) << leftStatus
                         << setw(20) << rightStatus
                         << setw(10) << mealsEaten[i].load() << endl;
                }
                cout << "--------------------------------------------------------------" << endl;
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
