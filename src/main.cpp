#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCMouseDispatcher.hpp>

using namespace geode::prelude;

// ── TECLADO ──────────────────────────────────────────────────────────────────
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

// ── MOUSE ────────────────────────────────────────────────────────────────────
class $modify(CCMouseDispatcher) {
    bool dispatchScrollMSG(float x, float y) {
        auto kbd = CCKeyboardDispatcher::get();
        if (kbd) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, true,  false, 0.0);
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, false, false, 0.0);
        }
        return true;
    }

    bool dispatchMouseButtonMSG(int button, bool down, float x, float y) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return CCMouseDispatcher::dispatchMouseButtonMSG(button, down, x, y);

        if (button == 1) { // Derecho -> Z
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, down, false, 0.0);
            return true;
        }
        if (button == 2) { // Central -> X
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, down, false, 0.0);
            return true;
        }

        return CCMouseDispatcher::dispatchMouseButtonMSG(button, down, x, y);
    }
};