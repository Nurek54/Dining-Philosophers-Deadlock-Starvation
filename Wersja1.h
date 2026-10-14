#ifndef WERSJA1_H
#define WERSJA1_H

#include <vector>
#include <thread>
#include <mutex>
#include <string>
#include <atomic>
#include "TableView.h"

using namespace std;

class Wersja1 {
public:
    Wersja1();
    void run();
    TableSnapshot snapshot();

private:
    static const int NUM_PHILOSOPHERS = 5;

    vector<string> names;

    struct Chopstick {
        mutex mtx;
        atomic<int> holder; // -1 oznacza, ze jest wolna
        Chopstick() : holder(-1) {}
    };

    vector<Chopstick> chopsticks;
    mutex printMutex;
    vector<atomic<int>> mealsEaten;
    vector<string> statuses;

    void printMessage(const string &msg);
    void philosopher(int id);
    int randomTime(int min, int max, int philosopherId);
};

#endif // WERSJA1_H
