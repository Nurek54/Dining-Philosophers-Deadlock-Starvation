#ifndef WERSJA1_H
#define WERSJA1_H

#include <vector>
#include <thread>
#include <mutex>
#include <string>
#include <atomic>

using namespace std;

class Wersja1 {
public:
    Wersja1();
    void run();

private:
    static const int NUM_PHILOSOPHERS = 5;

    // Nazwy filozofow
    vector<string> names;

    // Struktura reprezentujaca chopstick (paleczke)
    struct Chopstick {
        mutex mtx;
        atomic<int> holder; // -1 oznacza, ze jest wolna
        Chopstick() : holder(-1) {}
    };

    vector<Chopstick> chopsticks;
    mutex printMutex;
    vector<atomic<int>> mealsEaten; // licznik posilkow dla kazdego filozofa

    // Pomocnicze metody
    void printMessage(const string &msg);
    void philosopher(int id);
    int randomTime(int min, int max, int philosopherId);
};

#endif // WERSJA1_H
