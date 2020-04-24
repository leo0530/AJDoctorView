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
 //   timer->start(1000);//定时器周期是1秒

}

MainWindow::~MainWindow()
{
    m_udpThread->terminate();

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
    pixmap.loadFromData((uchar*)buf, (uint)len, "JPG");
 //   ui->label->setPixmap(pixmap);
    //QPixmap转成Mat，then调用目标提取函数
    QImage img = QPixmap2QImage(pixmap);
    Mat dst = QImage2cvMat(img);
    dst = DetectTarget(dst);

    //Mat转换成QPixmap，then显示
    img = Mat2QImage(dst);
    ui->label->setPixmap(QImage2QPixmap(img));

 //   imshow("dst",dst);
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

        QPixmap pixmap;
        pixmap = QImage2QPixmap(Mat2QImage(DetectTarget(image_l)));
        ui->label->setPixmap(pixmap);
      //  imshow("frame",);
    }
    else {
        capture.open(0);
    }
}

Mat MainWindow::DetectTarget(Mat img)//处理收到的图片，提取目标，画圆
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

    vecTarget.clear();//清空容器中的内容

    //blur(frame, dst, Size(5,5));
    inRange(img, Scalar(0, 80, 80), Scalar(50, 255, 255), dst);
    //开操作
    morphologyEx(dst, dst, MORPH_OPEN, kernel);
    //获取边界
    findContours(dst, contours, hireachy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));
    //框选面积最大的边界
    int index = 0;//轮廓序号
    if (contours.size() > 0)
    {
        double maxArea = 0;
        for (size_t i = 0; i < contours.size(); i++)
        {
            double area = contourArea(contours[static_cast<size_t>(i)]);//计算面积
            if (area > 50)
            {
                maxArea = area;
                rect = boundingRect(contours[static_cast<size_t>(i)]);//得到包覆此轮廓的最小正矩形
                minEnclosingCircle(contours[static_cast<size_t>(i)], center, radius);//求最小包围圆

                Target temp;
                temp.area = area;
                temp.rect = rect;
                temp.center = center;
                temp.index = ++index;
                vecTarget.push_back(temp);//记录面积大于50的轮廓的中心坐标
            }
        }
    }
    //矩形框
    //rectangle(img,rect, Scalar(0,255,0),2);
    //圆形框
    circle(img, Point(static_cast<int>(center.x),static_cast<int>(center.y)), static_cast<int>(radius), Scalar(0, 255, 0), 2);

    for(size_t j = 0;j < vecTarget.size();j++)
    {
        cout << vecTarget[j].area <<"," << vecTarget[j].rect <<"," << vecTarget[j].center <<"," << vecTarget[j].index << endl;
    }

    return img;
}

 QImage MainWindow::QPixmap2QImage(QPixmap pix) //QPixmap-->QImage
 {
    return pix.toImage();
 }

 QPixmap MainWindow::QImage2QPixmap(QImage img)            //QImage-->QPixmap
 {
    return  QPixmap::fromImage(img);
 }

Mat MainWindow::QImage2cvMat(QImage image)    //QImage-->Mat
{
    Mat mat;
    qDebug() << image.format();
    switch(image.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), static_cast<size_t>(image.bytesPerLine()));
        break;
    case QImage::Format_RGB888:
        mat = Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), static_cast<size_t>(image.bytesPerLine()));
        cvtColor(mat, mat, CV_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
        mat = Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), static_cast<size_t>(image.bytesPerLine()));
        break;
    }
    return mat;
}

QImage MainWindow::Mat2QImage(const Mat& mat) //Mat-->QImage
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if(mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if(mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else if(mat.type() == CV_8UC4)
    {
        qDebug() << "CV_8UC4";
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}
