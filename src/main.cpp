#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCMouseDispatcher.pch.hpp> // Header específico para v5

using namespace geode::prelude;

/**
 * REASIGNACIÓN DE TECLADO
 * Corregido: Firma de 4 argumentos y retorno bool (según tu log de error).
 */
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double time) {
        // Sustitución de identidad: Ctrl ahora es Space para todo el motor.
        if (key == enumKeyCodes::KEY_Control) {
            key = enumKeyCodes::KEY_Space;
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, time);
    }
};

/**
 * REASIGNACIÓN DE MOUSE
 * Corregido: En la v5/2.2081 la función es 'dispatchMouseButton'.
 */
class $modify(CCMouseDispatcher) {
    void dispatchMouseButton(MouseButton button, bool down, float x, float y) {
        auto kbd = CCKeyboardDispatcher::get();

        // Click Derecho -> Tecla Z
        if (button == MouseButton::Right) {
            // Mandamos la señal al teclado (con el 4to argumento 0.0)
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, down, false, 0.0);
            return; // Bloqueamos el comportamiento original del mouse
        }

        // Click Ruedita -> Tecla X
        if (button == MouseButton::Middle) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, down, false, 0.0);
            return;
        }

        // El resto (click izquierdo, etc) se procesa normal
        CCMouseDispatcher::dispatchMouseButton(button, down, x, y);
    }
};