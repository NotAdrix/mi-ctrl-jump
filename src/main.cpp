#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(CCKeyboardDispatcher) {
    // Usamos la firma exacta de la 2.2081 (con el double p3)
    bool dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat, double p3) {
        auto playLayer = PlayLayer::get();

        // Si no hay PlayLayer, no estamos en un nivel o el juego está pausado, 
        // dejamos que el juego maneje el teclado normalmente.
        if (!playLayer || playLayer->m_isPaused) {
            return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, p3);
        }

        // --- LÓGICA DE SALTO ---
        auto jump1 = Mod::get()->getSettingValue<enumKeyCodes>("jump-key-1");
        auto jump2 = Mod::get()->getSettingValue<enumKeyCodes>("jump-key-2");

        if (key == jump1 || (jump2 != enumKeyCodes::KEY_None && key == jump2)) {
            if (isKeyDown) {
                playLayer->m_player1->pushButton(PlayerButton::Jump);
            } else {
                playLayer->m_player1->releaseButton(PlayerButton::Jump);
            }
            return true; // Evita que la tecla haga otras cosas en el juego
        }

        // --- LÓGICA DE CHECKPOINTS ---
        // Solo funciona en modo práctica, cuando se presiona la tecla (no repetición)
        if (playLayer->m_isPracticeMode && isKeyDown && !isKeyRepeat) {
            auto checkKey = Mod::get()->getSettingValue<enumKeyCodes>("checkpoint-key");
            auto removeKey = Mod::get()->getSettingValue<enumKeyCodes>("remove-checkpoint-key");

            if (key == checkKey) {
                playLayer->createCheckpoint();
                return true;
            }
            if (key == removeKey) {
                playLayer->removeLastCheckpoint();
                return true;
            }
        }

        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, p3);
    }
};