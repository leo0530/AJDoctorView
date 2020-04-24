#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QDebug>
#include<QFileDialog>
#include "udpthread.h"
#include <QTimer>
#include<opencv2/opencv.hpp>
#include <iostream>
#include <vector>
using namespace cv;
using namespace std;

struct Target  //提取目标结构体
{
    Point2f center;//矩形中心点坐标
    Rect rect;     //包覆此轮廓的最小正矩形
    double area;   //轮廓面积
    int index;     //序号
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

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
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    UdpThread *m_udpThread;

    QTimer *timer;//定时器
    VideoCapture capture;//摄像头

    vector<Target> vecTarget;
};

#endif // MAINWINDOW_H
