#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCMouseDispatcher.hpp>

using namespace geode::prelude;

/**
 * REASIGNACIÓN GLOBAL DE TECLADO
 * Interceptamos el mensaje antes de que llegue a la lógica del juego.
 */
class $modify(CCKeyboardDispatcher) {
    void dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat) {
        // Si el sistema detecta Control, lo forzamos a ser Space.
        // KEY_Control es el identificador genérico en Geode.
        if (key == enumKeyCodes::KEY_Control) {
            key = enumKeyCodes::KEY_Space;
        }
        
        // Pasamos la tecla (ahora Space) a la función original.
        CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat);
    }
};

/**
 * REASIGNACIÓN DE MOUSE
 * Click Derecho -> Z
 * Click Ruedita -> X
 */
class $modify(CCMouseDispatcher) {
    bool dispatchMouseEvent(MouseEvent event, MouseButton button, float x, float y) {
        auto kbd = CCKeyboardDispatcher::get();

        // 1. Click Derecho actúa como Z
        if (button == MouseButton::Right) {
            if (event == MouseEvent::Down || event == MouseEvent::Up) {
                bool isDown = (event == MouseEvent::Down);
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, isDown, false);
            }
            return true; // Bloqueamos el comportamiento original del click derecho
        }

        // 2. Click Ruedita actúa como X
        if (button == MouseButton::Middle) {
            if (event == MouseEvent::Down || event == MouseEvent::Up) {
                bool isDown = (event == MouseEvent::Down);
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, isDown, false);
            }
            return true; // Bloqueamos el comportamiento original del click central
        }

        // Cualquier otro evento (click izquierdo, movimiento) sigue normal.
        return CCMouseDispatcher::dispatchMouseEvent(event, button, x, y);
    }
};