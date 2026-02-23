#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

/**
 * 1. TECLADO (Ctrl -> Space)
 * COMPATIBILIDAD CBF: Al interceptar el mensaje y reenviar el parámetro 'time',
 * el motor de físicas recibe el salto con la precisión original del hardware.
 */
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double time) {
        if (key == enumKeyCodes::KEY_Control) {
            // Enviamos un 'Espacio' con el TIEMPO EXACTO del sistema (CBF)
            CCKeyboardDispatcher::dispatchKeyboardMSG(enumKeyCodes::KEY_Space, down, repeat, time);
            
            // IMPORTANTE: Retornar 'true' bloquea el Ctrl original.
            // Esto evita que el juego detecte que pulsaste Ctrl.
            return true; 
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, time);
    }
};

/**
 * 2. MOUSE (Click Derecho -> Z, Ruedita -> X)
 * Usamos el sistema de Eventos de Geode v5.
 * Es inmune a los cambios de RobTop en Cocos2d-x y mantiene el CBF.
 */
$execute {
    // Escuchamos clics de mouse usando el filtro correcto de Geode v5
    new EventListener<MouseButtonEvent>(+[](MouseButtonEvent* event) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return ListenerResult::Propagate;

        // Click Derecho -> Tecla Z
        if (event->m_button == MouseButton::Right) {
            // El tiempo 0.0 es suficiente aquí ya que Z/X no son de gameplay crítico
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, event->m_down, false, 0.0);
            return ListenerResult::Stop; 
        }

        // Click Ruedita -> Tecla X
        if (event->m_button == MouseButton::Middle) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, event->m_down, false, 0.0);
            return ListenerResult::Stop;
        }

        return ListenerResult::Propagate;
    }, MouseButtonFilter()); // Filtro constructor para Geode v5
}