#ifndef CZEKANIE_H
#define CZEKANIE_H

#include <algorithm>
#include <atomic>
#include <chrono>
#include <thread>

extern std::atomic<bool> running;

// Spi `ms` milisekund, ale budzi sie od razu po zatrzymaniu symulacji
inline void sleepWhileRunning(int ms) {
    using namespace std::chrono;
    const auto end = steady_clock::now() + milliseconds(ms);
    while (running.load()) {
        const auto now = steady_clock::now();
        if (now >= end) return;
        std::this_thread::sleep_for(std::min<steady_clock::duration>(end - now, milliseconds(50)));
    }
}

#endif // CZEKANIE_H
