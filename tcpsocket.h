#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QObject>
#include <QTcpSocket>

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);
    bool requestToServer(QString cmd,QString address,uint32_t port);
signals:
private:
    QTcpSocket *socket = nullptr;
};

#endif // TCPSOCKET_H
