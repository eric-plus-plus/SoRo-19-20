#include <QCoreApplication>
#include "socket.h"
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    socket mySocket;

    return a.exec();
}
