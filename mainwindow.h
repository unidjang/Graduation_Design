#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/opencv.hpp>
#include <QLabel>
#include <QWidget>
#include <QPainter>
#include <QMenu>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
// 使用是opencv的命名空间
using namespace cv;
QT_END_NAMESPACE

struct myPoint
{
    QPoint point;
    QPoint movePoint;

    int m_r;
    int m_g;
    int m_b;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void CreateMenu();

    QImage imageCenter(QImage qimage, QLabel *qLabel);
    bool needUpdate;
    bool IsRead;
    QImage disimage;
    QPen *pen;

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

public slots:
    void onDeleteClicked();
    void onColorClicked();

private slots:
    void on_btn_LoadImage_clicked();

    void on_btn_Gray_clicked();

    void on_btn_MeanImage_clicked();

    void on_btn_canny_clicked();

private:
    QPainter *painter;
    Ui::MainWindow *ui;
    Mat srcImage; // 记录原图的变量  全称为cv::Mat

    // 从画图那里cv过来的
    bool m_bCliked=false;
    QPoint m_Point;
    QPoint m_movePoint;

    QMenu *m_pMenu=nullptr;

    QFont m_Font;

    QList<myPoint> pointList;

    int m_R=255;
    int m_G=0;
    int m_B=0;

};
#endif // MAINWINDOW_H
