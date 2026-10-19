#include "algorithm_platform/image_utils.h"

#include <QPainter>

namespace atp::image
{

cv::Mat qImageToBgrMat(const QImage& image)
{
    if (image.isNull()) {
        return cv::Mat();
    }

    const QImage rgbImage = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat rgbMat(rgbImage.height(),
                   rgbImage.width(),
                   CV_8UC3,
                   const_cast<uchar*>(rgbImage.bits()),
                   rgbImage.bytesPerLine());

    cv::Mat bgrMat;
    cv::cvtColor(rgbMat, bgrMat, cv::COLOR_RGB2BGR);
    return bgrMat.clone();
}

QImage matToQImage(const cv::Mat& mat)
{
    if (mat.empty()) {
        return QImage();
    }

    if (mat.type() == CV_8UC1) {
        QImage image(mat.data,
                     mat.cols,
                     mat.rows,
                     static_cast<int>(mat.step),
                     QImage::Format_Grayscale8);
        return image.copy();
    }

    if (mat.type() == CV_8UC3) {
        cv::Mat rgbMat;
        cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
        QImage image(rgbMat.data,
                     rgbMat.cols,
                     rgbMat.rows,
                     static_cast<int>(rgbMat.step),
                     QImage::Format_RGB888);
        return image.copy();
    }

    if (mat.type() == CV_8UC4) {
        QImage image(mat.data,
                     mat.cols,
                     mat.rows,
                     static_cast<int>(mat.step),
                     QImage::Format_ARGB32);
        return image.copy();
    }

    cv::Mat normalized;
    cv::normalize(mat, normalized, 0, 255, cv::NORM_MINMAX);
    normalized.convertTo(normalized, CV_8U);
    return matToQImage(normalized);
}

QImage drawTextPanel(const QImage& baseImage, const QStringList& lines)
{
    if (baseImage.isNull()) {
        return QImage();
    }

    QImage output = baseImage.convertToFormat(QImage::Format_ARGB32);
    QPainter painter(&output);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int margin = 12;
    const int lineHeight = 22;
    const int panelHeight = margin * 2 + qMax(1, lines.size()) * lineHeight;
    QRect panelRect(margin, margin, output.width() - margin * 2, panelHeight);

    painter.fillRect(panelRect, QColor(0, 0, 0, 150));
    painter.setPen(Qt::white);

    int y = margin + 18;
    for (const QString& line : lines) {
        painter.drawText(margin * 2, y, line);
        y += lineHeight;
    }

    return output;
}

} // namespace atp::image
