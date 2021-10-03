#pragma once
#include <windows.h>

bool isVista();
bool InitApplication(HINSTANCE hInstance);

class Application
{
public:
    Application();
    ~Application();

    void init();
    void run();
    static Application* getInstance();

    HINSTANCE getInstanceWin();

    bool IsElevated();

private:
    void elevated();
private:
    HINSTANCE   _instance;
    bool _isElevated = false;
};

