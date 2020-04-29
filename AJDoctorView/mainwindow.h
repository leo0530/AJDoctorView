﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QDebug>
#include<QFileDialog>
#include <QTimer>
#include <QStandardItemModel>
#include <QUdpSocket>
#include <QtNetwork>
#include <QHeaderView>
#include "udpthread.h"
#include "itemdelegate.h"
#include<opencv2/opencv.hpp>
#include <iostream>
#include <vector>
using namespace cv;
using namespace std;

const int DIAN_AN  = 1;
const int HUA_YUAN = 2;
const int QUE_ZUO  = 3;

#pragma pack(1)  //指定一字节对齐
struct Target  //提取目标结构体
{
    Point2f center;//矩形中心点坐标
    Rect rect;     //包覆此轮廓的最小正矩形
    double area;   //轮廓面积
    int index;     //序号
};

//治疗方案结构体
struct Cute_Solution
{
    int no;        //治疗顺序
    float x;      //中心点x坐标
    float y;      //中心点y坐标
    int cute;  //治疗方案：1:点按、2:画圆、3:雀琢
    int time;      //治疗时间，单位分钟
    Cute_Solution()
    {
        no = 0;
        x = 0.0;
        y = 0.0;
        cute = DIAN_AN;
        time = 0;
    }
};
#pragma pack()  //取消指定对齐，恢复缺省对齐

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void initTableView();

    Mat DetectTarget(Mat img);           //处理收到的图片，提取目标，画圆
    Mat QImage2cvMat(QImage image);    //QImage-->Mat
    QImage Mat2QImage(const Mat& mat); //Mat-->QImage
    QImage QPixmap2QImage(QPixmap pix); //QPixmap-->QImage
    QPixmap QImage2QPixmap(QImage img); //QImage-->QPixmap

    void RGB2HSV(double red, double green, double blue, double& hue, double& saturation, double& intensity);
    Mat DetectRedTarget(Mat input);              //提取红色目标
    vector<Point2f> GetTargetCoordinate(Mat in); //获取红色目标坐标
private slots:
    void slotRecv(char *buf, int len);
    void slotTimeout();  //超时处理函数
    void slotRowDoubleClicked(const QModelIndex index);
    void readPendingDatagrams();//udp接收
    void on_pushButton_clicked();

    void on_addButton_clicked();

    void on_deleteItemButton_clicked();

    void on_sendButton_clicked();

private:
    Ui::MainWindow *ui;
    UdpThread *m_udpThread;
    QUdpSocket *m_udpSocket;

    QTimer *timer;//定时器
    VideoCapture capture;//摄像头

    vector<Target> vecTarget;

    QModelIndex m_ModelIndex;
    QStandardItemModel  *model;
};

#endif // MAINWINDOW_H
