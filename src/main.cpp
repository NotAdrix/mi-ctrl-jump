#include <Geode/Geode.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

using namespace geode::prelude;

bool g_isBindingJump1 = false;
bool g_isBindingJump2 = false;

$execute {
    // --- 1. ESCUCHADORES (Se activan al poner el primer APPLY) ---
    listenForSettingChanges<bool>("enable-jump-1", [](bool enabled) {
        if (enabled) {
            g_isBindingJump1 = true;
            FLAlertLayer::create("Modo Escucha", "Presiona la tecla que quieres usar para el Salto 1.\n\nPresiona [ESC] para cancelar.", "OK")->show();
        } else {
            // Borrado silencioso de la memoria privada
            Mod::get()->setSavedValue("jump1-key-id", static_cast<int64_t>(0));
            geode::Notification::create("Salto 1 eliminado", geode::NotificationIcon::Error)->show();
        }
    });

    listenForSettingChanges<bool>("enable-jump-2", [](bool enabled) {
        if (enabled) {
            g_isBindingJump2 = true;
            FLAlertLayer::create("Modo Escucha", "Presiona la tecla que quieres usar para el Salto 2.\n\nPresiona [ESC] para cancelar.", "OK")->show();
        } else {
            Mod::get()->setSavedValue("jump2-key-id", static_cast<int64_t>(0));
            geode::Notification::create("Salto 2 eliminado", geode::NotificationIcon::Error)->show();
        }
    });

    // --- 2. TECLADO DINÁMICO ---
    geode::KeyboardInputEvent().listen([](geode::KeyboardInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        auto mod = Mod::get();
        if (!kbd) return ListenerResult::Propagate;

        int currentKey = static_cast<int>(data.key);
        
        // --- Modo Vinculación ---
        if (g_isBindingJump1 || g_isBindingJump2) {
            if (data.action == geode::KeyboardInputData::Action::Press) {
                
                // Si el usuario cancela con ESC
                if (currentKey == static_cast<int>(enumKeyCodes::KEY_Escape)) {
                    if (g_isBindingJump1) { g_isBindingJump1 = false; mod->setSettingValue("enable-jump-1", false); } 
                    else { g_isBindingJump2 = false; mod->setSettingValue("enable-jump-2", false); }
                    
                    FLAlertLayer::create("Cancelado", "Vinculacion cancelada.\n\nPresiona APPLY para confirmar que desmarcaste la casilla.", "OK")->show();
                    return ListenerResult::Stop;
                }

                // Leemos la memoria privada (0 es el valor por defecto si no hay nada)
                int64_t j1 = mod->getSavedValue<int64_t>("jump1-key-id", 0);
                int64_t j2 = mod->getSavedValue<int64_t>("jump2-key-id", 0);

                if (g_isBindingJump1 && currentKey != j2) {
                    mod->setSavedValue("jump1-key-id", static_cast<int64_t>(currentKey));
                    g_isBindingJump1 = false;
                    FLAlertLayer::create("Exito", "¡Tecla 1 vinculada con exito!\n\nYa puedes cerrar el menu sin presionar nada mas.", "OK")->show();
                } 
                else if (g_isBindingJump2 && currentKey != j1) {
                    mod->setSavedValue("jump2-key-id", static_cast<int64_t>(currentKey));
                    g_isBindingJump2 = false;
                    FLAlertLayer::create("Exito", "¡Tecla 2 vinculada con exito!\n\nYa puedes cerrar el menu sin presionar nada mas.", "OK")->show();
                }
            }
            return ListenerResult::Stop; 
        }

        // --- Modo Juego (Re-mapeo) ---
        bool isEnabled1 = mod->getSettingValue<bool>("enable-jump-1");
        bool isEnabled2 = mod->getSettingValue<bool>("enable-jump-2");
        int64_t jump1Key = mod->getSavedValue<int64_t>("jump1-key-id", 0);
        int64_t jump2Key = mod->getSavedValue<int64_t>("jump2-key-id", 0);

        bool isJump1Pressed = (isEnabled1 && jump1Key != 0 && currentKey == jump1Key);
        bool isJump2Pressed = (isEnabled2 && jump2Key != 0 && currentKey == jump2Key);

        if (isJump1Pressed || isJump2Pressed) {
            bool down = (data.action != geode::KeyboardInputData::Action::Release);
            bool isRepeat = (data.action == geode::KeyboardInputData::Action::Repeat);

            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, down, isRepeat, data.timestamp);
            return ListenerResult::Stop; 
        }

        return ListenerResult::Propagate;
    }).leak();

    // ── MOUSE (Rapid Checkpoints) ──────────────────────────────────────────
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
