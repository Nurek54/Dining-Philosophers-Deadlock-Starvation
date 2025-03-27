#ifndef WERSJA1_H
#define WERSJA1_H

#include <vector>
#include <thread>
#include <mutex>
#include <string>

class Wersja1 {
public:
    Wersja1();
    void run();

private:
    static const int NUM_PHILOSOPHERS = 5;
    static const int NUM_MEALS = 20;

    // Struktura reprezentujaca paleczke
    struct Chopstick {
        std::mutex mtx;
        int holder; // -1 oznacza, ze paleczka jest wolna
        Chopstick() : holder(-1) {}
    };

    std::vector<Chopstick> chopsticks;
    std::mutex printMutex;

    // Pomocnicze metody
    void printMessage(const std::string &msg);
    void philosopher(int id);
    int randomTime(int min, int max);
};

#endif // WERSJA1_H
