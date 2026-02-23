#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCMouseDispatcher.hpp>

using namespace geode::prelude;

/**
 * REASIGNACIÓN DE TECLADO
 * Corregido para GD 2.2081: Firma de 4 argumentos y retorno bool.
 */
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double time) {
        // Sustitución de identidad: Ctrl ahora es Space.
        if (key == enumKeyCodes::KEY_Control) {
            key = enumKeyCodes::KEY_Space;
        }
        // Pasamos los 4 parámetros (key, down, repeat, time) a la original.
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, time);
    }
};

/**
 * REASIGNACIÓN DE MOUSE
 * Corregido para GD 2.2081: La función se llama 'mouseButton'.
 * Usamos 'int' para el botón para evitar errores de tipos desconocidos.
 */
class $modify(CCMouseDispatcher) {
    void mouseButton(int button, bool down, bool doubleClick) {
        auto kbd = CCKeyboardDispatcher::get();

        // En Cocos2d-x (RobTop): 1 es Click Derecho, 2 es Click Ruedita.
        if (button == 1) { // Click Derecho
            // Mandamos la señal al teclado (Tecla Z, presionado, no repetir, tiempo 0.0)
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, down, false, 0.0);
            return; // Bloqueamos el click original
        }

        if (button == 2) { // Click Ruedita
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, down, false, 0.0);
            return;
        }

        // El resto (click izquierdo, etc) sigue normal
        CCMouseDispatcher::mouseButton(button, down, doubleClick);
    }
};