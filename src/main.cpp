#include <Geode/Geode.hpp>
#include <Geode/modify/UILayer.hpp>

using namespace geode::prelude;

class $modify(MyUILayer, UILayer) {
    void keyDown(cocos2d::enumKeyCodes key) {
        // Si presionas Ctrl, salta
        if (key == cocos2d::enumKeyCodes::KEY_Control || 
            key == cocos2d::enumKeyCodes::KEY_LControl || 
            key == cocos2d::enumKeyCodes::KEY_RControl) {
            
            if (auto playLayer = PlayLayer::get()) {
                playLayer->pushButton(PlayerButton::Jump, true);
            }
        }
        UILayer::keyDown(key);
    }

    void keyUp(cocos2d::enumKeyCodes key) {
        // Si sueltas Ctrl, deja de saltar
        if (key == cocos2d::enumKeyCodes::KEY_Control || 
            key == cocos2d::enumKeyCodes::KEY_LControl || 
            key == cocos2d::enumKeyCodes::KEY_RControl) {
            
            if (auto playLayer = PlayLayer::get()) {
                playLayer->releaseButton(PlayerButton::Jump, true);
            }
        }
        UILayer::keyUp(key);
    }
};