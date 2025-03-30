#include "Wersja2.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <iomanip>

using namespace std;

// Globalna flaga sterujaca symulacja zadeklarowana w main.cpp
extern atomic<bool> running;

Wersja2::Wersja2() : chopsticks(NUM_PHILOSOPHERS), mealsEaten(NUM_PHILOSOPHERS) {
    srand(static_cast<unsigned int>(time(NULL)));
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        mealsEaten[i] = 0;
    }
    names = {"Yoda", "Gandalf", "ObiWan", "Anakin", "Leia"};
}

void Wersja2::printMessage(const string &msg) {
    lock_guard<mutex> lock(printMutex);
    cout << msg << endl;
}

int Wersja2::randomTime(int min, int max, int philosopherId) {
    int bias = philosopherId * 300;
    return rand() % (max - min + 1) + min + bias;
}

void Wersja2::philosopher(int id) {
    int left = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;
    while (running.load()) {
        // Myslenie
        {
            lock_guard<mutex> lock(printMutex);
            cout << "[" << names[id] << " (Filozof " << id << ")] mysli." << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(randomTime(1000, 4000, id)));

        // Proba podniesienia lewej paleczki
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

        // Proba podniesienia prawej paleczki
        // Dla filozofa 4 (id == 4) wprowadzamy ograniczona liczbe prob -
        // jesli sie nie uda, zwalnia lewa paleczke i wraca do myslenia.
        int attempts = 0;
        bool rightAcquired = false;
        while (running.load() && attempts < 5) {
            if (chopsticks[right].mtx.try_lock()) {
                chopsticks[right].holder.store(id);
                {
                    lock_guard<mutex> lock(printMutex);
                    cout << "[" << names[id] << " (Filozof " << id
                         << ")] podniosl prawa paleczke (" << right << ")." << endl;
                }
                rightAcquired = true;
                break;
            }
            attempts++;
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        if (id == 4 && !rightAcquired) {
            {
                lock_guard<mutex> lock(printMutex);
                cout << "[" << names[id] << " (Filozof " << id << ")] nie udalo sie pobrac prawej paleczki po "
                     << attempts << " probach, zwalnianie lewej paleczki." << endl;
            }
            chopsticks[left].holder.store(-1);
            chopsticks[left].mtx.unlock();
            continue;
        }
        // Jedzenie
        {
            lock_guard<mutex> lock(printMutex);
            cout << "[" << names[id] << " (Filozof " << id << ")] je." << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(randomTime(1000, 4000, id)));
        mealsEaten[id]++;

        // Okladanie paleczek
        chopsticks[left].holder.store(-1);
        chopsticks[left].mtx.unlock();
        chopsticks[right].holder.store(-1);
        chopsticks[right].mtx.unlock();

        {
            lock_guard<mutex> lock(printMutex);
            cout << "[" << names[id] << " (Filozof " << id
                 << ")] zakonczyl posilek i mysli ponownie." << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(randomTime(2000, 5000, id)));
    }
}

void Wersja2::run() {
    vector<thread> threads;
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        threads.emplace_back(&Wersja2::philosopher, this, i);
    }

    // Monitor - wyswietla co 1 sekunde stan paleczek i licznik posilkow w jednej tabelce
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
                    if (chopsticks[i].holder.load() == i)
                        leftStatus = "W rece";
                    else if(chopsticks[i].holder.load() == -1)
                        leftStatus = "Wolna";
                    else
                        leftStatus = "P" + to_string(chopsticks[i].holder.load());

                    int rIndex = (i + 1) % NUM_PHILOSOPHERS;
                    if (chopsticks[rIndex].holder.load() == i)
                        rightStatus = "W rece";
                    else if(chopsticks[rIndex].holder.load() == -1)
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
