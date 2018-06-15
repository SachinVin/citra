// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <cmath>
#include <string>
#include <mutex>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <list>
#include "common/logging/log.h"
#include "common/math_util.h"
#include "common/param_package.h"
#include "input_common/main.h"
#include "input_common/sdl/sdl.h"
#include "button_manager.h"

namespace InputManager {

// Button Handler
class KeyButton final : public Input::ButtonDevice {
public:
    explicit KeyButton(std::shared_ptr<ButtonList> button_list_)
            : button_list(button_list_) {}

    ~KeyButton();

    bool GetStatus() const override {
        return status.load();
    }

    friend class ButtonList;

private:
    std::shared_ptr<ButtonList> button_list;
    std::atomic<bool> status{false};
};

struct KeyButtonPair {
    int button_id;
    KeyButton *key_button;
};

class ButtonList {
public:
    void AddButton(int button_id, KeyButton *key_button) {
        std::lock_guard<std::mutex> guard(mutex);
        list.push_back(KeyButtonPair{button_id, key_button});
    }

    void RemoveButton(const KeyButton *key_button) {
        std::lock_guard<std::mutex> guard(mutex);
        list.remove_if(
                [key_button](const KeyButtonPair &pair) { return pair.key_button == key_button; });
    }

    void ChangeButtonStatus(int button_id, bool pressed) {
        std::lock_guard<std::mutex> guard(mutex);
        for (const KeyButtonPair &pair : list) {
            if (pair.button_id == button_id)
                pair.key_button->status.store(pressed);
        }
    }

    void ChangeAllButtonStatus(bool pressed) {
        std::lock_guard<std::mutex> guard(mutex);
        for (const KeyButtonPair &pair : list) {
            pair.key_button->status.store(pressed);
        }
    }

private:
    std::mutex mutex;
    std::list<KeyButtonPair> list;
};

ButtonFactory::ButtonFactory() : button_list{std::make_shared<ButtonList>()} {}

KeyButton::~KeyButton() {
    button_list->RemoveButton(this);
}

std::unique_ptr<Input::ButtonDevice> ButtonFactory::Create(const Common::ParamPackage &params) {
    int button_id = params.Get("code", 0);
    std::unique_ptr<KeyButton> button = std::make_unique<KeyButton>(button_list);
    button_list->AddButton(button_id, button.get());
    return std::move(button);
}

void ButtonFactory::PressKey(int button_id) {
    button_list->ChangeButtonStatus(button_id, true);
}

void ButtonFactory::ReleaseKey(int button_id) {
    button_list->ChangeButtonStatus(button_id, false);
}


// Joystick Handler
class Joystick final : public Input::AnalogDevice {
public:
    explicit Joystick(std::shared_ptr<AnalogList> button_list_)
            : button_list(button_list_) {}

    ~Joystick();

    std::tuple<float, float> GetStatus() const override {
        return std::make_tuple(x_axis.load(), y_axis.load());
    }

    friend class AnalogList;

private:
    std::shared_ptr<AnalogList> button_list;
    std::atomic<float> x_axis{0.0f};
    std::atomic<float> y_axis{0.0f};
};

struct AnalogPair {
    int analog_id;
    Joystick *key_button;
};

class AnalogList {
public:
    void AddButton(int analog_id, Joystick *key_button) {
        std::lock_guard<std::mutex> guard(mutex);
        list.push_back(AnalogPair{analog_id, key_button});
    }

    void RemoveButton(const Joystick *key_button) {
        std::lock_guard<std::mutex> guard(mutex);
        list.remove_if(
                [key_button](const AnalogPair &pair) { return pair.key_button == key_button; });
    }

    void ChangeJoystickStatus(int analog_id, float x, float y) {
        std::lock_guard<std::mutex> guard(mutex);
        for (const AnalogPair &pair : list) {
            if (pair.analog_id == analog_id) {
                pair.key_button->x_axis.store(x);
                pair.key_button->y_axis.store(y);
            }
        }
    }

    void ChangeAllButtonStatus(int analog_id, float x, float y) {
        std::lock_guard<std::mutex> guard(mutex);
        for (const AnalogPair &pair : list) {
            pair.key_button->x_axis.store(x);
            pair.key_button->y_axis.store(y);
        }
    }

private:
    std::mutex mutex;
    std::list<AnalogPair> list;
};

AnalogFactory::AnalogFactory() : analog_list{std::make_shared<AnalogList>()} {}

Joystick::~Joystick() {
    button_list->RemoveButton(this);
}

std::unique_ptr<Input::AnalogDevice> AnalogFactory::Create(const Common::ParamPackage &params) {
    int analog_id = params.Get("code", 0);
    std::unique_ptr<Joystick> button = std::make_unique<Joystick>(analog_list);
    analog_list->AddButton(analog_id, button.get());
    return std::move(button);
}

void AnalogFactory::MoveJoystick(int analog_id, float x, float y) {
    analog_list->ChangeJoystickStatus(analog_id, x, y);
}

static std::shared_ptr<ButtonFactory> button;
static std::shared_ptr<AnalogFactory> analog;

void Init() {
    button = std::make_shared<ButtonFactory>();
    analog = std::make_shared<AnalogFactory>();
    Input::RegisterFactory<Input::ButtonDevice>("gamepad", button);
    Input::RegisterFactory<Input::AnalogDevice>("gamepad", analog);
}

void Shutdown() {
    Input::UnregisterFactory<Input::ButtonDevice>("gamepad");
    button.reset();
    analog.reset();
}

ButtonFactory* ButtonHandler() {
    return button.get();
}

AnalogFactory* AnalogHandler() {
    return analog.get();
}

std::string GenerateButtonParamPackage(int button){
    Common::ParamPackage param{
            {"engine", "gamepad"}, {"code", std::to_string(button)},
    };
    return param.Serialize();
}

std::string GenerateAnalogParamPackage(int button){
    Common::ParamPackage param{
            {"engine", "gamepad"}, {"code", std::to_string(button)},
    };
    return param.Serialize();
}
} // namespace InputManager
