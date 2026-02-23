#include <Geode/Geode.hpp>

using namespace geode::prelude;

$execute {
    // ── TECLADO: Remapeo y Enmascaramiento Puro ─────────────────────────────
    geode::KeyboardInputEvent().listen([](geode::KeyboardInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        auto mod = Mod::get();
        if (!kbd) return ListenerResult::Propagate;

        auto key1 = mod->getSettingValue<geode::Key>("jump-keybind");
        auto key2 = mod->getSettingValue<geode::Key>("jump-keybind-2");

        // Si coincide con cualquiera de los dos bindeos:
        if (data.key == key1 || data.key == key2) {
            bool down = (data.action != geode::KeyboardInputData::Action::Release);
            
            // 1. Enmascaramos el input original hacia el motor del juego
            // 2. Despachamos el salto (Espacio) con precisión de hardware (timestamp)
            kbd->dispatchKeyboardMSG(
                enumKeyCodes::KEY_Space, 
                down, 
                (data.action == geode::KeyboardInputData::Action::Repeat), 
                data.timestamp
            );

            // Bloqueamos la propagación: el juego nunca se entera de que tocaste la tecla original
            return ListenerResult::Stop;
        }

        return ListenerResult::Propagate;
    }).leak();

    // ── MOUSE: Remapeo funcional para Práctica ─────────────────────────────
    geode::MouseInputEvent().listen([](geode::MouseInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd || !Mod::get()->getSettingValue<bool>("rapid-checkpoints")) return ListenerResult::Propagate;

        bool down = (data.action == geode::MouseInputData::Action::Press);
        
        // Click Derecho -> Z (Enmascarado)
        if (data.button == geode::MouseInputData::Button::Right) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, down, false, data.timestamp);
            return ListenerResult::Stop;
        }
        
        // Click de Rueda (Middle) -> X (Enmascarado)
        if (data.button == geode::MouseInputData::Button::Middle) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, down, false, data.timestamp);
            return ListenerResult::Stop;
        }

        return ListenerResult::Propagate;
    }).leak();
}
