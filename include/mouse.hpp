#pragma once

/**
 * @brief C++ singleton class wrapper around the manymouse library. Offers manual polling or automatic polling via a std::jthread.
 *
 * @note Multiple mice not supported, only the first mouse found is used.
 */
class Mouse {
private:
    //! @cond Doxygen_Suppress
    // Internal variables to store mouse values.
    volatile int _x = 0, _y = 0;
    volatile int _active = 0;
    volatile int _buttons = 0;
    volatile int _wheelVertical = 0;
    volatile int _wheelHorizontal = 0;
    //! @endcond

    /**
     * @brief Polls mouse for input at regular intervals until stopped.
     *
     * @param updateTime_ms The `updateTime_ms` parameter in the `pollLoop` function represents the time
     * interval in milliseconds at which the mouse polling loop will be executed.
     */
    void pollLoop(int updateTime_ms);

    /**
     * @brief Construct a new Mouse object.
     * @note Because this is a singleton class, this is not accessible. Use the public instance `mouse` instead.
     */
    Mouse() = default;

public:
    /**
     * @brief Mouse button flags for the `buttons` bit field.
     *
     * @see Mouse::buttons
     */
    enum MouseButton {
        LButton = 1 << 0,  // Left Mouse
        RButton = 1 << 1,  // Right Mouse
        MButton = 1 << 2,  // Middle Mouse
        XButton1 = 1 << 3, // Backward
        XButton2 = 1 << 4, // Forward
    };

    /**
     * @brief The single public instance of `Mouse`
     */
    static Mouse *const mouse;

    /**
     * @brief Read-only coordinate of the mouse.
     *
     * @note Value is dependent on type of mouse used. Mice can return absolute coordinates or relative, where relative is internally accumulated.
     */
    const volatile int &x = _x, &y = _y;
    /**
     * @brief A read-only general timeout indicator to represent mouse activity in milliseconds.
     * @note The value is clamped to 1000ms but is allowed to go above that for high activity while it is occurring.
     * @warning Only when mouse polling is enabled through `Mouse::startPolling`.
     */
    const volatile int &active = _active;
    /**
     * @brief Read-only mouse button bitfield. Represents all of the buttons that are either up (0) or down (1)
     *
     * @see Mouse::MouseButton
     */
    const volatile int &buttons = _buttons;
    /**
     * @brief A Read-only accumulator for vertical mouse wheel movement
     */
    const volatile int &wheelVertical = _wheelVertical;
    /**
     * @brief A Read-only accumulator for horizontal mouse wheel movement
     */
    const volatile int &wheelHorizontal = _wheelHorizontal;

    //! @cond Doxygen_Suppress
    // Deleted copy and destructor functions
    ~Mouse() = delete;
    Mouse &operator=(Mouse &) = delete;
    Mouse(Mouse &) = delete;
    //! @endcond

    /**
     * @brief Set a modulus value for mouse coordinates using a single magnitude for either coordinate direction.
     *
     * @note Overrides `Mouse::setClamp` until disabled
     * @note Set all to 0 to disable.
     *
     * @param xMod The modulus value for `Mouse::x` in either direction.
     * @param yMod The modulus value for `Mouse::y` in either direction.
     *
     * @retval 0 if modulus was set
     * @retval 1 if modulus was not set
     */
    int setModulus(int xMod, int yMod);

    /**
     * @brief Set a clamping value for mouse coordinates using a single magnitude for either coordinate direction.
     *
     * @note Set all to 0 to disable clamping.
     * @note Recommended to set before calling `Mouse::startPolling`.
     *
     * @param xMax The max value `Mouse::x` should be in either direction.
     * @param yMax The max value `Mouse::y` should be in either direction.
     *
     * @retval 0 if clamp was set
     * @retval 1 if clamp was not set
     */
    int setClamp(int xMax, int yMax);

    /**
     * @brief Set a clamping value for mouse coordinates.
     *
     * @note Set all to 0 to disable clamping.
     * @note Recommended to set before calling `Mouse::startPolling`.
     *
     * @param xMax The max value `Mouse::x` should be.
     * @param yMax The max value `Mouse::y` should be.
     * @param xMin The min value `Mouse::x` should be.
     * @param yMin The min value `Mouse::y` should be.
     *
     * @retval 0 if clamp was set
     * @retval 1 if clamp was not set
     */
    int setClamp(int xMax, int yMax, int xMin, int yMin);

    /**
     * Initiates a polling loop with an update time in milliseconds.
     *
     * @param updateTime_ms The time interval in milliseconds at which the polling loop will be executed to check for mouse input.
     *
     * @retval 0 if the polling thread started successfully.
     * @retval -1 if the polling thread was already started.
     * @retval 1 if there were no mice detected (polling thread killed).
     * @retval 2 if the manymouse library failed to initalize (polling thread killed).
     */
    int startPolling(int updateTime_ms = 20);

    /**
     * Stops the polling thread and optionally waits for the polling thread to finish.
     *
     * @param join Whether the calling thread will wait for the polling thread to finish. Default is `true`.
     */
    void stopPolling(bool join = true);

    /**
     * @brief Manually poll for mouse events
     *
     * @warning Should not be used alongside `Mouse::startPolling`
     *
     * @see Mouse::startPolling
     * @see Mouse::active
     */
    void poll();

    /**
     * @brief Reset mouse values.
     *
     * @note Called each time internal polling is successfully started.
     */
    void reset();
};

extern Mouse &mouse;