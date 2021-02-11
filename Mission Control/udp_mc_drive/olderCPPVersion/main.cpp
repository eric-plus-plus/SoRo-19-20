#include <QCoreApplication>
#include "comms.h"
#include <iostream>
#include "gamepadmonitor.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    GamepadMonitor drivepad;

    return a.exec();
}
