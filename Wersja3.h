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

    // Nazwy filozofow
    vector<string> names;

    // Struktura reprezentujaca paleczke
    struct Chopstick {
        mutex mtx;
        atomic<int> holder; // -1 oznacza, ze paleczka jest wolna
        Chopstick() : holder(-1) {}
    };

    // Klasa Waiter zapobiegajaca zakleszczeniom
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
    vector<atomic<int>> mealsEaten; // licznik posilkow dla kazdego filozofa

    void printMessage(const string &msg);
    void philosopher(int id);
    int randomTime(int min, int max, int philosopherId);
    void printSummary();
};

#endif // WERSJA3_H
