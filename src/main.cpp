#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

// ── TECLADO ───────────────────────────────────────────────────────────────────
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool isRepeat, double unk) {
        if (key == enumKeyCodes::KEY_Control) {
            return CCKeyboardDispatcher::dispatchKeyboardMSG(
                enumKeyCodes::KEY_Space, down, isRepeat, unk
            );
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, isRepeat, unk);
    }
};

// ── MOUSE ─────────────────────────────────────────────────────────────────────
$execute {
    geode::MouseInputEvent().listen([](geode::MouseInputData data) -> ListenerResult {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return ListenerResult::Propagate;

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
