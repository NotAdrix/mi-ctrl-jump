#include <Geode/Geode.hpp>

using namespace geode::prelude;

$execute {
    // ── TECLADO: L-Ctrl -> Espacio (Automático y Enmascarado) ──────────────
    geode::KeyboardInputEvent().listen([](geode::KeyboardInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return ListenerResult::Propagate;

        // Si la tecla es CUALQUIER variante de Control
        if (data.key == geode::Key::LeftControl || 
            data.key == geode::Key::RightControl || 
            data.key == geode::Key::Control) {
            
            bool down = (data.action != geode::KeyboardInputData::Action::Release);
            bool repeat = (data.action == geode::KeyboardInputData::Action::Repeat);
            
            // ENMASCARAMIENTO: Guardamos el estado original y apagamos el "Control"
            // Esto evita que el motor lea "Ctrl + Espacio" y haga cosas raras
            bool oldCtrl = kbd->m_bControlPressed;
            kbd->m_bControlPressed = false;

            // Disparamos el Espacio puro
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, down, repeat, data.timestamp);

            // Restauramos el estado internamente por seguridad
            kbd->m_bControlPressed = oldCtrl;

            // Frenamos en seco la señal original de Control
            return ListenerResult::Stop;
        }

        // Dejamos pasar todas las demás teclas con normalidad (¡para que puedas jugar!)
        return ListenerResult::Propagate;
    }).leak();


    // ── MOUSE: Click Derecho -> Z | Rueda -> X (Automático) ────────────────
    geode::MouseInputEvent().listen([](geode::MouseInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return ListenerResult::Propagate;

        bool down = (data.action == geode::MouseInputData::Action::Press);

        // Click Derecho = Poner Checkpoint (Z)
        if (data.button == geode::MouseInputData::Button::Right) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, down, false, data.timestamp);
            return ListenerResult::Stop;
        }
        
        // Click de Rueda = Borrar Checkpoint (X)
        if (data.button == geode::MouseInputData::Button::Middle) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, down, false, data.timestamp);
            return ListenerResult::Stop;
        }

        return ListenerResult::Propagate;
    }).leak();
}
