geode::KeyboardInputEvent().listen(+[](geode::KeyboardInputData& data) {
    log::debug("key={} action={} mods={}", 
        (int)data.key, 
        (int)data.action, 
        (int)data.modifiers.value
    );
    return ListenerResult::Propagate;
}).leak();