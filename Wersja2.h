#ifndef WERSJA2_H
#define WERSJA2_H

#include <vector>
#include <thread>
#include <mutex>
#include <string>
#include <atomic>

using namespace std;

class Wersja2 {
public:
    Wersja2();
    void run();

private:
    static const int NUM_PHILOSOPHERS = 5;

    // Nazwy filozofow
    vector<string> names;

    // Struktura reprezentujaca paleczke
    struct Chopstick {
        mutex mtx;
        atomic<int> holder; // -1 oznacza, ze paleczka jest wolna
        Chopstick() : holder(-1) {}
    };

    vector<Chopstick> chopsticks;
    mutex printMutex;
    vector<atomic<int>> mealsEaten; // licznik posilkow dla kazdego filozofa

    void printMessage(const string &msg);
    void philosopher(int id);
    int randomTime(int min, int max, int philosopherId);
};

#endif // WERSJA2_H
