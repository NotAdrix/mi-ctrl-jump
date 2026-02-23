#include <Geode/Geode.hpp>

using namespace geode::prelude;

$execute {
    // ── TECLADO: Remapeo por ID con Autodetección de Tipo ──────────────────
    geode::KeyboardInputEvent().listen([](geode::KeyboardInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        auto mod = Mod::get();
        if (!kbd) return ListenerResult::Propagate;

        // Usamos decltype(data.key) para que el compilador use el tipo de dato correcto
        // sin importar si es geode::Key, int o enumKeyCodes en esta versión de la SDK.
        auto key1 = mod->getSettingValue<decltype(data.key)>("jump-keybind");
        auto key2 = mod->getSettingValue<decltype(data.key)>("jump-keybind-2");

        // Si la tecla presionada coincide con alguno de los bindeos
        if (data.key == key1 || data.key == key2) {
            bool down = (data.action != geode::KeyboardInputData::Action::Release);
            
            // Enmascaramos y enviamos el Espacio
            kbd->dispatchKeyboardMSG(
                enumKeyCodes::KEY_Space, 
                down, 
                (data.action == geode::KeyboardInputData::Action::Repeat), 
                data.timestamp
            );

            // Bloqueamos la tecla original (evita bugs y telemetría accidental)
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
