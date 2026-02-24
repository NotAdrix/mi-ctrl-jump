#include <Geode/Geode.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

using namespace geode::prelude;

$execute {
    // ── TECLADO DINÁMICO ────────────────────────────────────────────────────
    geode::KeyboardInputEvent().listen([](geode::KeyboardInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        auto mod = Mod::get();
        if (!kbd) return ListenerResult::Propagate;

        int currentKey = static_cast<int>(data.key);
        
        // --- Lógica de Vinculación (Leyendo las casillas del menú) ---
        bool isBinding1 = mod->getSettingValue<bool>("binding-mode-1");
        bool isBinding2 = mod->getSettingValue<bool>("binding-mode-2");

        if (isBinding1 || isBinding2) {
            if (data.action == geode::KeyboardInputData::Action::Press) {
                
                // Si el usuario toca la tecla Escape, cancelamos la vinculación
                if (currentKey == static_cast<int>(enumKeyCodes::KEY_Escape)) {
                    mod->setSettingValue("binding-mode-1", false);
                    mod->setSettingValue("binding-mode-2", false);
                    FLAlertLayer::create("Cancelado", "Vinculación cancelada.", "OK")->show();
                    return ListenerResult::Propagate;
                }

                int64_t j1 = mod->getSettingValue<int64_t>("jump1-key-id");
                int64_t j2 = mod->getSettingValue<int64_t>("jump2-key-id");

                if (isBinding1 && currentKey != j2) {
                    mod->setSettingValue("jump1-key-id", static_cast<int64_t>(currentKey));
                    mod->setSettingValue("binding-mode-1", false); // Apaga la casilla sola
                    FLAlertLayer::create("Éxito", "Tecla 1 vinculada con éxito.", "OK")->show();
                } 
                else if (isBinding2 && currentKey != j1) {
                    mod->setSettingValue("jump2-key-id", static_cast<int64_t>(currentKey));
                    mod->setSettingValue("binding-mode-2", false); // Apaga la casilla sola
                    FLAlertLayer::create("Éxito", "Tecla 2 vinculada con éxito.", "OK")->show();
                }
            }
            return ListenerResult::Stop; // Detenemos la tecla para que el juego no la lea
        }

        // --- Re-mapeo Dinámico al Espacio (Modo Juego) ---
        int64_t jump1Key = mod->getSettingValue<int64_t>("jump1-key-id");
        int64_t jump2Key = mod->getSettingValue<int64_t>("jump2-key-id");

        if (currentKey != 0 && (currentKey == jump1Key || currentKey == jump2Key)) {
            bool down = (data.action != geode::KeyboardInputData::Action::Release);
            bool isRepeat = (data.action == geode::KeyboardInputData::Action::Repeat);

            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, down, isRepeat, data.timestamp);
            return ListenerResult::Stop; 
        }

        return ListenerResult::Propagate;
    }).leak();

    // ── MOUSE (Rapid Checkpoints: Clic Derecho y Clic Rueda) ───────────────
    geode::MouseInputEvent().listen([](geode::MouseInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return ListenerResult::Propagate;

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
}
