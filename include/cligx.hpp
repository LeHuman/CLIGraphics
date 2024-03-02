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

static const CharSet CHARSET_braille = std::vector{"⠁", "⠄", "⠅", "⠕", "⢕", "⢝", "⢵", "⢽", "⢿", "⣿"};
static const CharSet CHARSET_extASCII = std::vector{"░", "▒", "▓", "█"};
static const CharSet CHARSET_ASCII = std::vector{".", ":", "-", "=", "+", "*", "#", "%", "@"};
static const CharSet CHARSET_combo = std::vector{".", "⠁", "⠄", ":", "⠅", "=", "+", "*", "⠕", "#", "%", "@", "⢕", "⢝", "░", "⢵", "⢽", "▒", "⢿", "⣿", "▓", "█"};
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
    using Buffer = const char *[height][width];

    const char *BLANK = " ";
    const char **charSet;
    std::size_t charLen;
    vec4 screenSpace{width, height, 0.0f, 1.0f};
    float screenSpaceMax;

    Buffer buffer, prebuffer;

    mat4 viewMatrix = glm::lookAt(eye, center, up);
    bool updateViewMatrix = true;

    std::mutex bufferMux;
    std::mutex viewMatrixMux;
    std::mutex prebufferMux;
    std::mutex charSetMux;
    std::jthread renderThread;
    bool renderThreadRunning = true;

    void renderLoop(int updateTime_ms) {
        const char **ptr = &buffer[0][0];
        const char **endPtr = ptr + (height * width);

        bufferMux.lock();
        for (; ptr != endPtr; ++ptr) {
            *ptr = BLANK;
        }
        bufferMux.unlock();

        std::ios_base::sync_with_stdio(false);

        while (renderThreadRunning) {
            {
                std::lock_guard<std::mutex> guard(bufferMux);
                clearScreen();
                for (ptr = &buffer[0][0]; ptr != endPtr; ++ptr) {
                    std::cout << *ptr;
                    if ((ptr + 1 - &buffer[0][0]) % width == 0)
                        std::cout << '\n';
                }
            }
            std::cout << '\n';
            std::this_thread::sleep_for(std::chrono::milliseconds(updateTime_ms));
        }
    }

public:
    // TODO: make read-only
    vec3 eye{0.0f, 0.0f, 1.0f};
    vec3 center{0.0f, 0.0f, -1.0f};
    vec3 up{0.0f, 1.0f, 0.0f};

    CLIGraphics(int updateTime_ms = 5, const CharSet &set = CHARSET_extASCII) {
        useCharset(set);
        renderThread = std::jthread(&CLIGraphics::renderLoop, this, updateTime_ms);
    };

    ~CLIGraphics() {
        renderThreadRunning = false;
        renderThread.join();
    }

    void setCameraPosition(vec3 pos) {
        eye = pos;
        std::lock_guard<std::mutex> guard(viewMatrixMux);
        updateViewMatrix = true;
    }

    void setCenterPosition(vec3 pos) {
        center = pos;
        std::lock_guard<std::mutex> guard(viewMatrixMux);
        updateViewMatrix = true;
    }

    void setUpPosition(vec3 pos) {
        up = pos;
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
        const char **ptr = &prebuffer[0][0];
        const char **endPtr = ptr + (height * width);
        for (; ptr != endPtr; ++ptr) {
            *ptr = BLANK;
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
        point.x = (point.x + 1.0f) * 0.5f * width;
        point.y = (1.0f - point.y) * 0.5f * height;
        point.z = (point.z + 1.0f) * 0.5f * charLen;

        size_t x = std::clamp((size_t)std::roundf(point.x), (size_t)0, width - 1);
        size_t y = std::clamp((size_t)std::roundf(point.y), (size_t)0, height - 1);
        size_t z = std::clamp((size_t)std::roundf(point.z), (size_t)0, charLen - 1);

        prebuffer[y][x] = std::min(prebuffer[y][x], charSet[z]); // FIXME: depth check needed
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
            viewMatrix = glm::lookAt(eye, center, up);
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