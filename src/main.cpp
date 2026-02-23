#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCMouseDispatcher.hpp>

using namespace geode::prelude;

/**
 * 1. TECLADO (Ctrl -> Space)
 * Esta es la parte vital para el CBF. 
 * Al usar el 'double time', pasamos la precisión de microsegundos del hardware.
 */
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double time) {
        if (key == enumKeyCodes::KEY_Control) {
            // Transformamos a Space pero pasamos el 'time' ORIGINAL del sistema.
            // Esto es lo que permite al CBF funcionar con el Ctrl.
            return CCKeyboardDispatcher::dispatchKeyboardMSG(enumKeyCodes::KEY_Space, down, repeat, time);
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, time);
    }
};

/**
 * 2. MOUSE (Click Derecho -> Z, Ruedita -> X)
 * Usamos CCMouseDispatcher con la firma de 2.2081.
 * Para evitar errores de tipo, usamos 'int' para el botón.
 */
class $modify(CCMouseDispatcher) {
    void dispatchMouseButton(int button, bool down) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) {
            CCMouseDispatcher::dispatchMouseButton(button, down);
            return;
        }

        // 1 = Right Click, 2 = Middle Click (Ruedita)
        if (button == 1) {
            // Mandamos Z. El 0.0 de tiempo hará que el motor use el tiempo actual.
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, down, false, 0.0);
            return; // Bloqueamos el click original
        }

        if (button == 2) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, down, false, 0.0);
            return;
        }

        CCMouseDispatcher::dispatchMouseButton(button, down);
    }
};