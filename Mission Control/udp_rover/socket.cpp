#include "socket.h"
// my laptop is 10.0.0.3 (WIFI)
// soro desktop is 192.168.1.100

socket::socket(QObject *parent) : QObject(parent)
{
    /*
    char tmp[10];
    // start the file
    for(int i = 0; i < 40; i++){
        sprintf(tmp, "dataFile%d.csv",i);
        if(!fopen(tmp,"r")){
            //create a new file
            dataFile = fopen(tmp,"w");
            break;
        }
    }
    */
    // timer to handle udp signal loss
    signalTimer = new QTimer();
    signalTimer->start(2000);
    connect(signalTimer, SIGNAL(timeout()), this, SLOT(zeroDevices()));

    // serial ports: create a serial object for every available port, and add to a QList
    serialList = serialInfo.availablePorts();
    for(int i = 0; i < serialList.size(); i++)
    {
        QSerialPort* temp = new QSerialPort();
        temp->setPortName(serialList.at(i).portName());
        temp->setBaudRate(QSerialPort::Baud9600);

        if(temp->open(QIODevice::ReadWrite))
        {
            qDebug() << "Opened serial port" << serialList.at(i).portName();
            serialPorts.append(temp);
            serialIds.append(69); // this number will actually not be a device id
            connect(temp, SIGNAL(readyRead()), this, SLOT(readSerial()));
        } else {
            qDebug() << "Error opening serial port:" << temp->errorString();
        }
    }

    // udp sockets
    socketIn = new QUdpSocket(this);
    //socketOut = new QUdpSocket(this);
    if(socketIn->bind(QHostAddress("127.0.0.1"), 1237))//"192.168.1.102"), 1234))
    {
        qDebug() << "Bound to port 1237";
    } else {
        qDebug() << "Error binding to port:" << socketIn->errorString();
    }
    connect(socketIn, SIGNAL(readyRead()), this, SLOT(readUDP()));
}

void socket::sendUDP(const char* message) // ******** CURRENTLY UNUSED
{
    QByteArray Data;
    Data.append(message);

    // qint64 QUdpSocket::writeDatagram(const QByteArray & datagram,
    //                      const QHostAddress & host, quint16 port)
    socketIn->writeDatagram(Data, QHostAddress("10.0.0.2"), 1237);//"192.168.1.103"), 1237);
}

void socket::sendUDP(QByteArray data) // ******** CURRENTLY UNUSED
{
    // qint64 QUdpSocket::writeDatagram(const QByteArray & datagram,
    //                      const QHostAddress & host, quint16 port)
    socketIn->writeDatagram(data, QHostAddress("10.0.0.2"), 1237);//"192.168.1.103"), 1237);
}

void socket::readUDP()
{
    QByteArray buffer;
    buffer.resize(socketIn->pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;
    socketIn->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

    qDebug() << "Message from: " << sender.toString();
    qDebug() << "Message port: " << senderPort;
    /*
    for(int i = 0; i < buffer.size(); i++)
    {
        int temp = buffer.at(i);
        printf("%d,\t", temp);
    }
    printf("\n");
    */

    signalTimer->stop();
    signalTimer->start(2000);

    // mc gives data to be sent to some device
    // format: -127, device id, [the rest of the message bytes]
    if(buffer.at(0) == -127)
    {
        for(int i = 0; i < serialPorts.size(); i++)
        {
            if(buffer.at(1) == serialIds.at(i))
            {
                sendSerial(serialPorts.at(i), buffer);

                //qDebug() << "Sent to serial device" << serialIds.at(i);
                //break;
            }
            else if(serialIds.at(i) == 69) // uninitialized serial device id
            {
                QByteArray tempBuffer = buffer;
                tempBuffer[1] = 69;
                sendSerial(serialPorts.at(i), tempBuffer);
            }
        }
    }
    // mc computer is requesting data from a device
    // format: -126, device id, ip1, ip2, ip3, ip4, port high, port low
    else if(buffer.at(0) == -126)
    {
        data_path temp;
        bool exists = false;
        char temp_address[15];

        temp.deviceId = buffer.at(1);
        sprintf(temp_address, "%d.%d.%d.%d", uint8_t(buffer.at(2)), uint8_t(buffer.at(3)), uint8_t(buffer.at(4)), uint8_t(buffer.at(5)));
        temp.address = temp_address;
        temp.port = (uint16_t(buffer.at(6)) << 8) | uint8_t(buffer.at(7));

        for(int i = 0; i < dataPaths.size(); i++)
        {
            if(dataPaths.at(i) == temp)
            {
                exists = true;
                break;
            }
        }
        if(!exists)
        {
            dataPaths.append(temp);
            qDebug() << temp.address << ":" << temp.port << "will recieve data from device" << temp.deviceId;
        }
        else
        {
            qDebug() << temp.address << ":" << temp.port << "is already recieving data from device" << temp.deviceId;
        }
    }
    // mc is requesting a list of the devices it recieves data from
    // format (input): -125, mc ip, mc port
    // format (output): -125, number of devices, device id 1, device id...
    if(buffer.at(0) == -125)
    {
        int temp_port;
        char temp_address[15];
        sprintf(temp_address, "%d.%d.%d.%d", uint8_t(buffer.at(1)), uint8_t(buffer.at(2)), uint8_t(buffer.at(3)), uint8_t(buffer.at(4)));
        temp_port = (uint16_t(buffer.at(6)) << 8) | uint8_t(buffer.at(7));

        QByteArray idArray;
        for(int i = 0; i < dataPaths.size(); i++)
        {
            if(dataPaths.at(i).address == temp_address && dataPaths.at(i).port == temp_port)
            {
                idArray.append(uint8_t(dataPaths.at(i).deviceId));
            }
        }
        QByteArray buffer;
        buffer.append(int8_t(-125));
        buffer.append(uint8_t(idArray.size()));
        buffer.append(idArray);

        socketIn->writeDatagram(buffer, QHostAddress(temp_address), temp_port);
    }
}

void socket::sendSerial(QSerialPort* port, const char* message) // not really necessary to make this a separate function
{
    QByteArray data;
    data.append(message);
    port->write(data);
}

void socket::sendSerial(QSerialPort* port, QByteArray message) // not really necessary to make this a separate function
{
    port->write(message);
}

void socket::sendSerialAll(const char* message) // for send serial all, do not include start line and device id bytes in data
{
    for(int i = 0; i < serialPorts.size(); i++)
    {
        QByteArray data;
        data.append(char(-127));
        data.append(char(serialIds.at(i)));
        data.append(message);
        serialPorts.at(i)->write(data);
    }
}

void socket::sendSerialAll(QByteArray message) // for send serial all, do not include start line and device id bytes in data
{
    // currently sends the data to all connected serial devices
    for(int i = 0; i < serialPorts.size(); i++)
    {
        QByteArray data;
        data.append(char(-127));
        data.append(char(serialIds.at(i)));
        data.append(message);
        serialPorts.at(i)->write(data);
    }
}

void socket::readSerial()
{
    QSerialPort* port;
    port = dynamic_cast<QSerialPort*>(sender());
    const QByteArray buffer = port->readAll();

    printf(buffer);
    //fprintf(dataFile,buffer);

    if(buffer.at(0) == -126) // incorrect id
    {
        for(int i = 0; i < serialPorts.size(); i++)
        {
            if(serialPorts.at(i) == port)
            {
                serialIds[i] = buffer.at(1);
                qDebug() << port->portName() << "id set to" << serialIds[i];
            }
        }
    }
    else if(buffer.at(0) == -127) // to be relayed to a mc computer
    {
        /*
        for(int i = 0; i < buffer.size(); i++)
        {
            int8_t temp = buffer.at(i);
            printf("%d,\t", temp);
        }
        printf("\n");
        */
        for(int i = 0; i < serialPorts.size(); i++)
        {
            if(serialPorts.at(i) == port)
            {
                for(int j = 0; j < dataPaths.size(); j++)
                {
                    if(dataPaths.at(j).deviceId == serialIds.at(i))
                        socketIn->writeDatagram(buffer, QHostAddress(dataPaths.at(j).address), dataPaths.at(j).port);
                }
            }
        }
    }
}

void socket::zeroDevices()
{
    //when you lose connection, tell all devices that all inputs are zero
    QByteArray stopBuffer;
    stopBuffer.append(char(90));
    stopBuffer.append(char(90));
    stopBuffer.append(char(90));
    stopBuffer.append(char(70));
    stopBuffer.append(char((90+ 90 + 90 + 70)/4));
    stopBuffer.append(char(0));

    sendSerialAll(stopBuffer);
}
