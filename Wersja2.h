#ifndef WERSJA2_H
#define WERSJA2_H

#include <vector>
#include <thread>
#include <mutex>
#include <string>

class Wersja2 {
public:
    Wersja2();
    void run();

private:
    static const int NUM_PHILOSOPHERS = 5;
    static const int NUM_MEALS = 20;

    struct Chopstick {
        std::mutex mtx;
        int holder;
        Chopstick() : holder(-1) {}
    };

    std::vector<Chopstick> chopsticks;
    std::mutex printMutex;

    void printMessage(const std::string &msg);
    void philosopher(int id);
    int randomTime(int min, int max);
};

#endif // WERSJA2_H
