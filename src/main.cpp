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
 * Corregido: La función es 'dispatchMouseButton'. 
 * Usamos MouseButton (sin geode::) ya que el prelude lo incluye.
 */
class $modify(CCMouseDispatcher) {
    void dispatchMouseButton(MouseButton button, bool down, float x, float y) {
        auto kbd = CCKeyboardDispatcher::get();

        // Click Derecho -> Tecla Z
        if (button == MouseButton::Right) {
            // Mandamos señal al teclado (Z, estado de pulsación, no repetir, tiempo 0)
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, down, false, 0.0);
            return; // Consumimos el evento para que no haga nada más en el juego
        }

        // Click Ruedita -> Tecla X
        if (button == MouseButton::Middle) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, down, false, 0.0);
            return;
        }

        // El resto (click izquierdo, etc) sigue su flujo normal
        CCMouseDispatcher::dispatchMouseButton(button, down, x, y);
    }
};