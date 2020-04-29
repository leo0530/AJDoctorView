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
    QImage img = pixmap.toImage();
    Mat dst = QImage2cvMat(img);

    Mat middle = DetectRedTarget(dst);//检测红色目标

    vector<Point2f> temp = GetTargetCoordinate(middle);//获取目标坐标

    for (size_t i = 0; i < temp.size(); i++)
    {
        cout << temp[i] << endl;
        circle(dst, Point(static_cast<int>(temp[i].x),static_cast<int>(temp[i].y)), static_cast<int>(20), Scalar(0, 255, 0), 2);
    }


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

    //Mat转换成QPixmap，then显示
    img = Mat2QImage(dst);//,mat转QImage
    ui->label->setPixmap(QPixmap::fromImage(img));
}

void MainWindow::slotTimeout()  //超时处理函数
{
    if(capture.isOpened())
    {
        Mat frame;
        capture>>frame;

        Rect rectLeft(0, 0, frame.cols / 2, frame.rows);
        Rect rectRight(frame.cols / 2, 0, frame.cols / 2, frame.rows);
        Mat image_l = Mat(frame, rectLeft);
        Mat image_r = Mat(frame, rectRight);
    }
    else {
        capture.open(0);
    }
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
    pixmap = QPixmap::fromImage(Mat2QImage(src));// (Mat2QImage(src));
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
