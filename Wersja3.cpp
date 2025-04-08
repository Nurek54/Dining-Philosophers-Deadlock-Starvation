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
          mealsEaten(NUM_PHILOSOPHERS),
          statuses(NUM_PHILOSOPHERS, "mysli")
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
    int right = id;
    int left = (id - 1 + NUM_PHILOSOPHERS) % NUM_PHILOSOPHERS;

    while (running.load()) {
        // Sprawdzamy liczbę posiłków w celu sprawiedliwego podziału
        int minMeals = INT_MAX;
        for (int j = 0; j < NUM_PHILOSOPHERS; j++) {
            minMeals = min(minMeals, mealsEaten[j].load());
        }
        if (mealsEaten[id].load() > minMeals + 1) {
            {
                lock_guard<mutex> lock(printMutex);
                statuses[id] = "czeka (fairness)";
                cout << "[" << names[id] << " (Filozof " << id
                     << ")] czeka (Posilki: " << mealsEaten[id].load()
                     << ", Min: " << minMeals << ")." << endl;
            }
            this_thread::sleep_for(chrono::milliseconds(500));
            continue;
        }

        // Myślenie
        {
            lock_guard<mutex> lock(printMutex);
            statuses[id] = "mysli";
            cout << "[" << names[id] << " (Filozof " << id << ")] mysli." << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(randomTime(1000, 4000, id)));

        // Uzyskujemy zgodę od waitera (eliminuje deadlock)
        waiter.acquire();
        {
            lock_guard<mutex> lock(printMutex);
            statuses[id] = "posiada zgode od waitera";
            cout << "[" << names[id] << " (Filozof " << id << ")] otrzymal zgode od waitera." << endl;
        }

        // Blokujemy pałeczki w kolejności rosnących indeksów, aby uniknąć kolizji
        int first = min(left, right);
        int second = max(left, right);
        {
            lock_guard<mutex> lock(printMutex);
            statuses[id] = "czeka na paleczki";
        }

        // Pobieranie first
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
        if (!running.load()) {
            // W razie przerwania symulacji, zwalniamy waitera i koniec
            chopsticks[first].mtx.unlock();
            waiter.release();
            break;
        }

        // Pobieranie second
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
        if (!running.load()) {
            chopsticks[first].holder.store(-1);
            chopsticks[first].mtx.unlock();
            waiter.release();
            break;
        }

        // Teraz mamy obie pałeczki -> jedzenie
        {
            lock_guard<mutex> lock(printMutex);
            statuses[id] = "je ryz";
            cout << "[" << names[id] << " (Filozof " << id << ")] je." << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(randomTime(1000, 4000, id)));
        mealsEaten[id]++;

        // Odkładanie pałeczek
        chopsticks[second].holder.store(-1);
        chopsticks[second].mtx.unlock();
        chopsticks[first].holder.store(-1);
        chopsticks[first].mtx.unlock();

        {
            lock_guard<mutex> lock(printMutex);
            statuses[id] = "odklada";
            cout << "[" << names[id] << " (Filozof " << id
                 << ")] zakonczyl posilek i odlozyl paleczki." << endl;
        }
        waiter.release();

        {
            lock_guard<mutex> lock(printMutex);
            statuses[id] = "mysli";
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

    thread monitor([this]() {
        while (running.load()) {
            {
                lock_guard<mutex> lock(printMutex);
                cout << "\n==== TABLICA STATUSU ====" << endl;
                cout << left << setw(12) << "Filozof"
                     << setw(20) << "Lewa paleczka"
                     << setw(20) << "Prawa paleczka"
                     << setw(10) << "Posilki"
                     << setw(20) << "Status" << endl;
                cout << "--------------------------------------------------------------------------------" << endl;

                for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
                    int lIndex = (i - 1 + NUM_PHILOSOPHERS) % NUM_PHILOSOPHERS;
                    string leftStatus;
                    int holderLeft = chopsticks[lIndex].holder.load();
                    if (holderLeft == -1) {
                        leftStatus = "wolna";
                    } else {
                        leftStatus = "F" + to_string(holderLeft);
                    }

                    string rightStatus;
                    int holderRight = chopsticks[i].holder.load();
                    if (holderRight == -1) {
                        rightStatus = "wolna";
                    } else {
                        rightStatus = "F" + to_string(holderRight);
                    }

                    cout << left << setw(12) << names[i]
                         << setw(20) << leftStatus
                         << setw(20) << rightStatus
                         << setw(10) << mealsEaten[i].load()
                         << setw(20) << statuses[i]
                         << endl;
                }
                cout << "--------------------------------------------------------------------------------" << endl;
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
