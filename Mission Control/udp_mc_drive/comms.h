#ifndef SOCKET_H
#define SOCKET_H

#include <QObject>
#include <QUdpSocket>
#include <QFile>

class comms : public QObject
{
    Q_OBJECT
public:
    explicit comms(const char* config_file, QObject *parent = nullptr);
    void sendMessage(QByteArray message);
    bool isValid() { return valid; }
    QString getError() { return errorString; }

signals:
        void messageReady(QByteArray message); // emitted whenever a message is recieved
        void errorEncountered(QString errorMessage); // emitted if setup is a failure

private:
    QString send_ip = nullptr;
    quint16 send_port = 0;
    QString recieve_ip = nullptr;
    quint16 recieve_port = 0;
    QUdpSocket* udpSocket;
    bool readConfig(const char* filename); // returns true on success, false on failure
    bool valid = true; // validity detemined in constructor, is immutable

    QString errorString = "No errors to report";

private slots:
    void readMessage();
};

#endif // SOCKET_H
