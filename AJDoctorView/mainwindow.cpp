#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //初始化udp线程
    m_udpThread = new UdpThread;
    connect(m_udpThread, SIGNAL(sigRecvOk(char*,int)), this, SLOT(slotRecv(char*,int)));
    m_udpThread->start();

    //初始化定时器
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
    timer->start(500);

}

MainWindow::~MainWindow()
{
//    if(m_udpThread)
//        delete m_udpThread;
    if(timer)
    {
        if(timer->isActive())
            timer->stop();
        delete  timer;
    }
    delete ui;
}

void MainWindow::slotRecv(char * buf, int len)
{

    QPixmap pixmap;
    pixmap.loadFromData((uchar*)buf, len, "JPG");
    ui->label->setPixmap(pixmap);

}

void MainWindow::slotTimeout()  //超时处理函数
{
    if(capture.isOpened())
    {
        Mat frame;
        capture>>frame;

        Rect rectLeft(0, 0, frame.cols / 2, frame.rows);
    //    Rect rectRight(frame.cols / 2, 0, frame.cols / 2, frame.rows);
        Mat image_l = Mat(frame, rectLeft);
     //   Mat image_r = Mat(frame, rectRight);

        imshow("frame",picProcess(image_l));
    }
    else {
        capture.open(0);
    }
}

Mat MainWindow::picProcess(Mat img)//处理收到的图片，提取目标，画圆
{
    Mat dst;
    Mat kernel;
    //开操作处理
    kernel = getStructuringElement(MORPH_RECT, Size(5, 5));

    std::vector<std::vector<Point>> contours;
    std::vector<Vec4i> hireachy;
    Rect rect;
    Point2f center;
    float radius = 20;

    //blur(frame, dst, Size(5,5));
    inRange(img, Scalar(0, 80, 80), Scalar(50, 255, 255), dst);
    //开操作
    morphologyEx(dst, dst, MORPH_OPEN, kernel);
    //获取边界
    findContours(dst, contours, hireachy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));
    //框选面积最大的边界
    if (contours.size() > 0)
    {
        double maxArea = 0;
        for (size_t i = 0; i < contours.size(); i++)
        {
            double area = contourArea(contours[static_cast<size_t>(i)]);//计算面积
            if (area > maxArea)
            {
                maxArea = area;
                rect = boundingRect(contours[static_cast<size_t>(i)]);
                minEnclosingCircle(contours[static_cast<size_t>(i)], center, radius);
            }
        }
    }
    //矩形框
    //rectangle(frame,rect, Scalar(0,255,0),2);
    //圆形框
    circle(img, Point(center.x, center.y), (int)radius, Scalar(0, 255, 0), 2);

    return img;
}
