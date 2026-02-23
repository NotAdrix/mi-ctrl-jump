#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp> // Header crítico para manejar geode::Key

using namespace geode::prelude;

$execute {
    // ── TECLADO: Remapeo con Cast Numérico para Latencia Cero ────────────────
    geode::KeyboardInputEvent().listen([](geode::KeyboardInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        auto mod = Mod::get();
        if (!kbd) return ListenerResult::Propagate;

        // Obtenemos el bindeo como geode::Key (el tipo que Geode v5 espera)
        auto key1 = mod->getSettingValue<geode::Key>("jump-keybind");
        auto key2 = mod->getSettingValue<geode::Key>("jump-keybind-2");

        // Convertimos ambos a int. Esto elimina errores de tipos y asegura 
        // que la comparación sea la operación más rápida posible del CPU.
        int pressedKey = static_cast<int>(data.key);
        int target1 = static_cast<int>(key1);
        int target2 = static_cast<int>(key2);

        if (pressedKey == target1 || pressedKey == target2) {
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
