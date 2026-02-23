#include <Geode/Geode.hpp>
#include <Geode/modify/UILayer.hpp>

using namespace geode::prelude;

class $modify(UILayer) {
    void keyDown(enumKeyCodes key) {
        if (key == enumKeyCodes::KEY_Control) {
            auto playLayer = PlayLayer::get();
            if (playLayer) {
                playLayer->pushButton(PlayerButton::Jump, true);
            }
        }
        UILayer::keyDown(key);
    }

    void keyUp(enumKeyCodes key) {
        if (key == enumKeyCodes::KEY_Control) {
            auto playLayer = PlayLayer::get();
            if (playLayer) {
                playLayer->pushButton(PlayerButton::Jump, false);
            }
        }
        // Agregamos el 0 que faltaba para el timestamp
        UILayer::keyUp(key, 0);
    }
};