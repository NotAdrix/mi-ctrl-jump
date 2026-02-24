#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCEGLView.hpp>

using namespace geode::prelude;

static constexpr enumKeyCodes JUMP_KEY    = enumKeyCodes::KEY_Space;
static constexpr const char*  SAVE_KEY_J1 = "jump1-key-id";
static constexpr const char*  SAVE_KEY_J2 = "jump2-key-id";

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

        geode::comm::ListenerHandle* j1SettingHandle = nullptr;
        geode::comm::ListenerHandle* j2SettingHandle = nullptr;
        geode::comm::ListenerHandle* rcSettingHandle = nullptr;
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

// ── Hook de teclado ───────────────────────────────────────────────────────
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool isRepeat, double timestamp) {
        auto* pl = PlayLayer::get();

        // ── Modo binding ──────────────────────────────────────────────────
        if (s.bindingSlot != BindingSlot::None) {
            if (pl) {
                // Fall-through intencional: cancela el binding Y deja pasar
                // el input al juego para que el jugador no pierda el salto
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
        // Permite input anticipado al despausar (la tecla llega como Space a GD)
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

// ── Hook de mouse via CCEGLView ───────────────────────────────────────────
// Click derecho y rueda no pasan por CCKeyboardDispatcher en GD,
// se interceptan en CCEGLView antes de que el motor los procese
class $modify(CCEGLView) {
    void onGLFWMouseCallBack(GLFWwindow* window, int button, int action, int mods) {
        if (!s.rapidCheckpoints) {
            CCEGLView::onGLFWMouseCallBack(window, button, action, mods);
            return;
        }

        auto* pl = PlayLayer::get();
        if (!pl || !pl->m_isPracticeMode || pl->m_isPaused) {
            CCEGLView::onGLFWMouseCallBack(window, button, action, mods);
            return;
        }

        bool down = (action == GLFW_PRESS);

        // GLFW_MOUSE_BUTTON_RIGHT = 1, GLFW_MOUSE_BUTTON_MIDDLE = 2
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (down) pl->markCheckpoint();
            return; // Consumimos el evento
        }

        if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
            if (down) pl->removeLastCheckpoint();
            return; // Consumimos el evento
        }

        CCEGLView::onGLFWMouseCallBack(window, button, action, mods);
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

    auto setupJumpSetting = [](std::string setting, BindingSlot slot, bool& enabledState) {
        return listenForSettingChanges<bool>(setting, [=, &enabledState](bool enabled) {
            enabledState = enabled;
            if (enabled) {
                s.bindingSlot = slot;
                geode::Notification::create(
                    "Presiona una tecla para vincular (ESC cancela)",
                    geode::NotificationIcon::Info
                )->show();
            } else {
                cancelBinding();
                clearKey(slot);
            }
        });
    };

    s.j1SettingHandle = setupJumpSetting("enable-jump-1", BindingSlot::Jump1, s.jump1Enabled);
    s.j2SettingHandle = setupJumpSetting("enable-jump-2", BindingSlot::Jump2, s.jump2Enabled);
    s.rcSettingHandle = listenForSettingChanges<bool>("rapid-checkpoints", [](bool enabled) {
        s.rapidCheckpoints = enabled;
    });
}
