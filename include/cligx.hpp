#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <mutex>
#include <ranges>
#include <thread>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/component_wise.hpp>

namespace CLIGx {

using vec2 = glm::lowp_vec2;
using vec3 = glm::lowp_vec3;
using vec4 = glm::lowp_vec4;
using mat4 = glm::lowp_mat4;

using Line = std::pair<vec3, vec3>;
using HLine = std::pair<vec4, vec4>;
using CharSet = std::vector<const char *>;

static const CharSet CHARSET_braille = std::vector{" ", "⠁", "⠄", "⠅", "⠕", "⢕", "⢝", "⢵", "⢽", "⢿", "⣿"};
static const CharSet CHARSET_braille_d = std::vector{" ", "⠁", " ", "⠁", "⠄", "⠁", "⠄", "⠁", "⠁", "⠄", "⠄", "⠅", "⠄", "⠅", "⠅", "⠅", "⠕", "⠅", "⠕", "⠕", "⢕", "⠕", "⢕", "⢕", "⢝", "⢝", "⢝", "⢵", "⢝", "⢵", "⢵", "⢽", "⢵", "⢽", "⢽", "⢿", "⣿"};
static const CharSet CHARSET_extASCII = std::vector{" ", "░", "▒", "▓", "█"};
static const CharSet CHARSET_extASCII_d = std::vector{" ", "░", "░", "▒", "░", "▒", "▒", "▒", "▓", "▒", "▓", "▓", "█", "▓", "█", "█", "█"};
static const CharSet CHARSET_ASCII = std::vector{" ", ".", ":", "-", "=", "+", "*", "#", "%", "@"};
static const CharSet CHARSET_combo = std::vector{" ", ".", "⠁", "⠄", ":", "⠅", "=", "+", "*", "⠕", "#", "%", "@", "⢕", "⢝", "░", "⢵", "⢽", "▒", "⢿", "⣿", "▓", "█"};
// .:-=+*#%@⠁⠄⠅⠕⢕⢝⢵⢽⢿⣿░▒▓█

constexpr float pi = glm::pi<float>();

struct VecHash {
    template <glm::length_t N, typename T, glm::qualifier Q>
    constexpr std::size_t operator()(const glm::vec<N, T, Q> &vertex) const {
        std::size_t hash = std::hash<float>{}(vertex.x);
        if (N > 1)
            hash ^= std::hash<float>{}(vertex.y);
        if (N > 2)
            hash ^= std::hash<float>{}(vertex.z);
        // if (N > 3)
        //     hash ^= std::hash<float>{}(vertex.w);
        return hash;
    }
};

struct LineHash {
    VecHash hash;
    std::size_t operator()(const Line &l) const {
        return hash(l.first) ^ hash(l.second);
    }
};

template <std::size_t width, std::size_t height>
class CLIGraphics {
private:
    using Buffer = int[height][width];

    // const char *BLANK = " ";
    const char **charSet;
    std::size_t charLen;
    vec4 screenSpace{width, height, 0.0f, 1.0f};
    float screenSpaceMax;

    Buffer buffer = {{0}}, prebuffer = {{0}};

    vec3 _eye{0.0f, 0.0f, 1.0f};
    vec3 _center{0.0f, 0.0f, -1.0f};
    vec3 _up{0.0f, 1.0f, 0.0f};
    mat4 viewMatrix = glm::lookAt(_eye, _center, _up);
    bool updateViewMatrix = false;

    std::mutex bufferMux;
    std::mutex viewMatrixMux;
    std::mutex prebufferMux;
    std::mutex charSetMux;
    std::jthread renderThread;
    bool renderThreadRunning = true;

    void renderLoop(int updateTime_ms) {
        int *ptr = &buffer[0][0];
        int *endPtr = ptr + (height * width);

        // bufferMux.lock();
        // for (; ptr != endPtr; ++ptr) {
        //     *ptr = BLANK;
        // }
        // bufferMux.unlock();

        std::ios_base::sync_with_stdio(false);
        std::string buf;

        while (renderThreadRunning) {
            {
                std::lock_guard<std::mutex> guardC(charSetMux);
                std::lock_guard<std::mutex> guardB(bufferMux);
                buf.clear();
                for (ptr = &buffer[0][0]; ptr != endPtr; ++ptr) {
                    buf += charSet[*ptr];
                    if ((ptr + 1 - &buffer[0][0]) % width == 0)
                        buf += '\n';
                }
            }
            buf += '\n';
            // clearScreen();
            std::cout << buf;
            std::this_thread::sleep_for(std::chrono::milliseconds(updateTime_ms));
        }
    }

public:
    const vec3 &eye = _eye;
    const vec3 &center = _center;
    const vec3 &up = _up;

    CLIGraphics(int updateTime_ms = 5, const CharSet &set = CHARSET_braille) {
        useCharset(set);
        renderThread = std::jthread(&CLIGraphics::renderLoop, this, updateTime_ms);
    };

    ~CLIGraphics() {
        renderThreadRunning = false;
        renderThread.join();
    }

    void setCameraPosition(vec3 pos) {
        _eye = pos;
        std::lock_guard<std::mutex> guard(viewMatrixMux);
        updateViewMatrix = true;
    }

    void setCenterPosition(vec3 pos) {
        _center = pos;
        std::lock_guard<std::mutex> guard(viewMatrixMux);
        updateViewMatrix = true;
    }

    void setUpPosition(vec3 pos) {
        _up = pos;
        std::lock_guard<std::mutex> guard(viewMatrixMux);
        updateViewMatrix = true;
    }

    void clearScreen() {
#if defined _WIN32
    #if defined _INC_CONIO
        clrscr(); // including header file : conio.h
    #else
        system("cls");
    #endif
#elif defined(__LINUX__) || defined(__gnu_linux__) || defined(__linux__)
        system("clear");
        // std::cout<< u8"\033[2J\033[1;1H"; //Using ANSI Escape Sequences
#elif defined(__APPLE__)
        system("clear");
#endif
    }

    void clearBuffer() {
        std::lock_guard<std::mutex> guard(prebufferMux);
        int *ptr = &prebuffer[0][0];
        int *endPtr = ptr + (height * width);
        for (; ptr != endPtr; ++ptr) {
            *ptr = 0;
        }
    }

    void useCharset(const CharSet &set) {
        std::lock_guard<std::mutex> guard(charSetMux);
        charSet = const_cast<const char **>(set.data());
        charLen = set.size();
        screenSpace = vec4{width, height, charLen, 1.0f};
        screenSpaceMax = std::fmax(width, std::fmax(height, std::fmax(charLen, 1.0f)));
        clearBuffer();
    }

    void drawPoint(vec4 point) {
        int x = (point.x + 1.0f) * 0.5f * (width);
        int y = (1.0f - point.y) * 0.5f * (height);
        int z = (point.z + 1.0f) * 0.5f * (charLen);

        int _x = std::clamp(x, 0, (int)width - 1);
        int _y = std::clamp(y, 0, (int)height - 1);
        z = std::clamp(z, 1, (int)charLen - 1);

        if (x == _x && y == _y)
            prebuffer[y][x] = std::max(prebuffer[y][x], z); // FIXME: depth check needed
    }

    void drawLine(HLine &line) {
        vec4 p0 = viewMatrix * line.first;
        vec4 p1 = viewMatrix * line.second;
        vec4 pd = glm::abs(p0 - p1) * vec4{p0.x > p1.x ? -1 : 1, p0.y > p1.y ? -1 : 1, p0.z > p1.z ? -1 : 1, p0.w > p1.w ? -1 : 1};
        vec4 pdv = pd / screenSpaceMax; // IMPROVE: reduce steps for smaller lines
        vec4 px = p0;

        for (size_t i = 0; i < screenSpaceMax; i++) {
            drawPoint(px);
            px += pdv;
        }
    }

    void drawLines(std::vector<HLine> lines) {
        if (updateViewMatrix) {
            viewMatrix = glm::lookAt(_eye, _center, _up);
            std::lock_guard<std::mutex> guard(viewMatrixMux);
            updateViewMatrix = false;
        }

        {
            std::lock_guard<std::mutex> guard(charSetMux);
            for (auto &&line : lines) {
                drawLine(line);
            }
        }

        std::lock_guard<std::mutex> preGuard(prebufferMux);
        std::lock_guard<std::mutex> guard(bufferMux);
        std::swap(buffer, prebuffer);
    }

    static std::vector<HLine> getHLines(std::vector<Line> lines) {
        return std::ranges::to<std::vector<HLine>>(std::views::transform(lines, [](Line &line) { return HLine{vec4{line.first, 1.0f}, vec4{line.second, 1.0f}}; }));
    }
};

} // namespace CLIGx