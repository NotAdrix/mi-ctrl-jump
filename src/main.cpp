#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// --- LÓGICA DE SALTO ---
class $modify(CCKeyboardDispatcher) {
    // Añadimos el cuarto argumento 'double p3' que pide el error
    bool dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat, double p3) {
        auto key1 = Mod::get()->getSettingValue<enumKeyCodes>("jump-key-1");
        auto key2 = Mod::get()->getSettingValue<enumKeyCodes>("jump-key-2");

        if (key == key1 || (key2 != enumKeyCodes::KEY_None && key == key2)) {
            auto playLayer = PlayLayer::get();
            if (playLayer && playLayer->m_player1 && !playLayer->m_isPaused) {
                if (isKeyDown) {
                    playLayer->m_player1->pushButton(PlayerButton::Jump);
                } else {
                    playLayer->m_player1->releaseButton(PlayerButton::Jump);
                }
                return true;
            }
        }
        // Pasamos p3 al llamar a la función original
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, p3);
    }
};

// --- LÓGICA DE CHECKPOINTS ---
class $modify(PlayLayer) {
    // Nota: handleButton es interno de Geode y suele ser más estable
    void handleButton(bool down, enumKeyCodes key) {
        auto checkpointKey = Mod::get()->getSettingValue<enumKeyCodes>("checkpoint-key");
        auto removeKey = Mod::get()->getSettingValue<enumKeyCodes>("remove-checkpoint-key");

        if (this->m_isPracticeMode && !this->m_isPaused && down) {
            if (key == checkpointKey) {
                this->createCheckpoint();
                return;
            }
            if (key == removeKey) {
                this->removeLastCheckpoint();
                return;
            }
        }
        // Llamada original para no romper el salto normal con espacio/flecha
        PlayLayer::handleButton(down, key);
    }
};