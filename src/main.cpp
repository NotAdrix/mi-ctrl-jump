#include <Geode/Geode.hpp>
#include <Geode/binding/PlayLayer.hpp>

using namespace geode::prelude;

// ── Teclas hardcodeadas ────────────────────────────────────────────────────
static constexpr int JUMP_KEY        = enumKeyCodes::KEY_Control; // tu tecla
static constexpr int CHECKPOINT_KEY  = enumKeyCodes::KEY_MB4;     // o lo que uses
static constexpr int DEL_CHECK_KEY   = enumKeyCodes::KEY_MB5;
static constexpr int RESET_KEY       = enumKeyCodes::KEY_MB4;     // boton4 mouse = R

$execute {
    geode::KeyboardInputEvent().listen([](geode::KeyboardInputData& data) {
        auto pl = PlayLayer::get();
        if (!pl) return ListenerResult::Propagate;

        bool down   = (data.action != geode::KeyboardInputData::Action::Release);
        bool repeat = (data.action == geode::KeyboardInputData::Action::Repeat);
        if (repeat) return ListenerResult::Propagate;

        int key = static_cast<int>(data.key);

        // Salto — directo a queueButton, sin rodeo por dispatcher
        if (key == JUMP_KEY) {
            pl->queueButton(1, down, false, data.timestamp);
            return ListenerResult::Stop;
        }

        return ListenerResult::Propagate;
    }).leak();

    geode::MouseInputEvent().listen([](geode::MouseInputData& data) {
        auto pl = PlayLayer::get();
        if (!pl) return ListenerResult::Propagate;

        bool down = (data.action == geode::MouseInputData::Action::Press);
        auto kbd  = CCKeyboardDispatcher::get();
        if (!kbd) return ListenerResult::Propagate;

        // Limpia modificadores antes de despachar, resuelve el bug de Ctrl/Alt
        auto shift = kbd->m_bShiftPressed;
        auto ctrl  = kbd->m_bControlPressed;
        auto alt   = kbd->m_bAltPressed;
        auto cmd   = kbd->m_bCommandPressed;
        kbd->m_bShiftPressed   = false;
        kbd->m_bControlPressed = false;
        kbd->m_bAltPressed     = false;
        kbd->m_bCommandPressed = false;

        ListenerResult result = ListenerResult::Propagate;

        if (data.button == geode::MouseInputData::Button::Right) {
            kbd->dispatchKeyboardMSG(KEY_Z, down, false, data.timestamp);
            result = ListenerResult::Stop;
        }
        else if (data.button == geode::MouseInputData::Button::Middle) {
            kbd->dispatchKeyboardMSG(KEY_X, down, false, data.timestamp);
            result = ListenerResult::Stop;
        }
        else if (data.button == geode::MouseInputData::Button::Button4) {
            kbd->dispatchKeyboardMSG(KEY_R, down, false, data.timestamp);
            result = ListenerResult::Stop;
        }

        // Restaura modificadores
        kbd->m_bShiftPressed   = shift;
        kbd->m_bControlPressed = ctrl;
        kbd->m_bAltPressed     = alt;
        kbd->m_bCommandPressed = cmd;

        return result;
    }).leak();
}
