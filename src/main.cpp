#include <Geode/Geode.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

$execute {
    // Ctrl -> Space (CBF-preciso: pasamos el mismo timestamp)
    geode::KeyboardInputEvent().listen(+[](geode::KeyboardInputData& data) {
        if (data.key == enumKeyCodes::KEY_Control) {
            auto kbd = CCKeyboardDispatcher::get();
            if (!kbd) return ListenerResult::Propagate;

            bool down   = (data.action == geode::KeyboardInputData::Action::Press);
            bool repeat = (data.action == geode::KeyboardInputData::Action::Repeat);

            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, down || repeat, repeat, data.timestamp);
            return ListenerResult::Stop;
        }
        return ListenerResult::Propagate;
    }).leak();

    // Click derecho -> Z, click central -> X
    geode::MouseInputEvent().listen(+[](geode::MouseInputData& data) {
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

    // Scroll -> X
    geode::ScrollWheelEvent().listen(+[](double x, double y) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return ListenerResult::Propagate;

        kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, true,  false, 0.0);
        kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, false, false, 0.0);
        return ListenerResult::Stop;
    }).leak();
}