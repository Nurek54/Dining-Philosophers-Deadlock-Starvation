#include "Wersja2.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sstream>
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

// Wersja 2: parzysti podnosza lewa, nieparzysti - prawa jako pierwszy
void Wersja2::philosopher(int id) {
    for (int i = 0; i < NUM_MEALS; i++) {
        std::stringstream ss;
        ss << "Filozof " << id << " mysli.";
        printMessage(ss.str());
        std::this_thread::sleep_for(std::chrono::milliseconds(randomTime(1000, 4000)));

        if (id % 2 == 0) {
            ss.str("");
            ss << "Filozof " << id << " probuje podniesc lewa paleczke (" << id << ").";
            printMessage(ss.str());
            chopsticks[id].mtx.lock();
            chopsticks[id].holder = id;
            ss.str("");
            ss << "Filozof " << id << " podniosl lewa paleczke (" << id << ").";
            printMessage(ss.str());

            int right = (id + 1) % NUM_PHILOSOPHERS;
            ss.str("");
            ss << "Filozof " << id << " probuje podniesc prawa paleczke (" << right << ").";
            printMessage(ss.str());
            chopsticks[right].mtx.lock();
            chopsticks[right].holder = id;
            ss.str("");
            ss << "Filozof " << id << " podniosl prawa paleczke (" << right << ").";
            printMessage(ss.str());
        } else {
            int right = (id + 1) % NUM_PHILOSOPHERS;
            ss.str("");
            ss << "Filozof " << id << " probuje podniesc prawa paleczke (" << right << ").";
            printMessage(ss.str());
            chopsticks[right].mtx.lock();
            chopsticks[right].holder = id;
            ss.str("");
            ss << "Filozof " << id << " podniosl prawa paleczke (" << right << ").";
            printMessage(ss.str());

            ss.str("");
            ss << "Filozof " << id << " probuje podniesc lewa paleczke (" << id << ").";
            printMessage(ss.str());
            chopsticks[id].mtx.lock();
            chopsticks[id].holder = id;
            ss.str("");
            ss << "Filozof " << id << " podniosl lewa paleczke (" << id << ").";
            printMessage(ss.str());
        }

        ss.str("");
        ss << "Filozof " << id << " je (posilek " << i+1 << ").";
        printMessage(ss.str());
        std::this_thread::sleep_for(std::chrono::milliseconds(randomTime(1000, 2000)));

        chopsticks[id].holder = -1;
        chopsticks[id].mtx.unlock();
        int right = (id + 1) % NUM_PHILOSOPHERS;
        chopsticks[right].holder = -1;
        chopsticks[right].mtx.unlock();

        ss.str("");
        ss << "Filozof " << id << " skonczyl posilek " << i+1 << " i odlozyl paleczki.";
        printMessage(ss.str());
        printMessage("-----------------------------------------------------");
    }
}

void Wersja2::run() {
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        threads.push_back(std::thread(&Wersja2::philosopher, this, i));
    }
    for (auto &t : threads) {
        t.join();
    }
    printMessage("Wszyscy filozofowie skonczyli posilki.");
}
