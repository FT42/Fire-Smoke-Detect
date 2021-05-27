#pragma once

#include <QtWidgets/QWidget>
#include <QFileDialog>
#include <QDebug>
#include <Qtimer>
#include <QPixmap>
#include <QMessageBox>
#include <QCloseEvent>
#include "ui_firedetect.h"
#include <opencv2/opencv.hpp>
#include "detect.h"

class firedetect : public QWidget
{
    Q_OBJECT;

public:
    firedetect(QWidget *parent = Q_NULLPTR);
    ~firedetect();
    QImage MatToQimage(cv::Mat& srcimg);
    void closeEvent(QCloseEvent *event);
private:
    Ui::firedetectClass ui;
    cv::Mat frame;
    cv::Mat frame2;
    cv::Mat outframe;
    cv::VideoCapture cap;
    Detect my_detect;
    QImage image;
    QLabel* label = NULL;
    QTimer* timer = NULL;
    double fps = NULL;
    int flag = NULL;
private slots:
    void onlinedetect();
    void offlinedetect();
    void imagedetect();
    void stopdetect();
    void exitsystem();
    void showoriginal();
    void showdetect();
    void showbinary();
    void nextframe();
};
