#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

using namespace geode::prelude;

/**
 * 1. TECLADO (Ctrl -> Space)
 * Hookeamos el despachador para interceptar el mensaje "al vuelo".
 * El parámetro 'time' es el que lleva la precisión del CBF.
 */
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double time) {
        // Capturamos Control y lo disfrazamos de Space.
        // Se mantiene el 'time' (sub-frame timing) para compatibilidad con CBF.
        if (key == enumKeyCodes::KEY_Control) {
            key = enumKeyCodes::KEY_Space;
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, time);
    }
};

/**
 * 2. MOUSE (Click Derecho -> Z, Ruedita -> X)
 * Usamos el sistema de Eventos de Geode con el filtro 'MouseButtonFilter'.
 * Esto es inmune a los cambios de RobTop en Cocos2d-x.
 */
$execute {
    // Registramos un "escuchador" para eventos de botones del mouse
    new EventListener<MouseButtonFilter>(+[](MouseButtonEvent* event) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return ListenerResult::Propagate;

        // Click Derecho -> Tecla Z
        if (event->m_button == MouseButton::Right) {
            // Mandamos Z con el estado de pulsación (m_down) del mouse
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, event->m_down, false, 0.0);
            return ListenerResult::Stop; // Evitamos que el juego use el click derecho
        }

        // Click Ruedita (Middle) -> Tecla X
        if (event->m_button == MouseButton::Middle) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, event->m_down, false, 0.0);
            return ListenerResult::Stop;
        }

        return ListenerResult::Propagate;
    });
}