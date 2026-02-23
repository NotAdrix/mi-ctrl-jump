#include <Geode/Geode.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

#ifdef GEODE_IS_WINDOWS
#include <Windows.h>
#endif

using namespace geode::prelude;

// Variables de estado para evitar spam de teclas
static bool g_ctrlState = false;
static bool g_rightState = false;
static bool g_middleState = false;

/**
 * ESTA ES LA SOLUCIÓN TÉCNICA DEFINITIVA PARA 2.2081
 * Al no usar hooks en clases inestables como CCMouseDispatcher,
 * garantizamos que compile en GitHub Actions a la primera.
 */
class $modify(CCScheduler) {
    void update(float dt) {
        CCScheduler::update(dt);

#ifdef GEODE_IS_WINDOWS
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return;

        // 1. SOLUCIÓN PARA CTRL (Cualquiera de los dos) -> SALTO
        // VK_CONTROL detecta tanto el Ctrl izquierdo como el derecho.
        bool currentCtrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
        if (currentCtrl != g_ctrlState) {
            g_ctrlState = currentCtrl;
            // Inyectamos el salto como un 'Espacio' real
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, g_ctrlState, false, 0.0);
        }

        // 2. CLICK DERECHO -> TECLA Z (Checkpoints)
        bool currentRight = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        if (currentRight != g_rightState) {
            g_rightState = currentRight;
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, g_rightState, false, 0.0);
        }

        // 3. RUEDITA (Click central) -> TECLA X (Borrar Checkpoints)
        bool currentMiddle = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
        if (currentMiddle != g_middleState) {
            g_middleState = currentMiddle;
            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, g_middleState, false, 0.0);
        }
#endif
    }
};