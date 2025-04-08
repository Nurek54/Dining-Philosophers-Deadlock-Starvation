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

    vector<string> names;

    struct Chopstick {
        mutex mtx;
        atomic<int> holder;  // -1 oznacza wolną pałeczkę
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

#endif // WERSJA2_H
