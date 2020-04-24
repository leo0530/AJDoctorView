#include "udpthread.h"

UdpThread::UdpThread(QObject *parent) :
    QThread(parent)
{
    m_buf = new char[1024*1024];
    memset(m_buf, 0, 1024);

}

UdpThread::~UdpThread()
{
    m_udpSocket->close();
    delete m_buf;
}

void UdpThread::run()
{
    m_udpSocket = new QUdpSocket;
    connect(m_udpSocket, SIGNAL(readyRead()), this, SLOT(slotRecv()),Qt::DirectConnection);
    qDebug()<< m_udpSocket->bind(QHostAddress::Any, 65522);
    exec();
}

void UdpThread::slotRecv()
{
    char *recvBuf = new char[1052];
    memset(recvBuf, 0, 1052);
    //qDebug("start");
    while(m_udpSocket->hasPendingDatagrams()) {
        memset(recvBuf, 0, 1052);
        qint64 size = m_udpSocket->pendingDatagramSize();

        //qDebug()<<"size"<<size;
        m_udpSocket->readDatagram(recvBuf, size);
        ImageFrameHead *mes = (ImageFrameHead *)recvBuf;
        //qDebug()<<mes->uDataFrameCurr<<mes->uDataFrameTotal;
        if (mes->funCode == 24) {
            memcpy(m_buf+mes->uDataInFrameOffset, (recvBuf+ sizeof(ImageFrameHead)), mes->uTransFrameSize);
            if (mes->uDataFrameCurr == mes->uDataFrameTotal) {
                qDebug("write");
                emit sigRecvOk(m_buf, mes->uDataFrameSize);
            }
        }

    }
    delete recvBuf;
}
