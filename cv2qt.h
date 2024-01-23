#ifndef CV2QT_H
#define CV2QT_H

#include <QDebug>
#include <QImage>
#include <QPixmap>

//#include "opencv2/opencv.hpp"
#include <opencv2/opencv.hpp>

class CV2Qt
{
public:
    CV2Qt();
    QImage  cvMatToQImage( const cv::Mat &inMat );
    QPixmap cvMatToQPixmap( const cv::Mat &inMat );
};

#endif // CV2QT_H
