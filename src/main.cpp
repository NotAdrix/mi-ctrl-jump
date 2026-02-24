#include <Geode/Geode.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

using namespace geode::prelude;
// Variables globales para controlar el estado de vinculación
bool g_isBindingJump1 = false;
bool g_isBindingJump2 = false;

$execute {
    static bool modIsActive = false;

    // Función para verificar si un modificador físico debe estar activado según Settings
    auto getEnabledMask = []() {
        int mask = 0;
        auto mod = Mod::get();
        // Si L-Ctrl o R-Ctrl están activados en settings, vigilamos el bit de Control (2)
        if (mod->getSettingValue<bool>("l-ctrl") || mod->getSettingValue<bool>("r-ctrl")) mask |= 2;
        // Si L-Shift o R-Shift están activados, vigilamos el bit de Shift (1)
        if (mod->getSettingValue<bool>("l-shift") || mod->getSettingValue<bool>("r-shift")) mask |= 1;
        // Si L-Alt o R-Alt están activados, vigilamos el bit de Alt (4)
        if (mod->getSettingValue<bool>("l-alt") || mod->getSettingValue<bool>("r-alt")) mask |= 4;
        return mask;
    };

    auto forceSync = [getEnabledMask](geode::KeyboardModifier mods, double timestamp) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return;

        int activeMask = getEnabledMask();
        bool anyTargetModifierDown = (static_cast<int>(mods) & activeMask);

        if (modIsActive && !anyTargetModifierDown) {
            modIsActive = false;
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, false, false, timestamp);
        }
    };

    // ── TECLADO DINÁMICO ────────────────────────────────────────────────────
    geode::KeyboardInputEvent().listen([](geode::KeyboardInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        auto mod = Mod::get();
        if (!kbd) return ListenerResult::Propagate;

        int currentKey = static_cast<int>(data.key);
        
        // --- PASO 3 y 4: Lógica de Vinculación (Settings) ---
        if (g_isBindingJump1 || g_isBindingJump2) {
            // Solo vinculamos cuando el usuario presiona la tecla (no cuando la suelta)
            if (data.action == geode::KeyboardInputData::Action::Press) {
                int64_t j1 = mod->getSettingValue<int64_t>("jump1-key-id");
                int64_t j2 = mod->getSettingValue<int64_t>("jump2-key-id");

                // Validación de duplicados
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
            return ListenerResult::Stop; // Detenemos todo mientras vinculamos
        }

        // --- PASO 5: Re-mapeo dinámico ---
        int64_t jump1Key = mod->getSettingValue<int64_t>("jump1-key-id");
        int64_t jump2Key = mod->getSettingValue<int64_t>("jump2-key-id");

        if (currentKey != 0 && (currentKey == jump1Key || currentKey == jump2Key)) {
            // Calculamos si está presionado o soltado
            bool down = (data.action != geode::KeyboardInputData::Action::Release);
            bool isRepeat = (data.action == geode::KeyboardInputData::Action::Repeat);

            // PASO 6: El "Engaño" al motor - Enviamos SPACE
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, down, isRepeat, data.timestamp);
            
            return ListenerResult::Stop; // La tecla original deja de existir
        }

        return ListenerResult::Propagate;
    }).leak();

    // ── MOUSE Y SCROLL (Rapid Checkpoints) ──────────────────────────────────
    geode::MouseInputEvent().listen([forceSync](geode::MouseInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return ListenerResult::Propagate;

        forceSync(data.modifiers, data.timestamp);

        // Solo actuamos si la configuración de Rapid Checkpoints está encendida
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

    geode::ScrollWheelEvent().listen(+[](double x, double y) {
        if (Mod::get()->getSettingValue<bool>("rapid-checkpoints")) {
            auto kbd = CCKeyboardDispatcher::get();
            if (kbd) {
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, true,  false, 0.0);
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, false, false, 0.0);
                return ListenerResult::Stop;
            }
        }
        return ListenerResult::Propagate;
    }).leak();
}
