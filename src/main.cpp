#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCScheduler.hpp>

#ifdef GEODE_IS_WINDOWS
#include <Windows.h>
#endif

using namespace geode::prelude;

// Variables para el mouse (Z y X)
static bool g_rightDownState = false;
static bool g_middleDownState = false;

/**
 * 1. EL SALTO PROFESIONAL (Ctrl -> Space)
 * Esta parte usa la "Vía Ideal". Al interceptar el mensaje y mantener
 * el parámetro 'time', conservamos la precisión del Click Between Frames (CBF).
 */
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double time) {
        if (key == enumKeyCodes::KEY_Control) {
            key = enumKeyCodes::KEY_Space;
        }
        // Pasamos el 'time' original del sistema operativo. ¡CBF activado!
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, time);
    }
};

/**
 * 2. UTILIDADES DEL MOUSE (Click Derecho -> Z, Ruedita -> X)
 * Usamos el reloj del juego para evitar errores de compilación con el mouse.
 * Como es para Checkpoints/Práctica, la precisión de los FPS es perfecta.
 */
class $modify(CCScheduler) {
    void update(float dt) {
        CCScheduler::update(dt);

#ifdef GEODE_IS_WINDOWS
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return;

        // Click Derecho -> Tecla Z
        bool currentRight = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        if (currentRight != g_rightDownState) {
            g_rightDownState = currentRight;
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, g_rightDownState, false, 0.0);
        }

        // Click Ruedita -> Tecla X
        bool currentMiddle = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
        if (currentMiddle != g_middleDownState) {
            g_middleDownState = currentMiddle;
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, g_middleDownState, false, 0.0);
        }
#endif
    }
};