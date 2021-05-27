#include "firedetect.h"

firedetect::firedetect(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(":/firedetect/firedetect.ico"));
}

firedetect::~firedetect()
{
}

void firedetect::onlinedetect()
{
    if (cap.isOpened())
        cap.release();
    cap.open(0);
    if (cap.isOpened())
    {
        cap >> frame;
        if (!frame.empty())
        {
            flag = 0;
            frame2 = frame.clone();
            cvtColor(frame, outframe, COLOR_BGR2RGB);
            image = MatToQimage(outframe);
            ui.label->setPixmap(QPixmap::fromImage(image));
            ui.label->resize(image.size());
            ui.label->show();
            timer = new QTimer(this);
            connect(timer, &QTimer::timeout, this, &firedetect::nextframe);
            timer->start(33);
        }

    }
}
void firedetect::offlinedetect()
{
    if (cap.isOpened())
        cap.release();
    QString file_name = QFileDialog::getOpenFileName(this, tr("打开视频"), ".", tr("视频类型(*.avi *.mp4 *.flv *.mkv)"));
    cap.open(file_name.toLocal8Bit().data());
    if (cap.isOpened())
    {
        fps = cap.get(cv::CAP_PROP_FPS);
        cap >> frame;
        if (!frame.empty())
        {
            flag = 0;
            frame2 = frame.clone();
            cvtColor(frame, outframe, COLOR_BGR2RGB);
            image = MatToQimage(outframe);
            ui.label->setPixmap(QPixmap::fromImage(image));
            ui.label->resize(image.size());
            ui.label->show();
            timer = new QTimer(this);
            timer->setInterval(1000 / fps);
            connect(timer, &QTimer::timeout, this, &firedetect::nextframe);
            timer->start();
        }
    
    }
}

void firedetect::imagedetect()
{
    if (cap.isOpened())
        cap.release();
    QString file_name = QFileDialog::getOpenFileName(this, tr("打开图片"), ".", tr("图片类型(*.jpg *.png)"));
    frame = cv::imread(file_name.toLocal8Bit().data());
    if (!frame.empty())
    {
        my_detect.F_Detect(frame);
        image = MatToQimage(frame);
        ui.label->setPixmap(QPixmap::fromImage(image));
        ui.label->resize(image.size());
        ui.label->show();
    }
}

void firedetect::stopdetect()
{
    if (cap.isOpened())
    {
        timer->stop();
        cap.release();
        ui.label->clear();
        ui.label->setText("本地视频/图片/摄像头");
    }
    else
    {
        ui.label->clear();
        ui.label->setText("本地视频/图片/摄像头");
    }
    my_detect.orginal.release();
    my_detect.finframe.release();
    my_detect.fin_bin.release();
}

void firedetect::exitsystem()
{
    this->close();
}

void firedetect::showoriginal()
{
    if (cap.isOpened())
    {
        flag = 1;
    }
    if (my_detect.orginal.empty()) {}
    else
    {
        image = MatToQimage(my_detect.orginal);
        ui.label->setPixmap(QPixmap::fromImage(image));
        ui.label->resize(image.size());
        ui.label->show();
    }
}

void firedetect::showdetect()
{
    if (cap.isOpened())
    {
        flag = 2;
    }
    if (my_detect.finframe.empty()) {}
    else
    {
        image = MatToQimage(my_detect.finframe);
        ui.label->setPixmap(QPixmap::fromImage(image));
        ui.label->resize(image.size());
        ui.label->show();
    }
}

void firedetect::showbinary()
{
    if (cap.isOpened())
    {
        flag = 3;
    }
    if (my_detect.fin_bin.empty()) {}
    else
    {
        image = MatToQimage(my_detect.fin_bin);
        ui.label->setPixmap(QPixmap::fromImage(image));
        ui.label->resize(image.size());
        ui.label->show();
    }
}

QImage firedetect::MatToQimage(cv::Mat &srcimg)
{
    QImage qimage;
    if (srcimg.channels() == 3)
    {
        qimage = QImage((const unsigned char*)(srcimg.data),
            srcimg.cols, srcimg.rows,
            srcimg.cols * srcimg.channels(),
            QImage::Format_RGB888);
    }
    else if (srcimg.channels() == 1)
    {
        qimage = QImage((const unsigned char*)(srcimg.data),
            srcimg.cols, srcimg.rows,
            srcimg.cols * srcimg.channels(),
            QImage::Format_Indexed8);
    }
    else
    {
        qimage = QImage((const unsigned char*)(srcimg.data),
            srcimg.cols, srcimg.rows,
            srcimg.cols * srcimg.channels(),
            QImage::Format_RGB888);
    }
    return QImage(qimage);
}

void firedetect::nextframe()
{
    cap >> frame;
    if (!frame.empty())
    {
        my_detect.F_Detect(frame2,frame);
        frame2 = frame.clone();
        if ((flag == 1) || (flag == 0))
        {
            image = MatToQimage(my_detect.orginal);
        }
        if (flag == 2)
        {
            image = MatToQimage(my_detect.finframe);
        }
        if (flag == 3)
        {
            image = MatToQimage(my_detect.fin_bin);
        }
        ui.label->setPixmap(QPixmap::fromImage(image));
        ui.label->resize(image.size());
        ui.label->show();
        timer->start();
    }
    
}

void firedetect::closeEvent(QCloseEvent *event)
{
    int choose;
    choose = QMessageBox::question(this, tr("退出程序"), tr("是否退出程序"), QMessageBox::Yes | QMessageBox::No);

    if (choose == QMessageBox::Yes)
    {
        event->accept();
    }
    else if (choose == QMessageBox::No)
    {
        event->ignore();
    }

}
