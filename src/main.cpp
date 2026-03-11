#include <Geode/Geode.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

using namespace geode::prelude;

static constexpr enumKeyCodes JUMP_KEY = enumKeyCodes::KEY_Control;

struct $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool isRepeat, double timestamp) {
        auto pl = PlayLayer::get();

        if (pl && !isRepeat && key == JUMP_KEY) {
            pl->queueButton(1, down, false, timestamp);
            return true;
        }

        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, isRepeat, timestamp);
    }
};

$execute {
    geode::MouseInputEvent().listen([](geode::MouseInputData& data) -> bool {
        auto pl = PlayLayer::get();
        if (!pl) return false;

        if (!Mod::get()->getSettingValue<bool>("rapid-checkpoints")) return false;

        bool down = (data.action == geode::MouseInputData::Action::Press);
        auto kbd  = CCKeyboardDispatcher::get();
        if (!kbd) return false;

        auto shift = kbd->m_bShiftPressed;
        auto ctrl  = kbd->m_bControlPressed;
        auto alt   = kbd->m_bAltPressed;
        auto cmd   = kbd->m_bCommandPressed;
        kbd->m_bShiftPressed   = false;
        kbd->m_bControlPressed = false;
        kbd->m_bAltPressed     = false;
        kbd->m_bCommandPressed = false;

        bool handled = false;

        if (data.button == geode::MouseInputData::Button::Right) {
            kbd->dispatchKeyboardMSG(KEY_Z, down, false, data.timestamp);
            handled = true;
        }
        else if (data.button == geode::MouseInputData::Button::Middle) {
            kbd->dispatchKeyboardMSG(KEY_X, down, false, data.timestamp);
            handled = true;
        }
        else if (data.button == geode::MouseInputData::Button::Button4) {
            kbd->dispatchKeyboardMSG(KEY_R, down, false, data.timestamp);
            handled = true;
        }

        kbd->m_bShiftPressed   = shift;
        kbd->m_bControlPressed = ctrl;
        kbd->m_bAltPressed     = alt;
        kbd->m_bCommandPressed = cmd;

        return handled;
    }).leak();
}
