#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCMouseDispatcher.hpp>

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

        std::optional<geode::comm::ListenerHandle> j1SettingHandle;
        std::optional<geode::comm::ListenerHandle> j2SettingHandle;
        std::optional<geode::comm::ListenerHandle> rcSettingHandle;
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
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool isRepeat) {
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

        // ── Fuera de nivel o en pausa: reset del contador y propagación ───
        if (!pl || pl->m_isPaused) {
            s.jumpKeysHeld = 0;
            return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, isRepeat);
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
                    CCKeyboardDispatcher::dispatchKeyboardMSG(JUMP_KEY, true, false);
                s.jumpKeysHeld++;
            } else if (down && isRepeat) {
                if (s.jumpKeysHeld == 1)
                    CCKeyboardDispatcher::dispatchKeyboardMSG(JUMP_KEY, true, true);
            } else {
                // Decrementar sin bajar de cero, sin depender de <algorithm>
                if (s.jumpKeysHeld > 0) s.jumpKeysHeld--;
                if (s.jumpKeysHeld == 0)
                    CCKeyboardDispatcher::dispatchKeyboardMSG(JUMP_KEY, false, false);
            }
            return true;
        }

        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, isRepeat);
    }
};

// ── Hook de mouse ─────────────────────────────────────────────────────────
class $modify(CCMouseDispatcher) {
    bool dispatchMouseMSG(cocos2d::CCMouseButton btn, bool down) {
        if (!s.rapidCheckpoints)
            return CCMouseDispatcher::dispatchMouseMSG(btn, down);

        auto* pl = PlayLayer::get();
        if (!pl || !pl->m_isPracticeMode || pl->m_isPaused)
            return CCMouseDispatcher::dispatchMouseMSG(btn, down);

        if (btn == cocos2d::CCMouseButton::ButtonRight) {
            if (down) pl->markCheckpoint();
            return true;
        }

        if (btn == cocos2d::CCMouseButton::ButtonMiddle) {
            if (down) pl->removeLastCheckpoint();
            return true;
        }

        return CCMouseDispatcher::dispatchMouseMSG(btn, down);
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

    // DRY: lambda con std::string para copia profunda segura del setting key
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
