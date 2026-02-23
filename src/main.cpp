#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>

using namespace geode::prelude;

$execute {
    // ── TECLADO: Remapeo usando geode::KeyCode ─────────────────────────────
    geode::KeyboardInputEvent().listen([](geode::KeyboardInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        auto mod = Mod::get();
        if (!kbd) return ListenerResult::Propagate;

        // En Geode v5, el tipo correcto para bindeos es geode::KeyCode
        auto key1 = mod->getSettingValue<geode::KeyCode>("jump-keybind");
        auto key2 = mod->getSettingValue<geode::KeyCode>("jump-keybind-2");

        // Comparamos directamente los KeyCodes
        if (data.key == key1 || data.key == key2) {
            bool down = (data.action != geode::KeyboardInputData::Action::Release);
            
            // Enviamos el salto (Space)
            kbd->dispatchKeyboardMSG(
                enumKeyCodes::KEY_Space, 
                down, 
                (data.action == geode::KeyboardInputData::Action::Repeat), 
                data.timestamp
            );

            // Bloqueamos la tecla original para evitar conflictos
            return ListenerResult::Stop;
        }

        return ListenerResult::Propagate;
    }).leak();

    // ── MOUSE: Click Derecho (Z) y Click de Rueda (X) ──────────────────────
    geode::MouseInputEvent().listen([](geode::MouseInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd || !Mod::get()->getSettingValue<bool>("rapid-checkpoints")) return ListenerResult::Propagate;

        bool down = (data.action == geode::MouseInputData::Action::Press);
        
        if (data.button == geode::MouseInputData::Button::Right) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, down, false, data.timestamp);
            return ListenerResult::Stop;
        }
        
        if (data.button == geode::MouseInputData::Button::Middle) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, down, false, data.timestamp);
            return ListenerResult::Stop;
        }

        return ListenerResult::Propagate;
    }).leak();
}
