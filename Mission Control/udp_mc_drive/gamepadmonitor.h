#ifndef GAMEPADMONITOR_H
#define GAMEPADMONITOR_H

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include "comms.h"

QT_BEGIN_NAMESPACE
class QGamepad;
QT_END_NAMESPACE

class GamepadMonitor : public QObject
{
    Q_OBJECT
public:
    explicit GamepadMonitor(QObject *parent = nullptr);
    ~GamepadMonitor();

signals:
    void errorEncountered(QString errorMessage); // emitted if setup of this or its subobject(s) fails

private:
    QString errorString = "No errors to report";
    QGamepad *m_gamepad;
    comms *myComms;
    QTimer *udpTimer;
    bool skidToggle = false;
    // bytes that are actally sent
    qint8 modifiers = 0;
    qint8 leftSide = 0;
    qint8 rightSide = 0;
    qint8 gimbleUD = 0;
    qint8 gimbleRL = 0;
    // variables that hold data from gamepad
    double lxAxis = 0; // left x axis
    double lyAxis = 0;
    double rxAxis = 0;
    double ryAxis = 0;
    double lTrigger = 0;
    double rTrigger = 0;
    bool buttonUp = false;
    bool buttonDown = false;
    bool buttonLeft = false;
    bool buttonRight = false;
    bool buttonRBumper = false;
    bool buttonLBumper = false;
    void printVals();

public slots:
    // these slots update values of the controller and handle toggles
    void onLXAxis(double value);
    void onLYAxis(double value);
    void onRXAxis(double value);
    void onRYAxis(double value);
    void onButtonRBumper(bool pressed);
    void onButtonLBumper(bool pressed);
    void onTriggerL(double value);
    void onTriggerR(double value);
    void onButtonUp(bool pressed);
    void onButtonDown(bool pressed);
    void onButtonLeft(bool pressed);
    void onButtonRight(bool pressed);
    // sendUDP actually calculates all the data to send over
    void sendUDP(); // connects to a QTimer timeout, sends drive control message to rover
};

#endif // GAMEPADMONITOR_H
