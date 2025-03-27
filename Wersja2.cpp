#include "Wersja2.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>

Wersja2::Wersja2() : chopsticks(NUM_PHILOSOPHERS) {
    srand(static_cast<unsigned int>(time(NULL)));
}

void Wersja2::printMessage(const std::string &msg) {
    std::lock_guard<std::mutex> lock(printMutex);
    std::cout << msg << std::endl;
}

int Wersja2::randomTime(int min, int max) {
    return rand() % (max - min + 1) + min;
}

void Wersja2::philosopher(int id) {
    for (int i = 0; i < NUM_MEALS; i++) {
        {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[Filozof " << id << "] mysli." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(randomTime(1000, 4000)));

        int left = id;
        int right = (id + 1) % NUM_PHILOSOPHERS;

        if (id % 2 == 0) {
            // Parzysty: najpierw lewa, potem prawa
            while (true) {
                bool locked = chopsticks[left].mtx.try_lock();
                {
                    std::lock_guard<std::mutex> lock(printMutex);
                    std::cout << "[Filozof " << id << "] probuje podniesc lewa paleczke (" << left << ")... ";
                    if (locked) {
                        chopsticks[left].holder = id;
                        std::cout << "- OK" << std::endl;
                    } else {
                        int h = chopsticks[left].holder;
                        if (h == -1)
                            std::cout << "- NIEUDANA (paleczka zajeta)" << std::endl;
                        else
                            std::cout << "- NIEUDANA (trzyma filozof " << h << ")" << std::endl;
                    }
                }
                if (locked) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }

            while (true) {
                bool locked = chopsticks[right].mtx.try_lock();
                {
                    std::lock_guard<std::mutex> lock(printMutex);
                    std::cout << "[Filozof " << id << "] probuje podniesc prawa paleczke (" << right << ")... ";
                    if (locked) {
                        chopsticks[right].holder = id;
                        std::cout << "- OK" << std::endl;
                    } else {
                        int h = chopsticks[right].holder;
                        if (h == -1)
                            std::cout << "- NIEUDANA (paleczka zajeta)" << std::endl;
                        else
                            std::cout << "- NIEUDANA (trzyma filozof " << h << ")" << std::endl;
                    }
                }
                if (locked) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        } else {
            // Nieparzysty: najpierw prawa, potem lewa
            while (true) {
                bool locked = chopsticks[right].mtx.try_lock();
                {
                    std::lock_guard<std::mutex> lock(printMutex);
                    std::cout << "[Filozof " << id << "] probuje podniesc prawa paleczke (" << right << ")... ";
                    if (locked) {
                        chopsticks[right].holder = id;
                        std::cout << "- OK" << std::endl;
                    } else {
                        int h = chopsticks[right].holder;
                        if (h == -1)
                            std::cout << "- NIEUDANA (paleczka zajeta)" << std::endl;
                        else
                            std::cout << "- NIEUDANA (trzyma filozof " << h << ")" << std::endl;
                    }
                }
                if (locked) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }

            while (true) {
                bool locked = chopsticks[left].mtx.try_lock();
                {
                    std::lock_guard<std::mutex> lock(printMutex);
                    std::cout << "[Filozof " << id << "] probuje podniesc lewa paleczke (" << left << ")... ";
                    if (locked) {
                        chopsticks[left].holder = id;
                        std::cout << "- OK" << std::endl;
                    } else {
                        int h = chopsticks[left].holder;
                        if (h == -1)
                            std::cout << "- NIEUDANA (paleczka zajeta)" << std::endl;
                        else
                            std::cout << "- NIEUDANA (trzyma filozof " << h << ")" << std::endl;
                    }
                }
                if (locked) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }

        {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[Filozof " << id << "] je (posilek " << i + 1 << ")." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(randomTime(1000, 2000)));

        // Odkładanie
        chopsticks[left].holder = -1;
        chopsticks[left].mtx.unlock();
        chopsticks[right].holder = -1;
        chopsticks[right].mtx.unlock();

        {
            std::lock_guard<std::mutex> lock(printMutex);
            std::cout << "[Filozof " << id << "] skonczyl posilek " << i + 1 << " i odlozyl paleczki." << std::endl;
            std::cout << "-----------------------------------------------------" << std::endl;
        }
    }
}

void Wersja2::run() {
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        threads.emplace_back(&Wersja2::philosopher, this, i);
    }
    for (auto &t : threads) {
        t.join();
    }
    printMessage("Wszyscy filozofowie skonczyli posilki.");
}
