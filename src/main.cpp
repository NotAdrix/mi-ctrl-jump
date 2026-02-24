#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

// --- Configuración y Constantes ---
static constexpr enumKeyCodes JUMP_KEY = enumKeyCodes::KEY_Space;
static constexpr enumKeyCodes CHECKPOINT_SAVE = enumKeyCodes::KEY_Z;
static constexpr enumKeyCodes CHECKPOINT_LOAD = enumKeyCodes::KEY_X;
static constexpr const char* SAVE_KEY_J1 = "jump1-key-id";
static constexpr const char* SAVE_KEY_J2 = "jump2-key-id";

enum class BindingSlot { None, Jump1, Jump2 };

struct RemapState {
    enumKeyCodes jump1Key = enumKeyCodes::KEY_Unknown;
    enumKeyCodes jump2Key = enumKeyCodes::KEY_Unknown;
    bool jump1Enabled = false;
    bool jump2Enabled = false;
    bool rapidCheckpoints = false;
    BindingSlot bindingSlot = BindingSlot::None;
    int jumpKeysHeld = 0; 
} s;

// --- Funciones de Utilidad ---
static void saveKeybinds() {
    auto* mod = Mod::get();
    mod->setSavedValue(SAVE_KEY_J1, static_cast<int64_t>(s.jump1Key));
    mod->setSavedValue(SAVE_KEY_J2, static_cast<int64_t>(s.jump2Key));
    (void)mod->save(); 
}

static void clearKey(BindingSlot slot) {
    if (slot == BindingSlot::Jump1) s.jump1Key = enumKeyCodes::KEY_Unknown;
    else s.jump2Key = enumKeyCodes::KEY_Unknown;
    saveKeybinds();
    geode::Notification::create("Tecla eliminada", geode::NotificationIcon::Error)->show();
}

// --- Hook de Teclado ---
class $modify(MyKeyboardHandler, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool isRepeat, double timestamp) {
        
        // 1. Lógica de Vinculación
        if (s.bindingSlot != BindingSlot::None) {
            if (down && !isRepeat) {
                if (key == enumKeyCodes::KEY_Escape) {
                    s.bindingSlot = BindingSlot::None;
                    geode::Notification::create("Cancelado", geode::NotificationIcon::Warning)->show();
                } else {
                    if (s.bindingSlot == BindingSlot::Jump1) s.jump1Key = key;
                    else s.jump2Key = key;
                    s.bindingSlot = BindingSlot::None;
                    saveKeybinds();
                    FLAlertLayer::create("Éxito", "Tecla vinculada correctamente.", "OK")->show();
                }
            }
            return true; 
        }

        // 2. Verificación de Juego
        auto* pl = PlayLayer::get();
        if (!pl || pl->m_isPaused) {
            s.jumpKeysHeld = 0; // Corrección: Resetear si el juego no está activo
            return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, isRepeat, timestamp);
        }

        // 3. Re-mapeo de Saltos con Optimización de Latencia y Corrección de Repeat
        bool isJ1 = s.jump1Enabled && key == s.jump1Key && s.jump1Key != enumKeyCodes::KEY_Unknown;
        bool isJ2 = s.jump2Enabled && key == s.jump2Key && s.jump2Key != enumKeyCodes::KEY_Unknown;

        if ((isJ1 || isJ2) && key != JUMP_KEY) {
            if (down) {
                if (!isRepeat) { // Corrección: Solo incrementamos en la pulsación inicial
                    s.jumpKeysHeld++;
                    if (s.jumpKeysHeld == 1) {
                        pl->handleButton(true, static_cast<int>(PlayerButton::Jump), false);
                    }
                }
            } else {
                if (s.jumpKeysHeld > 0) s.jumpKeysHeld--;
                if (s.jumpKeysHeld == 0) {
                    pl->handleButton(false, static_cast<int>(PlayerButton::Jump), false);
                }
            }
            return true;
        }

        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, isRepeat, timestamp);
    }
};

// --- Hook de PlayLayer para asegurar limpieza de estado ---
class $modify(PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontSave) {
        if (!PlayLayer::init(level, useReplay, dontSave)) return false;
        s.jumpKeysHeld = 0; // Corrección: Evitar ghosting al entrar al nivel
        return true;
    }
};

// --- Entrada del Mod ---
$on_mod(Loaded) {
    auto* mod = Mod::get();

    s.jump1Enabled = mod->getSettingValue<bool>("enable-jump-1");
    s.jump2Enabled = mod->getSettingValue<bool>("enable-jump-2");
    s.rapidCheckpoints = mod->getSettingValue<bool>("rapid-checkpoints");

    s.jump1Key = static_cast<enumKeyCodes>(mod->getSavedValue<int64_t>(SAVE_KEY_J1, 0));
    s.jump2Key = static_cast<enumKeyCodes>(mod->getSavedValue<int64_t>(SAVE_KEY_J2, 0));

    // Listeners
    listenForSettingChanges<bool>("enable-jump-1", [](bool enabled) {
        s.jump1Enabled = enabled;
        if (enabled) s.bindingSlot = BindingSlot::Jump1;
        else clearKey(BindingSlot::Jump1);
    });

    listenForSettingChanges<bool>("enable-jump-2", [](bool enabled) {
        s.jump2Enabled = enabled;
        if (enabled) s.bindingSlot = BindingSlot::Jump2;
        else clearKey(BindingSlot::Jump2);
    });

    listenForSettingChanges<bool>("rapid-checkpoints", [](bool enabled) {
        s.rapidCheckpoints = enabled;
    });

    // Mantenemos tu lógica original de Mouse -> Dispatcher para Checkpoints
    geode::MouseInputEvent().listen([](geode::MouseInputData& data) {
        if (!s.rapidCheckpoints) return ListenerResult::Propagate;
        
        auto* pl = PlayLayer::get();
        if (!pl || !pl->m_isPracticeMode || pl->m_isPaused) return ListenerResult::Propagate;

        auto* kbd = CCKeyboardDispatcher::get();
        if (!kbd) return ListenerResult::Propagate;

        bool down = (data.action == geode::MouseInputData::Action::Press);

        if (data.button == geode::MouseInputData::Button::Right) {
            kbd->dispatchKeyboardMSG(CHECKPOINT_SAVE, down, false, data.timestamp);
            return ListenerResult::Stop;
        }
        else if (data.button == geode::MouseInputData::Button::Middle) {
            kbd->dispatchKeyboardMSG(CHECKPOINT_LOAD, down, false, data.timestamp);
            return ListenerResult::Stop;
        }
        return ListenerResult::Propagate;
    });
}
