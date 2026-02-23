#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>

#ifdef GEODE_IS_WINDOWS
  #include <windows.h>
#endif

#ifdef GEODE_IS_LINUX
  #include <cstdio>
  #include <cstring>
  #include <dirent.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <linux/input.h>
  #include <sys/ioctl.h>

  // OPTIMIZACIÓN LINUX: Busca el teclado una vez y mantiene el archivo abierto
  static int getKbdFd() {
      static int kbdFd = -2;
      if (kbdFd != -2) return kbdFd;

      DIR* dir = opendir("/dev/input");
      if (!dir) return kbdFd = -1;

      struct dirent* ent;
      while ((ent = readdir(dir)) != nullptr) {
          if (strncmp(ent->d_name, "event", 5) != 0) continue;
          char path[64];
          snprintf(path, sizeof(path), "/dev/input/%s", ent->d_name);
          int fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
          if (fd < 0) continue;

          uint8_t evbits[(EV_MAX + 7) / 8] = {};
          if (ioctl(fd, EVIOCGBIT(0, sizeof(evbits)), evbits) >= 0) {
              if (evbits[EV_KEY / 8] & (1 << (EV_KEY % 8))) {
                  kbdFd = fd;
                  closedir(dir);
                  return kbdFd;
              }
          }
          close(fd);
      }
      closedir(dir);
      return kbdFd = -1;
  }

  static bool evdevKeyDown(int linuxKey) {
      int fd = getKbdFd();
      if (fd < 0) return false;
      uint8_t keybits[(KEY_MAX + 7) / 8] = {};
      if (ioctl(fd, EVIOCGKEY(sizeof(keybits)), keybits) >= 0) {
          return (keybits[linuxKey / 8] & (1 << (linuxKey % 8))) != 0;
      }
      return false;
  }
#endif

#ifdef GEODE_IS_MAC
  #include <Carbon/Carbon.h>
  static bool macKeyDown(uint16_t vk) {
      return CGEventSourceKeyState(kCGEventSourceStateHIDSystemState, vk);
  }
#endif

using namespace geode::prelude;

// ── HELPERS DE HARDWARE: Detectan qué lado (L/R) se está presionando ───────

static bool isLCtrlDown() {
#ifdef GEODE_IS_WINDOWS
    return (GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0;
#elif defined(GEODE_IS_LINUX)
    return evdevKeyDown(KEY_LEFTCTRL);
#elif defined(GEODE_IS_MAC)
    return macKeyDown(0x3B);
#else
    return false;
#endif
}

static bool isRCtrlDown() {
#ifdef GEODE_IS_WINDOWS
    return (GetAsyncKeyState(VK_RCONTROL) & 0x8000) != 0;
#elif defined(GEODE_IS_LINUX)
    return evdevKeyDown(KEY_RIGHTCTRL);
#elif defined(GEODE_IS_MAC)
    return macKeyDown(0x3E);
#else
    return false;
#endif
}

static bool isLShiftDown() {
#ifdef GEODE_IS_WINDOWS
    return (GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0;
#elif defined(GEODE_IS_LINUX)
    return evdevKeyDown(KEY_LEFTSHIFT);
#elif defined(GEODE_IS_MAC)
    return macKeyDown(0x38);
#else
    return false;
#endif
}

static bool isRShiftDown() {
#ifdef GEODE_IS_WINDOWS
    return (GetAsyncKeyState(VK_RSHIFT) & 0x8000) != 0;
#elif defined(GEODE_IS_LINUX)
    return evdevKeyDown(KEY_RIGHTSHIFT);
#elif defined(GEODE_IS_MAC)
    return macKeyDown(0x3C);
#else
    return false;
#endif
}

static bool isLAltDown() {
#ifdef GEODE_IS_WINDOWS
    return (GetAsyncKeyState(VK_LMENU) & 0x8000) != 0;
#elif defined(GEODE_IS_LINUX)
    return evdevKeyDown(KEY_LEFTALT);
#elif defined(GEODE_IS_MAC)
    return macKeyDown(0x3A);
#else
    return false;
#endif
}

static bool isRAltDown() {
#ifdef GEODE_IS_WINDOWS
    return (GetAsyncKeyState(VK_RMENU) & 0x8000) != 0;
#elif defined(GEODE_IS_LINUX)
    return evdevKeyDown(KEY_RIGHTALT);
#elif defined(GEODE_IS_MAC)
    return macKeyDown(0x3D);
#else
    return false;
#endif
}

// ─────────────────────────────────────────────────────────────────────────────

$execute {
    static bool modIsActive = false;

    // ── TECLADO: Remapeo Universal ───────────────────────────────────────────
    geode::KeyboardInputEvent().listen([](geode::KeyboardInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        auto mod = Mod::get();
        if (!kbd) return ListenerResult::Propagate;

        bool shouldRemap = false;
        auto key = data.key;

        // Comprobación de Control
        if (key == KEY_Control) {
            if ((mod->getSettingValue<bool>("l-ctrl") && isLCtrlDown()) || 
                (mod->getSettingValue<bool>("r-ctrl") && isRCtrlDown())) shouldRemap = true;
        }
        // Comprobación de Shift
        else if (key == KEY_Shift) {
            if ((mod->getSettingValue<bool>("l-shift") && isLShiftDown()) || 
                (mod->getSettingValue<bool>("r-shift") && isRShiftDown())) shouldRemap = true;
        }
        // Comprobación de Alt
        else if (key == KEY_Alt) {
            if ((mod->getSettingValue<bool>("l-alt") && isLAltDown()) || 
                (mod->getSettingValue<bool>("r-alt") && isRAltDown())) shouldRemap = true;
        }

        if (shouldRemap) {
            bool down = (data.action != geode::KeyboardInputData::Action::Release);
            modIsActive = down;
            kbd->dispatchKeyboardMSG(KEY_Space, down, (data.action == geode::KeyboardInputData::Action::Repeat), data.timestamp);
            return ListenerResult::Stop;
        }

        // Sincronización pasiva (Evita saltos pegados si se pierde un evento)
        if (modIsActive && !isLCtrlDown() && !isRCtrlDown() && !isLShiftDown() && !isRShiftDown() && !isLAltDown() && !isRAltDown()) {
            modIsActive = false;
            kbd->dispatchKeyboardMSG(KEY_Space, false, false, data.timestamp);
        }

        return ListenerResult::Propagate;
    }).leak();

    // ── MOUSE Y SCROLL: Checkpoints Rápidos ──────────────────────────────────
    geode::MouseInputEvent().listen([](geode::MouseInputData& data) {
        auto kbd = CCKeyboardDispatcher::get();
        if (!kbd) return ListenerResult::Propagate;

        if (Mod::get()->getSettingValue<bool>("rapid-checkpoints")) {
            bool down = (data.action == geode::MouseInputData::Action::Press);
            if (data.button == geode::MouseInputData::Button::Right) {
                kbd->dispatchKeyboardMSG(KEY_Z, down, false, data.timestamp);
                return ListenerResult::Stop;
            }
            if (data.button == geode::MouseInputData::Button::Middle) {
                kbd->dispatchKeyboardMSG(KEY_X, down, false, data.timestamp);
                return ListenerResult::Stop;
            }
        }
        return ListenerResult::Propagate;
    }).leak();

    geode::ScrollWheelEvent().listen(+[](double x, double y) {
        if (Mod::get()->getSettingValue<bool>("rapid-checkpoints")) {
            auto kbd = CCKeyboardDispatcher::get();
            if (kbd) {
                kbd->dispatchKeyboardMSG(KEY_X, true,  false, 0.0);
                kbd->dispatchKeyboardMSG(KEY_X, false, false, 0.0);
                return ListenerResult::Stop;
            }
        }
        return ListenerResult::Propagate;
    }).leak();
}
