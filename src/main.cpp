#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCMouseDispatcher.hpp>

using namespace geode::prelude;

/**
 * PARTE 1: REASIGNACIÓN DE TECLADO
 * Físico Ctrl -> El juego lee Space
 */
class $modify(CCKeyboardDispatcher) {
    void dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat) {
        if (key == enumKeyCodes::KEY_Control) {
            key = enumKeyCodes::KEY_Space;
        }
        CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat);
    }
};

/**
 * PARTE 2: REASIGNACIÓN DE MOUSE
 * Click Derecho -> El juego lee Z
 * Botón Ruedita -> El juego lee X
 */
class $modify(CCMouseDispatcher) {
    bool dispatchMouseEvent(MouseEvent event, MouseButton button, float x, float y) {
        auto kbd = CCKeyboardDispatcher::get();

        // Manejar Click Derecho -> Tecla Z
        if (button == MouseButton::Right) {
            // Detectamos si se está presionando (Down) o soltando (Up)
            bool isDown = (event == MouseEvent::Down);
            
            // Enviamos la señal de la tecla Z al dispatcher de teclado
            if (event == MouseEvent::Down || event == MouseEvent::Up) {
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, isDown, false);
            }
            // Retornamos true para "consumir" el evento y que el juego 
            // no intente usar el click derecho para otra cosa (como borrar objetos en el editor)
            return true; 
        }

        // Manejar Botón de la Ruedita (Middle) -> Tecla X
        if (button == MouseButton::Middle) {
            bool isDown = (event == MouseEvent::Down);
            
            if (event == MouseEvent::Down || event == MouseEvent::Up) {
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, isDown, false);
            }
            return true;
        }

        // Si es cualquier otro botón (Click izquierdo, etc), dejamos que el juego actúe normal
        return CCMouseDispatcher::dispatchMouseEvent(event, button, x, y);
    }
};