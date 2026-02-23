#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

using namespace geode::prelude;

/**
 * ESTA ES LA SOLUCIÓN CBF-COMPATIBLE.
 * Interceptamos el evento en el aire y le cambiamos el nombre a la tecla 
 * ANTES de que el motor la procese, conservando el 'time' (sub-frame timing).
 */
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double time) {
        // En Geode v5/GD 2.2081, KEY_Control atrapa cualquier Ctrl físico.
        if (key == enumKeyCodes::KEY_Control) {
            // Cambiamos 'key' a Space, pero mantenemos el 'time' intacto.
            // Esto permite que el CBF lea el espacio con la precisión del Ctrl.
            key = enumKeyCodes::KEY_Space;
        }

        // Si es el Click Derecho (en algunos teclados se mapea aquí) o si quieres
        // usar el despachador para la Z y X:
        // (Nota: Para máxima precisión en mouse, se suele usar otro hook, 
        // pero esto es 100% seguro para el teclado).

        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, time);
    }
};

/**
 * Para el mouse, si CCMouseDispatcher fallaba, usaremos un hook de bajo nivel
 * en el protocolo de la ventana para mantener la compatibilidad y el CBF.
 */
#include <Geode/modify/CCEGLViewProtocol.hpp>

class $modify(CCEGLViewProtocol) {
    void onMouseButton(int button, int action, int mods) {
        // action 1 = presionar, 0 = soltar
        bool isDown = (action == 1);
        auto kbd = CCKeyboardDispatcher::get();

        // 1 = Click Derecho, 2 = Ruedita
        if (button == 1) {
            if (kbd) kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, isDown, false, 0.0);
            return;
        }
        if (button == 2) {
            if (kbd) kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, isDown, false, 0.0);
            return;
        }

        CCEGLViewProtocol::onMouseButton(button, action, mods);
    }
};