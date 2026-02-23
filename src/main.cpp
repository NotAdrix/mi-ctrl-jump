#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCEGLViewProtocol.hpp>

using namespace geode::prelude;

/**
 * 1. TECLADO (Ctrl -> Space)
 * Usamos KEY_Control (el único que existe en el enum de v5).
 * Se añaden los 4 argumentos y el retorno bool para cumplir con GD 2.2081.
 */
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double time) {
        // En Geode v5, KEY_Control engloba ambos controles físicos.
        if (key == enumKeyCodes::KEY_Control) {
            key = enumKeyCodes::KEY_Space;
        }
        
        // Retornamos la función original con los 4 parámetros exactos.
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, time);
    }
};

/**
 * 2. MOUSE (Click Derecho -> Z, Ruedita -> X)
 * Hookeamos CCEGLViewProtocol, que es infalible en Geode v5.
 * Mantiene la precisión de alta velocidad (ideal para CBF).
 */
class $modify(CCEGLViewProtocol) {
    void onMouseButton(int button, int action, int mods) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) {
            CCEGLViewProtocol::onMouseButton(button, action, mods);
            return;
        }

        // action 1 = Presionado, action 0 = Soltado
        bool isDown = (action == 1);

        // Click Derecho (ID 1 en el protocolo de ventana) -> Tecla Z
        if (button == 1) { 
            // Mandamos Z con timestamp 0.0 (el motor le asignará el tiempo actual)
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, isDown, false, 0.0);
            return; 
        }

        // Click Ruedita (ID 2 en el protocolo de ventana) -> Tecla X
        if (button == 2) {
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, isDown, false, 0.0);
            return;
        }

        // Todo lo demás sigue su curso
        CCEGLViewProtocol::onMouseButton(button, action, mods);
    }
};