#include <Geode/Geode.hpp>
#include <Geode/binding/PlayLayer.hpp>

using namespace geode::prelude;

static constexpr enumKeyCodes JUMP_KEY = enumKeyCodes::KEY_Control;

$execute {
    geode::KeyboardInputEvent().listen([](geode::KeyboardInputData& data) {
        auto pl = PlayLayer::get();
        if (!pl) return ListenerResult::Propagate;

        bool down   = (data.action != geode::KeyboardInputData::Action::Release);
        bool repeat = (data.action == geode::KeyboardInputData::Action::Repeat);
        if (repeat) return ListenerResult::Propagate;

        if (data.key == JUMP_KEY) {
            pl->queueButton(1, down, false, data.timestamp);
            return ListenerResult::Stop;
        }

        return ListenerResult::Propagate;
    }).leak();

    geode::MouseInputEvent().listen([](geode::MouseInputData& data) -> bool {
        auto pl = PlayLayer::get();
        if (!pl) return false;

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
