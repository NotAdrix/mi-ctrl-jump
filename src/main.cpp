#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCMouseDispatcher.hpp>

using namespace geode::prelude;

/**
 * 1. REASIGNACIÓN DE TECLADO (Ctrl -> Space)
 * Compatible con CBF: Al reasignar la identidad de la tecla y mantener 'time',
 * el motor de físicas recibe el salto con la precisión original del hardware.
 */
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double time) {
        // En Geode v5 para 2.2081, KEY_Control es el identificador estándar.
        if (key == enumKeyCodes::KEY_Control) {
            key = enumKeyCodes::KEY_Space;
        }
        // Retornamos la función original con la tecla cambiada pero el TIEMPO intacto.
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, time);
    }
};

/**
 * 2. REASIGNACIÓN DE MOUSE (Click Derecho -> Z, Ruedita -> X)
 * En la versión 2.2081, la función se llama 'dispatchMouseButton'.
 * Usamos el namespace explícito geode::MouseButton para evitar errores.
 */
class $modify(CCMouseDispatcher) {
    void dispatchMouseButton(geode::MouseButton button, bool down) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) {
            CCMouseDispatcher::dispatchMouseButton(button, down);
            return;
        }

        // Click Derecho -> Tecla Z
        if (button == geode::MouseButton::Right) {
            // Mandamos Z con tiempo 0.0 (el juego le asignará el tiempo actual de proceso)
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, down, false, 0.0);
            return; // Bloqueamos el click original
        }

        // Click Ruedita -> Tecla X
        if (button == geode::MouseButton::Middle) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, down, false, 0.0);
            return;
        }

        // El resto (click izquierdo, etc.) sigue normal
        CCMouseDispatcher::dispatchMouseButton(button, down);
    }
};