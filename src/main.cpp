#include <Geode/Geode.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/loader/SettingNode.hpp>

using namespace geode::prelude;

// Variables globales para controlar el estado de vinculación
bool g_isBindingJump1 = false;
bool g_isBindingJump2 = false;

$execute {
    // ── TECLADO DINÁMICO ────────────────────────────────────────────────────
    geode::KeyboardInputEvent().listen([](geode::KeyboardInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        auto mod = Mod::get();
        if (!kbd) return ListenerResult::Propagate;

        int currentKey = static_cast<int>(data.key);
        
        // --- Lógica de Vinculación (Modo Escucha) ---
        if (g_isBindingJump1 || g_isBindingJump2) {
            if (data.action == geode::KeyboardInputData::Action::Press) {
                int64_t j1 = mod->getSettingValue<int64_t>("jump1-key-id");
                int64_t j2 = mod->getSettingValue<int64_t>("jump2-key-id");

                if (g_isBindingJump1 && currentKey != j2) {
                    mod->setSettingValue("jump1-key-id", static_cast<int64_t>(currentKey));
                    g_isBindingJump1 = false;
                    FLAlertLayer::create("Éxito", "Tecla 1 vinculada", "OK")->show();
                } 
                else if (g_isBindingJump2 && currentKey != j1) {
                    mod->setSettingValue("jump2-key-id", static_cast<int64_t>(currentKey));
                    g_isBindingJump2 = false;
                    FLAlertLayer::create("Éxito", "Tecla 2 vinculada", "OK")->show();
                }
            }
            return ListenerResult::Stop; 
        }

        // --- Re-mapeo Dinámico al Espacio ---
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
            
            // Clic Derecho -> Z
            if (data.button == geode::MouseInputData::Button::Right) {
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, down, false, data.timestamp);
                return ListenerResult::Stop;
            }
            // Clic de la Rueda (Middle Click) -> X
            if (data.button == geode::MouseInputData::Button::Middle) {
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, down, false, data.timestamp);
                return ListenerResult::Stop;
            }
        }
        return ListenerResult::Propagate;
    }).leak();
}
