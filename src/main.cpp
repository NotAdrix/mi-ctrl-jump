#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCEGLView.hpp>

using namespace geode::prelude;

/**
 * 1. TECLADO (Ctrl -> Space)
 * Usamos el despachador de mensajes para mantener la precisión del 'time'.
 * Esto permite que el salto sea "Frame Perfect" (compatible con CBF).
 */
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double time) {
        // Capturamos cualquier tipo de Control (Izquierdo, Derecho o Genérico)
        if (key == enumKeyCodes::KEY_Control || 
            key == enumKeyCodes::KEY_LControl || 
            key == enumKeyCodes::KEY_RControl) {
            
            // Lo transformamos en un Espacio manteniendo el 'time' exacto del hardware
            return CCKeyboardDispatcher::dispatchKeyboardMSG(enumKeyCodes::KEY_Space, down, repeat, time);
        }
        
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, time);
    }
};

/**
 * 2. MOUSE (Click Derecho -> Z, Ruedita -> X)
 * Usamos CCEGLView porque es el punto de entrada de la ventana. 
 * Esto evita los errores de "member not found" de CCMouseDispatcher.
 */
class $modify(CCEGLView) {
    void onMouseButton(int button, int action, int mods) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) {
            CCEGLView::onMouseButton(button, action, mods);
            return;
        }

        // action 1 = Presionado, action 0 = Soltado
        bool isDown = (action == 1);

        // Click Derecho (ID 1 en GLFW/Windows) -> Tecla Z
        if (button == 1) { 
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, isDown, false, 0.0);
            return; // Bloqueamos el click original para que no interfiera
        }

        // Click Ruedita (ID 2 en GLFW/Windows) -> Tecla X
        if (button == 2) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, isDown, false, 0.0);
            return;
        }

        // Dejamos pasar el click izquierdo (ID 0) y otros normalmente
        CCEGLView::onMouseButton(button, action, mods);
    }
};