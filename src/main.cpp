#include <Geode/Geode.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

$execute {
    static bool modIsActive = false;

    // Función de sincronización por bitmask para evitar el "salto infinito"
    auto forceSync = [](geode::KeyboardModifier mods, double timestamp) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return;

        auto mod = Mod::get();
        int mask = 0;
        // Bitmask: Shift = 1, Control = 2, Alt = 4
        if (mod->getSettingValue<bool>("l-ctrl") || mod->getSettingValue<bool>("r-ctrl")) mask |= 2;
        if (mod->getSettingValue<bool>("l-shift") || mod->getSettingValue<bool>("r-shift")) mask |= 1;
        if (mod->getSettingValue<bool>("l-alt") || mod->getSettingValue<bool>("r-alt")) mask |= 4;

        bool anyPhysicallyDown = (static_cast<int>(mods) & mask);

        if (modIsActive && !anyPhysicallyDown) {
            modIsActive = false;
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, false, false, timestamp);
        }
    };

    // ── TECLADO: Remapeo Universal ───────────────────────────────────────────
    geode::KeyboardInputEvent().listen([forceSync](geode::KeyboardInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        auto mod = Mod::get();
        if (!kbd) return ListenerResult::Propagate;

        bool shouldRemap = false;

        // Usamos geode::Key::Control para máxima compatibilidad (unifica L/R)
        if (data.key == geode::Key::Control && (mod->getSettingValue<bool>("l-ctrl") || mod->getSettingValue<bool>("r-ctrl"))) shouldRemap = true;
        if (data.key == geode::Key::Shift   && (mod->getSettingValue<bool>("l-shift") || mod->getSettingValue<bool>("r-shift"))) shouldRemap = true;
        if (data.key == geode::Key::Alt     && (mod->getSettingValue<bool>("l-alt") || mod->getSettingValue<bool>("r-alt"))) shouldRemap = true;

        if (shouldRemap) {
            bool down = (data.action != geode::KeyboardInputData::Action::Release);
            modIsActive = down;
            // Precisión CBF: pasamos el timestamp original del hardware
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, down, (data.action == geode::KeyboardInputData::Action::Repeat), data.timestamp);
            return ListenerResult::Stop;
        }

        forceSync(data.modifiers, data.timestamp);
        return ListenerResult::Propagate;
    }).leak();

    // ── MOUSE Y SCROLL: Rapid Checkpoints ───────────────────────────────────
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

    geode::ScrollWheelEvent().listen(+[](double x, double y) {
        if (Mod::get()->getSettingValue<bool>("rapid-checkpoints")) {
            auto kbd = CCKeyboardDispatcher::get();
            if (kbd) {
                // Simula una pulsación rápida de X (Checkpoints)
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, true,  false, 0.0);
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, false, false, 0.0);
                return ListenerResult::Stop;
            }
        }
        return ListenerResult::Propagate;
    }).leak();
}
