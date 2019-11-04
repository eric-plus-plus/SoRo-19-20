#include "gamepadmonitor.h"
#include <cmath>
#include <QtGamepad/QGamepad>
#include <QDebug>
#include <stdio.h>


GamepadMonitor::GamepadMonitor( QObject *parent)
    : QObject(parent)
    , m_gamepad(nullptr)
{
    // make sure gamepads are connected
    auto gamepads = QGamepadManager::instance()->connectedGamepads();
    if (gamepads.isEmpty()) {
        errorString = "No gamepads detected";
        qDebug() << errorString;
        emit errorEncountered(errorString);
        return;
    }

    // attempt to set up communication ip's and stuff
    myComms = new comms("udp_mc_drive.conf");
    if(!myComms->isValid())
    {
        errorString = "Communications - " + myComms->getError();
        qDebug() << errorString;
        emit errorEncountered(errorString);
        return;
    }

    // set up signals and slots to react to changes in xbox controller buttons/joysticks
    m_gamepad = new QGamepad(*gamepads.begin(), this);

    connect(m_gamepad, SIGNAL(axisLeftXChanged(double)), this, SLOT(onLXAxis(double)));
    connect(m_gamepad, SIGNAL(axisLeftYChanged(double)), this, SLOT(onLYAxis(double)));
    connect(m_gamepad, SIGNAL(axisRightXChanged(double)), this, SLOT(onRXAxis(double)));
    connect(m_gamepad, SIGNAL(axisRightYChanged(double)), this, SLOT(onRYAxis(double)));

    connect(m_gamepad, SIGNAL(buttonR2Changed(double)), this, SLOT(onTriggerR(double)));
    connect(m_gamepad, SIGNAL(buttonL2Changed(double)), this, SLOT(onTriggerL(double)));

    connect(m_gamepad, SIGNAL(buttonR1Changed(bool)), this, SLOT(onButtonRBumper(bool)));
    connect(m_gamepad, SIGNAL(buttonL1Changed(bool)), this, SLOT(onButtonLBumper(bool)));

    connect(m_gamepad, SIGNAL(buttonUpChanged(bool)), this, SLOT(onButtonUp(bool)));
    connect(m_gamepad, SIGNAL(buttonDownChanged(bool)), this, SLOT(onButtonDown(bool)));
    connect(m_gamepad, SIGNAL(buttonLeftChanged(bool)), this, SLOT(onButtonLeft(bool)));
    connect(m_gamepad, SIGNAL(buttonRightChanged(bool)), this, SLOT(onButtonRight(bool)));


    // set up a timer to go off every 100 ms, and when that timer goes off send control data to rover
    udpTimer = new QTimer();
    connect(udpTimer, SIGNAL(timeout()), this, SLOT(sendUDP()));
    udpTimer->start(100);
}

void GamepadMonitor::onLXAxis(double value){
    lxAxis = value;
}

void GamepadMonitor::onLYAxis(double value){
    lyAxis = value;
}

void GamepadMonitor::onRYAxis(double value) {
    ryAxis = value;
}

void GamepadMonitor::onRXAxis(double value) {
    rxAxis = value;
}

void GamepadMonitor::onButtonLBumper(bool pressed) {
    // toggle skid steering mode on button down
    if(pressed) {
        skidToggle = !skidToggle;

        if(skidToggle)
            qDebug() << "Steering mode: Skid control";
        else
            qDebug() << "Steering mode: One stick";
    }
}

void GamepadMonitor::onButtonRBumper(bool pressed) {
    // toggle break on button down
    if(pressed) {
         modifiers ^= (1 << 0);

         if( modifiers & (1 << 0) )
             qDebug() << "Break enabled";
         else
             qDebug() << "Break disabled";
    }

}

void GamepadMonitor::onTriggerL(double value) {
    lTrigger = value;
}

void GamepadMonitor::onTriggerR(double value) {
    rTrigger = value;
}

void GamepadMonitor::onButtonUp(bool pressed) {
    buttonUp = pressed;
}

void GamepadMonitor::onButtonDown(bool pressed) {
    buttonDown = pressed;
}

void GamepadMonitor::onButtonLeft(bool pressed) {
    buttonLeft = pressed;
}

void GamepadMonitor::onButtonRight(bool pressed) {
    buttonRight = pressed;
}

void GamepadMonitor::sendUDP() {
    // slower control of gimble with D-Pad
    gimbleUD = 0;
    gimbleRL = 0;
    if(buttonUp)
        gimbleUD += 1;
    if(buttonDown)
        gimbleUD -= 1;
    if(buttonLeft)
        gimbleRL += 1;
    if(buttonRight)
        gimbleRL -= 1;

    // control wheel speeds and gimble with joysticks
    if(!skidToggle)
    {
        // default (one stick) steering mode with gimble on right stick:

        // joystick values will be between -1 and 1
        // if your joystick characteristic equation is f(x) (has to have points (0,0) and (1,1) i.e. x^2),
        // then to include the deadzone you use: f( (x-deadzone) / (1-deadzone) )

        double deadZone = 0.05;
        // wheels: x axis deadzone
        double scaledX = 0;
        if (lxAxis > deadZone) {
            scaledX = (lxAxis-deadZone)/(1.0-deadZone);
        }
        else if (lxAxis < -deadZone) {
            scaledX = (lxAxis+deadZone)/(1.0-deadZone);
        }
        // wheels: y axis deadzone
        double scaledY = 0;
        if (lyAxis > deadZone ) {
            scaledY = (lyAxis-deadZone)/(1.0-deadZone);
        }
        else if (lyAxis < -deadZone) {
            scaledY = (lyAxis+deadZone)/(1.0-deadZone);
        }
        // wheels: mix values
        // the code below is from https://www.impulseadventure.com/elec/robot-differential-steering.html
        double fPivYLimit = 0.5; // The threshold at which the pivot action starts
        double premixLeftSide; // Motor (left)  premixed output
        double premixRightSide; // Motor (right) premixed output
        double nPivSpeed; // Pivot Speed
        double fPivScale; // Balance scale b/w drive and pivot

        // Calculate Drive Turn output due to Joystick X input
        if (scaledY <= 0) {
          // Forward
          premixLeftSide = (scaledX >= 0) ? 1.0 : (1.0 + scaledX);
          premixRightSide = (scaledX >= 0) ? (1.0 - scaledX) : 1.0;
        } else {
          // Reverse
          premixLeftSide = (scaledX >= 0) ? (1.0 - scaledX) : 1.0;
          premixRightSide = (scaledX >= 0 ) ? 1.0 : (1.0 + scaledX);
        }
        // Scale Drive output due to Joystick Y input (throttle)
        premixLeftSide = premixLeftSide * (-scaledY);
        premixRightSide = premixRightSide * (-scaledY);
        // Now calculate pivot amount
        // - Strength of pivot (nPivSpeed) based on Joystick X input
        // - Blending of pivot vs drive (fPivScale) based on Joystick Y input
        nPivSpeed = scaledX;
        fPivScale = (abs(scaledY)>fPivYLimit)? 0.0 : (1.0 - abs(scaledY));
        // Calculate final mix of Drive and Pivot
        double mixLeftSide = (1.0-fPivScale)*premixLeftSide + fPivScale*( nPivSpeed);
        double mixRightSide = (1.0-fPivScale)*premixRightSide + fPivScale*(-nPivSpeed);
        // scale to motor range
        leftSide = static_cast<qint8>(mixLeftSide*90.0);
        rightSide = static_cast<qint8>(mixRightSide*90.0);

        // gimble: vertical axis
        if (ryAxis > deadZone ) {
            double x = (ryAxis-deadZone)/(1.0-deadZone);
            gimbleUD = static_cast<qint8>(round(x * 5.0));
        }
        else if (ryAxis < -deadZone) {
            double x = (ryAxis+deadZone)/(1.0-deadZone);
            gimbleUD = static_cast<qint8>(round(x * 5.0));
        }
        // gimble: horizontal axis
        if (rxAxis > deadZone ) {
            double x = (rxAxis-deadZone)/(1.0-deadZone);
            gimbleRL = static_cast<qint8>(round(x * 5.0));
        }
        else if (rxAxis < -deadZone) {
            double x = (rxAxis+deadZone)/(1.0-deadZone);
            gimbleRL = static_cast<qint8>(round(x * 5.0));
        }
    }
    else
    {
        // skid steering mode:

        // joystick values will be between -1 and 1
        // if your joystick characteristic equation is f(x) (has to have points (0,0) and (1,1) i.e. x^2),
        // then to include the deadzone you use: f( (x-deadzone) / (1-deadzone) )

        // left wheels
        double deadZone = 0.05;
        if (lyAxis > deadZone) {
            double x = (lyAxis-deadZone)/(1.0-deadZone);
            leftSide = static_cast<qint8>(round(-x * 90.0));
        }
        else if (lyAxis < -deadZone) {
            double x = (lyAxis+deadZone)/(1.0-deadZone);
            leftSide = static_cast<qint8>(round(-x * 90.0));
        }
        else {
            leftSide = 0;
        }
        // right wheels
        if (ryAxis > deadZone) {
            double x = (ryAxis-deadZone)/(1.0-deadZone);
            rightSide = static_cast<qint8>(round(-x * 90.0));
        }
        else if (ryAxis < -deadZone) {
            double x = (ryAxis+deadZone)/(1.0-deadZone);
            rightSide = static_cast<qint8>(round(-x * 90.0));
        }
        else {
            rightSide = 0;
        }
    }

    // hold left trigger for rear modifier
    if(lTrigger > 0.5)
        modifiers |= (1 << 2);
    else
        modifiers &= ~(1 << 2);

    // hold right trigger for front modifier
    if(rTrigger > 0.5)
        modifiers |= (1 << 1);
    else
        modifiers &= ~(1 << 1);

    // build the drive control message
    QByteArray out;
    out.append(char(-127));
    out.append(char(0));
    out.append(modifiers);
    out.append(leftSide);
    out.append(rightSide);
    out.append(gimbleUD);
    out.append(gimbleRL);
    qint8 hash = (modifiers + leftSide + rightSide + gimbleUD + gimbleRL)/5;
    out.append(hash);
    // send the drive control message
    myComms->sendMessage(out);
    printVals();
}

void GamepadMonitor::printVals() {
    // just for debugging
    int hash = (modifiers + leftSide + rightSide +gimbleUD + gimbleRL)/5;
    printf("%c, %d, %d, %d, %d, %d, %d, %d\n", char(-127), 0, modifiers, leftSide, rightSide, gimbleUD, gimbleRL, hash);
}

GamepadMonitor::~GamepadMonitor() {
    delete m_gamepad;
}

