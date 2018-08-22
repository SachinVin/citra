// Copyright 2013 Dolphin Emulator Project / 2017 Citra Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <map>
#include <memory>
#include <string>
#include "core/frontend/input.h"

namespace InputManager {

enum ButtonType {
    // GC
    BUTTON_A = 0,
    BUTTON_B = 1,
    BUTTON_START = 2,
    BUTTON_X = 3,
    BUTTON_Y = 4,
    BUTTON_Z = 5,
    BUTTON_UP = 6,
    BUTTON_DOWN = 7,
    BUTTON_LEFT = 8,
    BUTTON_RIGHT = 9,
    STICK_MAIN = 10, // Used on Java Side
    STICK_MAIN_UP = 11,
    STICK_MAIN_DOWN = 12,
    STICK_MAIN_LEFT = 13,
    STICK_MAIN_RIGHT = 14,
    STICK_C = 15, // Used on Java Side
    STICK_C_UP = 16,
    STICK_C_DOWN = 17,
    STICK_C_LEFT = 18,
    STICK_C_RIGHT = 19,
    TRIGGER_L = 20,
    TRIGGER_R = 21,
    // Wiimote
    WIIMOTE_BUTTON_A = 100,
    WIIMOTE_BUTTON_B = 101,
    WIIMOTE_BUTTON_MINUS = 102,
    WIIMOTE_BUTTON_PLUS = 103,
    WIIMOTE_BUTTON_HOME = 104,
    WIIMOTE_BUTTON_1 = 105,
    WIIMOTE_BUTTON_2 = 106,
    WIIMOTE_UP = 107,
    WIIMOTE_DOWN = 108,
    WIIMOTE_LEFT = 109,
    WIIMOTE_RIGHT = 110,
    WIIMOTE_IR = 111, // To Be Used on Java Side
    WIIMOTE_IR_UP = 112,
    WIIMOTE_IR_DOWN = 113,
    WIIMOTE_IR_LEFT = 114,
    WIIMOTE_IR_RIGHT = 115,
    WIIMOTE_IR_FORWARD = 116,
    WIIMOTE_IR_BACKWARD = 117,
    WIIMOTE_IR_HIDE = 118,
    WIIMOTE_SWING = 119, // To Be Used on Java Side
    WIIMOTE_SWING_UP = 120,
    WIIMOTE_SWING_DOWN = 121,
    WIIMOTE_SWING_LEFT = 122,
    WIIMOTE_SWING_RIGHT = 123,
    WIIMOTE_SWING_FORWARD = 124,
    WIIMOTE_SWING_BACKWARD = 125,
    WIIMOTE_TILT = 126, // To Be Used on Java Side
    WIIMOTE_TILT_FORWARD = 127,
    WIIMOTE_TILT_BACKWARD = 128,
    WIIMOTE_TILT_LEFT = 129,
    WIIMOTE_TILT_RIGHT = 130,
    WIIMOTE_TILT_MODIFIER = 131,
    WIIMOTE_SHAKE_X = 132,
    WIIMOTE_SHAKE_Y = 133,
    WIIMOTE_SHAKE_Z = 134,
    // Nunchuk
    NUNCHUK_BUTTON_C = 200,
    NUNCHUK_BUTTON_Z = 201,
    NUNCHUK_STICK = 202, // To Be Used on Java Side
    NUNCHUK_STICK_UP = 203,
    NUNCHUK_STICK_DOWN = 204,
    NUNCHUK_STICK_LEFT = 205,
    NUNCHUK_STICK_RIGHT = 206,
    NUNCHUK_SWING = 207, // To Be Used on Java Side
    NUNCHUK_SWING_UP = 208,
    NUNCHUK_SWING_DOWN = 209,
    NUNCHUK_SWING_LEFT = 210,
    NUNCHUK_SWING_RIGHT = 211,
    NUNCHUK_SWING_FORWARD = 212,
    NUNCHUK_SWING_BACKWARD = 213,
    NUNCHUK_TILT = 214, // To Be Used on Java Side
    NUNCHUK_TILT_FORWARD = 215,
    NUNCHUK_TILT_BACKWARD = 216,
    NUNCHUK_TILT_LEFT = 217,
    NUNCHUK_TILT_RIGHT = 218,
    NUNCHUK_TILT_MODIFIER = 219,
    NUNCHUK_SHAKE_X = 220,
    NUNCHUK_SHAKE_Y = 221,
    NUNCHUK_SHAKE_Z = 222,
    // Classic
    CLASSIC_BUTTON_A = 300,
    CLASSIC_BUTTON_B = 301,
    CLASSIC_BUTTON_X = 302,
    CLASSIC_BUTTON_Y = 303,
    CLASSIC_BUTTON_MINUS = 304,
    CLASSIC_BUTTON_PLUS = 305,
    CLASSIC_BUTTON_HOME = 306,
    CLASSIC_BUTTON_ZL = 307,
    CLASSIC_BUTTON_ZR = 308,
    CLASSIC_DPAD_UP = 309,
    CLASSIC_DPAD_DOWN = 310,
    CLASSIC_DPAD_LEFT = 311,
    CLASSIC_DPAD_RIGHT = 312,
    CLASSIC_STICK_LEFT = 313, // To Be Used on Java Side
    CLASSIC_STICK_LEFT_UP = 314,
    CLASSIC_STICK_LEFT_DOWN = 315,
    CLASSIC_STICK_LEFT_LEFT = 316,
    CLASSIC_STICK_LEFT_RIGHT = 317,
    CLASSIC_STICK_RIGHT = 318, // To Be Used on Java Side
    CLASSIC_STICK_RIGHT_UP = 319,
    CLASSIC_STICK_RIGHT_DOWN = 320,
    CLASSIC_STICK_RIGHT_LEFT = 321,
    CLASSIC_STICK_RIGHT_RIGHT = 322,
    CLASSIC_TRIGGER_L = 323,
    CLASSIC_TRIGGER_R = 324,
    // Guitar
    GUITAR_BUTTON_MINUS = 400,
    GUITAR_BUTTON_PLUS = 401,
    GUITAR_FRET_GREEN = 402,
    GUITAR_FRET_RED = 403,
    GUITAR_FRET_YELLOW = 404,
    GUITAR_FRET_BLUE = 405,
    GUITAR_FRET_ORANGE = 406,
    GUITAR_STRUM_UP = 407,
    GUITAR_STRUM_DOWN = 408,
    GUITAR_STICK = 409, // To Be Used on Java Side
    GUITAR_STICK_UP = 410,
    GUITAR_STICK_DOWN = 411,
    GUITAR_STICK_LEFT = 412,
    GUITAR_STICK_RIGHT = 413,
    GUITAR_WHAMMY_BAR = 414,
    // Drums
    DRUMS_BUTTON_MINUS = 500,
    DRUMS_BUTTON_PLUS = 501,
    DRUMS_PAD_RED = 502,
    DRUMS_PAD_YELLOW = 503,
    DRUMS_PAD_BLUE = 504,
    DRUMS_PAD_GREEN = 505,
    DRUMS_PAD_ORANGE = 506,
    DRUMS_PAD_BASS = 507,
    DRUMS_STICK = 508, // To Be Used on Java Side
    DRUMS_STICK_UP = 509,
    DRUMS_STICK_DOWN = 510,
    DRUMS_STICK_LEFT = 511,
    DRUMS_STICK_RIGHT = 512,
    // Turntable
    TURNTABLE_BUTTON_GREEN_LEFT = 600,
    TURNTABLE_BUTTON_RED_LEFT = 601,
    TURNTABLE_BUTTON_BLUE_LEFT = 602,
    TURNTABLE_BUTTON_GREEN_RIGHT = 603,
    TURNTABLE_BUTTON_RED_RIGHT = 604,
    TURNTABLE_BUTTON_BLUE_RIGHT = 605,
    TURNTABLE_BUTTON_MINUS = 606,
    TURNTABLE_BUTTON_PLUS = 607,
    TURNTABLE_BUTTON_HOME = 608,
    TURNTABLE_BUTTON_EUPHORIA = 609,
    TURNTABLE_TABLE_LEFT = 610, // To Be Used on Java Side
    TURNTABLE_TABLE_LEFT_LEFT = 611,
    TURNTABLE_TABLE_LEFT_RIGHT = 612,
    TURNTABLE_TABLE_RIGHT = 613, // To Be Used on Java Side
    TURNTABLE_TABLE_RIGHT_LEFT = 614,
    TURNTABLE_TABLE_RIGHT_RIGHT = 615,
    TURNTABLE_STICK = 616, // To Be Used on Java Side
    TURNTABLE_STICK_UP = 617,
    TURNTABLE_STICK_DOWN = 618,
    TURNTABLE_STICK_LEFT = 619,
    TURNTABLE_STICK_RIGHT = 620,
    TURNTABLE_EFFECT_DIAL = 621,
    TURNTABLE_CROSSFADE = 622, // To Be Used on Java Side
    TURNTABLE_CROSSFADE_LEFT = 623,
    TURNTABLE_CROSSFADE_RIGHT = 624,
    // 3DS Controls
    N3DS_BUTTON_A = 700,
    N3DS_BUTTON_B = 701,
    N3DS_BUTTON_X = 702,
    N3DS_BUTTON_Y = 703,
    N3DS_BUTTON_START = 704,
    N3DS_BUTTON_SELECT = 705,
    N3DS_BUTTON_HOME = 706,
    N3DS_BUTTON_ZL = 707,
    N3DS_BUTTON_ZR = 708,
    N3DS_DPAD_UP = 709,
    N3DS_DPAD_DOWN = 710,
    N3DS_DPAD_LEFT = 711,
    N3DS_DPAD_RIGHT = 712,
    N3DS_CIRCLEPAD = 713,
    N3DS_CIRCLEPAD_UP = 714,
    N3DS_CIRCLEPAD_DOWN = 715,
    N3DS_CIRCLEPAD_LEFT = 716,
    N3DS_CIRCLEPAD_RIGHT = 717,
    N3DS_STICK_C = 718,
    N3DS_STICK_C_UP = 719,
    N3DS_STICK_C_DOWN = 720,
    N3DS_STICK_C_LEFT = 771,
    N3DS_STICK_C_RIGHT = 772,
    N3DS_TRIGGER_L = 773,
    N3DS_TRIGGER_R = 774,
};

class ButtonList;
/**
 * A button device factory representing a gamepad. It receives input events and forward them
 * to all button devices it created.
 */
class ButtonFactory final : public Input::Factory<Input::ButtonDevice> {
public:
    ButtonFactory();

    /**
     * Creates a button device from a gamepad button
     * @param params contains parameters for creating the device:
     *     - "code": the code of the key to bind with the button
     */
    std::unique_ptr<Input::ButtonDevice> Create(const Common::ParamPackage& params) override;

    /**
     * Sets the status of all buttons bound with the key to pressed
     * @param key_code the code of the key to press
     */
    void PressKey(int button_id);

    /**
     * Sets the status of all buttons bound with the key to released
     * @param key_code the code of the key to release
     */
    void ReleaseKey(int key_code);

    void ReleaseAllKeys();

private:
    std::shared_ptr<ButtonList> button_list;
};

class AnalogList;
/**
 * An analog device factory representing a gamepad(virtual or physical). It receives input events
 * and forward them to all analog devices it created.
 */
class AnalogFactory final : public Input::Factory<Input::AnalogDevice> {
public:
    AnalogFactory();

    /**
     * Creates an analog device from the gamepad joystick
     * @param params contains parameters for creating the device:
     *     - "code": the code of the key to bind with the button
     */
    std::unique_ptr<Input::AnalogDevice> Create(const Common::ParamPackage& params) override;

    /**
     * Sets the status of all buttons bound with the key to pressed
     * @param key_code the code of the analog stick
     * @param x the x-axis value of the analog stick
     * @param y the y-axis value of the analog stick
     */
    void MoveJoystick(int analog_id, float x, float y);

private:
    std::shared_ptr<AnalogList> analog_list;
};

/// Initializes and registers all built-in input device factories.
void Init();

/// Deregisters all built-in input device factories and shuts them down.
void Shutdown();

/// Gets the gamepad button device factory.
ButtonFactory* ButtonHandler();

/// Gets the gamepad analog device factory.
AnalogFactory* AnalogHandler();

std::string GenerateButtonParamPackage(int type);

std::string GenerateAnalogParamPackage(int type);
} // namespace InputManager