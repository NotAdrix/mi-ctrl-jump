#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

// --- Configuración y Constantes ---
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
    // Forzamos el guardado en disco para evitar pérdida por crash
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
            s.jumpKeysHeld = 0; // Reset preventivo
            return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, isRepeat, timestamp);
        }

        // 3. Re-mapeo de Saltos con Optimización de Latencia
        bool isJ1 = s.jump1Enabled && key == s.jump1Key && s.jump1Key != enumKeyCodes::KEY_Unknown;
        bool isJ2 = s.jump2Enabled && key == s.jump2Key && s.jump2Key != enumKeyCodes::KEY_Unknown;

        if (isJ1 || isJ2) {
            if (down) {
                if (!isRepeat) {
                    s.jumpKeysHeld++;
                    // OPTIMIZACIÓN: Llamada directa al botón del jugador (salta el despachador)
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

// --- Hook de PlayLayer para limpieza ---
class $modify(PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontSave) {
        if (!PlayLayer::init(level, useReplay, dontSave)) return false;
        s.jumpKeysHeld = 0; // Aseguramos que el contador inicie limpio en cada nivel
        return true;
    }
};

// --- Entrada del Mod ---
$on_mod(Loaded) {
    auto* mod = Mod::get();

    // Carga de persistencia
    s.jump1Enabled = mod->getSettingValue<bool>("enable-jump-1");
    s.jump2Enabled = mod->getSettingValue<bool>("enable-jump-2");
    s.rapidCheckpoints = mod->getSettingValue<bool>("rapid-checkpoints");

    s.jump1Key = static_cast<enumKeyCodes>(mod->getSavedValue<int64_t>(SAVE_KEY_J1, 0));
    s.jump2Key = static_cast<enumKeyCodes>(mod->getSavedValue<int64_t>(SAVE_KEY_J2, 0));

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

    // Mouse Input sin .leak() innecesario (los listeners de Geode son inteligentes)
    geode::MouseInputEvent().listen([](geode::MouseInputData& data) {
        if (!s.rapidCheckpoints) return ListenerResult::Propagate;
        
        auto* pl = PlayLayer::get();
        if (!pl || !pl->m_isPracticeMode || pl->m_isPaused) return ListenerResult::Propagate;

        bool down = (data.action == geode::MouseInputData::Action::Press);

        if (data.button == geode::MouseInputData::Button::Right) {
            pl->createCheckpoint(); // Usamos la función nativa de PlayLayer directamente
            return ListenerResult::Stop;
        }
        else if (data.button == geode::MouseInputData::Button::Middle) {
            pl->removeLastCheckpoint(); // Función nativa para cargar/eliminar
            return ListenerResult::Stop;
        }
        return ListenerResult::Propagate;
    });
}
