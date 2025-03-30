#include "Wersja3.h"
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <algorithm>
#include <thread>
#include <iomanip>

using namespace std;

// Globalna flaga sterujaca symulacja zadeklarowana w main.cpp
extern atomic<bool> running;

// Implementacja klasy Waiter
Wersja3::Waiter::Waiter(int count) : available(count) {}

void Wersja3::Waiter::acquire() {
    unique_lock<mutex> lock(mtx);
    while (available <= 0) {
        cv.wait(lock);
    }
    available--;
}

void Wersja3::Waiter::release() {
    unique_lock<mutex> lock(mtx);
    available++;
    cv.notify_all();
}

Wersja3::Wersja3()
        : chopsticks(NUM_PHILOSOPHERS),
          waiter(NUM_PHILOSOPHERS - 1),
          mealsEaten(NUM_PHILOSOPHERS)
{
    srand(static_cast<unsigned int>(time(NULL)));
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        mealsEaten[i] = 0;
    }
    names = {"Yoda", "Gandalf", "ObiWan", "Anakin", "Leia"};
}

void Wersja3::printMessage(const string &msg) {
    lock_guard<mutex> lock(printMutex);
    cout << msg << endl;
}

int Wersja3::randomTime(int min, int max, int philosopherId) {
    int bias = philosopherId * 300;
    return rand() % (max - min + 1) + min + bias;
}

void Wersja3::philosopher(int id) {
    int left = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;
    while (running.load()) {
        // Sprawiedliwosc - sprawdzamy minimalna liczbe posilkow
        int minMeals = INT_MAX;
        for (int j = 0; j < NUM_PHILOSOPHERS; j++) {
            minMeals = min(minMeals, mealsEaten[j].load());
        }
        if (mealsEaten[id].load() > minMeals + 1) {
            {
                lock_guard<mutex> lock(printMutex);
                cout << "[" << names[id] << " (Filozof " << id
                     << ")] czeka (Posilki: " << mealsEaten[id].load()
                     << ", Min: " << minMeals << ")." << endl;
            }
            this_thread::sleep_for(chrono::milliseconds(500));
            continue;
        }

        {
            lock_guard<mutex> lock(printMutex);
            cout << "[" << names[id] << " (Filozof " << id << ")] mysli." << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(randomTime(1000, 4000, id)));

        // Uzyskujemy zgode od waitera
        waiter.acquire();
        {
            lock_guard<mutex> lock(printMutex);
            cout << "[" << names[id] << " (Filozof " << id
                 << ")] otrzymal zgode od waitera." << endl;
        }

        // Pobieranie paleczek - w kolejnosci rosnacej indeksu
        int first = min(left, right);
        int second = max(left, right);

        while (running.load()) {
            if (chopsticks[first].mtx.try_lock()) {
                chopsticks[first].holder.store(id);
                {
                    lock_guard<mutex> lock(printMutex);
                    cout << "[" << names[id] << " (Filozof " << id
                         << ")] podniosl paleczke (" << first << ")." << endl;
                }
                break;
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        while (running.load()) {
            if (chopsticks[second].mtx.try_lock()) {
                chopsticks[second].holder.store(id);
                {
                    lock_guard<mutex> lock(printMutex);
                    cout << "[" << names[id] << " (Filozof " << id
                         << ")] podniosl paleczke (" << second << ")." << endl;
                }
                break;
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }

        // Jedzenie
        {
            lock_guard<mutex> lock(printMutex);
            cout << "[" << names[id] << " (Filozof " << id << ")] je." << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(randomTime(1000, 4000, id)));
        mealsEaten[id]++;

        // Okladanie paleczek
        chopsticks[second].holder.store(-1);
        chopsticks[second].mtx.unlock();
        chopsticks[first].holder.store(-1);
        chopsticks[first].mtx.unlock();

        {
            lock_guard<mutex> lock(printMutex);
            cout << "[" << names[id] << " (Filozof " << id
                 << ")] zakonczyl posilek i odlozyl paleczki." << endl;
        }
        waiter.release();

        {
            lock_guard<mutex> lock(printMutex);
            cout << "[" << names[id] << " (Filozof " << id << ")] mysli." << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(randomTime(2000, 5000, id)));
    }
}

void Wersja3::printSummary() {
    lock_guard<mutex> lock(printMutex);
    cout << "\n==== PODSUMOWANIE POSILKOW ====" << endl;
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        cout << names[i] << " (Filozof " << i << ") zjadl " << mealsEaten[i].load()
             << " posilkow." << endl;
    }
}

void Wersja3::run() {
    vector<thread> threads;
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        threads.emplace_back(&Wersja3::philosopher, this, i);
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
    printSummary();
}
