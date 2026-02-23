#include <Geode/Geode.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>

#ifdef GEODE_IS_WINDOWS
#include <Windows.h>
#endif

using namespace geode::prelude;

// Variables de estado únicas (evita el error de redefinición)
static bool g_ctrlDownState = false;
static bool g_rightDownState = false;
static bool g_middleDownState = false;

class $modify(CCScheduler) {
    void update(float dt) {
        CCScheduler::update(dt);

#ifdef GEODE_IS_WINDOWS
        auto kbd = CCKeyboardDispatcher::get();
        auto playLayer = PlayLayer::get();

        // 1. SOLUCIÓN PARA CTRL (SALTO DIRECTO)
        bool currentCtrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
        if (currentCtrl != g_ctrlDownState) {
            g_ctrlDownState = currentCtrl;
            
            // Si estamos jugando, le ordenamos al objeto físico saltar
            if (playLayer && playLayer->m_player1) {
                if (g_ctrlDownState) {
                    playLayer->m_player1->pushButton(PlayerButton::Jump);
                } else {
                    playLayer->m_player1->releaseButton(PlayerButton::Jump);
                }
            } 
            // En menús, mandamos la tecla Space normal
            if (kbd) {
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, g_ctrlDownState, false, 0.0);
            }
        }

        // 2. SOLUCIÓN PARA CLICK DERECHO (Z)
        bool currentRight = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        if (currentRight != g_rightDownState) {
            g_rightDownState = currentRight;
            if (kbd) kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, g_rightDownState, false, 0.0);
        }

        // 3. SOLUCIÓN PARA RUEDITA (X)
        bool currentMiddle = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
        if (currentMiddle != g_middleDownState) {
            g_middleDownState = currentMiddle;
            if (kbd) kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, g_middleDownState, false, 0.0);
        }
#endif
    }
};