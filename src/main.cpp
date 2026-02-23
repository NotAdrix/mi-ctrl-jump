#include <Geode/Geode.hpp>
#include <Geode/modify/UILayer.hpp>
// Estos son los bindeos correctos para Cocos2d en Geode v5
#include <Geode/cocos/include/cocos2d.h>

using namespace geode::prelude;

struct $modify(MyUILayer, UILayer) {
    static void onModify(auto& self) {
        // Prioridad Late para interceptar antes que el juego procese el input
        (void)self.setHookPriority("UILayer::handleKeypress", Priority::Late);
    }

    // Bandera de control para evitar bucles infinitos
    static inline bool allowKeyDownThrough = false;

    // EL HACK PRO: Simula una tecla mientras manipula los modificadores (Ctrl/Shift)
    void pressKeyFallthrough(enumKeyCodes key, bool down, double timestamp) {
        if (!this->isCurrentPlayLayer()) return;

        auto dispatcher = CCDirector::sharedDirector()->getKeyboardDispatcher();

        // Guardamos el estado real de las teclas modificadoras
        auto oShift = dispatcher->getShiftKeyPressed();
        auto oCtrl = dispatcher->getControlKeyPressed();
        auto oAlt = dispatcher->getAltKeyPressed();
        auto oCmd = dispatcher->getCommandKeyPressed();

        // DESACTIVAMOS el Control para el motor (enmascaramiento)
        // Esto evita que el juego crea que es un comando de sistema (como Ctrl+R)
        dispatcher->updateModifierKeys(oShift, false, oAlt, oCmd);

        allowKeyDownThrough = true;
        if (down) {
            this->keyDown(key, timestamp);
        } else {
            this->keyUp(key, timestamp);
        }
        allowKeyDownThrough = false;

        // Restauramos el estado original para no romper el sistema
        dispatcher->updateModifierKeys(oShift, oCtrl, oAlt, oCmd);
    }

    bool isCurrentPlayLayer() {
        auto playLayer = PlayLayer::get();
        return playLayer != nullptr && playLayer->getChildByType<UILayer>(0) == this;
    }

    bool init(GJBaseGameLayer* layer) {
        if (!UILayer::init(layer)) return false;

        // Esperamos un frame para que PlayLayer esté listo
        geode::Loader::get()->queueInMainThread([this] {
            if (!PlayLayer::get()) return;

            // --- Lógica de Teclado (Ctrl -> Space) ---
            this->defineKeybind("jump-p1", [this](bool down, bool repeat, double timestamp) {
                if (repeat) return ListenerResult::Propagate;
                
                // Usamos queueButton para la latencia cero (físicas directas)
                PlayLayer::get()->queueButton(1, down, false, timestamp);
                
                // Realizamos el enmascaramiento: el juego "ve" un Space sin Ctrl
                this->pressKeyFallthrough(KEY_Space, down, timestamp);
                
                return ListenerResult::Stop; 
            });

            // --- Lógica de Mouse (Right Click -> Z) ---
            this->defineKeybind("place-checkpoint", [this](bool down, bool repeat, double timestamp) {
                if (repeat) return ListenerResult::Propagate;
                this->pressKeyFallthrough(KEY_Z, down, timestamp);
                return ListenerResult::Stop;
            });
        });

        return true;
    }

    // Helper para registrar el evento de Geode v5 sin colisionar
    void defineKeybind(std::string id, std::function<ListenerResult(bool, bool, double)> callback) {
        PlayLayer::get()->addEventListener(
            KeybindSettingPressedEventV3(Mod::get(), std::move(id)),
            [callback = std::move(callback)](Keybind const&, bool down, bool repeat, double timestamp) {
                return callback(down, repeat, timestamp);
            }
        );
    }

    // Capturamos la pulsación para dejarla pasar solo si nuestro mod lo permite
    void handleKeypress(cocos2d::enumKeyCodes key, bool down, double timestamp) {
        if (key == enumKeyCodes::KEY_Escape || allowKeyDownThrough) {
            UILayer::handleKeypress(key, down, timestamp);
            allowKeyDownThrough = false;
        }
    }
};
