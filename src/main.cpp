#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

/**
 * 1. TECLADO (Ctrl -> Space)
 * COMPATIBILIDAD CBF: Al interceptar el mensaje y reenviar el parámetro 'time',
 * el motor de físicas recibe el salto con la precisión original del hardware (microsegundos).
 */
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double time) {
        if (key == enumKeyCodes::KEY_Control) {
            // Re-despachamos como 'Espacio' usando el TIEMPO EXACTO del sistema (CBF)
            CCKeyboardDispatcher::get()->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, down, repeat, time);
            
            // BLOQUEO DE IDENTIDAD: Retornar 'true' hace que el juego ignore el Ctrl original.
            return true; 
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, time);
    }
};

/**
 * 2. MOUSE (Click Derecho -> Z, Ruedita -> X)
 * Usamos el sistema de Eventos de Geode. 
 * Es inmune a los cambios de RobTop y preserva la cadena de eventos para el CBF.
 */
$execute {
    // Usamos el namespace explícito para que el compilador no se pierda
    new EventListener<geode::MouseButtonEventFilter>(+[](geode::MouseButtonEvent* event) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return ListenerResult::Propagate;

        // Click Derecho -> Tecla Z
        if (event->m_button == geode::MouseButton::Right) {
            // Mandamos Z (el tiempo 0.0 es suficiente para acciones no-físicas)
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, event->m_down, false, 0.0);
            return ListenerResult::Stop; // Bloqueamos el click derecho original
        }

        // Click Ruedita (Central) -> Tecla X
        if (event->m_button == geode::MouseButton::Middle) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, event->m_down, false, 0.0);
            return ListenerResult::Stop;
        }

        return ListenerResult::Propagate;
    });
}