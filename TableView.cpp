#include "TableView.h"

#include "raylib.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <string>
#include <vector>

namespace {

constexpr int W = 1000;
constexpr int H = 660;
constexpr int N = 5;
constexpr int RACE_PAUSE_MS = 300;
const char *NAMES[N] = {"Yoda", "Gandalf", "ObiWan", "Anakin", "Leia"};

enum State : int { THINK, WAIT, EAT, GIVEUP, FAIR, WAITER, STATE_COUNT };

Color hex(unsigned v, unsigned char a = 255) {
    return Color{static_cast<unsigned char>(v >> 16), static_cast<unsigned char>(v >> 8),
                 static_cast<unsigned char>(v), a};
}

const Color STATE_COLOR[STATE_COUNT] = {hex(0x64748B), hex(0xF59E0B), hex(0x10B981),
                                        hex(0xF43F5E), hex(0xA855F7), hex(0x3B82F6)};
const Color BG = hex(0x0D1117);
const Color CARD = hex(0x161B22);
const Color EDGE = hex(0x30363D);
const Color FG = hex(0xE6EDF3);
const Color MUTED = hex(0x8B949E);
const Color TABLE = hex(0x1F2937);
const Color TABLE_EDGE = hex(0x374151);
const Color STICK = hex(0xF59E0B);
const Color STICK_FREE = hex(0xB45309);
const Color RICE = hex(0xFDE68A);

struct Lang {
    const char *title[3];
    const char *sub[3];
    const char *state[STATE_COUNT];
    const char *meals;
    const char *events;
    const char *dead;
    const char *deadSub;
    const char *raceOn;
    const char *raceOff;
    const char *keys;
    const char *picks;
    const char *puts;
    const char *windowTitle;
};

const Lang PL = {
    {"Wersja 1 · naiwna", "Wersja 2 · Leia odpuszcza", "Wersja 3 · kelner + sprawiedliwość"},
    {"najpierw prawa, potem lewa pałeczka", "Leia ma 5 prób po lewą pałeczkę",
     "najwyżej 4 filozofów sięga po pałeczki, stała kolejność blokad"},
    {"myśli", "czeka", "je", "odpuszcza", "przepuszcza innych", "zgoda kelnera"},
    "Posiłki",
    "Zdarzenia",
    "ZAKLESZCZENIE",
    "każdy trzyma jedną pałeczkę i czeka na drugą",
    "okno wyścigu: 300 ms",
    "okno wyścigu: wyłączone",
    "[R] okno wyścigu      [N] od nowa      [L] English      [Esc] koniec",
    "podnosi pałeczkę",
    "odkłada pałeczkę",
    "Problem jedzących filozofów",
};

const Lang EN = {
    {"Version 1 · naive", "Version 2 · Leia gives up", "Version 3 · waiter + fairness"},
    {"right chopstick first, then left", "Leia gets 5 tries for the left chopstick",
     "at most 4 philosophers compete, fixed lock order"},
    {"thinking", "waiting", "eating", "gives up", "lets others eat", "waiter said yes"},
    "Meals",
    "Events",
    "DEADLOCK",
    "everyone holds one chopstick and waits for the other",
    "race window: 300 ms",
    "race window: off",
    "[R] race window      [N] restart      [L] Polski      [Esc] quit",
    "picks up chopstick",
    "puts down chopstick",
    "Dining Philosophers",
};

struct Fonts {
    Font regular{};
    Font bold{};
    Font mono{};
    bool custom = false;
};

std::vector<int> codepoints() {
    std::vector<int> cps;
    for (int c = 32; c < 127; ++c) cps.push_back(c);
    const char *extra = "ąćęłńóśźżĄĆĘŁŃÓŚŹŻ·−×≤→";
    int count = 0;
    int *more = LoadCodepoints(extra, &count);
    for (int i = 0; i < count; ++i) cps.push_back(more[i]);
    UnloadCodepoints(more);
    return cps;
}

Font loadFirst(const std::vector<const char *> &paths, int size, std::vector<int> &cps, bool &ok) {
    for (const char *p : paths) {
        if (!FileExists(p)) continue;
        Font f = LoadFontEx(p, size, cps.data(), static_cast<int>(cps.size()));
        if (f.texture.id != 0 && f.glyphCount > 0) {
            GenTextureMipmaps(&f.texture);
            SetTextureFilter(f.texture, TEXTURE_FILTER_TRILINEAR);
            return f;
        }
    }
    ok = false;
    return GetFontDefault();
}

Fonts loadFonts() {
    std::vector<int> cps = codepoints();
    Fonts f;
    bool ok = true;
    f.regular = loadFirst({"C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/arial.ttf",
                           "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
                           "/System/Library/Fonts/Supplemental/Arial.ttf"}, 48, cps, ok);
    f.bold = loadFirst({"C:/Windows/Fonts/segoeuib.ttf", "C:/Windows/Fonts/arialbd.ttf",
                        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
                        "/System/Library/Fonts/Supplemental/Arial Bold.ttf"}, 56, cps, ok);
    f.mono = loadFirst({"C:/Windows/Fonts/consola.ttf", "C:/Windows/Fonts/cour.ttf",
                        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
                        "/System/Library/Fonts/Supplemental/Courier New.ttf"}, 40, cps, ok);
    f.custom = ok;
    return f;
}

void unloadFonts(Fonts &f) {
    for (Font *font : {&f.regular, &f.bold, &f.mono}) {
        if (font->texture.id != GetFontDefault().texture.id) UnloadFont(*font);
    }
}

// Domyslna czcionka raylib nie ma polskich znakow, wiec bez czcionki systemowej zamieniamy je na ASCII.
std::string ascii(const std::string &s) {
    static const std::vector<std::pair<std::string, std::string>> map = {
        {"ą", "a"}, {"ć", "c"}, {"ę", "e"}, {"ł", "l"}, {"ń", "n"}, {"ó", "o"}, {"ś", "s"}, {"ź", "z"},
        {"ż", "z"}, {"Ą", "A"}, {"Ć", "C"}, {"Ę", "E"}, {"Ł", "L"}, {"Ń", "N"}, {"Ó", "O"}, {"Ś", "S"},
        {"Ź", "Z"}, {"Ż", "Z"}, {"·", "-"}, {"−", "-"}, {"×", "x"}, {"→", "->"}};
    std::string out = s;
    for (const auto &[from, to] : map) {
        for (size_t pos = out.find(from); pos != std::string::npos; pos = out.find(from, pos + to.size())) {
            out.replace(pos, from.size(), to);
        }
    }
    return out;
}

enum Align { LEFT, CENTER, RIGHT };

void text(const Fonts &fonts, const Font &font, const std::string &raw, float x, float y, float size, Color color,
          Align align = LEFT) {
    const std::string s = fonts.custom ? raw : ascii(raw);
    const float spacing = fonts.custom ? 0.0f : size / 10.0f;
    Vector2 m = MeasureTextEx(font, s.c_str(), size, spacing);
    if (align == CENTER) x -= m.x / 2;
    if (align == RIGHT) x -= m.x;
    DrawTextEx(font, s.c_str(), {std::round(x), std::round(y)}, size, spacing, color);
}

float textWidth(const Fonts &fonts, const Font &font, const std::string &raw, float size) {
    const std::string s = fonts.custom ? raw : ascii(raw);
    return MeasureTextEx(font, s.c_str(), size, fonts.custom ? 0.0f : size / 10.0f).x;
}

State parseStatus(const std::string &s) {
    if (s == "je ryz") return EAT;
    if (s.rfind("czeka (fairness)", 0) == 0) return FAIR;
    if (s.rfind("czeka", 0) == 0) return WAIT;
    if (s == "odpuszcza") return GIVEUP;
    if (s == "posiada zgode od waitera") return WAITER;
    return THINK;
}

enum EventKind : int { EV_PICK, EV_PUT, EV_STATE };

struct Event {
    double t;
    int who;
    EventKind kind;
    int value;
};

struct StickPose {
    float angle, inner, outer;
};

float lerp(float a, float b, float k) { return a + (b - a) * k; }

float lerpAngle(float a, float b, float k) {
    float d = std::fmod(b - a + 3.0f * PI, 2.0f * PI) - PI;
    return a + d * k;
}

void drawStick(StickPose p, Vector2 c, Color col, float thick) {
    Vector2 a{c.x + p.inner * std::cos(p.angle), c.y + p.inner * std::sin(p.angle)};
    Vector2 b{c.x + p.outer * std::cos(p.angle), c.y + p.outer * std::sin(p.angle)};
    DrawLineEx(a, b, thick, col);
    DrawCircleV(a, thick / 2, col);
    DrawCircleV(b, thick / 2, col);
}

}

void runTableView(int version, const std::function<TableSnapshot()> &snapshot, const std::function<void()> &restart,
                  std::atomic<bool> &running, std::atomic<int> &racePauseMs) {
    using clock = std::chrono::steady_clock;
    const int v = version - 1;

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    bool screen = true;
#if defined(__linux__)
    auto set = [](const char *name) {
        const char *v = std::getenv(name);
        return v != nullptr && *v != '\0';
    };
    screen = set("DISPLAY") || set("WAYLAND_DISPLAY");
#endif
    if (screen) InitWindow(W, H, PL.windowTitle);
    if (!screen || !IsWindowReady()) {
        // Brak ekranu: symulacja dziala dalej tylko w konsoli
        std::cout << "Nie udalo sie otworzyc okna wizualizacji, symulacja dziala tylko w konsoli." << std::endl;
        while (running.load()) std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return;
    }
    SetTargetFPS(60);
    Fonts fonts = loadFonts();
    const Lang *L = &PL;

    auto t0 = clock::now();
    TableSnapshot prev = snapshot();
    std::array<double, N> giveUpUntil{};
    std::deque<Event> events;
    double deadSince = -1.0;

    const Vector2 c{306, 345};
    const float seatR = 150, tableR = 88;
    float seatAngle[N];
    for (int i = 0; i < N; ++i) seatAngle[i] = -PI / 2 + i * 2 * PI / N;
    auto freePose = [&](int k) { return StickPose{seatAngle[k] + PI / N, 36, 76}; };
    std::array<StickPose, N> pose{};
    for (int k = 0; k < N; ++k) pose[k] = freePose(k);

    while (running.load() && !WindowShouldClose()) {
        if (IsKeyPressed(KEY_R)) racePauseMs = racePauseMs.load() > 0 ? 0 : RACE_PAUSE_MS;
        if (IsKeyPressed(KEY_L)) {
            L = (L == &PL) ? &EN : &PL;
            SetWindowTitle(L->windowTitle);
        }
        if (IsKeyPressed(KEY_N)) {
            restart();
            if (!running.load()) break;
            t0 = clock::now();
            prev = snapshot();
            giveUpUntil = {};
            events.clear();
            deadSince = -1.0;
            for (int k = 0; k < N; ++k) pose[k] = freePose(k);
        }
        const double now = std::chrono::duration<double>(clock::now() - t0).count();

        TableSnapshot s = snapshot();
        for (int k = 0; k < N; ++k) {
            if (s.holder[k] == prev.holder[k]) continue;
            if (prev.holder[k] >= 0) events.push_back({now, prev.holder[k], EV_PUT, k});
            if (s.holder[k] >= 0) events.push_back({now, s.holder[k], EV_PICK, k});
        }
        State st[N];
        for (int i = 0; i < N; ++i) {
            if (s.giveUps[i] > prev.giveUps[i]) {
                giveUpUntil[i] = now + 1.5;  // odpuszczenie trwa chwile, a chcemy je zobaczyc
                events.push_back({now, i, EV_STATE, GIVEUP});
            }
            st[i] = parseStatus(s.status[i]);
            State before = parseStatus(prev.status[i]);
            if (st[i] != before && (st[i] == EAT || st[i] == FAIR || st[i] == WAITER)) {
                events.push_back({now, i, EV_STATE, st[i]});
            }
            if (now < giveUpUntil[i] && st[i] == THINK) st[i] = GIVEUP;
        }
        while (events.size() > 9) events.pop_front();
        prev = s;

        // Zakleszczenie: kazdy trzyma dokladnie jedna paleczke i nikt nie je
        int held[N] = {};
        for (int k = 0; k < N; ++k)
            if (s.holder[k] >= 0) held[s.holder[k]]++;
        bool circular = true;
        for (int i = 0; i < N; ++i) circular = circular && held[i] == 1 && st[i] != EAT;
        if (!circular) deadSince = -1.0;
        else if (deadSince < 0) deadSince = now;
        const bool deadlock = deadSince >= 0 && now - deadSince > 0.8;

        const float k = 1.0f - std::pow(0.001f, GetFrameTime() * 4.0f);
        for (int stick = 0; stick < N; ++stick) {
            StickPose target = freePose(stick);
            if (int h = s.holder[stick]; h >= 0) {
                float side = (stick == h) ? -0.26f : 0.26f;
                target = {seatAngle[h] + side, 100, 136};
            }
            pose[stick] = {lerpAngle(pose[stick].angle, target.angle, k), lerp(pose[stick].inner, target.inner, k),
                           lerp(pose[stick].outer, target.outer, k)};
        }

        BeginDrawing();
        ClearBackground(BG);

        DrawRectangleRounded({16, 16, W - 32, 72}, 0.25f, 8, CARD);
        DrawRectangleRoundedLinesEx({16, 16, W - 32, 72}, 0.25f, 8, 1, EDGE);
        text(fonts, fonts.bold, L->title[v], 34, 26, 26, FG);
        text(fonts, fonts.regular, L->sub[v], 34, 58, 16, MUTED);
        char clockText[32];
        std::snprintf(clockText, sizeof clockText, "t = %5.1f s", now);
        text(fonts, fonts.mono, clockText, W - 34, 26, 18, MUTED, RIGHT);
        const bool raceOn = racePauseMs.load() > 0;
        const char *race = raceOn ? L->raceOn : L->raceOff;
        float rw = textWidth(fonts, fonts.bold, race, 14) + 36;
        Rectangle chip{W - 34 - rw, 54, rw, 24};
        DrawRectangleRounded(chip, 1.0f, 8, raceOn ? Fade(STATE_COLOR[GIVEUP], 0.2f) : BG);
        DrawRectangleRoundedLinesEx(chip, 1.0f, 8, 1, raceOn ? STATE_COLOR[GIVEUP] : EDGE);
        DrawCircle(static_cast<int>(chip.x + 13), static_cast<int>(chip.y + 12), 4, raceOn ? STATE_COLOR[GIVEUP] : MUTED);
        text(fonts, fonts.bold, race, chip.x + 22, chip.y + 4, 14, raceOn ? STATE_COLOR[GIVEUP] : MUTED);

        DrawRectangleRounded({16, 100, 580, 490}, 0.06f, 8, CARD);
        DrawRectangleRoundedLinesEx({16, 100, 580, 490}, 0.06f, 8, 1, EDGE);
        DrawCircleV(c, tableR + 2, TABLE_EDGE);
        DrawCircleV(c, tableR, TABLE);
        DrawCircleV(c, 16, TABLE_EDGE);
        DrawCircleV(c, 14, CARD);
        DrawCircleV(c, 8, RICE);
        for (int stick = 0; stick < N; ++stick) {
            const bool held = s.holder[stick] >= 0;
            drawStick(pose[stick], c, held ? STICK : STICK_FREE, held ? 7.0f : 6.0f);
        }
        for (int i = 0; i < N; ++i) {
            Vector2 p{c.x + seatR * std::cos(seatAngle[i]), c.y + seatR * std::sin(seatAngle[i])};
            Color col = STATE_COLOR[st[i]];
            if (st[i] == EAT) {
                float pulse = 36 + 3 * static_cast<float>(std::sin(now * 6.0));
                DrawRing(p, pulse, pulse + 3, 0, 360, 48, Fade(col, 0.8f));
            }
            DrawCircleV(p, 30, col);
            std::string initial(1, NAMES[i][0]);
            text(fonts, fonts.bold, initial, p.x, p.y - 15, 26, WHITE, CENTER);
            const bool below = p.y > c.y - 10;
            float ty = below ? p.y + 40 : p.y - 74;
            text(fonts, fonts.bold, NAMES[i], p.x, ty, 17, FG, CENTER);
            text(fonts, fonts.regular, L->state[st[i]], p.x, ty + 20, 14, col, CENTER);
        }
        if (deadlock) {
            Rectangle b{c.x - 210, c.y - 26, 420, 52};
            DrawRectangleRounded(b, 0.3f, 8, STATE_COLOR[GIVEUP]);
            text(fonts, fonts.bold, L->dead, b.x + b.width / 2, b.y + 6, 20, WHITE, CENTER);
            text(fonts, fonts.regular, L->deadSub, b.x + b.width / 2, b.y + 30, 13, WHITE, CENTER);
        }

        DrawRectangleRounded({612, 100, W - 628, 490}, 0.06f, 8, CARD);
        DrawRectangleRoundedLinesEx({612, 100, W - 628, 490}, 0.06f, 8, 1, EDGE);
        text(fonts, fonts.bold, L->meals, 632, 114, 18, FG);
        int maxMeals = 1;
        for (int i = 0; i < N; ++i) maxMeals = std::max(maxMeals, s.meals[i]);
        for (int i = 0; i < N; ++i) {
            float y = 148 + i * 30;
            text(fonts, fonts.regular, NAMES[i], 632, y, 15, MUTED);
            Rectangle track{712, y + 4, 200, 10};
            DrawRectangleRounded(track, 1.0f, 8, EDGE);
            if (s.meals[i] > 0) {
                float wbar = std::max(10.0f, track.width * s.meals[i] / maxMeals);
                DrawRectangleRounded({track.x, track.y, wbar, track.height}, 1.0f, 8, STATE_COLOR[EAT]);
            }
            text(fonts, fonts.bold, std::to_string(s.meals[i]), W - 34, y, 15, FG, RIGHT);
        }

        text(fonts, fonts.bold, L->events, 632, 312, 18, FG);
        int row = 0;
        for (auto it = events.rbegin(); it != events.rend(); ++it, ++row) {
            std::string line = NAMES[it->who];
            if (it->kind == EV_PICK) line += std::string(" ") + L->picks + " " + std::to_string(it->value);
            else if (it->kind == EV_PUT) line += std::string(" ") + L->puts + " " + std::to_string(it->value);
            else line += std::string(": ") + L->state[it->value];
            Color col = row == 0 ? FG : Fade(MUTED, std::max(0.35f, 1.0f - row * 0.08f));
            if (it->kind == EV_STATE) col = row == 0 ? STATE_COLOR[it->value] : Fade(STATE_COLOR[it->value], 0.7f);
            text(fonts, fonts.mono, line, 632, 344 + row * 25, 14, col);
        }

        float lx = 0;
        for (int i = 0; i < STATE_COUNT; ++i) lx += textWidth(fonts, fonts.regular, L->state[i], 14) + 34;
        lx = (W - lx) / 2;
        for (int i = 0; i < STATE_COUNT; ++i) {
            DrawCircle(static_cast<int>(lx + 6), 612, 6, STATE_COLOR[i]);
            text(fonts, fonts.regular, L->state[i], lx + 16, 603, 14, MUTED);
            lx += textWidth(fonts, fonts.regular, L->state[i], 14) + 34;
        }
        text(fonts, fonts.regular, L->keys, W / 2.0f, 632, 13, Fade(MUTED, 0.8f), CENTER);

        EndDrawing();
    }

    running = false;
    unloadFonts(fonts);
    CloseWindow();
}
