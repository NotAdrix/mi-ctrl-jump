#include <Geode/Geode.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

$execute {
    static bool modIsActive = false;

    // Función que calcula qué bits de hardware debemos vigilar según los Settings
    auto getTargetMask = []() {
        int mask = 0;
        auto mod = Mod::get();
        // Bit 2: Control | Bit 1: Shift | Bit 4: Alt
        if (mod->getSettingValue<bool>("l-ctrl") || mod->getSettingValue<bool>("r-ctrl")) mask |= 2;
        if (mod->getSettingValue<bool>("l-shift") || mod->getSettingValue<bool>("r-shift")) mask |= 1;
        if (mod->getSettingValue<bool>("l-alt") || mod->getSettingValue<bool>("r-alt")) mask |= 4;
        return mask;
    };

    // Sincronización pasiva: actúa cada vez que el usuario interactúa con el juego
    auto forceSync = [getTargetMask](geode::KeyboardModifier mods, double timestamp) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return;

        int activeMask = getTargetMask();
        bool anyTargetPhysicallyDown = (static_cast<int>(mods) & activeMask);

        // Si el mod cree que está saltando pero el hardware dice que no hay modificadores activos:
        if (modIsActive && !anyTargetPhysicallyDown) {
            modIsActive = false;
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, false, false, timestamp);
        }
    };

    // ── TECLADO: Remapeo Dinámico ───────────────────────────────────────────
    geode::KeyboardInputEvent().listen([forceSync](geode::KeyboardInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        auto mod = Mod::get();
        if (!kbd) return ListenerResult::Propagate;

        int keyId = static_cast<int>(data.key);
        bool shouldRemap = false;

        // Verificamos si la tecla que se tocó está habilitada en los settings
        if (data.key == geode::enumKeyCodes::KEY_Control && mod->getSettingValue<bool>("l-ctrl"))  shouldRemap = true;
        if (data.key == geode::enumKeyCodes::KEY_Control && mod->getSettingValue<bool>("r-ctrl"))  shouldRemap = true;
        if (data.key == geode::enumKeyCodes::KEY_Shift   && mod->getSettingValue<bool>("l-shift")) shouldRemap = true;
        if (data.key == geode::enumKeyCodes::KEY_Shift   && mod->getSettingValue<bool>("r-shift")) shouldRemap = true;
        if (data.key == geode::enumKeyCodes::KEY_Alt     && mod->getSettingValue<bool>("l-alt"))   shouldRemap = true;
        if (data.key == geode::enumKeyCodes::KEY_Alt     && mod->getSettingValue<bool>("r-alt"))   shouldRemap = true;

        if (shouldRemap) {
            bool down = (data.action != geode::KeyboardInputData::Action::Release);
            modIsActive = down;
            // CBF Precise: pasamos el timestamp del hardware
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
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, true,  false, 0.0);
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, false, false, 0.0);
                return ListenerResult::Stop;
            }
        }
        return ListenerResult::Propagate;
    }).leak();
} 
