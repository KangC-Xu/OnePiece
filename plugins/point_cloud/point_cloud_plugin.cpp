#include "point_cloud_plugin.h"

#include <numeric>
#include <vector>

#include <QColor>
#include <QPainter>

#include "algorithm_platform/image_utils.h"

namespace
{

struct Point3D
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

std::vector<Point3D> imageToPointCloud(const QImage& image, int step)
{
    std::vector<Point3D> points;
    const QImage gray = image.convertToFormat(QImage::Format_Grayscale8);

    for (int y = 0; y < gray.height(); y += step) {
        const uchar* row = gray.constScanLine(y);
        for (int x = 0; x < gray.width(); x += step) {
            const int brightness = row[x];
            points.push_back({static_cast<double>(x),
                              static_cast<double>(y),
                              static_cast<double>(brightness)});
        }
    }

    return points;
}

QImage renderDepthMap(const QImage& source, int step)
{
    QImage preview(source.size(), QImage::Format_ARGB32);
    preview.fill(QColor(20, 24, 28));

    QPainter painter(&preview);
    painter.setRenderHint(QPainter::Antialiasing, false);

    const QImage gray = source.convertToFormat(QImage::Format_Grayscale8);
    for (int y = 0; y < gray.height(); y += step) {
        const uchar* row = gray.constScanLine(y);
        for (int x = 0; x < gray.width(); x += step) {
            const int z = row[x];
            painter.fillRect(QRect(x, y, step, step), QColor::fromHsv(220 - z * 220 / 255, 210, 235));
        }
    }

    return preview;
}

double averageZ(const std::vector<Point3D>& points)
{
    if (points.empty()) {
        return 0.0;
    }

    const double sum = std::accumulate(points.begin(), points.end(), 0.0, [](double value, const Point3D& point) {
        return value + point.z;
    });
    return sum / static_cast<double>(points.size());
}

} // namespace

atp::AlgorithmMeta PointCloudPlugin::meta() const
{
    atp::AlgorithmMeta meta;
    meta.id = QStringLiteral("point_cloud");
    meta.name = QStringLiteral("点云基础");
    meta.category = QStringLiteral("点云算法");
    meta.teachingPoint = QStringLiteral("把灰度图模拟成高度图，学习点云下采样、Z 值统计和深度图预览。");

    atp::ParamMeta step;
    step.key = QStringLiteral("sample_step");
    step.name = QStringLiteral("采样步长");
    step.hint = QStringLiteral("步长越大，点越少，运行越快，但细节越少。");
    step.editor = atp::ParamEditor::Int;
    step.defaultValue = 6;
    step.minValue = 1;
    step.maxValue = 40;
    step.stepValue = 1;
    meta.params.push_back(step);

    atp::ParamMeta fitPlane;
    fitPlane.key = QStringLiteral("fit_plane");
    fitPlane.name = QStringLiteral("计算平均平面");
    fitPlane.hint = QStringLiteral("教学版先用平均 Z 值模拟平面高度。");
    fitPlane.editor = atp::ParamEditor::Bool;
    fitPlane.defaultValue = true;
    meta.params.push_back(fitPlane);

    return meta;
}

bool PointCloudPlugin::initialize(QString* errorMessage)
{
    if (errorMessage) {
        errorMessage->clear();
    }
    _initialized = true;
    return true;
}

atp::AlgorithmResult PointCloudPlugin::run(const atp::AlgorithmRequest& request)
{
    atp::AlgorithmResult result;
    if (!_initialized) {
        result.message = QStringLiteral("插件还没有初始化");
        return result;
    }
    if (request.image.isNull()) {
        result.message = QStringLiteral("输入图像为空");
        return result;
    }

    const int step = qMax(1, request.params.value(QStringLiteral("sample_step"), 6).toInt());
    const bool fitPlane = request.params.value(QStringLiteral("fit_plane"), true).toBool();
    const std::vector<Point3D> points = imageToPointCloud(request.image, step);
    const double meanZ = averageZ(points);

    result.success = true;
    result.message = QStringLiteral("点云样例执行完成");
    result.preview = atp::image::drawTextPanel(
        renderDepthMap(request.image, step),
        {QStringLiteral("插件：点云基础"),
         QStringLiteral("点数量：%1").arg(points.size()),
         QStringLiteral("平均 Z：%1").arg(meanZ, 0, 'f', 2)});
    result.notes.push_back(QStringLiteral("这里用灰度值模拟 Z 高度，真实项目可替换为相机或 PCD 文件读入。"));
    result.notes.push_back(QStringLiteral("下采样是点云处理里最常见的第一步，用来降低点数和计算量。"));

    result.values.insert(QStringLiteral("点数量"), static_cast<int>(points.size()));
    result.values.insert(QStringLiteral("平均Z"), meanZ);
    result.values.insert(QStringLiteral("是否拟合平面"), fitPlane);
    return result;
}
