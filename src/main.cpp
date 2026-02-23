#include <Geode/Geode.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <Geode/loader/Event.hpp>

using namespace geode::prelude;

$execute {

    static bool isJumping = false;

    auto sendSpace = [](bool down, bool repeat, double timestamp) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return;

        kbd->dispatchKeyboardMSG(
            enumKeyCodes::KEY_Space,
            down,
            repeat,
            timestamp
        );
    };

    // ───────────────────────────────
    // KEYBOARD
    // ───────────────────────────────
    geode::KeyboardInputEvent().listen([forceSync](geode::KeyboardInputData& data) {

    auto kbd = CCKeyboardDispatcher::get();
    auto mod = Mod::get();
    if (!kbd) return ListenerResult::Propagate;

    bool shouldRemap = false;

    // Control
    if (data.key == enumKeyCodes::KEY_LCONTROL &&
        mod->getSettingValue<bool>("l-ctrl"))  shouldRemap = true;

    if (data.key == enumKeyCodes::KEY_RCONTROL &&
        mod->getSettingValue<bool>("r-ctrl"))  shouldRemap = true;

    // Shift
    if (data.key == enumKeyCodes::KEY_LSHIFT &&
        mod->getSettingValue<bool>("l-shift")) shouldRemap = true;

    if (data.key == enumKeyCodes::KEY_RSHIFT &&
        mod->getSettingValue<bool>("r-shift")) shouldRemap = true;

    // Alt
    if (data.key == enumKeyCodes::KEY_LMENU &&
        mod->getSettingValue<bool>("l-alt"))   shouldRemap = true;

    if (data.key == enumKeyCodes::KEY_RMENU &&
        mod->getSettingValue<bool>("r-alt"))   shouldRemap = true;

    if (shouldRemap) {
        bool down = (data.action != geode::KeyboardInputData::Action::Release);
        bool repeat = (data.action == geode::KeyboardInputData::Action::Repeat);

        modIsActive = down;

        kbd->dispatchKeyboardMSG(
            enumKeyCodes::KEY_Space,
            down,
            repeat,
            data.timestamp
        );

        return ListenerResult::Stop;
    }

    forceSync(data.modifiers, data.timestamp);
    return ListenerResult::Propagate;

}).leak();


    // ───────────────────────────────
    // FAILSAFE — Si por alguna razón GD pierde el release
    // Se ejecuta en cualquier evento de input
    // ───────────────────────────────
    auto safetyCheck = [](KeyboardModifier mods, double timestamp) {

        if (!isJumping)
            return;

        // Si ningún modificador relevante está físicamente activo
        bool ctrl  = mods & KeyboardModifier::Control;
        bool shift = mods & KeyboardModifier::Shift;
        bool alt   = mods & KeyboardModifier::Alt;

        if (!ctrl && !shift && !alt) {
            isJumping = false;
            sendSpace(false, false, timestamp);
        }
    };

    geode::MouseInputEvent().listen([safetyCheck](MouseInputData& data) {
        safetyCheck(data.modifiers, data.timestamp);
        return ListenerResult::Propagate;
    }).leak();

    geode::ScrollWheelEvent().listen([safetyCheck](double x, double y) {
        safetyCheck(KeyboardModifier::None, 0.0);
        return ListenerResult::Propagate;
    }).leak();
}
