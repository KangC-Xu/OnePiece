#include "traditional_vision_plugin.h"

#include <opencv2/imgproc.hpp>

#include "algorithm_platform/image_utils.h"

namespace
{

atp::ParamMeta choiceParam()
{
    atp::ParamMeta param;
    param.key = QStringLiteral("method");
    param.name = QStringLiteral("算法");
    param.hint = QStringLiteral("同一个插件内可以用参数选择不同传统算子。");
    param.editor = atp::ParamEditor::Choice;
    param.defaultValue = QStringLiteral("threshold");
    param.choices = {
        {QStringLiteral("二值阈值"), QStringLiteral("threshold")},
        {QStringLiteral("Canny 边缘"), QStringLiteral("canny")},
        {QStringLiteral("轮廓提取"), QStringLiteral("contour")}
    };
    return param;
}

cv::Mat toGray(const cv::Mat& bgr)
{
    cv::Mat gray;
    if (bgr.channels() == 1) {
        gray = bgr.clone();
    } else {
        cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    }
    return gray;
}

} // namespace

atp::AlgorithmMeta TraditionalVisionPlugin::meta() const
{
    atp::AlgorithmMeta meta;
    meta.id = QStringLiteral("traditional_vision");
    meta.name = QStringLiteral("传统视觉基础");
    meta.category = QStringLiteral("传统算法");
    meta.teachingPoint = QStringLiteral("学习 OpenCV 的灰度、阈值、边缘和轮廓，并理解插件只负责算法实现。");

    meta.params.push_back(choiceParam());

    atp::ParamMeta threshold;
    threshold.key = QStringLiteral("threshold");
    threshold.name = QStringLiteral("阈值");
    threshold.hint = QStringLiteral("二值化和轮廓提取都会用到这个阈值。");
    threshold.editor = atp::ParamEditor::Int;
    threshold.defaultValue = 120;
    threshold.minValue = 0;
    threshold.maxValue = 255;
    threshold.stepValue = 1;
    meta.params.push_back(threshold);

    atp::ParamMeta low;
    low.key = QStringLiteral("canny_low");
    low.name = QStringLiteral("Canny 低阈值");
    low.hint = QStringLiteral("边缘检测的低阈值。");
    low.editor = atp::ParamEditor::Int;
    low.defaultValue = 60;
    low.minValue = 0;
    low.maxValue = 255;
    low.stepValue = 1;
    meta.params.push_back(low);

    atp::ParamMeta high;
    high.key = QStringLiteral("canny_high");
    high.name = QStringLiteral("Canny 高阈值");
    high.hint = QStringLiteral("边缘检测的高阈值。");
    high.editor = atp::ParamEditor::Int;
    high.defaultValue = 180;
    high.minValue = 0;
    high.maxValue = 255;
    high.stepValue = 1;
    meta.params.push_back(high);

    return meta;
}

bool TraditionalVisionPlugin::initialize(QString* errorMessage)
{
    if (errorMessage) {
        errorMessage->clear();
    }
    _initialized = true;
    return true;
}

atp::AlgorithmResult TraditionalVisionPlugin::run(const atp::AlgorithmRequest& request)
{
    atp::AlgorithmResult result;
    if (!_initialized) {
        result.message = QStringLiteral("插件还没有初始化");
        return result;
    }

    const cv::Mat bgr = atp::image::qImageToBgrMat(request.image);
    if (bgr.empty()) {
        result.message = QStringLiteral("输入图像为空");
        return result;
    }

    const QString method = request.params.value(QStringLiteral("method"), QStringLiteral("threshold")).toString();
    const int thresholdValue = request.params.value(QStringLiteral("threshold"), 120).toInt();
    const cv::Mat gray = toGray(bgr);

    cv::Mat output;
    if (method == QStringLiteral("canny")) {
        const int low = request.params.value(QStringLiteral("canny_low"), 60).toInt();
        const int high = request.params.value(QStringLiteral("canny_high"), 180).toInt();
        cv::Canny(gray, output, low, high);
        result.notes.push_back(QStringLiteral("Canny 用一高一低两个阈值找灰度突变边界。"));
    } else if (method == QStringLiteral("contour")) {
        cv::Mat binary;
        cv::threshold(gray, binary, thresholdValue, 255, cv::THRESH_BINARY);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        output = bgr.clone();
        cv::drawContours(output, contours, -1, cv::Scalar(0, 255, 0), 2);
        result.values.insert(QStringLiteral("轮廓数量"), static_cast<int>(contours.size()));
        result.notes.push_back(QStringLiteral("轮廓提取通常先二值化，再找连通边界。"));
    } else {
        cv::threshold(gray, output, thresholdValue, 255, cv::THRESH_BINARY);
        result.notes.push_back(QStringLiteral("二值阈值是最基础的传统视觉分割方法。"));
    }

    result.success = true;
    result.message = QStringLiteral("传统视觉算法执行完成");
    result.preview = atp::image::drawTextPanel(
        atp::image::matToQImage(output),
        {QStringLiteral("插件：传统视觉基础"),
         QStringLiteral("方法：%1").arg(method),
         QStringLiteral("输入：%1 x %2").arg(request.image.width()).arg(request.image.height())});
    return result;
}
