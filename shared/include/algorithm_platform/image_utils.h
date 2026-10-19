#pragma once

#include <QImage>

#include <opencv2/opencv.hpp>

namespace atp::image
{

cv::Mat qImageToBgrMat(const QImage& image);
QImage matToQImage(const cv::Mat& mat);
QImage drawTextPanel(const QImage& baseImage, const QStringList& lines);

} // namespace atp::image
