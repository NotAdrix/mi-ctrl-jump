#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/cocos/robtop/glfw/glfw3.h>

// Resolución dinámica cross-platform
#ifdef GEODE_IS_WINDOWS
  #include <Windows.h>
#else
  #include <dlfcn.h>
#endif

using namespace geode::prelude;

// Puntero a la función resuelta en runtime
using PFN_glfwSetMouseButtonCallback =
    GLFWmousebuttonfun(*)(GLFWwindow*, GLFWmousebuttonfun);

static PFN_glfwSetMouseButtonCallback s_glfwSetMouseButtonCallback = nullptr;

static bool resolveGLFW() {
#ifdef GEODE_IS_WINDOWS
    // Buscamos en todos los módulos cargados; GD embebe GLFW en libcocos2d.dll
    static const char* candidates[] = {
        "libcocos2d.dll",
        "GeometryDash.exe",  // a veces está en el ejecutable principal
        nullptr
    };
    for (int i = 0; candidates[i]; ++i) {
        HMODULE h = GetModuleHandleA(candidates[i]);
        if (!h) continue;
        auto fn = reinterpret_cast<PFN_glfwSetMouseButtonCallback>(
            GetProcAddress(h, "glfwSetMouseButtonCallback")
        );
        if (fn) { s_glfwSetMouseButtonCallback = fn; return true; }
    }
    // Último recurso: buscar en el espacio de proceso completo
    // (no hay equivalente directo, así que logueamos el fallo)
    log::error("glfwSetMouseButtonCallback no encontrada en ningún módulo Windows");
    return false;
#else
    // Linux / Mac: RTLD_DEFAULT busca en todo el proceso
    auto fn = reinterpret_cast<PFN_glfwSetMouseButtonCallback>(
        dlsym(RTLD_DEFAULT, "glfwSetMouseButtonCallback")
    );
    if (fn) { s_glfwSetMouseButtonCallback = fn; return true; }
    log::error("glfwSetMouseButtonCallback no encontrada: {}", dlerror());
    return false;
#endif
}

// ─────────────────────────────────────────────────────────────────────────────

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

// ── Callback de mouse ─────────────────────────────────────────────────────
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

    if (!handled && s.originalMouseCallback)
        s.originalMouseCallback(window, button, action, mods);
}

// ── Hook de teclado ───────────────────────────────────────────────────────
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool isRepeat, double timestamp) {
        auto* pl = PlayLayer::get();

        if (s.bindingSlot != BindingSlot::None) {
            if (pl) {
                cancelBinding();
            } else {
                if (down && !isRepeat) {
                    if (key == enumKeyCodes::KEY_Escape) cancelBinding();
                    else tryBindKey(key);
                }
                return true;
            }
        }

        if (!pl) {
            s.jumpKeysHeld = 0;
            return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, isRepeat, timestamp);
        }

        if (pl->m_isPaused) {
            s.jumpKeysHeld = 0;
            bool isJ1 = s.jump1Enabled && s.jump1Key != enumKeyCodes::KEY_Unknown && key == s.jump1Key;
            bool isJ2 = s.jump2Enabled && s.jump2Key != enumKeyCodes::KEY_Unknown && key == s.jump2Key;
            if ((isJ1 || isJ2) && key != JUMP_KEY)
                return CCKeyboardDispatcher::dispatchKeyboardMSG(JUMP_KEY, down, isRepeat, timestamp);
            return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, isRepeat, timestamp);
        }

        bool isJ1 = s.jump1Enabled && s.jump1Key != enumKeyCodes::KEY_Unknown && key == s.jump1Key;
        bool isJ2 = s.jump2Enabled && s.jump2Key != enumKeyCodes::KEY_Unknown && key == s.jump2Key;

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

    listenForSettingChanges<bool>("enable-jump-1", [](bool enabled) {
        s.jump1Enabled = enabled;
        if (enabled) {
            s.bindingSlot = BindingSlot::Jump1;
            geode::Notification::create("Presiona una tecla para vincular (ESC cancela)", geode::NotificationIcon::Info)->show();
        } else { cancelBinding(); clearKey(BindingSlot::Jump1); }
    });

    listenForSettingChanges<bool>("enable-jump-2", [](bool enabled) {
        s.jump2Enabled = enabled;
        if (enabled) {
            s.bindingSlot = BindingSlot::Jump2;
            geode::Notification::create("Presiona una tecla para vincular (ESC cancela)", geode::NotificationIcon::Info)->show();
        } else { cancelBinding(); clearKey(BindingSlot::Jump2); }
    });

    listenForSettingChanges<bool>("rapid-checkpoints", [](bool enabled) {
        s.rapidCheckpoints = enabled;
    });

    // ── Registrar callback de mouse via resolución dinámica ───────────────
    if (resolveGLFW()) {
        auto* view = CCEGLView::sharedOpenGLView();
        if (view) {
            GLFWwindow* window = view->getWindow();
            if (window) {
                s.originalMouseCallback = s_glfwSetMouseButtonCallback(window, mouseButtonCallback);
            }
        }
    }
}
