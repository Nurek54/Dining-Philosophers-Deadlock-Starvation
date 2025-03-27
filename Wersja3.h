#ifndef WERSJA3_H
#define WERSJA3_H

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

class Wersja3 {
public:
    Wersja3();
    void run();

private:
    static const int NUM_PHILOSOPHERS = 5;
    static const int NUM_MEALS = 5;

    struct Chopstick {
        std::mutex mtx;
        std::atomic<int> holder;
        Chopstick() : holder(-1) {}
    };

    class Waiter {
    public:
        Waiter(int count);
        void acquire();
        void release();
    private:
        int available;
        std::mutex mtx;
        std::condition_variable cv;
    };

    std::vector<Chopstick> chopsticks;
    Waiter waiter;
    std::mutex printMutex;
    std::vector<std::atomic<int>> mealsEaten;

    void printMessage(const std::string &msg);
    void philosopher(int id);
    int randomTime(int min, int max);
    void printSummary();
};

#endif // WERSJA3_H
