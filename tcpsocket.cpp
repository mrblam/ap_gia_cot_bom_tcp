#include "tcpsocket.h"

Client::Client(QObject *parent)
    : QObject{parent}
{

}
bool Client::requestToServer(QString cmd,QString address,uint32_t port)
{
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
            qDebug() << socket->readAll();
            status = true;
        }
        socket->close();
    }
    else
    {
        qDebug() << "Connect false";
        status = false;
    }
    socket->deleteLater();
    return status;
}
