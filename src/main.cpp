#include <Geode/Geode.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/PlayerObject.hpp>

#ifdef GEODE_IS_WINDOWS
#include <Windows.h>
#endif

using namespace geode::prelude;

// Variables de estado para evitar spam de teclas
static bool g_ctrlDown = false;
static bool g_rightDown = false;
static bool g_middleDown = false;

/**
 * REASIGNACIÓN GLOBAL Y DE GAMEPLAY
 * Esta versión lee el hardware (infalible) y ejecuta el salto directamente
 * en el objeto del jugador (compatibilidad total con 2.2 y CBF).
 */
class $modify(CCScheduler) {
    void update(float dt) {
        CCScheduler::update(dt);

#ifdef GEODE_IS_WINDOWS
        // Obtenemos el estado de la partida actual
        auto playLayer = PlayLayer::get();
        auto kbd = CCKeyboardDispatcher::get();

        // 1. DETECCIÓN DE CONTROL (Salto / Espacio)
        bool currentCtrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
        if (currentCtrl != g_ctrlDown) {
            g_ctrlDown = currentCtrl;
            
            if (playLayer && playLayer->m_player1) {
                // Si estamos jugando, forzamos el salto directo (Precisión Máxima)
                if (g_ctrlDown) playLayer->m_player1->pushButton(PlayerButton::Jump);
                else playLayer->m_player1->releaseButton(PlayerButton::Jump);
            } else if (kbd) {
                // Si estamos en un menú, mandamos un Espacio normal
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, g_ctrlDown, false, 0.0);
            }
        }

        // 2. DETECCIÓN DE CLICK DERECHO (Tecla Z)
        bool currentRight = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        if (currentRight != g_rightDown) {
            g_rightDown = currentRight;
            if (kbd) kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, g_rightDown, false, 0.0);
        }

        // 3. DETECCIÓN DE RUEDITA (Tecla X)
        bool currentMiddle = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
        if (currentMiddle != g_middleDown) {
            g_middleDown = currentMiddle;
            if (kbd) kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, g_middleDown, false, 0.0);
        }
#endif
    }
};#include <Geode/Geode.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/PlayerObject.hpp>

#ifdef GEODE_IS_WINDOWS
#include <Windows.h>
#endif

using namespace geode::prelude;

// Variables de estado para evitar spam de teclas
static bool g_ctrlDown = false;
static bool g_rightDown = false;
static bool g_middleDown = false;

/**
 * REASIGNACIÓN GLOBAL Y DE GAMEPLAY
 * Esta versión lee el hardware (infalible) y ejecuta el salto directamente
 * en el objeto del jugador (compatibilidad total con 2.2 y CBF).
 */
class $modify(CCScheduler) {
    void update(float dt) {
        CCScheduler::update(dt);

#ifdef GEODE_IS_WINDOWS
        // Obtenemos el estado de la partida actual
        auto playLayer = PlayLayer::get();
        auto kbd = CCKeyboardDispatcher::get();

        // 1. DETECCIÓN DE CONTROL (Salto / Espacio)
        bool currentCtrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
        if (currentCtrl != g_ctrlDown) {
            g_ctrlDown = currentCtrl;
            
            if (playLayer && playLayer->m_player1) {
                // Si estamos jugando, forzamos el salto directo (Precisión Máxima)
                if (g_ctrlDown) playLayer->m_player1->pushButton(PlayerButton::Jump);
                else playLayer->m_player1->releaseButton(PlayerButton::Jump);
            } else if (kbd) {
                // Si estamos en un menú, mandamos un Espacio normal
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, g_ctrlDown, false, 0.0);
            }
        }

        // 2. DETECCIÓN DE CLICK DERECHO (Tecla Z)
        bool currentRight = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        if (currentRight != g_rightDown) {
            g_rightDown = currentRight;
            if (kbd) kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, g_rightDown, false, 0.0);
        }

        // 3. DETECCIÓN DE RUEDITA (Tecla X)
        bool currentMiddle = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
        if (currentMiddle != g_middleDown) {
            g_middleDown = currentMiddle;
            if (kbd) kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, g_middleDown, false, 0.0);
        }
#endif
    }
};