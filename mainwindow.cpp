#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QPainter>
#include <QMouseEvent>
#include <QJsonObject>
#include <QColorDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // TODO
    CreateMenu();
    needUpdate = false;
    IsRead = false;
    m_Font.setPointSize(5);
    m_Font.setFamily("Microsoft YaHei");
    ui->lbl_show1->setAttribute(Qt::WA_OpaquePaintEvent);   // 设为不透明
    // 初始化画笔
    pen = new QPen();
    // 初始画具为划线工具
    ToolType = 0;

    ui->horizontalSlider->setValue(6);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief MainWindow::on_btn_LoadImage_clicked
 * 图像加载事件
 */
void MainWindow::on_btn_LoadImage_clicked()
{
    clearLeft();
    QString imageFilePath = QFileDialog::getOpenFileName(this, tr("打开图像"),"F:/QTCV/Graduation_Design/Pictures","(所有图像(*.jpg *.png *.bmp))");
    if (imageFilePath.isEmpty())
    {
        return;
    }
    // Mat类型看成数组，元组（可能本质上就是Array）
    // srcImage是MainWindow类的私有变量，在mainwindow.h中定义
    srcImage = imread(imageFilePath.toStdString());  // 通过路径获取原始图片，返回值为Mat类型。

    // Mat默认是BGR，要转为RGB，不然颜色就会跟原图不一样
    cvtColor(srcImage, srcImage,CV_BGR2RGB);
    // 然后把Mat图像的各类信息输入QImage函数，转换为QImage对象
    QImage displayImg = QImage(srcImage.data, srcImage.cols, srcImage.rows, srcImage.cols * srcImage.channels(), QImage::Format_RGB888); // Format_RGB888格式化为8位的图像
    disimage = imageCenter(displayImg, ui->lbl_show1);   // 调用下面自定义的imageCenter函数，得到居中的QImage图像
    // 得到mat版disimage，保持尺寸一致
    cv::Mat tempMat(disimage.height(), disimage.width(), CV_8UC4, const_cast<uchar*>(disimage.bits()), disimage.bytesPerLine());
    disimage_mat = tempMat;

    // 显示图像到页面lbl_show1组件中（往setPixmap函数传入QImage对象）
    ui->lbl_show1->setPixmap(QPixmap::fromImage(disimage));
    IsRead = true;

    // 初始化画布
    pixmap_mouse = new QPixmap(ui->lbl_show1->pixmap()->size());   // 设为跟lbl_show1的图片尺寸一样大
    pixmap_mouse->fill(Qt::transparent);    // 设为透明
}

/**
 * @brief MainWindow::on_btn_canny_clicked
 * 边缘检测:针对是灰度
 */
void MainWindow::on_btn_canny_clicked()
{
    if (srcImage.empty())
    {
        return;
    }

    // 如果画布不为空，将两张pixmap合并，然后canny
    if(ui->label->pixmap() != nullptr){
//        qDebug()<<ui->lbl_show1->pixmap()->size();//    QSize(411, 315)  ，随导入的图片而变化
//        qDebug()<<ui->label->pixmap()->size();//    QSize(411, 315)  ,跟上面保持一致

        QPixmap OLPix = overlapping(ui->lbl_show1->pixmap(),ui->label->pixmap());  // 用setPixmap验证，能正确叠加
        // 将叠加后的QPixmap转为Mat
        QImage image = OLPix.toImage();
        cv::Mat temp_OLPmat(image.height(), image.width(), CV_8UC4, image.bits(), image.bytesPerLine());
        OLPix_mat = temp_OLPmat;
        qDebug()<<"OLPix_mat:"<<OLPix_mat.rows<<OLPix_mat.cols;  // 尺寸也是315 411
        // canny
        // 此时的尺寸是经过center处理后的disimage_mat的尺寸。没办法，叠加后的图片大小只能是这么大，而不能是原图大小，可能损失精度
        dst.create(OLPix_mat.size(), OLPix_mat.type());

        cv::cvtColor(OLPix_mat, grayImage, cv::COLOR_BGR2GRAY);  // 得到灰度图grayImage
        qDebug()<<"grayImage:"<<grayImage.rows<<grayImage.cols;
        cannyProc(grayImage, 6);
        qDebug()<<"dst1:"<<dst.rows<<dst.cols;  // 尺寸也是315 411
        // 进行显示
        // 如果是Format_RGB888，那么图片显示时将发生伸展
        QImage temp_disimage = QImage(dst.data, dst.cols, dst.rows, dst.cols * dst.channels(), QImage::Format_RGB32);
        temp_disimage = imageCenter(temp_disimage, ui->lbl_show2); //水平伸展跟这个没关系
        // 显示图像到页面
        ui->lbl_show2->setPixmap(QPixmap::fromImage(temp_disimage));
    }
    // 如果画布为空
    else{
        // canny
        dst.create(disimage_mat.size(), disimage_mat.type());  // TODO 移到这里了
        cv::cvtColor(disimage_mat, grayImage, cv::COLOR_BGR2GRAY);
        cannyProc(grayImage, 6);
        // 进行显示
        QImage temp_disimage = QImage(dst.data, dst.cols, dst.rows, dst.cols * dst.channels(), QImage::Format_RGB888);
        temp_disimage = imageCenter(temp_disimage, ui->lbl_show2);
        // 显示图像到页面
        ui->lbl_show2->setPixmap(QPixmap::fromImage(temp_disimage));
    }

//    sobel();   // 法二法三
}

void MainWindow::cannyProc(cv::Mat greyImg, int low)
{
    if(greyImg.empty())
        return;
    if(greyImg.channels()>1)
        return;
//    qDebug() << "start blur";
    cv::blur(greyImg, detected_edges, cv::Size(3,3));
//    qDebug() << "start canny";
    cv::Canny(detected_edges, detected_edges, low, low*ratio, kernel_size);
    dst= cv::Scalar::all(0);
//    qDebug() << "start copy";

//    qDebug()<<disimage_mat.rows<<disimage_mat.cols;  // 526 686（这是测试图片的原图size）
//    qDebug()<<dst.rows<<dst.cols;  // 526 686（315 411是QPixmap最终显示时的size）
//    qDebug()<<detected_edges.rows<<detected_edges.cols;  // 526 686（315 411）

    if(ui->label->pixmap() == nullptr){
        disimage_mat.copyTo(dst, detected_edges); // 将源图像（disimage_mat）中的边缘信息复制到目标图像（dst）中
    }else{
        disimage_mat.copyTo(dst, detected_edges);  // 这行代码能让滑块正常运行，但一滑动就会出现图片水平伸展，紧接着改为Format_RGB32即可
//        OLPix_mat.copyTo(dst, detected_edges);  // 这代码的滑块异动就异常结束。
    }
    //图像的水平伸展跟下面这段代码无关
    //查找轮廓
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(detected_edges,contours,hierarchy,cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE,cv::Point());
    cv::Mat imageContours=cv::Mat::zeros(disimage_mat.size(),CV_8UC1);  //轮廓
    cv::Mat marks(disimage_mat.size(),CV_32S);   //Opencv分水岭第二个矩阵参数
    marks=cv::Scalar::all(0);
    int index = 0;
    int compCount = 0;
    for( ; index >= 0; index = hierarchy[index][0], compCount++ )
    {
        //对marks进行标记，对不同区域的轮廓进行编号，相当于设置注水点，有多少轮廓，就有多少注水点
        cv::drawContours(marks, contours, index, cv::Scalar::all(compCount+1), 1, 8, hierarchy);
        cv::drawContours(imageContours,contours,index,cv::Scalar(255),1,8,hierarchy);
        cv::drawContours(dst, contours, index, cv::Scalar::all(compCount+1), 1, 8, hierarchy);
    }
}

// 滑动滑块
void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    qDebug() << "value: " << value;
    // TODO 这里需要实时更新grayImage（或者必须得先点击一下“边缘检测”按钮，表示这次画完了，才能滑动滑块）
    if(grayImage.empty())
        return;
    cv::Mat tmp;
    cannyProc(grayImage, double(value));
    //cv::threshold(grayImage, tmp, double(value), 255., cv::THRESH_BINARY);
    QImage temp_disimage;
    if(ui->label->pixmap() == nullptr){
        temp_disimage = QImage(dst.data, dst.cols, dst.rows, dst.cols * dst.channels(), QImage::Format_RGB888);
    }else{
        temp_disimage = QImage(dst.data, dst.cols, dst.rows, dst.cols * dst.channels(), QImage::Format_RGB32);
    }

    temp_disimage = imageCenter(temp_disimage, ui->lbl_show2);  // 居中
    // 显示图像到页面
    ui->lbl_show2->setPixmap(QPixmap::fromImage(temp_disimage));

}


void MainWindow::mouseMoveEvent(QMouseEvent *event)
{

    // 把坐标原点从窗口左上角转为label->pixmap左上角，得到转化坐标系后的点坐标，保存到pointList
    QPoint labelPos = ui->label->mapFrom(this, event->pos());
    // 下面不能用ui->label->pixmap()->size()，因为此时画布上还是空的呢。
    int edgeHeight = (ui->lbl_show1->size().height()- ui->lbl_show1->pixmap()->size().height())/2;
    labelPos.setY(labelPos.y() - edgeHeight);   // 把原本的y值减去Label和pixmap之间的上方空白高度，得到pixmap坐标系下y值

    // 之前调用drawLine每次就是画个点，是因为这俩点原本都直接等于同一个点labelPos，所以鼠标动得快时会出现断断续续的线。
    if(m_movePoint.isNull()){
        m_Point = labelPos;
    }else{
        m_Point = m_movePoint;    // 每次都让直线的起始点等于上一次的终点
    }
    m_movePoint = labelPos;

    // ToolType画笔
    if(event->buttons()==Qt::LeftButton && m_bCliked && IsRead && ToolType == 0)
    {
        // 调用画图函数
        MyDraw(m_Point,m_movePoint);
        // 保存点信息
        myPoint mypoint;
        mypoint.point=m_Point;
        mypoint.movePoint=m_movePoint;
        mypoint.m_r=m_R;
        mypoint.m_g=m_G;
        mypoint.m_b=m_B;

        pointList.append(mypoint);   // pointList仍然保存了所划过的所有点的信息，但是除了保存之外没其他用处了。
    }
    // ToolType橡皮painter.eraseRect
    else if(event->buttons()==Qt::LeftButton && m_bCliked && IsRead && ToolType==1){
        QPainter painter(pixmap_mouse);  // 获取画过了的画布

        int eraserSize = 10;   // 橡皮大小
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.eraseRect(QRect(m_Point, m_movePoint).normalized().adjusted(-eraserSize, -eraserSize, eraserSize, eraserSize));  // TODO 橡皮擦大小怎么调整
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

        ui->label->setPixmap(*pixmap_mouse);
    }
    update();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_bCliked=false;
    // 放开鼠标时，重新初始化这两点为空，避免了两条线段头尾相连
    m_Point = QPoint();
    m_movePoint = QPoint();

    if(event->button()==Qt::RightButton)
    {
        m_pMenu->move(mapToGlobal(event->pos()));
        m_pMenu->show();
    }

    needUpdate = true;
    update();  // 调用 update 函数触发 paintEvent
}


void MainWindow::CreateMenu()
{
    m_pMenu=new QMenu(this);
    QAction *clearAction=new QAction(tr("Clear"),this);
    QAction *colorAction=new QAction(tr("Color"),this);
    m_pMenu->addAction(colorAction);
    m_pMenu->addSeparator();
    m_pMenu->addAction(clearAction);
    connect(clearAction,&QAction::triggered,this,&MainWindow::onDeleteClicked);
    connect(colorAction,&QAction::triggered,this,&MainWindow::onColorClicked);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    event->ignore();
    // 此处代码已封装到Draw函数
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons()==Qt::LeftButton)
    {
        m_bCliked=true;
        m_Point=event->pos();
    }
}

void MainWindow::onDeleteClicked()
{
    pointList.clear();
    needUpdate = true;
    update();
//    ui->lbl_show1->setPixmap(QPixmap::fromImage(disimage));
    ui->label->clear();   // 清空画布
}

void MainWindow::onColorClicked()
{
    QColor color = QColorDialog::getColor(QColor(255,0,0));

    m_R=color.red();
    m_G=color.green();
    m_B=color.blue();
}

void MainWindow::on_pushButton_clicked()
{
    if(ToolType == 0){
        ToolType = 1;
    }
    else{
        ToolType = 0;
    }

}

QPixmap MainWindow::overlapping(const QPixmap *backgroundPixmap, const QPixmap *overlayPixmap)
{
    QPixmap resultPixmap = *backgroundPixmap;

    // 使用 QPainter 在 resultPixmap 上绘制 overlayPixmap
    QPainter painter(&resultPixmap);
    painter.drawPixmap(0, 0, *overlayPixmap);

    return resultPixmap;
}


void MainWindow::MyDraw(QPoint m_Point,QPoint m_movePoint){
    QPainter painter(pixmap_mouse);

//    QPainter painter(ui->TestLabel);   // 虽然语法不报错，但运行后输出：QPainter::setPen: Painter not active；
    painter.setFont(m_Font);

    pen->setCapStyle(Qt::PenCapStyle::RoundCap);   //设为圆角
    pen->setWidth(5);  //粗细
//    pen->setColor(QColor(mypoint.m_r,mypoint.m_g,mypoint.m_b));
    pen->setColor(Qt::yellow);
    pen->setStyle(Qt::PenStyle::SolidLine);

    painter.setPen(*pen);
    painter.drawLine(m_Point.x(),m_Point.y(),m_movePoint.x(),m_movePoint.y());

    // 下面是原本用pointList来画图的做法
//    qDebug()<<"pointList的长度为："<<pointList.length();   // 实时输出当前点的个数
//    // 在画布pixmap上，以某点为原点建立坐标系，根据列表里的点坐标画点
//    for(int i=0;i<pointList.size();i++)
//    {
//        myPoint mypoint=pointList[i];

//        pen->setCapStyle(Qt::PenCapStyle::RoundCap);   //设为圆角
//        pen->setWidth(5);  //粗细
//        pen->setColor(QColor(mypoint.m_r,mypoint.m_g,mypoint.m_b));
//        pen->setStyle(Qt::PenStyle::SolidLine);

////            painter.setPen(pen);
//        painter.setPen(*pen);
//        painter.drawLine(m_Point.x(),m_Point.y(),m_movePoint.x(),m_movePoint.y());
//    }

////     ================将绘制好的图像设置为Label的显示内容
////     出现了循环调用的原因：setPixmap重绘QLabel时，会内置调用QWidget的paintEvent。
////     解决方法就是给QLabel指定“WA_OpaquePaintEvent”属性，避免透明背景，这样下层的控件图层不需要重绘来实现本控件的透明效果。
////        ui->lbl_show1->setPixmap(pixmap);

    ui->label->setPixmap(*pixmap_mouse);  // TODO：但是为啥label的setPixmap不会触发paintEvent？
}



// 法二法三
void MainWindow::sobel(){
    // 把划线后的QPixmap图片类型转为Mat类型
    QImage myImage = ui->lbl_show1->pixmap()->toImage();
    Mat srcImage(myImage.height(), myImage.width(), CV_8UC4, myImage.bits(), myImage.bytesPerLine());


    // 然后就是视频中的原装代码
    // 把srcImage转为单通道灰度图grayImage（因为这样便于处理，毕竟彩色图像就要分析3组原色的梯度，而灰度图像只要1组）
    cvtColor(srcImage, grayImage, COLOR_BGR2GRAY);


    // 法一：使用Sobel函数，计算xy方向的边缘检测图并加权（三个方法中最精密的，轮廓最清晰）
    Mat sobel_x, sobel_y;
    Sobel(grayImage, sobel_x,CV_8U,1, 0);  // 最后两个参数，参数dx表示x轴方向的求导阶数,参数dy表示y轴方向的求导阶数
    Sobel(grayImage, sobel_y,CV_8U,0, 1);
    // 把xy方向加权平均法（直接拿sobel_x当成edgeImage也行，就是只能显示出一个方向上检测到的边缘线）
    // 在这部分就是可以进行更好的算法设计，让边缘检测的效果更好。
    addWeighted(sobel_x, 0.5, sobel_y, 0.5, 0, edgeImage);

    threshold(edgeImage, edgeImage, 30, 255, cv::THRESH_BINARY);  // 灰度超过30就设为255

    // ==========================最后进行显示
    // 转换为RGB类型图像
    cvtColor(edgeImage, edgeImage, COLOR_GRAY2BGR);
    // Mat转换为QImage对象
    QImage displayImg = QImage(edgeImage.data, edgeImage.cols, edgeImage.rows, edgeImage.cols * edgeImage.channels(), QImage::Format_RGB888);
    QImage disimage = imageCenter(displayImg, ui->lbl_show2);
    // 显示图像到页面
    ui->lbl_show2->setPixmap(QPixmap::fromImage(disimage));

    // 设置鼠标跟踪，以便捕获鼠标移动事件
    ui->lbl_show2->setMouseTracking(true);

//    // 在这个pixmap上划线？画完之后再转为mat，再进行一次上面的边缘检测
//    const QPixmap *pixmap = ui->lbl_show2->pixmap();


    // =========================还有两种边缘检测算法如下，移到上面即可使用
    // 法二：调用canny边缘检测函数
    // 图片grayImage是单通道的，所以推测Canny就是要输入单通道灰度图才能进行边缘检测。输出的edgeImage也是单通道灰度图
//    Canny(grayImage, edgeImage, 200, 1);    // 两个阈值分别设为200和1（but具体啥意思）
    // 法三：拉普拉斯边缘检测
//     Laplacian(grayImage, edgeImage, grayImage.depth());
}

void MainWindow::clearLeft()
{
    QImage displayImg(srcImage.cols,srcImage.rows,QImage::Format_RGB32);
    displayImg.fill(Qt::black);

//    QImage displayImg = QImage(whiteImage.data, srcImage.cols, srcImage.rows, srcImage.cols * srcImage.channels(), QImage::Format_RGB888);
    QImage disimage = imageCenter(displayImg, ui->lbl_show1);
    // 显示图像到页面
    ui->lbl_show1->setPixmap(QPixmap::fromImage(disimage));
    // 清空画板
    ui->label->clear();
}


// 改变图片尺寸，使图片居中显示,图片大小与label大小相适应（该函数在mainwindow.h文件中声明，是public的函数）
QImage MainWindow::imageCenter(QImage  qimage,QLabel *qLabel)
{
    QImage image;
    QSize imageSize = qimage.size();
    QSize labelSize = qLabel->size();

    double dWidthRatio = 1.0*imageSize.width() / labelSize.width();
    double dHeightRatio = 1.0*imageSize.height() / labelSize.height();
    if (dWidthRatio>dHeightRatio)
    {
        image = qimage.scaledToWidth(labelSize.width());
    }
    else
    {
        image = qimage.scaledToHeight(labelSize.height());
    }
    return image;
}
