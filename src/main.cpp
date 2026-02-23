#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCScheduler.hpp>

#ifdef GEODE_IS_WINDOWS
#include <Windows.h>
#endif

using namespace geode::prelude;

/**
 * 1. REASIGNACIÓN DE TECLADO (Ctrl -> Space)
 * (Esta parte funcionó perfectamente en tu última compilación)
 */
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double time) {
        if (key == enumKeyCodes::KEY_Control) {
            key = enumKeyCodes::KEY_Space;
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, time);
    }
};

/**
 * 2. REASIGNACIÓN DE RATÓN (Click Derecho -> Z, Ruedita -> X)
 * Leemos el ratón directamente desde el sistema, ignorando los errores de Cocos2d-x.
 */
#ifdef GEODE_IS_WINDOWS

// Variables para recordar si el botón ya estaba presionado y no mandar spam
static bool g_rightDown = false;
static bool g_middleDown = false;

class $modify(CCScheduler) {
    void update(float dt) {
        // Llamamos al reloj original para no romper el juego
        CCScheduler::update(dt);
        
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return; // Por seguridad
        
        // --- Interceptar Click Derecho (VK_RBUTTON) ---
        bool currentRight = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        if (currentRight != g_rightDown) {
            g_rightDown = currentRight;
            // Enviamos la tecla Z al juego
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, currentRight, false, 0.0);
        }
        
        // --- Interceptar Click de Ruedita (VK_MBUTTON) ---
        bool currentMiddle = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
        if (currentMiddle != g_middleDown) {
            g_middleDown = currentMiddle;
            // Enviamos la tecla X al juego
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, currentMiddle, false, 0.0);
        }
    }
};
#endif