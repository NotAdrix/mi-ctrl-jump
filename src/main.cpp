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
        // Si detectamos Control, le cambiamos el nombre a Space
        // pero mantenemos el 'time' original del sistema.
        if (key == enumKeyCodes::KEY_Control) {
            key = enumKeyCodes::KEY_Space;
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, time);
    }
};

/**
 * 2. MOUSE (Click Derecho -> Z, Ruedita -> X)
 * Usamos el sistema de Eventos de Geode. 
 * Es 100% inmune a errores de "member not found" y es compatible con CBF.
 */
$execute {
    // Escuchamos los clics del mouse de forma global
    new EventListener<AttributeSetFilter>(+[](MouseButtonEvent* event) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return ListenerResult::Propagate;

        // Click Derecho (Right) -> Tecla Z
        if (event->m_button == MouseButton::Right) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, event->m_down, false, 0.0);
            return ListenerResult::Stop; // Detenemos el clic original
        }

        // Click Ruedita (Middle) -> Tecla X
        if (event->m_button == MouseButton::Middle) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, event->m_down, false, 0.0);
            return ListenerResult::Stop;
        }

        return ListenerResult::Propagate;
    }, MouseButtonFilter());
}