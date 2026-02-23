#include <Geode/Geode.hpp>

using namespace geode::prelude;

$execute {
    // ── TECLADO: Remapeo usando IDs Universales ─────────────────────────────
    geode::KeyboardInputEvent().listen([](geode::KeyboardInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        auto mod = Mod::get();
        if (!kbd) return ListenerResult::Propagate;

        // Usamos enumKeyCodes para evitar errores de nombres en el compilador
        auto key1 = mod->getSettingValue<enumKeyCodes>("jump-keybind");
        auto key2 = mod->getSettingValue<enumKeyCodes>("jump-keybind-2");

        // Convertimos la tecla presionada al mismo tipo para comparar
        auto pressedKey = static_cast<enumKeyCodes>(data.key);

        if (pressedKey == key1 || pressedKey == key2) {
            bool down = (data.action != geode::KeyboardInputData::Action::Release);
            
            // Enviamos el salto (Space)
            kbd->dispatchKeyboardMSG(
                enumKeyCodes::KEY_Space, 
                down, 
                (data.action == geode::KeyboardInputData::Action::Repeat), 
                data.timestamp
            );

            return ListenerResult::Stop;
        }

        return ListenerResult::Propagate;
    }).leak();

    // ── MOUSE: Remapeo funcional para Práctica ─────────────────────────────
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
