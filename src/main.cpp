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
