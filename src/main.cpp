#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/PlayLayer.hpp>

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
    int jumpKeysHeld = 0; // Contador para evitar que soltar una tecla cancele la otra
} s;

// --- Funciones de Utilidad ---
static void cancelBinding() {
    if (s.bindingSlot == BindingSlot::None) return;
    s.bindingSlot = BindingSlot::None;
    geode::Notification::create("Vinculación cancelada", geode::NotificationIcon::Warning)->show();
}

static void clearKey(BindingSlot slot) {
    auto* mod = Mod::get();
    if (slot == BindingSlot::Jump1) {
        s.jump1Key = enumKeyCodes::KEY_Unknown;
        mod->setSavedValue(SAVE_KEY_J1, static_cast<int64_t>(0));
        geode::Notification::create("Salto 1 eliminado", geode::NotificationIcon::Error)->show();
    } else {
        s.jump2Key = enumKeyCodes::KEY_Unknown;
        mod->setSavedValue(SAVE_KEY_J2, static_cast<int64_t>(0));
        geode::Notification::create("Salto 2 eliminado", geode::NotificationIcon::Error)->show();
    }
}

static bool tryBindKey(enumKeyCodes key) {
    enumKeyCodes otherKey = (s.bindingSlot == BindingSlot::Jump1) ? s.jump2Key : s.jump1Key;
    if (key == otherKey && key != enumKeyCodes::KEY_Unknown) {
        FLAlertLayer::create("Conflicto", "Esa tecla ya está usada.", "OK")->show();
        return false;
    }

    auto* mod = Mod::get();
    if (s.bindingSlot == BindingSlot::Jump1) {
        s.jump1Key = key;
        mod->setSavedValue(SAVE_KEY_J1, static_cast<int64_t>(key));
    } else {
        s.jump2Key = key;
        mod->setSavedValue(SAVE_KEY_J2, static_cast<int64_t>(key));
    }

    s.bindingSlot = BindingSlot::None;
    FLAlertLayer::create("Éxito", "¡Tecla vinculada!\nDesactiva y activa la casilla para cambiarla.", "OK")->show();
    return true;
}

// --- Hook de Teclado ---
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool isRepeat, double timestamp) {
        
        // 1. Lógica de Vinculación (Settings)
        if (s.bindingSlot != BindingSlot::None) {
            if (down && !isRepeat) {
                if (key == enumKeyCodes::KEY_Escape) cancelBinding();
                else tryBindKey(key);
            }
            return true; // Bloquear otras acciones mientras bindeas
        }

        // 2. Verificación de Juego
        auto* pl = PlayLayer::get();
        if (!pl || pl->m_isPaused) {
            s.jumpKeysHeld = 0;
            return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, isRepeat, timestamp);
        }

        // 3. Re-mapeo de Saltos
        bool isJ1 = s.jump1Enabled && s.jump1Key != enumKeyCodes::KEY_Unknown && key == s.jump1Key;
        bool isJ2 = s.jump2Enabled && s.jump2Key != enumKeyCodes::KEY_Unknown && key == s.jump2Key;

        if ((isJ1 || isJ2) && key != JUMP_KEY) {
            if (down) {
                if (!isRepeat) {
                    if (s.jumpKeysHeld == 0)
                        CCKeyboardDispatcher::dispatchKeyboardMSG(JUMP_KEY, true, false, timestamp);
                    s.jumpKeysHeld++;
                } else {
                    // Soporte para salto mantenido (Repeat)
                    CCKeyboardDispatcher::dispatchKeyboardMSG(JUMP_KEY, true, true, timestamp);
                }
            } else {
                if (s.jumpKeysHeld > 0) s.jumpKeysHeld--;
                if (s.jumpKeysHeld == 0)
                    CCKeyboardDispatcher::dispatchKeyboardMSG(JUMP_KEY, false, false, timestamp);
            }
            return true;
        }

        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, isRepeat, timestamp);
    }
};

// --- Manejo de Mouse ---
void handleMouseInput(geode::MouseInputData& data) {
    if (!s.rapidCheckpoints) return;
    
    auto* pl = PlayLayer::get();
    // Solo en modo práctica y que no esté pausado
    if (!pl || !pl->m_isPracticeMode || pl->m_isPaused) return;

    auto* kbd = CCKeyboardDispatcher::get();
    if (!kbd) return;

    bool down = (data.action == geode::MouseInputData::Action::Press);

    if (data.button == geode::MouseInputData::Button::Right) {
        kbd->dispatchKeyboardMSG(CHECKPOINT_SAVE, down, false, data.timestamp);
    }
    else if (data.button == geode::MouseInputData::Button::Middle) {
        kbd->dispatchKeyboardMSG(CHECKPOINT_LOAD, down, false, data.timestamp);
    }
}

// --- Entrada del Mod ---
$on_mod(Loaded) {
    auto* mod = Mod::get();

    // Cargar persistencia
    s.jump1Enabled = mod->getSettingValue<bool>("enable-jump-1");
    s.jump2Enabled = mod->getSettingValue<bool>("enable-jump-2");
    s.rapidCheckpoints = mod->getSettingValue<bool>("rapid-checkpoints");

    auto raw1 = mod->getSavedValue<int64_t>(SAVE_KEY_J1, 0);
    auto raw2 = mod->getSavedValue<int64_t>(SAVE_KEY_J2, 0);
    s.jump1Key = raw1 ? static_cast<enumKeyCodes>(raw1) : enumKeyCodes::KEY_Unknown;
    s.jump2Key = raw2 ? static_cast<enumKeyCodes>(raw2) : enumKeyCodes::KEY_Unknown;

    // Listeners de configuración
    listenForSettingChanges<bool>("enable-jump-1", [](bool enabled) {
        s.jump1Enabled = enabled;
        if (enabled) {
            s.bindingSlot = BindingSlot::Jump1;
            geode::Notification::create("Presiona una tecla para Salto 1", geode::NotificationIcon::Info)->show();
        } else {
            clearKey(BindingSlot::Jump1);
        }
    });

    listenForSettingChanges<bool>("enable-jump-2", [](bool enabled) {
        s.jump2Enabled = enabled;
        if (enabled) {
            s.bindingSlot = BindingSlot::Jump2;
            geode::Notification::create("Presiona una tecla para Salto 2", geode::NotificationIcon::Info)->show();
        } else {
            clearKey(BindingSlot::Jump2);
        }
    });

    listenForSettingChanges<bool>("rapid-checkpoints", [](bool enabled) {
        s.rapidCheckpoints = enabled;
    });

    // REGISTRO CORRECTO DEL MOUSE
    geode::MouseInputEvent().listen([](geode::MouseInputData& data) {
        handleMouseInput(data);
        return ListenerResult::Propagate;
    }).leak();
}
