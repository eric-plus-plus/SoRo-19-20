#ifndef SOCKET_H
#define SOCKET_H

#include <QObject>
#include <QUdpSocket>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QList>
#include <QTimer>
#include <stdio.h>

// struct to hold info about what data (from a serial device) goes to what computer (udp stuff)
struct data_path {
    int deviceId;
    QString address;
    int port;
};

inline bool operator ==(const data_path &dp1, const data_path &dp2)
{
    return dp1.address == dp2.address && dp1.port == dp2.port && dp1.deviceId == dp2.deviceId;
}

class socket : public QObject
{
    Q_OBJECT
public:
    FILE *dataFile;
    explicit socket(QObject *parent = nullptr);
    void sendUDP(QByteArray data);
    void sendUDP(const char* message);
    void sendSerial(QSerialPort* port, QByteArray message);
    void sendSerial(QSerialPort* port, const char* message);
    void sendSerialAll(QByteArray message);
    void sendSerialAll(const char* message);

signals:

public slots:
    void readUDP();
    void readSerial();
    void zeroDevices();

private:
    QUdpSocket* socketIn;
    QUdpSocket* socketInAuto;
    QUdpSocket* socketOut;

    QTimer* signalTimer;

    QSerialPortInfo serialInfo;
    QList<QSerialPortInfo> serialList;
    QList<QSerialPort*> serialPorts;
    QList<int> serialIds;
    QList<data_path> dataPaths;
};

#endif // SOCKET_H
