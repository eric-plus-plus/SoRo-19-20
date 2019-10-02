#include "comms.h"

comms::comms(const char* config_file, QObject *parent) : QObject(parent)
{
    // try to read config file for ip parameters
    if(!readConfig(config_file))
    {
        valid = false;
        emit errorEncountered(errorString);
        return;
    }
    // create udp socket and try to bind
    udpSocket = new QUdpSocket(this);
    if(udpSocket->bind(QHostAddress(recieve_ip), recieve_port))
    {
        qDebug() << "Bound to" << recieve_ip << ":" << recieve_port;
        connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));
    }
    else
    {
        errorString = "Can't bind to " + recieve_ip + ":" + QString::number(recieve_port) + " - " + udpSocket->errorString();
        //qDebug() << errorString;
        valid = false;
        emit errorEncountered(errorString);
        return;
    }
}

void comms::sendMessage(QByteArray Data)
{
    udpSocket->writeDatagram(Data, QHostAddress(send_ip), send_port);
}

void comms::readMessage()
{
    // when data comes in
    QByteArray buffer;
    buffer.resize(static_cast<int>(udpSocket->pendingDatagramSize()));
    QHostAddress sender;
    quint16 senderPort;

    udpSocket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

    // debugging stuff. We probably won't use the sender and sender port, but it's here if we need it
    qDebug() << "Message from: " << sender.toString();
    qDebug() << "Message port: " << senderPort;
    qDebug() << "Message: " << buffer;

    emit(messageReady(buffer));
}

bool comms::readConfig(const char* filename)
{
    // instead of standard c++ ifstream and std::string, we have QFile, QTextStream, and QString
    QFile config_file(filename);
    QTextStream config_stream(&config_file);
    QString line;

    // check that file exists
    if(!config_file.exists())
    {
        errorString = "Config file not found";
        //qDebug() << errorString;
        return false;
    }

    // open file, if possible
    if(!config_file.open(QIODevice::ReadOnly))
    {
        errorString = "Config file can't be opened";
        //qDebug() << errorString;
        return false;
    }

    // read config file line by line
    while(!config_stream.atEnd())
    {
        line = config_stream.readLine();
        QStringList fields = line.split("=");

        if(QString::compare(fields[0], "sendip", Qt::CaseInsensitive) == 0)
        {
            send_ip = fields[1];
            qDebug() << fields[0] << "set to" << send_ip;
        }
        else if(QString::compare(fields[0], "sendport", Qt::CaseInsensitive) == 0)
        {
            send_port = fields[1].toUShort();
            qDebug() << fields[0] << "set to" << send_port;
        }
        else if(QString::compare(fields[0], "recieveip", Qt::CaseInsensitive) == 0)
        {
            recieve_ip = fields[1];
            qDebug() << fields[0] << "set to" << recieve_ip;
        }
        else if(QString::compare(fields[0], "recieveport", Qt::CaseInsensitive) == 0)
        {
            recieve_port = fields[1].toUShort();
            qDebug() << fields[0] << "set to" << recieve_port;
        }
    }

    config_file.close();

    if(recieve_ip == nullptr || recieve_port == 0 || send_ip == nullptr || send_port == 0)
    {
        errorString = "Config missing info.";
        //qDebug() << errorString;
        return false;
    }

    return true;
}
