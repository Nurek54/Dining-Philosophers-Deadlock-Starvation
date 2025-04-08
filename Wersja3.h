#ifndef WERSJA3_H
#define WERSJA3_H

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <string>

using namespace std;

class Wersja3 {
public:
    Wersja3();
    void run();

private:
    static const int NUM_PHILOSOPHERS = 5;

    vector<string> names;

    struct Chopstick {
        mutex mtx;
        atomic<int> holder;  // -1 oznacza wolną pałeczkę
        Chopstick() : holder(-1) {}
    };

    // Klasa Waiter – ogranicza liczbę filozofów próbujących jednocześnie brać dwie pałeczki
    class Waiter {
    public:
        Waiter(int count);
        void acquire();
        void release();
    private:
        int available;
        mutex mtx;
        condition_variable cv;
    };

    vector<Chopstick> chopsticks;
    Waiter waiter;
    mutex printMutex;
    vector<atomic<int>> mealsEaten;
    vector<string> statuses;

    void printMessage(const string &msg);
    void philosopher(int id);
    int randomTime(int min, int max, int philosopherId);
    void printSummary();
};

#endif // WERSJA3_H
