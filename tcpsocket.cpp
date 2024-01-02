#include "tcpsocket.h"

Client::Client(QObject *parent)
    : QObject{parent}
{

}
bool Client::requestToServer(QString cmd,QString address,uint32_t port)
{
    QByteArray rx_data;
    bool status = false;
    socket = new QTcpSocket(this);
    socket->connectToHost(address, port);//QHostAddress::

    if(socket->waitForConnected(3000))
    {
        qDebug() << "Connected!";
        socket->write(cmd.toLatin1());
        socket->waitForBytesWritten(200);
        if(socket->waitForReadyRead(3000)){
            qDebug() << "Reading: " << socket->bytesAvailable();
            rx_data = socket->readAll();
            qDebug() << rx_data;
            if(rx_data == "ACK"){
                status = true;
            }else if(rx_data == "NAK"){
                status = false;
            }
        }
        socket->close();
    }
    else
    {
        qDebug() << "Connect false!!!";
        status = false;
    }
    socket->deleteLater();
    return status;
}
