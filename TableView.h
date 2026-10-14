#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <array>
#include <atomic>
#include <functional>
#include <string>

// Migawka stanu stolu, ktora kazda wersja udostepnia oknu wizualizacji.
struct TableSnapshot {
    std::array<int, 5> holder{};  // paleczka k -> id filozofa (-1 = wolna)
    std::array<int, 5> meals{};
    std::array<std::string, 5> status{};
    std::array<int, 5> giveUps{};
};

// Rysuje stol, dopoki okno nie zostanie zamkniete albo `running` nie zmieni sie na false.
// Klawisze: R okno wyscigu, N restart, L jezyk.
void runTableView(int version,
                  const std::function<TableSnapshot()> &snapshot,
                  const std::function<void()> &restart,
                  std::atomic<bool> &running,
                  std::atomic<int> &racePauseMs);

#endif // TABLEVIEW_H
