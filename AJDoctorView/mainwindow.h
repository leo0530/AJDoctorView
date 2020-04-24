#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "udpthread.h"
#include <QTimer>
#include<opencv2/opencv.hpp>
#include <iostream>
#include <vector>
using namespace cv;
using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Mat picProcess(Mat img);          //处理收到的图片，提取目标，画圆
private slots:
    void slotRecv(char *buf, int len);
    void slotTimeout();  //超时处理函数
private:
    Ui::MainWindow *ui;
    UdpThread *m_udpThread;

    QTimer *timer;//定时器

    VideoCapture capture;//摄像头
};

#endif // MAINWINDOW_H
