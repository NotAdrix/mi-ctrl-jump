#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

using namespace geode::prelude;

// Hook CCKeyboardDispatcher to intercept key events at the lowest level
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool isRepeat, double unk) {
        auto mod = Mod::get();

        // Read configured keybinds (stored as int64_t, cast to enumKeyCodes)
        auto key1 = static_cast<enumKeyCodes>(mod->getSettingValue<int64_t>("jump-keybind"));
        auto key2 = static_cast<enumKeyCodes>(mod->getSettingValue<int64_t>("jump-keybind-2"));

        // If the pressed key matches either configured keybind, remap to Space
        if (key == key1 || key == key2) {
            return CCKeyboardDispatcher::dispatchKeyboardMSG(enumKeyCodes::KEY_Space, down, isRepeat, unk);
        }

        // Otherwise pass through normally
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, isRepeat, unk);
    }
};

#include <Geode/modify/CCMouseDispatcher.hpp>

class $modify(CCMouseDispatcher) {
    bool dispatchScrollMSG(float x, float y) {
        return CCMouseDispatcher::dispatchScrollMSG(x, y);
    }
};

$execute {
    geode::MouseInputEvent().listen([](geode::MouseInputData data) -> ListenerResult {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd || !Mod::get()->getSettingValue<bool>("rapid-checkpoints")) {
            return ListenerResult::Propagate;
        }

        bool down = (data.action == geode::MouseInputData::Action::Press);

        if (data.button == geode::MouseInputData::Button::Right) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, down, false, 0.0);
            return ListenerResult::Stop;
        }

        if (data.button == geode::MouseInputData::Button::Middle) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, down, false, 0.0);
            return ListenerResult::Stop;
        }

        return ListenerResult::Propagate;
    }).leak();
}
