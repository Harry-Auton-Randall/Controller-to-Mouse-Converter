#include <iostream>
#include <string>

#include <chrono> //for delays
#include <thread>

#include <Windows.h> //keyboard/controller inputs
#include <Xinput.h>
//#pragma comment(lib, "Xinput.lib")

#include <algorithm> //clamp

void helpText()
{
    std::cout << 
    "--CONTROLS--\n\n"
    "(Controller)\n"
    "- Right stick = move\n"
    "- Right bumper = left click\n"
    "- Right trigger = right click\n"
    "- B button = scroll up\n"
    "- A button = scroll up\n"
    "- Y button = scroll wheel click\n"
    "(Keyboard)\n"
    "- O = options\n"
    "- H = help\n"
    "- E = exit\n"
    "----------------\n"
    ;
}

void optionsMenu(int* sensPtr, int* framePtr)
{
    
}

int main() {
    using namespace std::this_thread; // sleep_for, sleep_until
    using namespace std::chrono; // nanoseconds, system_clock, seconds
    auto nextFrame = steady_clock::now();

    int framerateDefault = 144;
    int framerate = framerateDefault;
    int* frameratePtr = &framerate;
    int sensitivityDefault = 1200;
    int sensitivity = sensitivityDefault;
    int* sensitivityPtr = &sensitivity;

    int maxScrollSpeedDefault = 20;
    int maxScrollSpeed = maxScrollSpeedDefault;
    int timeSinceLastScroll = 0;
    //int screenX = GetSystemMetrics(SM_CXSCREEN);
    //int screenY = GetSystemMetrics(SM_CYSCREEN);

    float unmovedDistX = 0;
    float unmovedDistY = 0;
    POINT cursorPoint;

    XINPUT_STATE state;
    DWORD result;
    float stickX = 0;
    float stickY = 0;
    bool rbPressed = false;
    bool rtPressed = false;
    bool rbPrev = false;
    bool rtPrev = false;

    bool aPressed = false;
    bool bPressed = false;
    bool yPressed = false;
    bool aPrev = false;
    bool bPrev = false;
    bool yPrev = false;
    int aPressTime = 0;
    int bPressTime = 0;

    //clicking
    INPUT inputL = {};
    INPUT inputR = {};
    INPUT inputM = {};
    inputL.type = INPUT_MOUSE;
    inputR.type = INPUT_MOUSE;
    inputM.type = INPUT_MOUSE;

    //scrolling
    INPUT inputSD = {};
    INPUT inputSU = {};
    inputSD.type = INPUT_MOUSE;
    inputSU.type = INPUT_MOUSE;

    inputSU.mi.dwFlags = MOUSEEVENTF_WHEEL;
    inputSU.mi.mouseData = WHEEL_DELTA;
    inputSD.mi.dwFlags = MOUSEEVENTF_WHEEL;
    inputSD.mi.mouseData = -WHEEL_DELTA;

    //key presses
    bool hPressed = false;
    bool hPrev = false;

    helpText();
    

    while (true)
    {
        nextFrame += nanoseconds(1000000000 / framerate);

        //checks for keyboard presses
        if(GetKeyState('H') & 0x8000)
        {
            if (!hPressed)
            {
                helpText();
            }
            hPressed = true;
        }
        else
        {
            hPressed = false;
        
            if(GetKeyState('O') & 0x8000)
            {
                optionsMenu(sensitivityPtr, frameratePtr);
            }
            else if(GetKeyState('E') & 0x8000)
            {
                std::cout << "Exit button pressed.\n";
                break;
            }
        }


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
        aPressed = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A);
        bPressed = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B);
        yPressed = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y);

        //Moves mouse cursor
        if ((stickX * stickX) + (stickY * stickY) >= 0.15f * 0.15f)
        {
            GetCursorPos(&cursorPoint);
            unmovedDistX += (stickX * (static_cast<float>(sensitivity) / framerate));
            unmovedDistY -= (stickY * (static_cast<float>(sensitivity) / framerate));
            SetCursorPos(cursorPoint.x + unmovedDistX, cursorPoint.y + unmovedDistY);
            unmovedDistX -= static_cast<int>(unmovedDistX);
            unmovedDistY -= static_cast<int>(unmovedDistY);
        }

        //Sorts out mouse clicking, only sends inputs when controller inputs change
        inputL.mi.dwFlags = rbPressed ? MOUSEEVENTF_LEFTDOWN: MOUSEEVENTF_LEFTUP;
        inputR.mi.dwFlags = rtPressed ? MOUSEEVENTF_RIGHTDOWN: MOUSEEVENTF_RIGHTUP;
        inputM.mi.dwFlags = yPressed ? MOUSEEVENTF_MIDDLEDOWN: MOUSEEVENTF_MIDDLEUP;
        if (rbPressed != rbPrev)
        {
            SendInput(1, &inputL, sizeof(INPUT));
        }
        if (rtPressed != rtPrev)
        {
            SendInput(1, &inputR, sizeof(INPUT));
        }
        if (yPressed != yPrev)
        {
            SendInput(1, &inputM, sizeof(INPUT));
        }

        //mouse scrolling
        if (aPressed)
        {
            if (bPressTime > (framerate * 0.6f) && !aPrev)
            {
                aPressTime = bPressTime;
            }
            aPressTime += 1;

            if (!aPrev)
            {
                SendInput(1, &inputSD, sizeof(INPUT));
            }
        }
        else {aPressTime = 0;}

        if (bPressed)
        {
            if (aPressTime > (framerate * 0.6f) && !bPrev)
            {
                bPressTime = aPressTime;
            }
            else {bPressTime += 1;}

            if (!bPrev)
            {
                SendInput(1, &inputSU, sizeof(INPUT));
            }
        }
        else {bPressTime = 0;}

        //If either scroll button is held down long enough, it'll begin scrolling automatically
        //Keeps track of how much time has passed since the last scroll, and uses the while loop
        //to trigger as much scrolling as necessary
        //(helps framerate independence, including when the scroll speed is faster than the framerate)
        if (aPressTime > (framerate * 0.6f) || bPressTime > (framerate * 0.6f))
        {
            while (timeSinceLastScroll >= 1000000000 / maxScrollSpeed)
            {
                if (aPressed && bPressed) {}
                else if (aPressed) {SendInput(1, &inputSD, sizeof(INPUT));}
                else {SendInput(1, &inputSU, sizeof(INPUT));}
                timeSinceLastScroll -= 1000000000 / maxScrollSpeed;
            }
            timeSinceLastScroll += 1000000000 / framerate;
        }
        else
        {
            timeSinceLastScroll = 1000000000 / maxScrollSpeed;
        }

        rbPrev = rbPressed;
        rtPrev = rtPressed;
        aPrev = aPressed;
        bPrev = bPressed;
        yPrev = yPressed;
        hPrev = hPressed;
        sleep_until(nextFrame);
    }

    std::cout << "Exiting in 3 seconds...";
    sleep_for(seconds(3));
    return 0;

}