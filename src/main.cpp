#include <Geode/Geode.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/loader/SettingNode.hpp>

using namespace geode::prelude;

// Variables globales para controlar el estado de vinculación
bool g_isBindingJump1 = false;
bool g_isBindingJump2 = false;

// Creamos la clase que dibujará nuestro botón en el menú de ajustes
class BotonVincularNode : public SettingNode {
protected:
    bool init(std::shared_ptr<geode::Setting> setting, float width) {
        if (!SettingNode::init(setting)) return false;
        
        // Ajustamos el tamaño del "cuadro" donde irá el botón
        this->setContentSize({width, 40.f});
        auto menu = CCMenu::create();
        
        // Creamos el botón visual estilo Geometry Dash
        auto btnSprite = ButtonSprite::create("Vincular Tecla");
        btnSprite->setScale(0.8f);
        
        auto btn = CCMenuItemSpriteExtra::create(
            btnSprite, this, menu_selector(BotonVincularNode::onBotonPresionado)
        );
        
        menu->addChild(btn);
        menu->setPosition({width / 2, 20.f});
        this->addChild(menu);
        
        return true;
    }

    // Esta función se ejecuta cuando el usuario hace clic en el botón
    void onBotonPresionado(CCObject*) {
        // Averiguamos cuál de los dos botones presionó mirando el ID del JSON
        if (this->getSetting()->getKey() == "jump1-button") {
            g_isBindingJump1 = true;
            g_isBindingJump2 = false;
        } else {
            g_isBindingJump2 = true;
            g_isBindingJump1 = false;
        }
        
        // Le avisamos al usuario que estamos escuchando
        FLAlertLayer::create("Escuchando...", "Presiona CUALQUIER tecla ahora mismo para vincularla.", "OK")->show();
    }
    
public:
    // Funciones obligatorias de Geode para que el ajuste no dé errores
    void commit() override {}
    bool hasUncommittedChanges() override { return false; }
    bool hasNonDefaultValue() override { return false; }
    void resetToDefault() override {}
    
    // Función creadora
    static BotonVincularNode* create(std::shared_ptr<geode::Setting> setting, float width) {
        auto ret = new BotonVincularNode();
        if (ret && ret->init(setting, width)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// Le decimos a Geode que conecte nuestro JSON con esta clase de C++
$on_mod(Loaded) {
    Mod::get()->registerCustomSetting("boton-vincular", [](std::shared_ptr<geode::Setting> setting, float width) {
        return BotonVincularNode::create(setting, width);
    });
}


$execute {
    // ── TECLADO DINÁMICO ────────────────────────────────────────────────────
    geode::KeyboardInputEvent().listen([](geode::KeyboardInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        auto mod = Mod::get();
        if (!kbd) return ListenerResult::Propagate;

        int currentKey = static_cast<int>(data.key);
        
        // --- Lógica de Vinculación (Modo Escucha) ---
        if (g_isBindingJump1 || g_isBindingJump2) {
            if (data.action == geode::KeyboardInputData::Action::Press) {
                int64_t j1 = mod->getSettingValue<int64_t>("jump1-key-id");
                int64_t j2 = mod->getSettingValue<int64_t>("jump2-key-id");

                if (g_isBindingJump1 && currentKey != j2) {
                    mod->setSettingValue("jump1-key-id", static_cast<int64_t>(currentKey));
                    g_isBindingJump1 = false;
                    FLAlertLayer::create("Éxito", "Tecla 1 vinculada", "OK")->show();
                } 
                else if (g_isBindingJump2 && currentKey != j1) {
                    mod->setSettingValue("jump2-key-id", static_cast<int64_t>(currentKey));
                    g_isBindingJump2 = false;
                    FLAlertLayer::create("Éxito", "Tecla 2 vinculada", "OK")->show();
                }
            }
            return ListenerResult::Stop; 
        }

        // --- Re-mapeo Dinámico al Espacio ---
        int64_t jump1Key = mod->getSettingValue<int64_t>("jump1-key-id");
        int64_t jump2Key = mod->getSettingValue<int64_t>("jump2-key-id");

        if (currentKey != 0 && (currentKey == jump1Key || currentKey == jump2Key)) {
            bool down = (data.action != geode::KeyboardInputData::Action::Release);
            bool isRepeat = (data.action == geode::KeyboardInputData::Action::Repeat);

            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, down, isRepeat, data.timestamp);
            return ListenerResult::Stop; 
        }

        return ListenerResult::Propagate;
    }).leak();

    // ── MOUSE (Rapid Checkpoints: Clic Derecho y Clic Rueda) ───────────────
    geode::MouseInputEvent().listen([](geode::MouseInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return ListenerResult::Propagate;

        if (Mod::get()->getSettingValue<bool>("rapid-checkpoints")) {
            bool down = (data.action == geode::MouseInputData::Action::Press);
            
            // Clic Derecho -> Z
            if (data.button == geode::MouseInputData::Button::Right) {
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, down, false, data.timestamp);
                return ListenerResult::Stop;
            }
            // Clic de la Rueda (Middle Click) -> X
            if (data.button == geode::MouseInputData::Button::Middle) {
                kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, down, false, data.timestamp);
                return ListenerResult::Stop;
            }
        }
        return ListenerResult::Propagate;
    }).leak();
}
