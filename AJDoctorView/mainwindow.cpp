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

    m_udpSocket = new QUdpSocket(this);  //创建对象
   cout << m_udpSocket->bind(QHostAddress::LocalHost, 9999); //绑定端口
    //当UDP收到消息后，会发送readyRead信号，
    //连接成功有 connected，断开连接有，disconnected信号。
//    connect(m_udpSocket, SIGNAL(readyRead()),this, SLOT(readPendingDatagrams()));

    //初始化定时器
    timer = NULL;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
 //   timer->start(1000);//定时器周期是1秒

    initTableView();//初始化列表控件

}

MainWindow::~MainWindow()
{
    m_udpThread->exit();//退出udp接收线程

    //释放资源
    if(model)
        delete model;

    if(timer)
    {
        if(timer->isActive())
            timer->stop();
        delete  timer;
    }

    if(m_udpSocket)
        delete m_udpSocket;

    delete ui;
}

void MainWindow::readPendingDatagrams()
  {
      while (m_udpSocket->hasPendingDatagrams()) {
      //从缓冲区中读取数据，
          QNetworkDatagram datagram = m_udpSocket->receiveDatagram();

      }
  }


void MainWindow::initTableView()
{
    model = new QStandardItemModel();

    model->setColumnCount(4);
    model->setHeaderData(0,Qt::Horizontal,QString("顺序"));
    model->setHeaderData(1,Qt::Horizontal,QString("坐标"));
    model->setHeaderData(2,Qt::Horizontal,QString("疗法"));
    model->setHeaderData(3,Qt::Horizontal,QString("时间(分钟)"));
    ui->tableView->setModel(model);
    //表头信息显示居左
    ui->tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Fixed);  //设定表头列宽不可变
    ui->tableView->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Fixed);  //设定表头列宽不可变
    ui->tableView->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Fixed);  //设定表头列宽不可变
    ui->tableView->horizontalHeader()->setSectionResizeMode(3,QHeaderView::Fixed);  //设定表头列宽不可变
   //
    ui->tableView->setColumnWidth(0,70);
    ui->tableView->setColumnWidth(1,120);
    ui->tableView->setColumnWidth(2,70);
    ui->tableView->setColumnWidth(3,70);

    ReadOnlyDelegate *itemReadOnly   = new ReadOnlyDelegate(this);
    ComboDelegate *itemComboDelegate = new ComboDelegate(this);
    ui->tableView->setItemDelegateForColumn(1, itemReadOnly);//设置第2列
    ui->tableView->setItemDelegateForColumn(2, itemComboDelegate);//设置第3列

    ui->tableView->verticalHeader()->hide();//隐藏行号
    //整行选择
  //  ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(ui->tableView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(slotRowDoubleClicked(const QModelIndex &)));
}

void MainWindow::slotRowDoubleClicked(const QModelIndex index)
{
    //通过Model获取一行
    m_ModelIndex = ui->tableView->currentIndex();
}

void MainWindow::slotRecv(char * buf, int len)
{
    QPixmap pixmap;
    pixmap.loadFromData((uchar*)buf, (uint)len, "JPG");

    //QPixmap转成Mat，then调用目标提取函数
    QImage img = QPixmap2QImage(pixmap);
    Mat dst = QImage2cvMat(img);
    dst = DetectTarget(dst);

    //Mat转换成QPixmap，then显示
    img = Mat2QImage(dst);
    ui->label->setPixmap(QImage2QPixmap(img));

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
            if (area > 500)
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
//    //圆形框
//    circle(img, Point(static_cast<int>(center.x),static_cast<int>(center.y)), static_cast<int>(radius), Scalar(0, 255, 0), 2);

    for(size_t j = 0;j < vecTarget.size();j++)
    {
        cout << vecTarget[j].area <<"," << vecTarget[j].rect <<"," << vecTarget[j].center <<"," << vecTarget[j].index << endl;
        //    //圆形框
        circle(img, Point(static_cast<int>(center.x),static_cast<int>(center.y)), static_cast<int>(radius), Scalar(0, 255, 0), 2);
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

//颜色转换：RGB-->HSV
void MainWindow::RGB2HSV(double red, double green, double blue, double& hue, double& saturation, double& intensity)
{

#define PI 3.1415926

    double r, g, b;
    double h, s, i;

    double sum;
    double minRGB, maxRGB;
    double theta;

    r = red / 255.0;
    g = green / 255.0;
    b = blue / 255.0;

    minRGB = ((r<g) ? (r) : (g));
    minRGB = (minRGB<b) ? (minRGB) : (b);

    maxRGB = ((r>g) ? (r) : (g));
    maxRGB = (maxRGB>b) ? (maxRGB) : (b);

    sum = r + g + b;
    i = sum / 3.0;

    if (i<0.001 || maxRGB - minRGB<0.001)
    {
        h = 0.0;
        s = 0.0;
    }
    else
    {
        s = 1.0 - 3.0*minRGB / sum;
        theta = sqrt((r - g)*(r - g) + (r - b)*(g - b));
        theta = acos((r - g + r - b)*0.5 / theta);
        if (b <= g)
            h = theta;
        else
            h = 2 * PI - theta;
        if (s <= 0.01)
            h = 0;
    }

    hue = static_cast<int>(h * 180 / PI);
    saturation = static_cast<int>(s * 100);
    intensity  = static_cast<int>(i * 100);
}
//找出图像中红色的物体
Mat MainWindow::DetectRedTarget(Mat input)
{

    Mat frame;
    Mat srcImg = input;
    frame = srcImg;
  //  waitKey(1);
    int width = srcImg.cols;
    int height = srcImg.rows;

    int x, y;
    double B = 0.0, G = 0.0, R = 0.0, H = 0.0, S = 0.0, V = 0.0;
    Mat vec_rgb = Mat::zeros(srcImg.size(), CV_8UC1);
    for (x = 0; x < height; x++)
    {
        for (y = 0; y < width; y++)
        {
            B = srcImg.at<Vec3b>(x, y)[0];
            G = srcImg.at<Vec3b>(x, y)[1];
            R = srcImg.at<Vec3b>(x, y)[2];
            RGB2HSV(R, G, B, H, S, V);
            //红色范围，范围参考的网上。可以自己调
            if ((H >= 312 && H <= 360) && (S >= 17 && S <= 100) && (V>18 && V < 100))
                vec_rgb.at<uchar>(x, y) = 255;
            /*cout << H << "," << S << "," << V << endl;*/
        }
    }

    return vec_rgb;
}

vector<Point2f> MainWindow::GetTargetCoordinate(Mat in) //获取红色目标坐标
{
    Mat matSrc = in;

    GaussianBlur(matSrc, matSrc, Size(5, 5), 0);//高斯滤波，除噪点

    vector<vector<Point> > contours;//contours的类型，双重的vector

    vector<Vec4i> hierarchy;//Vec4i是指每一个vector元素中有四个int型数据。

    //阈值
    threshold(matSrc, matSrc, 100, 255, THRESH_BINARY);//图像二值化

    //寻找轮廓，这里注意，findContours的输入参数要求是二值图像，二值图像的来源大致有两种，第一种用threshold，第二种用canny
    findContours(matSrc.clone(), contours, hierarchy, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));

    if(contours.size() == 0)
    {
        return vector<Point2f>();
    }
    // 计算矩
    vector<Moments> mu(contours.size());

    for (size_t i = 0; i < contours.size(); i++)
    {
        mu[i] = moments(contours[i], false);
    }

    ///  计算矩中心:

    vector<Point2f> mc(contours.size());

    for (size_t i = 0; i < contours.size(); i++)
    {
        mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
    }

    return mc;

    /// 绘制轮廓
//    Mat drawing = Mat::zeros(matSrc.size(), CV_8UC1);
//    for (size_t i = 0; i < contours.size(); i++)
//    {
//        Scalar color = Scalar(255);
//        //drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());//绘制轮廓函数
//        circle(drawing, mc[i], 4, color, -1, 8, 0);
//    }
//    *x1 = mc[0].x;
//    *y1 = mc[0].y;
//    *x2 = mc[contours.size() - 1].x;
//    *y2 = mc[contours.size() - 1].y;

 //   imshow("outImage", drawing);

}

void MainWindow::on_pushButton_clicked()
{


    Mat src = imread("d:\\1.JPG");

    Mat middle = DetectRedTarget(src);

    vector<Point2f> temp = GetTargetCoordinate(middle);

    for (size_t i = 0; i < temp.size(); i++)
    {
        cout << temp[i] << endl;
        circle(src, Point(static_cast<int>(temp[i].x),static_cast<int>(temp[i].y)), static_cast<int>(20), Scalar(0, 255, 0), 2);
    }

    QPixmap pixmap;
    pixmap = QImage2QPixmap(Mat2QImage(src));
    ui->label->setPixmap(pixmap);

    for (size_t i = 0; i < temp.size(); i++)
    {
        //顺序、坐标、疗法、时间
        int count = model->rowCount();
        model->setItem(count,0,new QStandardItem(QString::number(i+1)));
        QString strPos;
        strPos.append(QString::number(temp[i].x,'f',2));//x坐标
        strPos.append(",");
        strPos.append(QString::number(temp[i].y,'f',2));//y坐标
        model->setItem(count,1,new QStandardItem(strPos));
        model->setItem(count,2,new QStandardItem(QString("")));
    }
}

void MainWindow::on_addButton_clicked()
{

}

void MainWindow::on_deleteItemButton_clicked()
{
    //x是指定删除哪一行

  //  model->removeRow(x);

    //删除所有行

    model->removeRows(0,model->rowCount());
}


void MainWindow::on_sendButton_clicked()
{
    QByteArray byteArray;//QByteArray用于将自定义的结构体转化为字节流，用于网络传输
    for(int i=0;i<model->rowCount();i++)
    {
        QModelIndex index = model->index(i,0);
        QModelIndex pos = model->index(i,1);
        QModelIndex cute = model->index(i,2);
        QModelIndex time = model->index(i,3);

        QString point = model->data(pos).toString();
        QStringList list = point.split(",");
        QString x = list[0]; //a = "hello"
        QString y = list[1]; //b = "world"

        Cute_Solution solution;
        solution.no   = model->data(index).toInt();  //治疗顺序
        //QString转换String
        //string s = qstr.toStdString();
        if(model->data(cute).toString() == "点按")
        {
            solution.cute = DIAN_AN;
        }
        else  if(model->data(cute).toString() == "画圆")
        {
            solution.cute = HUA_YUAN;
        }
        else  if(model->data(cute).toString() == "雀琢")
        {
            solution.cute = QUE_ZUO;
        }

        solution.time = model->data(time).toInt();   //治疗时间
        solution.x = x.toFloat();//x坐标
        solution.y = y.toFloat();//y坐标

        byteArray.append((char*)&solution, sizeof(solution));
   }

    qint64 size =  m_udpSocket->writeDatagram(byteArray, QHostAddress::Broadcast, 65520);
    qDebug()<<QString::number(size)<<","<<QString::number(byteArray.size())<<endl;

}
