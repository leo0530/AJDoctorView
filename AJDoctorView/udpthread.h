#ifndef UDPTHREAD_H
#define UDPTHREAD_H

#include <QThread>
#include <QUdpSocket>
struct ImageFrameHead {
    int funCode;                        //!������
    unsigned int uTransFrameHdrSize;	//!sizeof(WIFI_FRAME_HEADER)
    unsigned int uTransFrameSize;		//!sizeof(WIFI_FRAME_HEADER) + Data Size

    //����֡����
    unsigned int uDataFrameSize;		//����֡���ܴ�С
    unsigned int uDataFrameTotal;		//һ֡���ݱ��ֳɴ���֡�ĸ���
    unsigned int uDataFrameCurr;		//����֡��ǰ��֡��
    unsigned int uDataInFrameOffset;   	//����֡����֡��ƫ��
};

class UdpThread : public QThread
{
    Q_OBJECT
public:
    explicit UdpThread(QObject *parent = 0);
    ~UdpThread();

protected:
    virtual void run();

signals:
    void sigRecvOk(char *buf, int len);

public slots:
    void slotRecv();
private:
    QUdpSocket *m_udpSocket;
    char *m_buf;

};

#endif // UDPTHREAD_H
