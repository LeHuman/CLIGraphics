#include "mouse.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <thread>

#include <manymouse.h>

bool running;
std::atomic_int status;
std::jthread thread;

ManyMouseEvent event;
ManyMouseEventType lastMotion = MANYMOUSE_EVENT_RELMOTION;

int _xMod, _yMod;
int _xMax, _yMax, _xMin, _yMin;
bool clamping = false;
bool modulus = false;

enum EventItems {
    X = 0,
    Y = 1,
    VERT = 0,
    HORZ = 1,
    DOWN = 0,
    UP = 1,
};

int Mouse::setModulus(int xMod, int yMod) {
    bool noModulus = xMod == 0 && yMod == 0;
    if (noModulus) {
        modulus = false;
        return 0;
    } else if (xMod != 0 && yMod != 0) {
        _xMod = xMod;
        _yMod = yMod;
        modulus = true;
        return 0;
    }

    return 1;
}

int Mouse::setClamp(int xMax, int yMax) {
    return setClamp(xMax, yMax, -xMax, -yMax);
};

int Mouse::setClamp(int xMax, int yMax, int xMin, int yMin) {
    bool notClamping = xMax == 0 && yMax == 0 && xMin == 0 && yMin == 0;
    if ((xMax > xMin && yMax > yMin) || notClamping) {
        _xMax = xMax;
        _yMax = yMax;
        _xMin = xMin;
        _yMin = yMin;
        clamping = !notClamping;
        return 0;
    }
    return 1;
};

int Mouse::startPolling(int updateTime_ms) {
    if (running)
        return -1;

    status = -1;
    running = true;
    updateTime_ms = std::max(0, updateTime_ms);
    thread = std::jthread(&Mouse::pollLoop, this, updateTime_ms);
    while (status == -1) {
    }

    return status;
}

void Mouse::stopPolling(bool join) {
    running = false;
    if (join)
        thread.join();
}

void Mouse::reset() {
    _x = 0;
    _y = 0;
    _active = 0;
    _buttons = 0;
    _wheelVertical = 0;
    _wheelHorizontal = 0;
}

void Mouse::poll() {
    while (ManyMouse_PollEvent(&event)) {
        if (event.device != 0) // We only care about the first mouse found
            continue;
        switch (event.type) {
            case MANYMOUSE_EVENT_ABSMOTION:
                _active += 1000;
                lastMotion = MANYMOUSE_EVENT_ABSMOTION;
                if (event.item == X)
                    _x = event.value;
                else if (event.item == Y)
                    _y = event.value;
                break;
            case MANYMOUSE_EVENT_RELMOTION:
                _active += 1000;
                if (lastMotion != MANYMOUSE_EVENT_RELMOTION) {
                    lastMotion = MANYMOUSE_EVENT_RELMOTION;
                    _x = 0;
                    _y = 0;
                }
                if (event.item == X)
                    _x += event.value;
                else if (event.item == Y)
                    _y -= event.value;
                break;
            case MANYMOUSE_EVENT_BUTTON:
                _active += 500;
                if (event.value)
                    _buttons |= (1 << event.item);
                else
                    _buttons &= ~(1 << event.item);
                break;
            case MANYMOUSE_EVENT_SCROLL:
                _active += 300;
                if (event.item == VERT) {
                    _wheelVertical += event.value;
                } else if (event.item == HORZ) {
                    _wheelHorizontal += event.value;
                }
                break;
            case MANYMOUSE_EVENT_DISCONNECT:
                _active = 0;
                break;
            case MANYMOUSE_EVENT_MAX:
                break;
        }
    }
    if (modulus) {
        _x %= _xMod;
        _y %= _yMod;
    } else if (clamping) {
        _x = std::clamp((int)_x, _xMin, _xMax);
        _y = std::clamp((int)_y, _yMin, _yMax);
    }
}

void Mouse::pollLoop(int updateTime_ms) {
    int available_mice = ManyMouse_Init();
    if (available_mice <= 0) {
        ManyMouse_Quit();
        status = available_mice == 0 ? 1 : 2;
        running = false;
        return;
    }

    status = 0;

    reset();

    while (running) {
        poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(updateTime_ms));
        if (_active)
            _active = std::clamp(_active - updateTime_ms, 0, 1000);
    }
    ManyMouse_Quit();
}

Mouse *const Mouse::mouse = new Mouse();
Mouse &mouse = *Mouse::mouse;
