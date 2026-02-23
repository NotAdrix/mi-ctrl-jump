#include <Geode/Geode.hpp>
#include <Geode/modify/UILayer.hpp>

using namespace geode::prelude;

class $modify(UILayer) {
    void keyDown(enumKeyCodes key, double timestamp) {
        if (key == enumKeyCodes::KEY_Control) {
            auto playLayer = PlayLayer::get();
            if (playLayer && playLayer->m_player1) {
                // En la 2.2081 se usa m_player1 para saltar
                playLayer->m_player1->pushButton(PlayerButton::Jump);
            }
        }
        UILayer::keyDown(key, timestamp);
    }

    void keyUp(enumKeyCodes key, double timestamp) {
        if (key == enumKeyCodes::KEY_Control) {
            auto playLayer = PlayLayer::get();
            if (playLayer && playLayer->m_player1) {
                // Para soltar el salto
                playLayer->m_player1->releaseButton(PlayerButton::Jump);
            }
        }
        UILayer::keyUp(key, timestamp);
    }
};