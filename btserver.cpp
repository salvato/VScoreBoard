#include "btserver.h"

#include <QtBluetooth/qbluetoothserver.h>
#include <QtBluetooth/qbluetoothsocket.h>

using namespace Qt::StringLiterals;

//! [Service UUID]
static constexpr auto serviceUuid = "e8e10f95-1a70-4b27-9ccf-02010264e9c8"_L1;
//! [Service UUID]

BtServer::BtServer(QObject *parent)
    : QObject{parent}
{}


BtServer::~BtServer() {
    stopServer();
}


void
BtServer::startServer(const QBluetoothAddress& localAdapter) {
    if(rfcommServer)
        return;

    // Create the server
    rfcommServer = new QBluetoothServer(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(rfcommServer, &QBluetoothServer::newConnection,
            this, QOverload<>::of(&BtServer::clientConnected));
    bool result = rfcommServer->listen(localAdapter);
    if(!result) {
        qWarning() << "Cannot bind chat server to" << localAdapter.toString();
        return;
    }

    //serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceRecordHandle, (uint)0x00010010);

    QBluetoothServiceInfo::Sequence profileSequence;
    QBluetoothServiceInfo::Sequence classId;
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort));
    classId << QVariant::fromValue(quint16(0x100));
    profileSequence.append(QVariant::fromValue(classId));
    serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList,
                             profileSequence);

    classId.clear();
    classId << QVariant::fromValue(QBluetoothUuid(serviceUuid));
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort));

    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);

    // Service name, description and provider
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceName, tr("Bt Panel Server"));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceDescription,
                             tr("Bluetooth Panel server"));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceProvider, tr("qt-project.org"));

    // Service UUID set
    serviceInfo.setServiceUuid(QBluetoothUuid(serviceUuid));

    // Service Discoverability
    const auto groupUuid = QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::PublicBrowseGroup);
    QBluetoothServiceInfo::Sequence publicBrowse;
    publicBrowse << QVariant::fromValue(groupUuid);
    serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList, publicBrowse);

    // Protocol descriptor list
    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::L2cap));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    protocol.clear();
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::Rfcomm))
             << QVariant::fromValue(quint8(rfcommServer->serverPort()));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList,
                             protocolDescriptorList);

    // Register service
    serviceInfo.registerService(localAdapter);
}

// stopServer
void
BtServer::stopServer() {
    // Unregister service
    serviceInfo.unregisterService();

    // Close sockets
    qDeleteAll(clientSockets);
    clientNames.clear();

    // Close server
    delete rfcommServer;
    rfcommServer = nullptr;
}


// sendMessage
void
BtServer::sendMessage(const QString &message) {
    QByteArray text = message.toUtf8() + '\n';
    for(QBluetoothSocket *socket : std::as_const(clientSockets))
        socket->write(text);
}


// clientConnected
void
BtServer::clientConnected() {
    QBluetoothSocket *socket = rfcommServer->nextPendingConnection();
    if(!socket)
        return;

    connect(socket, &QBluetoothSocket::readyRead,
            this, &BtServer::readSocket);
    connect(socket, &QBluetoothSocket::disconnected,
            this, QOverload<>::of(&BtServer::clientDisconnected));
    clientSockets.append(socket);
    clientNames[socket] = socket->peerName();
    emit clientConnected(socket->peerName());
}


// clientDisconnected
void
BtServer::clientDisconnected() {
    QBluetoothSocket *socket = qobject_cast<QBluetoothSocket *>(sender());
    if (!socket)
        return;

    emit clientDisconnected(clientNames[socket]);

    clientSockets.removeOne(socket);
    clientNames.remove(socket);

    socket->deleteLater();
}


// readSocket
void
BtServer::readSocket() {
    QBluetoothSocket *socket = qobject_cast<QBluetoothSocket *>(sender());
    if (!socket)
        return;

    while (socket->canReadLine()) {
        QByteArray line = socket->readLine().trimmed();
        emit messageReceived(clientNames[socket],
                             QString::fromUtf8(line.constData(), line.length()));
    }
}

