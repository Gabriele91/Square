//
//  Square
//
//  Created by Gabriele on 14/08/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#include "Screen_private.h"
#include "Window_private.h"
#include "Input_private.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cerrno>
#include <linux/input.h>

namespace Square
{
namespace Video
{
namespace Xorg
{

    void InputXorg::send_keyboard_event(KeyboardEvent kevent, short mode, ActionEvent action)
    {
        if (m_keyboard) m_keyboard(kevent, mode, action);
    }
    void InputXorg::send_character_event(int character, short mode, bool plain)
    {
        if (m_character) m_character(character, mode, plain);
    }
    void InputXorg::send_mouse_button_event(MouseButtonEvent mevent, ActionEvent action)
    {
        if (m_mouse_button) m_mouse_button(mevent, action);
    }
    void InputXorg::send_mouse_move_event(double x, double y)
    {
        if (m_mouse_move) m_mouse_move(x, y);
    }
    void InputXorg::send_mouse_scroll_event(double scroll)
    {
        if (m_scroll) m_scroll(scroll);
    }
    void InputXorg::send_touch_event(FingerEvent fevent, ActionEvent action)
    {
        if (m_touch) m_touch(fevent, action);
    }
    void InputXorg::send_window_event(WindowEvent wevent)
    {
        //(no focus: its gamepads as nothing held)
        if (wevent == WindowEvent::GET_FOCUS || wevent == WindowEvent::LOST_FOCUS)
        {
            m_focus = wevent == WindowEvent::GET_FOCUS;
        }
        if (m_window) m_window(wevent);
    }
    void InputXorg::send_window_move_event(double x, double y)
    {
        if (m_window_move) m_window_move(x, y);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // GAMEPADS (evdev)
    namespace AuxGamepads
    {
        //a bit of the bits of a device (EVIOCGBIT)
        static bool has_bit(const unsigned long* bits, int bit)
        {
            const int size = int(sizeof(unsigned long) * 8);
            return (bits[bit / size] >> (bit % size)) & 1UL;
        }

        //its keys as the buttons of a gamepad (the layout of xpad: BTN_X the left one)
        struct ButtonOf
        {
            int           code;
            GamepadButton button;
        };
        static constexpr ButtonOf s_buttons[] =
        {
            { BTN_A,          GAMEPAD_A              },
            { BTN_B,          GAMEPAD_B              },
            { BTN_X,          GAMEPAD_X              },
            { BTN_Y,          GAMEPAD_Y              },
            { BTN_TL,         GAMEPAD_LEFT_SHOULDER  },
            { BTN_TR,         GAMEPAD_RIGHT_SHOULDER },
            { BTN_TL2,        GAMEPAD_LEFT_TRIGGER   },
            { BTN_TR2,        GAMEPAD_RIGHT_TRIGGER  },
            { BTN_SELECT,     GAMEPAD_BACK           },
            { BTN_START,      GAMEPAD_START          },
            { BTN_THUMBL,     GAMEPAD_LEFT_THUMB     },
            { BTN_THUMBR,     GAMEPAD_RIGHT_THUMB    },
            { BTN_DPAD_UP,    GAMEPAD_DPAD_UP        },
            { BTN_DPAD_DOWN,  GAMEPAD_DPAD_DOWN      },
            { BTN_DPAD_LEFT,  GAMEPAD_DPAD_LEFT      },
            { BTN_DPAD_RIGHT, GAMEPAD_DPAD_RIGHT     },
        };

        //its axes looked at (their range read when it opens)
        static constexpr int s_axes[] = { ABS_X, ABS_Y, ABS_Z, ABS_RX, ABS_RY, ABS_RZ, ABS_HAT0X, ABS_HAT0Y };

        //a raw axis from -1 to 1 (a trigger: 0 to 1, its min at 0)
        static float share(const InputXorg::GamepadDevice& device, int axis)
        {
            const float range = device.max[size_t(axis)] - device.min[size_t(axis)];
            if (range <= 0.0f)
            {
                return 0.0f;
            }
            return ((device.raw[size_t(axis)] - device.min[size_t(axis)]) / range) * 2.0f - 1.0f;
        }

        static constexpr float s_stick_dead = 0.15f;
        static constexpr float s_trigger_dead = 0.05f;
    }

    InputXorg::~InputXorg()
    {
        for (GamepadDevice& device : m_gamepad_devices)
        {
            if (device.fd >= 0)
            {
                close(device.fd);
            }
        }
    }

    void InputXorg::gamepads_update()
    {
        using namespace AuxGamepads;
        //the devices plugged in: the ones with the buttons of a gamepad, not open yet
        const auto now = std::chrono::steady_clock::now();
        if (!m_gamepad_scanned || now - m_gamepad_scan >= std::chrono::seconds(2))
        {
            m_gamepad_scanned = true;
            m_gamepad_scan = now;
            for (int event = 0; event != 32; ++event)
            {
                const std::string path = "/dev/input/event" + std::to_string(event);
                bool open_yet = false;
                int free_pad = -1;
                for (int pad = 0; pad != GAMEPAD_MAX; ++pad)
                {
                    open_yet = open_yet || (m_gamepad_devices[size_t(pad)].fd >= 0 && m_gamepad_devices[size_t(pad)].path == path);
                    if (free_pad < 0 && m_gamepad_devices[size_t(pad)].fd < 0)
                    {
                        free_pad = pad;
                    }
                }
                if (open_yet || free_pad < 0)
                {
                    continue;
                }
                const int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
                if (fd < 0)
                {
                    continue;
                }
                unsigned long keys[(KEY_MAX + 1) / (sizeof(unsigned long) * 8) + 1] = {};
                const bool pad = ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keys)), keys) >= 0 && has_bit(keys, BTN_GAMEPAD);
                if (!pad)
                {
                    close(fd);
                    continue;
                }
                GamepadDevice& device = m_gamepad_devices[size_t(free_pad)];
                device = GamepadDevice();
                device.fd = fd;
                device.path = path;
                for (int axis : s_axes)
                {
                    input_absinfo info{};
                    if (ioctl(fd, EVIOCGABS(axis), &info) >= 0)
                    {
                        device.min[size_t(axis)] = float(info.minimum);
                        device.max[size_t(axis)] = float(info.maximum);
                        device.raw[size_t(axis)] = float(info.value);
                    }
                }
                m_gamepad_states[size_t(free_pad)] = GamepadState();
                m_gamepad_states[size_t(free_pad)].connected = true;
            }
        }
        //their events: its keys, its axes (gone: closed, not connected)
        for (int pad = 0; pad != GAMEPAD_MAX; ++pad)
        {
            GamepadDevice& device = m_gamepad_devices[size_t(pad)];
            GamepadState& state = m_gamepad_states[size_t(pad)];
            if (device.fd >= 0)
            {
                input_event event{};
                ssize_t got = 0;
                while ((got = read(device.fd, &event, sizeof(event))) == ssize_t(sizeof(event)))
                {
                    if (event.type == EV_KEY)
                    {
                        for (const ButtonOf& button : s_buttons)
                        {
                            if (button.code == event.code)
                            {
                                state.buttons[size_t(button.button)] = event.value != 0;
                            }
                        }
                    }
                    else if (event.type == EV_ABS && event.code < device.raw.size())
                    {
                        device.raw[event.code] = float(event.value);
                    }
                }
                if (got < 0 && errno != EAGAIN)
                {
                    close(device.fd);
                    device = GamepadDevice();
                    state = GamepadState();
                }
                else
                {
                    //its sticks (y up: evdev down), its triggers, its hat as its d-pad
                    gamepad_stick(share(device, ABS_X), -share(device, ABS_Y), s_stick_dead, state.axes[GAMEPAD_LEFT_X], state.axes[GAMEPAD_LEFT_Y]);
                    gamepad_stick(share(device, ABS_RX), -share(device, ABS_RY), s_stick_dead, state.axes[GAMEPAD_RIGHT_X], state.axes[GAMEPAD_RIGHT_Y]);
                    state.axes[GAMEPAD_LEFT_TRIGGER_AXIS] = gamepad_trigger(share(device, ABS_Z) * 0.5f + 0.5f, s_trigger_dead);
                    state.axes[GAMEPAD_RIGHT_TRIGGER_AXIS] = gamepad_trigger(share(device, ABS_RZ) * 0.5f + 0.5f, s_trigger_dead);
                    if (device.max[ABS_HAT0X] > device.min[ABS_HAT0X])
                    {
                        state.buttons[GAMEPAD_DPAD_LEFT] = device.raw[ABS_HAT0X] < 0.0f;
                        state.buttons[GAMEPAD_DPAD_RIGHT] = device.raw[ABS_HAT0X] > 0.0f;
                        state.buttons[GAMEPAD_DPAD_UP] = device.raw[ABS_HAT0Y] < 0.0f;
                        state.buttons[GAMEPAD_DPAD_DOWN] = device.raw[ABS_HAT0Y] > 0.0f;
                    }
                }
            }
            //(no focus: as nothing held)
            GamepadState sent = state;
            if (!m_focus)
            {
                sent = GamepadState();
                sent.connected = state.connected;
            }
            m_gamepads.send(pad, sent);
        }
    }
}
}
}
