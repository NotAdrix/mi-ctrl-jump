#include <Geode/binding/PlayLayer.hpp>
#include <Geode/modify/UILayer.hpp>
#include <Geode/binding/CCKeyboardDispatcher.hpp>
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

struct $modify(MyUILayer, UILayer) {
    static void onModify(auto& self) {
        // Prioridad Late para interceptar después de otros mods, pero antes que el juego
        (void)self.setHookPriority("UILayer::handleKeypress", Priority::Late);
    }

    // El "Amazing Hack": Simula una tecla mientras manipula los modificadores
    void pressKeyFallthrough(enumKeyCodes key, bool down, double timestamp) {
        if (!this->isCurrentPlayLayer()) return;

        auto GM = GameManager::sharedState();
        auto dispatcher = CCDirector::get()->getKeyboardDispatcher();

        // Guardamos estado original
        auto oShift = dispatcher->getShiftKeyPressed();
        auto oCtrl = dispatcher->getControlKeyPressed();

        // Desactivamos temporalmente el flag de Control para que el juego 
        // no piense que es un comando de sistema (como Ctrl+S)
        dispatcher->updateModifierKeys(oShift, false, false, false);

        allowKeyDownThrough = true;
        if (down) {
            this->keyDown(key, timestamp);
        } else {
            this->keyUp(key, timestamp);
        }
        allowKeyDownThrough = false;

        // Restauramos estado
        dispatcher->updateModifierKeys(oShift, oCtrl, false, false);
    }

    bool isCurrentPlayLayer() {
        auto playLayer = PlayLayer::get();
        return playLayer != nullptr && playLayer->getChildByType<UILayer>(0) == this;
    }

    bool init(GJBaseGameLayer* layer) {
        if (!UILayer::init(layer)) return false;

        geode::Loader::get()->queueInMainThread([this] {
            if (!PlayLayer::get()) return;

            // --- ACCIÓN: JUMP (Ctrl -> Space) ---
            this->defineKeybind("jump-p1", [this](bool down, bool repeat, double timestamp) {
                if (repeat) return ListenerResult::Propagate;
                
                // El método profesional: queueButton para latencia de hardware
                PlayLayer::get()->queueButton(1, down, false, timestamp);
                
                // Enmascaramos como un Space físico por si otros mods escuchan keyDown
                this->pressKeyFallthrough(KEY_Space, down, timestamp);
                
                return ListenerResult::Stop; // Aquí ocurre el enmascaramiento total
            });

            // --- ACCIÓN: CHECKPOINT (Mouse Right -> Z) ---
            this->defineKeybind("place-checkpoint", [this](bool down, bool repeat, double timestamp) {
                if (repeat) return ListenerResult::Propagate;
                this->pressKeyFallthrough(KEY_Z, down, timestamp);
                return ListenerResult::Stop;
            });
        });

        return true;
    }

    // Helper para registrar el evento de Geode v5
    void defineKeybind(std::string id, std::function<ListenerResult(bool, bool, double)> callback) {
        PlayLayer::get()->addEventListener(
            KeybindSettingPressedEventV3(Mod::get(), std::move(id)),
            [callback = std::move(callback)](Keybind const&, bool down, bool repeat, double timestamp) {
                return callback(down, repeat, timestamp);
            }
        );
    }

    static inline bool allowKeyDownThrough = false;
    void handleKeypress(cocos2d::enumKeyCodes key, bool down, double timestamp) {
        if (allowKeyDownThrough) {
            UILayer::handleKeypress(key, down, timestamp);
            allowKeyDownThrough = false;
        }
    }
};
