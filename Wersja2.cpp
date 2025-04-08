#include "Wersja2.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <iomanip>
#include <vector>

using namespace std;

extern atomic<bool> running;

Wersja2::Wersja2()
        : chopsticks(NUM_PHILOSOPHERS),
          mealsEaten(NUM_PHILOSOPHERS),
          statuses(NUM_PHILOSOPHERS, "mysli")
{
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
    int right = id;
    int left = (id - 1 + NUM_PHILOSOPHERS) % NUM_PHILOSOPHERS;

    while (running.load()) {
        // Filozof myśli
        {
            lock_guard<mutex> lock(printMutex);
            statuses[id] = "mysli";
            cout << "[" << names[id] << " (Filozof " << id << ")] mysli." << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(randomTime(1000, 4000, id)));

        // Próba pobrania prawej pałeczki (własnej)
        {
            lock_guard<mutex> lock(printMutex);
            statuses[id] = "czeka na prawa";
        }
        while (running.load()) {
            if (chopsticks[right].mtx.try_lock()) {
                // Udało się podnieść prawą pałeczkę
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
        if (!running.load()) break;

        // Próba pobrania lewej pałeczki
        {
            lock_guard<mutex> lock(printMutex);
            statuses[id] = "czeka na lewa";
        }

        // --- SPECJALNE TRAKTOWANIE FILOZOFA 4 DLA ZAGŁODZENIA ---
        if (id == 4) {
            int attempts = 0;
            bool leftAcquired = false;

            // Wersja z limitem prób - jeśli się nie uda, filozof 4 „odpuszcza”
            while (running.load() && attempts < 5) {
                if (chopsticks[left].mtx.try_lock()) {
                    chopsticks[left].holder.store(id);
                    {
                        lock_guard<mutex> lock(printMutex);
                        cout << "[" << names[id] << " (Filozof " << id
                             << ")] podniosl lewa paleczke (" << left << ")." << endl;
                    }
                    leftAcquired = true;
                    break;
                }
                attempts++;
                this_thread::sleep_for(chrono::milliseconds(100));
            }

            if (!leftAcquired) {
                // Nie udało się pobrać lewej pałeczki - zwalniamy prawą i wracamy do myślenia
                {
                    lock_guard<mutex> lock(printMutex);
                    statuses[id] = "odpuszcza";
                    cout << "[" << names[id] << " (Filozof " << id
                         << ")] nie udalo sie pobrac lewej paleczki po "
                         << attempts << " probach, zwalnianie prawej paleczki." << endl;
                }
                chopsticks[right].holder.store(-1);
                chopsticks[right].mtx.unlock();
                continue; // wraca do początku pętli
            }
        }
        else {
            // Pozostali filozofowie czekają do skutku
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
        }

        if (!running.load()) {
            chopsticks[right].holder.store(-1);
            chopsticks[right].mtx.unlock();
            break;
        }

        // obie pałeczki zostały pobrane -> można jeść
        {
            lock_guard<mutex> lock(printMutex);
            statuses[id] = "je ryz";
            cout << "[" << names[id] << " (Filozof " << id << ")] je." << endl;
        }
        this_thread::sleep_for(chrono::milliseconds(randomTime(1000, 4000, id)));

        // Zwiększamy licznik posiłków – na pewno mamy obie pałeczki
        mealsEaten[id]++;

        // Odkładanie pałeczek (najpierw lewa, potem prawa)
        chopsticks[left].holder.store(-1);
        chopsticks[left].mtx.unlock();
        chopsticks[right].holder.store(-1);
        chopsticks[right].mtx.unlock();

        {
            lock_guard<mutex> lock(printMutex);
            statuses[id] = "mysli";
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
                cout << "---------------------------------------------------------------------" << endl;

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
                cout << "---------------------------------------------------------------------" << endl;
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
