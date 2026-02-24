#include <Geode/Geode.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

$execute {
    static bool modIsActive = false;

    // Función para verificar si un modificador físico debe estar activado según Settings
    auto getEnabledMask = []() {
        int mask = 0;
        auto mod = Mod::get();
        // Si L-Ctrl o R-Ctrl están activados en settings, vigilamos el bit de Control (2)
        if (mod->getSettingValue<bool>("l-ctrl") || mod->getSettingValue<bool>("r-ctrl")) mask |= 2;
        // Si L-Shift o R-Shift están activados, vigilamos el bit de Shift (1)
        if (mod->getSettingValue<bool>("l-shift") || mod->getSettingValue<bool>("r-shift")) mask |= 1;
        // Si L-Alt o R-Alt están activados, vigilamos el bit de Alt (4)
        if (mod->getSettingValue<bool>("l-alt") || mod->getSettingValue<bool>("r-alt")) mask |= 4;
        return mask;
    };

    auto forceSync = [getEnabledMask](geode::KeyboardModifier mods, double timestamp) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return;

        int activeMask = getEnabledMask();
        bool anyTargetModifierDown = (static_cast<int>(mods) & activeMask);

        if (modIsActive && !anyTargetModifierDown) {
            modIsActive = false;
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, false, false, timestamp);
        }
    };

    // ── TECLADO DINÁMICO ────────────────────────────────────────────────────
    geode::KeyboardInputEvent().listen([forceSync](geode::KeyboardInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        auto mod = Mod::get();
        if (!kbd) return ListenerResult::Propagate;

        int keyId = static_cast<int>(data.key);
        bool shouldRemap = false;

        // Validamos si la tecla presionada es una de las habilitadas
        if (keyId == 162 && mod->getSettingValue<bool>("l-ctrl"))  shouldRemap = true; // L-Ctrl
        if (keyId == 163 && mod->getSettingValue<bool>("r-ctrl"))  shouldRemap = true; // R-Ctrl
        if (keyId == 160 && mod->getSettingValue<bool>("l-shift")) shouldRemap = true; // L-Shift
        if (keyId == 161 && mod->getSettingValue<bool>("r-shift")) shouldRemap = true; // R-Shift
        if (keyId == 164 && mod->getSettingValue<bool>("l-alt"))   shouldRemap = true; // L-Alt
        if (keyId == 165 && mod->getSettingValue<bool>("r-alt"))   shouldRemap = true; // R-Alt

        if (shouldRemap) {
            bool down = (data.action != geode::KeyboardInputData::Action::Release);
            modIsActive = down;
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, down, (data.action == geode::KeyboardInputData::Action::Repeat), data.timestamp);
            return ListenerResult::Stop;
        }

        forceSync(data.modifiers, data.timestamp);
        return ListenerResult::Propagate;
    }).leak();

    // ── MOUSE Y SCROLL (Rapid Checkpoints) ──────────────────────────────────
    geode::MouseInputEvent().listen([forceSync](geode::MouseInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return ListenerResult::Propagate;

        forceSync(data.modifiers, data.timestamp);

        // Solo actuamos si la configuración de Rapid Checkpoints está encendida
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
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, true,  false, 0.0);
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, false, false, 0.0);
                return ListenerResult::Stop;
            }
        }
        return ListenerResult::Propagate;
    }).leak();
}
