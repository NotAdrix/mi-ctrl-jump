#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

using namespace geode::prelude;

class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat) {
        // Si es la tecla Control
        if (key == enumKeyCodes::KEY_Control) {
            auto playLayer = PlayLayer::get();
            
            // Solo actuar si estamos jugando y el puntero al jugador es válido
            if (playLayer && playLayer->m_player1) {
                if (isKeyDown) {
                    playLayer->m_player1->pushButton(PlayerButton::Jump);
                } else {
                    playLayer->m_player1->releaseButton(PlayerButton::Jump);
                }
                // Retornamos true para que el juego ignore la tecla nativa
                return true;
            }
        }
        // Para todo lo demás, comportamiento normal
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat);
    }
};