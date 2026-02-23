#include <Geode/Geode.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

$execute {

    static bool modIsActive = false;

    // ─────────────────────────────────────────────
    // Calcula qué modificadores vigilar según settings
    // Bitmask: Shift = 1, Ctrl = 2, Alt = 4
    // ─────────────────────────────────────────────
    auto getTargetMask = []() {
        int mask = 0;
        auto mod = Mod::get();

        if (mod->getSettingValue<bool>("l-ctrl")  ||
            mod->getSettingValue<bool>("r-ctrl"))  mask |= 2;

        if (mod->getSettingValue<bool>("l-shift") ||
            mod->getSettingValue<bool>("r-shift")) mask |= 1;

        if (mod->getSettingValue<bool>("l-alt")   ||
            mod->getSettingValue<bool>("r-alt"))   mask |= 4;

        return mask;
    };

    // ─────────────────────────────────────────────
    // Sincronización pasiva para evitar salto infinito
    // ─────────────────────────────────────────────
    auto forceSync = [getTargetMask](geode::KeyboardModifier mods, double timestamp) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return;

        int activeMask = getTargetMask();
        bool anyTargetPhysicallyDown = (static_cast<int>(mods) & activeMask);

        if (modIsActive && !anyTargetPhysicallyDown) {
            modIsActive = false;
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, false, false, timestamp);
        }
    };

    // ─────────────────────────────────────────────
    // TECLADO — Remapeo dinámico (Geode 5 API)
    // ─────────────────────────────────────────────
    geode::KeyboardInputEvent().listen([forceSync](geode::KeyboardInputData& data) {

        auto kbd = CCKeyboardDispatcher::get();
        auto mod = Mod::get();
        if (!kbd) return ListenerResult::Propagate;

        bool shouldRemap = false;

        // Control
        if (data.key == geode::KeyCode::LeftControl  &&
            mod->getSettingValue<bool>("l-ctrl"))  shouldRemap = true;

        if (data.key == geode::KeyCode::RightControl &&
            mod->getSettingValue<bool>("r-ctrl"))  shouldRemap = true;

        // Shift
        if (data.key == geode::KeyCode::LeftShift &&
            mod->getSettingValue<bool>("l-shift")) shouldRemap = true;

        if (data.key == geode::KeyCode::RightShift &&
            mod->getSettingValue<bool>("r-shift")) shouldRemap = true;

        // Alt
        if (data.key == geode::KeyCode::LeftAlt &&
            mod->getSettingValue<bool>("l-alt")) shouldRemap = true;

        if (data.key == geode::KeyCode::RightAlt &&
            mod->getSettingValue<bool>("r-alt")) shouldRemap = true;

        if (shouldRemap) {
            bool down = (data.action != geode::KeyboardInputData::Action::Release);
            bool repeat = (data.action == geode::KeyboardInputData::Action::Repeat);

            modIsActive = down;

            kbd->dispatchKeyboardMSG(
                enumKeyCodes::KEY_Space,
                down,
                repeat,
                data.timestamp
            );

            return ListenerResult::Stop;
        }

        forceSync(data.modifiers, data.timestamp);
        return ListenerResult::Propagate;

    }).leak();


    // ─────────────────────────────────────────────
    // MOUSE — Rapid Checkpoints
    // ─────────────────────────────────────────────
    geode::MouseInputEvent().listen([forceSync](geode::MouseInputData& data) {

        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return ListenerResult::Propagate;

        forceSync(data.modifiers, data.timestamp);

        if (Mod::get()->getSettingValue<bool>("rapid-checkpoints")) {

            bool down = (data.action == geode::MouseInputData::Action::Press);

            if (data.button == geode::MouseInputData::Button::Right) {
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, down, false, data.timestamp);
                return ListenerResult::Stop;
            }

            if (data.button == geode::MouseInputData::Button::Middle) {
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, down, false, data.timestamp);
                return ListenerResult::Stop;
            }
        }

        return ListenerResult::Propagate;

    }).leak();


    // ─────────────────────────────────────────────
    // Scroll Wheel → Checkpoint tap
    // ─────────────────────────────────────────────
    geode::ScrollWheelEvent().listen(+[](double x, double y) {

        if (!Mod::get()->getSettingValue<bool>("rapid-checkpoints"))
            return ListenerResult::Propagate;

        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return ListenerResult::Propagate;

        kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, true,  false, 0.0);
        kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, false, false, 0.0);

        return ListenerResult::Stop;

    }).leak();
}
