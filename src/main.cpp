#include <Geode/Geode.hpp>
#include <Geode/loader/Input.hpp> // <-- ESTO es lo que hace que "Key" exista

using namespace geode::prelude;

$execute {
    // ── TECLADO: Remapeo funcional ──────────────────────────────────────────
    geode::KeyboardInputEvent().listen([](geode::KeyboardInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        auto mod = Mod::get();
        if (!kbd) return ListenerResult::Propagate;

        // Mejoramos tu código: Pedimos el bindeo como 'geode::Key'
        // Es el único lenguaje que entiende un ajuste de tipo "keybind"
        auto key1 = mod->getSettingValue<geode::Key>("jump-keybind");
        auto key2 = mod->getSettingValue<geode::Key>("jump-keybind-2");

        // Comparamos directamente. Es ultra rápido y no falla.
        if (data.key == key1 || data.key == key2) {
            bool down = (data.action != geode::KeyboardInputData::Action::Release);
            
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

    // ── MOUSE: Tu lógica que ya funcionaba ──────────────────────────────────
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
