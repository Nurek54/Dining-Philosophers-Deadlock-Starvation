#include "Wersja3.h"
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <algorithm>

Wersja3::Waiter::Waiter(int count) : available(count) {}

void Wersja3::Waiter::acquire() {
    std::unique_lock<std::mutex> lock(mtx);
    while (available <= 0) {
        cv.wait(lock);
    }
    available--;
}

void Wersja3::Waiter::release() {
    std::unique_lock<std::mutex> lock(mtx);
    available++;
    cv.notify_all();
}

Wersja3::Wersja3() : chopsticks(NUM_PHILOSOPHERS), waiter(NUM_PHILOSOPHERS - 1), mealsEaten(NUM_PHILOSOPHERS) {
    srand(static_cast<unsigned int>(time(NULL)));
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        mealsEaten[i] = 0;
    }
}

void Wersja3::printMessage(const std::string &msg) {
    std::lock_guard<std::mutex> lock(printMutex);
    std::cout << msg << std::endl;
}

int Wersja3::randomTime(int min, int max) {
    return rand() % (max - min + 1) + min;
}

void Wersja3::philosopher(int id) {
    for (int i = 0; i < NUM_MEALS; i++) {
        // Sprawiedliwosc
        int minMeals = INT_MAX;
        for (int j = 0; j < NUM_PHILOSOPHERS; j++) {
            minMeals = std::min(minMeals, mealsEaten[j].load());
        }

        if (mealsEaten[id].load() > minMeals + 1) {
            {
                std::lock_guard<std::mutex> lock(printMutex);
                std::cout << "[Filozof " << id << "] czeka (zjadl " << mealsEaten[id] << ", min " << minMeals << ")." << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(randomTime(1000, 4000)));
            i--;
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[Filozof " << id << "] mysli." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(randomTime(1000, 4000)));

        waiter.acquire();
        {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[Filozof " << id << "] otrzymal zgode od waitera." << std::endl;
        }

        int left = id;
        int right = (id + 1) % NUM_PHILOSOPHERS;
        int first = std::min(left, right);
        int second = std::max(left, right);

        // Pierwsza paleczka
        while (true) {
            bool locked = chopsticks[first].mtx.try_lock();
            {
                std::lock_guard<std::mutex> lock(printMutex);
                std::cout << "[Filozof " << id << "] probuje podniesc paleczke (" << first << ")... ";
                if (locked) {
                    chopsticks[first].holder.store(id);
                    std::cout << "- OK" << std::endl;
                } else {
                    int h = chopsticks[first].holder.load();
                    if (h != -1)
                        std::cout << "- NIEUDANA (trzyma filozof " << h << ")" << std::endl;
                    else
                        std::cout << "- NIEUDANA (paleczka zajeta)" << std::endl;
                }
            }
            if (locked) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        // Druga paleczka
        while (true) {
            bool locked = chopsticks[second].mtx.try_lock();
            {
                std::lock_guard<std::mutex> lock(printMutex);
                std::cout << "[Filozof " << id << "] probuje podniesc paleczke (" << second << ")... ";
                if (locked) {
                    chopsticks[second].holder.store(id);
                    std::cout << "- OK" << std::endl;
                } else {
                    int h = chopsticks[second].holder.load();
                    if (h != -1)
                        std::cout << "- NIEUDANA (trzyma filozof " << h << ")" << std::endl;
                    else
                        std::cout << "- NIEUDANA (paleczka zajeta)" << std::endl;
                }
            }
            if (locked) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[Filozof " << id << "] je (posilek " << i + 1 << ")." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(randomTime(1000, 2000)));
        mealsEaten[id]++;

        chopsticks[second].holder.store(-1);
        chopsticks[second].mtx.unlock();
        chopsticks[first].holder.store(-1);
        chopsticks[first].mtx.unlock();

        {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[Filozof " << id << "] skonczyl posilek " << i + 1 << " i odlozyl paleczki." << std::endl;
            std::cout << "-----------------------------------------------------" << std::endl;
        }

        waiter.release();
    }
}

void Wersja3::printSummary() {
    std::lock_guard<std::mutex> lock(printMutex);
    std::cout << "\n==== Podsumowanie posilkow ====" << std::endl;
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        std::cout << "Filozof " << i << " zjadl " << mealsEaten[i] << " posilkow" << std::endl;
    }
}

void Wersja3::run() {
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        threads.emplace_back(&Wersja3::philosopher, this, i);
    }
    for (auto &t : threads) {
        t.join();
    }
    printMessage("Wszyscy filozofowie skonczyli posilki.");
    printSummary();
}
