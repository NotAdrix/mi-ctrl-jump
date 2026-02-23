#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

/**
 * 1. TECLADO (Ctrl -> Space)
 * Esta parte es vital para el CBF. Al usar 'double time', enviamos
 * la precisión de microsegundos del hardware al motor de físicas.
 */
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double time) {
        if (key == enumKeyCodes::KEY_Control) {
            // Enviamos un 'Espacio' con el tiempo exacto del sistema (CBF)
            CCKeyboardDispatcher::dispatchKeyboardMSG(enumKeyCodes::KEY_Space, down, repeat, time);
            
            // Bloqueamos el Ctrl original para que el juego no lo detecte como Ctrl
            return true; 
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, time);
    }
};

/**
 * 2. MOUSE (Click Derecho -> Z, Ruedita -> X)
 * Usamos el sistema de Eventos de Geode v5. 
 * Es inmune a los errores de nombres de Cocos2d-x y mantiene la precisión.
 */
$execute {
    // Registramos un escuchador usando la sintaxis exacta de Geode v5
    new EventListener<geode::MouseButtonEventFilter>(+[](geode::MouseButtonEvent* event) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return ListenerResult::Propagate;

        // Click Derecho -> Tecla Z
        if (event->m_button == geode::MouseButton::Right) {
            // El tiempo 0.0 le dice al juego que use el tiempo actual de proceso
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, event->m_down, false, 0.0);
            return ListenerResult::Stop; // Bloqueamos el click original
        }

        // Click Ruedita (Botón Central) -> Tecla X
        if (event->m_button == geode::MouseButton::Middle) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, event->m_down, false, 0.0);
            return ListenerResult::Stop;
        }

        return ListenerResult::Propagate;
    });
}