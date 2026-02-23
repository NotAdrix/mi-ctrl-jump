#include <Geode/Geode.hpp>

using namespace geode::prelude;

$execute {
    // ── TECLADO: Extracción por ID plano (Bypassing Geode::Key) ─────────────
    geode::KeyboardInputEvent().listen([](geode::KeyboardInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        auto mod = Mod::get();
        if (!kbd) return ListenerResult::Propagate;

        // Obtenemos el objeto de ajuste directamente
        auto sett1 = mod->getSetting("jump-keybind");
        auto sett2 = mod->getSetting("jump-keybind-2");

        if (sett1 && sett2) {
            // Geode guarda los bindeos internamente como {"key": ID, "modifiers": X}
            // Extraemos el ID numérico directamente del JSON para que compile SIEMPRE
            int64_t id1 = sett1->getValue()["key"].asInt().unwrapOr(0);
            int64_t id2 = sett2->getValue()["key"].asInt().unwrapOr(0);

            // Convertimos la tecla que acabas de presionar a un número plano
            int64_t pressed = static_cast<int64_t>(data.key);

            if (pressed == id1 || (id2 != 0 && pressed == id2)) {
                bool down = (data.action != geode::KeyboardInputData::Action::Release);
                
                // Transformamos el input en un salto de Barra Espaciadora
                kbd->dispatchKeyboardMSG(
                    enumKeyCodes::KEY_Space, 
                    down, 
                    (data.action == geode::KeyboardInputData::Action::Repeat), 
                    data.timestamp
                );

                return ListenerResult::Stop;
            }
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
