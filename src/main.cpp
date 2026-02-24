#include <Geode/Geode.hpp>

#include <Geode/utils/Keyboard.hpp>

#include <Geode/loader/Event.hpp>



using namespace geode::prelude;



$execute {

    // Estado lógico: ¿El mod envió un "Presionar"?

    static bool modIsActive = false;



    // Función de Sincronización Infalible:

    // No necesita saber qué otras teclas existen. Solo compara el bit de hardware.

    auto forceSync = [](geode::KeyboardModifier mods, double timestamp) {

        auto kbd = CCKeyboardDispatcher::get();

        if (!kbd) return;



        // El bit 2 representa el estado físico real del Control

        bool ctrlIsPhysicallyDown = (static_cast<int>(mods) & 2);



        // Si el mod cree que está saltando, pero el hardware dice que Ctrl ya no está:

        if (modIsActive && !ctrlIsPhysicallyDown) {

            modIsActive = false;

            // Forzamos el levantamiento del salto usando el tiempo del hardware

            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, false, false, timestamp);

        }

    };



    // ── INTERCEPTOR DE TECLADO ──────────────────────────────────────────────

    geode::KeyboardInputEvent().listen([forceSync](geode::KeyboardInputData& data) {

        auto kbd = CCKeyboardDispatcher::get();

        if (!kbd) return ListenerResult::Propagate;



        // Si es nuestra tecla (ID 162 detectado en logs)

        if (static_cast<int>(data.key) == 162) {

            bool down = (data.action != geode::KeyboardInputData::Action::Release);

            bool repeat = (data.action == geode::KeyboardInputData::Action::Repeat);

            

            modIsActive = down;

            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Space, down, repeat, data.timestamp);

            return ListenerResult::Stop;

        }



        // Para CUALQUIER otra tecla que se cruce, validamos el estado del Ctrl

        forceSync(data.modifiers, data.timestamp);

        return ListenerResult::Propagate;

    }).leak(); // Mantiene el listener vivo



    // ── INTERCEPTOR DE MOUSE ────────────────────────────────────────────────

    geode::MouseInputEvent().listen([forceSync](geode::MouseInputData& data) {

        auto kbd = CCKeyboardDispatcher::get();

        if (!kbd) return ListenerResult::Propagate;



        // Cada click de mouse también trae el estado de los modificadores.

        // Si sueltas Ctrl mientras haces click derecho, aquí se corregirá.

        forceSync(data.modifiers, data.timestamp);



        bool down = (data.action == geode::MouseInputData::Action::Press);



        if (data.button == geode::MouseInputData::Button::Right) {

            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_Z, down, false, data.timestamp);

            return ListenerResult::Stop;

        }

        if (data.button == geode::MouseInputData::Button::Middle) {

            kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, down, false, data.timestamp);

            return ListenerResult::Stop;

        }

        return ListenerResult::Propagate;

    }).leak();



    // ── INTERCEPTOR DE SCROLL ───────────────────────────────────────────────

    geode::ScrollWheelEvent().listen(+[](double x, double y) {

        auto kbd = CCKeyboardDispatcher::get();

        if (!kbd) return ListenerResult::Propagate;

        // El scroll es un evento instantáneo de checkpoint

        kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, true,  false, 0.0);

        kbd->dispatchKeyboardMSG(enumKeyCodes::KEY_X, false, false, 0.0);

        return ListenerResult::Stop;

    }).leak();

}
