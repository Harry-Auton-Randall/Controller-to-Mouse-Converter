#include <iostream>
#include <string>

#include <chrono> //for delays
#include <thread>

#include <Windows.h> //keyboard/controller inputs
#include <Xinput.h>
//#pragma comment(lib, "Xinput.lib")

#include <algorithm> //clamp

int main() {
    using namespace std::this_thread; // sleep_for, sleep_until
    using namespace std::chrono; // nanoseconds, system_clock, seconds

    int framerateDefault = 144;
    int framerate = framerateDefault;
    int sensitivityDefault = 1500;
    int sensitivity = sensitivityDefault;

    int screenX = GetSystemMetrics(SM_CXSCREEN);
    int screenY = GetSystemMetrics(SM_CYSCREEN);

    float cursorTargetX = screenX / 2;
    float cursorTargetY = screenY / 2;

    XINPUT_STATE state;
    DWORD result;
    float stickX = 0;
    float stickY = 0;
    bool rbPressed = false;
    bool rtPressed = false;
    bool rbPrev = false;
    bool rtPrev = false;

    INPUT inputL = {};
    INPUT inputR = {};
    inputL.type = INPUT_MOUSE;
    
    inputR.type = INPUT_MOUSE;

    std::cout << 
    "--CONTROLS--\n\n"
    //"(Controller)\n"
    "- Right stick = move\n"
    "- Right bumper = left click\n"
    "- Right trigger = right click\n"
    //"(Keyboard)\n"
    //"- S = settings\n"
    //"- E = exit"
    ;

    while (true)
    {
        //Get inputs from controller (verify's if it's connected, then stick, then bumper/trigger)
        result = XInputGetState(0, &state);
        if (result != ERROR_SUCCESS){
            std::cout << "Controller is unplugged.\n";
            break;
        }

        stickX = state.Gamepad.sThumbRX / 32767.0f;
        stickY = state.Gamepad.sThumbRY / 32767.0f;
        //std::cout << (std::to_string(stickX) + ", ");
        //std::cout << (std::to_string(stickY) + "\n");

        rbPressed = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
        rtPressed = (state.Gamepad.bRightTrigger) > 127;

        //Moves mouse cursor
        if ((stickX * stickX) + (stickY * stickY) >= 0.15f * 0.15f)
        {
            cursorTargetX += (stickX * (static_cast<float>(sensitivity) / framerate));
            cursorTargetY -= (stickY * (static_cast<float>(sensitivity) / framerate));
        }
        cursorTargetX = std::clamp(cursorTargetX, 0.0f, static_cast<float>(screenX));
        cursorTargetY = std::clamp(cursorTargetY, 0.0f, static_cast<float>(screenY));
        SetCursorPos(cursorTargetX, cursorTargetY);

        //Sorts out mouse clicking, only sends inputs when controller inputs change
        inputL.mi.dwFlags = rbPressed ? MOUSEEVENTF_LEFTDOWN: MOUSEEVENTF_LEFTUP;
        inputR.mi.dwFlags = rtPressed ? MOUSEEVENTF_RIGHTDOWN: MOUSEEVENTF_RIGHTUP;
        if (rbPressed != rbPrev)
        {
            SendInput(1, &inputL, sizeof(INPUT));
        }
        if (rtPressed != rtPrev)
        {
            SendInput(1, &inputR, sizeof(INPUT));
        }

        sleep_for(nanoseconds(1000000000 / framerate));
        rbPrev = rbPressed;
        rtPrev = rtPressed;
    }

    std::cout << "Exiting in 3 seconds...";
    sleep_for(seconds(3));

}