# Problem jedzących filozofów / Dining Philosophers Problem

## PL

Symulacja klasycznego problemu jedzących filozofów w C++ z wykorzystaniem wątków (`std::thread`) i mechanizmów synchronizacji (`std::mutex`, `std::atomic`, `std::condition_variable`).

Pięciu filozofów (Yoda, Gandalf, ObiWan, Anakin, Leia) siedzi przy okrągłym stole. Każdy na przemian myśli i je, współdzieląc pałeczki z sąsiadami. Program demonstruje trzy warianty rozwiązania:

**Wersja 1 - Zakleszczenie (Deadlock)**
Każdy filozof najpierw podnosi prawą, potem lewą pałeczkę. Może dojść do sytuacji, w której wszyscy trzymają jedną pałeczkę i czekają na drugą w nieskończoność.

**Wersja 2 - Zagłodzenie (Starvation)**
Filozof 4 (Leia) ma ograniczoną liczbę prób pobrania lewej pałeczki — jeśli się nie uda, odpuszcza i wraca do myślenia. Pozostali czekają do skutku, przez co Leia je znacznie rzadziej.

**Wersja 3 - Rozwiązanie sprawiedliwe (Fair)**
Wykorzystuje wzorzec kelnera (Waiter) ograniczającego liczbę filozofów przy stole do N−1 oraz mechanizm fairness — filozof, który zjadł za dużo w porównaniu z innymi, dobrowolnie czeka. Brak zakleszczenia i zagłodzenia.

Po uruchomieniu wybierz wersję (1, 2 lub 3). Symulację kończy naciśnięcie Enter.

---

## EN

A simulation of the classic Dining Philosophers problem in C++ using threads (`std::thread`) and synchronization primitives (`std::mutex`, `std::atomic`, `std::condition_variable`).

Five philosophers (Yoda, Gandalf, ObiWan, Anakin, Leia) sit at a round table, alternating between thinking and eating while sharing chopsticks with their neighbors. The program demonstrates three variants:

**Version 1 - Deadlock**
Each philosopher picks up the right chopstick first, then the left. This can lead to a circular wait where everyone holds one chopstick indefinitely.

**Version 2 - Starvation**
Philosopher 4 (Leia) has a limited number of attempts to acquire the left chopstick — if she fails, she releases the right one and goes back to thinking. Others wait indefinitely, so Leia eats far less often.

**Version 3 - Fair solution**
Uses a Waiter pattern that limits concurrent diners to N−1, combined with a fairness mechanism — a philosopher who has eaten too much compared to others voluntarily waits. No deadlock or starvation.

After launching, choose a version (1, 2, or 3). Press Enter to stop the simulation.
