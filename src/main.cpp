#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCMouseDispatcher.hpp>

using namespace geode::prelude;

/**
 * REASIGNACIÓN DE TECLADO
 * Actualizado para GD 2.2081 (4 argumentos)
 */
class $modify(CCKeyboardDispatcher) {
    // Se añade el cuarto parámetro 'double time'
    void dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat, double time) {
        if (key == enumKeyCodes::KEY_Control) {
            key = enumKeyCodes::KEY_Space;
        }
        // Pasamos los 4 argumentos a la función original
        CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, time);
    }
};

/**
 * REASIGNACIÓN DE MOUSE
 * Actualizado con namespaces de Geode
 */
class $modify(CCMouseDispatcher) {
    // Usamos geode::MouseEvent y geode::MouseButton explícitamente
    bool dispatchMouseEvent(geode::MouseEvent event, geode::MouseButton button, float x, float y) {
        auto kbd = CCKeyboardDispatcher::get();

        if (button == geode::MouseButton::Right) {
            if (event == geode::MouseEvent::Down || event == geode::MouseEvent::Up) {
                bool isDown = (event == geode::MouseEvent::Down);
                // Enviamos 0 como timestamp para la tecla Z
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, isDown, false, 0);
            }
            return true;
        }

        if (button == geode::MouseButton::Middle) {
            if (event == geode::MouseEvent::Down || event == geode::MouseEvent::Up) {
                bool isDown = (event == geode::MouseEvent::Down);
                // Enviamos 0 como timestamp para la tecla X
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, isDown, false, 0);
            }
            return true;
        }

        return CCMouseDispatcher::dispatchMouseEvent(event, button, x, y);
    }
};