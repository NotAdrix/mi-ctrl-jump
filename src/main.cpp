#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

// ── TECLADO (CBF-preciso) ─────────────────────────────────────────────────────
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double time) {
        if (key == enumKeyCodes::KEY_Control) {
            return CCKeyboardDispatcher::dispatchKeyboardMSG(
                enumKeyCodes::KEY_Space, down, repeat, time
            );
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, time);
    }
};

// ── MOUSE ─────────────────────────────────────────────────────────────────────
$execute {
    new EventListener<geode::MouseInputEvent>(+[](geode::MouseInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return false;

        bool down = (data.action == geode::MouseInputData::Action::Press);

        if (data.button == geode::MouseInputData::Button::Right) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, down, false, 0.0);
            return true;
        }
        if (data.button == geode::MouseInputData::Button::Middle) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, down, false, 0.0);
            return true;
        }

        return false;
    });

    new EventListener<geode::ScrollWheelEvent>(+[](double x, double y) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return false;

        kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, true,  false, 0.0);
        kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, false, false, 0.0);
        return true;
    });
}