#include "Wersja3.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <climits>

// Implementacja klasy Waiter
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
        // Mechanizm sprawiedliwosci: jezeli filozof zjadl juz wiecej niz pozostali, dodatkowo czeka
        int minMeals = INT_MAX;
        for (int j = 0; j < NUM_PHILOSOPHERS; j++) {
            int eaten = mealsEaten[j].load();
            if (eaten < minMeals) {
                minMeals = eaten;
            }
        }
        if (mealsEaten[id].load() > minMeals + 1) {
            std::stringstream ss;
            ss << "Filozof " << id << " czeka dla sprawiedliwosci (zjadl " << mealsEaten[id].load()
               << ", minimum " << minMeals << ").";
            printMessage(ss.str());
            std::this_thread::sleep_for(std::chrono::milliseconds(randomTime(1000, 4000)));
            i--; // iteracja nie jest liczona
            continue;
        }

        std::stringstream ss;
        ss << "Filozof " << id << " mysli.";
        printMessage(ss.str());
        std::this_thread::sleep_for(std::chrono::milliseconds(randomTime(1000, 4000)));

        // Uzyskujemy zgode od waitera
        waiter.acquire();
        ss.str("");
        ss << "Filozof " << id << " otrzymal zgode od waitera.";
        printMessage(ss.str());

        // Aby uniknac zakleszczenia - zawsze podnosimy paleczki w kolejnosci rosnacych indeksow
        int left = id;
        int right = (id + 1) % NUM_PHILOSOPHERS;
        int first = std::min(left, right);
        int second = std::max(left, right);

        ss.str("");
        ss << "Filozof " << id << " probuje podniesc paleczke (" << first << ").";
        printMessage(ss.str());
        chopsticks[first].mtx.lock();
        chopsticks[first].holder = id;
        ss.str("");
        ss << "Filozof " << id << " podniosl paleczke (" << first << ").";
        printMessage(ss.str());

        ss.str("");
        ss << "Filozof " << id << " probuje podniesc paleczke (" << second << ").";
        printMessage(ss.str());
        chopsticks[second].mtx.lock();
        chopsticks[second].holder = id;
        ss.str("");
        ss << "Filozof " << id << " podniosl paleczke (" << second << ").";
        printMessage(ss.str());

        ss.str("");
        ss << "Filozof " << id << " je (posilek " << i+1 << ").";
        printMessage(ss.str());
        std::this_thread::sleep_for(std::chrono::milliseconds(randomTime(1000, 2000)));
        mealsEaten[id]++;

        // Odkladanie paleczek
        chopsticks[second].holder = -1;
        chopsticks[second].mtx.unlock();
        chopsticks[first].holder = -1;
        chopsticks[first].mtx.unlock();

        ss.str("");
        ss << "Filozof " << id << " skonczyl posilek " << i+1 << " i odlozyl paleczki.";
        printMessage(ss.str());
        printMessage("-----------------------------------------------------");

        // Zwolnienie waitera
        waiter.release();
    }
}

void Wersja3::run() {
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        threads.push_back(std::thread(&Wersja3::philosopher, this, i));
    }
    for (auto &t : threads) {
        t.join();
    }
    printMessage("Wszyscy filozofowie skonczyli posilki.");
}
