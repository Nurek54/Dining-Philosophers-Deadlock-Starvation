#include "Wersja1.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>

Wersja1::Wersja1() : chopsticks(NUM_PHILOSOPHERS) {
    srand(static_cast<unsigned int>(time(NULL)));
}

void Wersja1::printMessage(const std::string &msg) {
    std::lock_guard<std::mutex> lock(printMutex);
    std::cout << msg << std::endl;
}

int Wersja1::randomTime(int min, int max) {
    return rand() % (max - min + 1) + min;
}

void Wersja1::philosopher(int id) {
    for (int i = 0; i < NUM_MEALS; i++) {
        {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[Filozof " << id << "] mysli." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(randomTime(1000, 4000)));

        // Próba lewa paleczka
        while (true) {
            bool locked = chopsticks[id].mtx.try_lock();
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[Filozof " << id << "] probuje podniesc lewa paleczke (" << id << ")... ";
            if (locked) {
                chopsticks[id].holder = id;
                std::cout << "- OK" << std::endl;
                break;
            } else {
                std::cout << "- NIEUDANA (trzyma filozof " << chopsticks[id].holder << ")" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }

        // Próba prawa paleczka
        int right = (id + 1) % NUM_PHILOSOPHERS;
        while (true) {
            bool locked = chopsticks[right].mtx.try_lock();
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[Filozof " << id << "] probuje podniesc prawa paleczke (" << right << ")... ";
            if (locked) {
                chopsticks[right].holder = id;
                std::cout << "- OK" << std::endl;
                break;
            } else {
                std::cout << "- NIEUDANA (trzyma filozof " << chopsticks[right].holder << ")" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }

        {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[Filozof " << id << "] je (posilek " << i + 1 << ")." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(randomTime(1000, 2000)));

        // Odkładanie
        chopsticks[right].holder = -1;
        chopsticks[right].mtx.unlock();
        chopsticks[id].holder = -1;
        chopsticks[id].mtx.unlock();

        {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[Filozof " << id << "] skonczyl posilek " << i + 1 << " i odlozyl paleczki." << std::endl;
            std::cout << "-----------------------------------------------------" << std::endl;
        }
    }
}

void Wersja1::run() {
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        threads.emplace_back(&Wersja1::philosopher, this, i);
    }
    for (auto &t : threads) {
        t.join();
    }
    printMessage("Wszyscy filozofowie skonczyli posilki.");
}
