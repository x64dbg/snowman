#ifndef QTPLUGIN_H
#define QTPLUGIN_H

#include "pluginmain.h"

namespace QtPlugin
{
    void Init();
    void Setup();
    void WaitForSetup();
    void Stop();
    void WaitForStop();
    void ShowTab();
    void MenuPrepare(int hMenu);
    void MenuEntry(int hMenu);
} //QtPlugin

#endif // QTPLUGIN_H
