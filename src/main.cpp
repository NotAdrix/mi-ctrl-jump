#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

// GLFW está disponible en Windows a través del SDK de Cocos2d incluido en Geode
#include <Geode/cocos/robtop/glfw/glfw3.h>

using namespace geode::prelude;

static constexpr enumKeyCodes JUMP_KEY    = enumKeyCodes::KEY_Space;
static constexpr const char* SAVE_KEY_J1 = "jump1-key-id";
static constexpr const char* SAVE_KEY_J2 = "jump2-key-id";

enum class BindingSlot { None, Jump1, Jump2 };

namespace {
    struct RemapState {
        enumKeyCodes jump1Key   = enumKeyCodes::KEY_Unknown;
        enumKeyCodes jump2Key   = enumKeyCodes::KEY_Unknown;
        bool jump1Enabled       = false;
        bool jump2Enabled       = false;
        bool rapidCheckpoints   = false;
        BindingSlot bindingSlot = BindingSlot::None;
        int jumpKeysHeld        = 0;

        // Guardamos el callback original de GLFW para encadenarlo
        GLFWmousebuttonfun originalMouseCallback = nullptr;
    } s;
}

static void clearKey(BindingSlot slot) {
    auto* mod = Mod::get();
    if (slot == BindingSlot::Jump1) {
        s.jump1Key = enumKeyCodes::KEY_Unknown;
        mod->setSavedValue(SAVE_KEY_J1, static_cast<int>(0));
        geode::Notification::create("Salto 1 desactivado", geode::NotificationIcon::Info)->show();
    } else {
        s.jump2Key = enumKeyCodes::KEY_Unknown;
        mod->setSavedValue(SAVE_KEY_J2, static_cast<int>(0));
        geode::Notification::create("Salto 2 desactivado", geode::NotificationIcon::Info)->show();
    }
}

static void cancelBinding() {
    if (s.bindingSlot == BindingSlot::None) return;
    s.bindingSlot = BindingSlot::None;
    geode::Notification::create("Vinculación cancelada", geode::NotificationIcon::Warning)->show();
}

static bool tryBindKey(enumKeyCodes key) {
    enumKeyCodes otherKey = (s.bindingSlot == BindingSlot::Jump1) ? s.jump2Key : s.jump1Key;

    if (key == otherKey && key != enumKeyCodes::KEY_Unknown) {
        Loader::get()->queueInMainThread([] {
            FLAlertLayer::create("Conflicto", "Esa tecla ya está usada por el otro salto.", "OK")->show();
        });
        return false;
    }

    auto* mod = Mod::get();
    if (s.bindingSlot == BindingSlot::Jump1) {
        s.jump1Key = key;
        mod->setSavedValue(SAVE_KEY_J1, static_cast<int>(key));
    } else {
        s.jump2Key = key;
        mod->setSavedValue(SAVE_KEY_J2, static_cast<int>(key));
    }

    s.bindingSlot = BindingSlot::None;

    Loader::get()->queueInMainThread([] {
        FLAlertLayer::create("Éxito", "¡Tecla vinculada correctamente!", "OK")->show();
    });
    return true;
}

// ── Callback de mouse registrado directamente en GLFW ─────────────────────
static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    bool handled = false;

    if (s.rapidCheckpoints) {
        auto* pl = PlayLayer::get();
        if (pl && pl->m_isPracticeMode && !pl->m_isPaused) {
            auto* kbd = CCKeyboardDispatcher::get();
            if (kbd) {
                bool down = (action == GLFW_PRESS);

                if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                    kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, down, false, 0.0);
                    handled = true;
                } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
                    kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, down, false, 0.0);
                    handled = true;
                }
            }
        }
    }

    // Si no lo manejamos nosotros, propagamos al callback original de Cocos
    if (!handled && s.originalMouseCallback) {
        s.originalMouseCallback(window, button, action, mods);
    }
}

// ── Hook de teclado ───────────────────────────────────────────────────────
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool isRepeat, double timestamp) {
        auto* pl = PlayLayer::get();

        // ── Modo binding ──────────────────────────────────────────────────
        if (s.bindingSlot != BindingSlot::None) {
            if (pl) {
                cancelBinding();
            } else {
                if (down && !isRepeat) {
                    if (key == enumKeyCodes::KEY_Escape) {
                        cancelBinding();
                    } else {
                        tryBindKey(key);
                    }
                }
                return true;
            }
        }

        // ── Fuera de nivel: reset y propagación normal ────────────────────
        if (!pl) {
            s.jumpKeysHeld = 0;
            return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, isRepeat, timestamp);
        }

        // ── En pausa: remapeo sin trackear contador ───────────────────────
        if (pl->m_isPaused) {
            s.jumpKeysHeld = 0;
            bool isJ1 = s.jump1Enabled && s.jump1Key != enumKeyCodes::KEY_Unknown && key == s.jump1Key;
            bool isJ2 = s.jump2Enabled && s.jump2Key != enumKeyCodes::KEY_Unknown && key == s.jump2Key;
            if ((isJ1 || isJ2) && key != JUMP_KEY)
                return CCKeyboardDispatcher::dispatchKeyboardMSG(JUMP_KEY, down, isRepeat, timestamp);
            return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, isRepeat, timestamp);
        }

        // ── Modo juego ────────────────────────────────────────────────────
        bool isJ1 = s.jump1Enabled
                 && s.jump1Key != enumKeyCodes::KEY_Unknown
                 && key == s.jump1Key;

        bool isJ2 = s.jump2Enabled
                 && s.jump2Key != enumKeyCodes::KEY_Unknown
                 && key == s.jump2Key;

        if ((isJ1 || isJ2) && key != JUMP_KEY) {
            if (down && !isRepeat) {
                if (s.jumpKeysHeld == 0)
                    CCKeyboardDispatcher::dispatchKeyboardMSG(JUMP_KEY, true, false, timestamp);
                s.jumpKeysHeld++;
            } else if (down && isRepeat) {
                if (s.jumpKeysHeld == 1)
                    CCKeyboardDispatcher::dispatchKeyboardMSG(JUMP_KEY, true, true, timestamp);
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

// ── Entry point ───────────────────────────────────────────────────────────
$on_mod(Loaded) {
    auto* mod = Mod::get();

    s.jump1Enabled     = mod->getSettingValue<bool>("enable-jump-1");
    s.jump2Enabled     = mod->getSettingValue<bool>("enable-jump-2");
    s.rapidCheckpoints = mod->getSettingValue<bool>("rapid-checkpoints");

    auto raw1 = mod->getSavedValue<int>(SAVE_KEY_J1, 0);
    auto raw2 = mod->getSavedValue<int>(SAVE_KEY_J2, 0);
    s.jump1Key = raw1 ? static_cast<enumKeyCodes>(raw1) : enumKeyCodes::KEY_Unknown;
    s.jump2Key = raw2 ? static_cast<enumKeyCodes>(raw2) : enumKeyCodes::KEY_Unknown;

    // Registro de settings
    listenForSettingChanges<bool>("enable-jump-1", [](bool enabled) {
        s.jump1Enabled = enabled;
        if (enabled) {
            s.bindingSlot = BindingSlot::Jump1;
            geode::Notification::create(
                "Presiona una tecla para vincular (ESC cancela)",
                geode::NotificationIcon::Info
            )->show();
        } else {
            cancelBinding();
            clearKey(BindingSlot::Jump1);
        }
    });

    listenForSettingChanges<bool>("enable-jump-2", [](bool enabled) {
        s.jump2Enabled = enabled;
        if (enabled) {
            s.bindingSlot = BindingSlot::Jump2;
            geode::Notification::create(
                "Presiona una tecla para vincular (ESC cancela)",
                geode::NotificationIcon::Info
            )->show();
        } else {
            cancelBinding();
            clearKey(BindingSlot::Jump2);
        }
    });

    listenForSettingChanges<bool>("rapid-checkpoints", [](bool enabled) {
        s.rapidCheckpoints = enabled;
    });

    // ── Registrar callback de mouse directo en GLFW ───────────────────────
    // Geode corre en el main thread, CCEGLView ya está inicializado en este punto.
    // Obtenemos la ventana GLFW y reemplazamos el callback guardando el original.
    auto* view = CCEGLView::sharedOpenGLView();
    if (view) {
        GLFWwindow* window = view->getWindow();
        if (window) {
            s.originalMouseCallback = glfwSetMouseButtonCallback(window, mouseButtonCallback);
        }
    }
}
