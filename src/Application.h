#pragma once
#include <windows.h>

bool isVista();
bool InitApplication(HINSTANCE hInstance);

class Application
{
public:
    Application();
    ~Application();

    int exec();
    static Application* getInstance();

    HINSTANCE getInstanceWin();

    bool IsElevated();

private:
    void elevated();
    int init();

private:
    HINSTANCE   _instance;
    bool _isElevated = false;
};

