#include "deep_learning_plugin.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <vector>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QPen>

#include <opencv2/imgproc.hpp>

#if ATP_ONNXRUNTIME_ENABLED
#include <onnxruntime_cxx_api.h>
#endif

#include "algorithm_platform/image_utils.h"

namespace
{

QString resolveRuntimePath(const QString& path)
{
    const QString trimmed = path.trimmed();
    if (trimmed.isEmpty()) {
        return QString();
    }

    QFileInfo info(trimmed);
    if (info.isAbsolute() && info.exists()) {
        return info.absoluteFilePath();
    }

    QDir dir(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 6; ++i) {
        const QString candidate = QDir(dir.absolutePath()).absoluteFilePath(trimmed);
        if (QFileInfo::exists(candidate)) {
            return QDir::cleanPath(candidate);
        }
        if (!dir.cdUp()) {
            break;
        }
    }

    return QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(trimmed);
}

std::vector<float> softmax(const std::vector<float>& logits)
{
    if (logits.empty()) {
        return {};
    }

    const float maxValue = *std::max_element(logits.begin(), logits.end());
    std::vector<float> expValues(logits.size());
    float sum = 0.0f;

    for (size_t i = 0; i < logits.size(); ++i) {
        expValues[i] = std::exp(logits[i] - maxValue);
        sum += expValues[i];
    }

    if (sum <= 0.0f) {
        return expValues;
    }

    for (float& value : expValues) {
        value /= sum;
    }
    return expValues;
}

QImage renderModelPreview(const QImage& image, int classIndex, float score, double threshold)
{
    QImage output = image.convertToFormat(QImage::Format_ARGB32);
    QPainter painter(&output);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const bool confident = score >= threshold;
    painter.setPen(QPen(confident ? QColor(60, 200, 110) : QColor(255, 180, 60), 5));
    painter.drawRect(output.rect().adjusted(8, 8, -8, -8));

    Q_UNUSED(classIndex);
    return output;
}

#if ATP_ONNXRUNTIME_ENABLED

std::vector<float> buildMobileNetInput(const QImage& image, int inputSize)
{
    const cv::Mat bgr = atp::image::qImageToBgrMat(image);
    if (bgr.empty()) {
        return {};
    }

    cv::Mat resized;
    cv::resize(bgr, resized, cv::Size(inputSize, inputSize), 0.0, 0.0, cv::INTER_LINEAR);
    cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB);
    resized.convertTo(resized, CV_32FC3, 1.0 / 255.0);

    const float mean[3] = {0.485f, 0.456f, 0.406f};
    const float stddev[3] = {0.229f, 0.224f, 0.225f};

    std::vector<float> input(3 * inputSize * inputSize);
    for (int y = 0; y < inputSize; ++y) {
        const cv::Vec3f* row = resized.ptr<cv::Vec3f>(y);
        for (int x = 0; x < inputSize; ++x) {
            for (int c = 0; c < 3; ++c) {
                const int dstIndex = c * inputSize * inputSize + y * inputSize + x;
                input[dstIndex] = (row[x][c] - mean[c]) / stddev[c];
            }
        }
    }

    return input;
}

bool runOnnxClassification(const QString& modelPath,
                           const QImage& image,
                           int inputSize,
                           int& classIndex,
                           float& score,
                           QString& errorMessage)
{
    errorMessage.clear();
    classIndex = -1;
    score = 0.0f;

    const std::vector<float> input = buildMobileNetInput(image, inputSize);
    if (input.empty()) {
        errorMessage = QStringLiteral("图像预处理失败");
        return false;
    }

    try {
        Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "AlgorithmTeachingPlatform");
        Ort::SessionOptions options;
        options.SetIntraOpNumThreads(1);
        options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);

        const std::wstring wideModelPath = QDir::toNativeSeparators(modelPath).toStdWString();
        Ort::Session session(env, wideModelPath.c_str(), options);
        Ort::AllocatorWithDefaultOptions allocator;

        Ort::AllocatedStringPtr inputName = session.GetInputNameAllocated(0, allocator);
        Ort::AllocatedStringPtr outputName = session.GetOutputNameAllocated(0, allocator);
        const char* inputNames[] = {inputName.get()};
        const char* outputNames[] = {outputName.get()};

        std::array<int64_t, 4> inputShape = {1, 3, inputSize, inputSize};
        Ort::MemoryInfo memoryInfo =
            Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memoryInfo,
            const_cast<float*>(input.data()),
            input.size(),
            inputShape.data(),
            inputShape.size());

        std::vector<Ort::Value> outputs = session.Run(Ort::RunOptions{nullptr},
                                                      inputNames,
                                                      &inputTensor,
                                                      1,
                                                      outputNames,
                                                      1);

        if (outputs.empty() || !outputs[0].IsTensor()) {
            errorMessage = QStringLiteral("模型输出为空");
            return false;
        }

        float* outputData = outputs[0].GetTensorMutableData<float>();
        const Ort::TensorTypeAndShapeInfo shapeInfo = outputs[0].GetTensorTypeAndShapeInfo();
        const size_t outputCount = shapeInfo.GetElementCount();
        if (!outputData || outputCount == 0) {
            errorMessage = QStringLiteral("模型输出数据为空");
            return false;
        }

        std::vector<float> logits(outputData, outputData + outputCount);
        const std::vector<float> scores = softmax(logits);
        const auto it = std::max_element(scores.begin(), scores.end());
        classIndex = static_cast<int>(std::distance(scores.begin(), it));
        score = it == scores.end() ? 0.0f : *it;
        return true;
    } catch (const Ort::Exception& ex) {
        errorMessage = QStringLiteral("ONNX Runtime 推理异常：%1").arg(QString::fromUtf8(ex.what()));
        return false;
    } catch (const std::exception& ex) {
        errorMessage = QStringLiteral("模型推理异常：%1").arg(QString::fromUtf8(ex.what()));
        return false;
    }
}

#endif

} // namespace

atp::AlgorithmMeta DeepLearningPlugin::meta() const
{
    atp::AlgorithmMeta meta;
    meta.id = QStringLiteral("deep_learning");
    meta.name = QStringLiteral("MobileNet 模型推理");
    meta.category = QStringLiteral("深度学习");
    meta.teachingPoint = QStringLiteral("学习模型路径、预处理、ONNX Runtime 推理、Top1 分类结果和置信度阈值。");

    atp::ParamMeta modelPath;
    modelPath.key = QStringLiteral("model_path");
    modelPath.name = QStringLiteral("模型路径");
    modelPath.hint = QStringLiteral("默认使用 3rd/models/mobilenet/mobilenet_v2.onnx。");
    modelPath.editor = atp::ParamEditor::Text;
    modelPath.defaultValue = QStringLiteral("3rd/models/mobilenet/mobilenet_v2.onnx");
    meta.params.push_back(modelPath);

    atp::ParamMeta inputSize;
    inputSize.key = QStringLiteral("input_size");
    inputSize.name = QStringLiteral("输入尺寸");
    inputSize.hint = QStringLiteral("MobileNet 常用 224。这里按正方形输入处理。");
    inputSize.editor = atp::ParamEditor::Int;
    inputSize.defaultValue = 224;
    inputSize.minValue = 32;
    inputSize.maxValue = 1024;
    inputSize.stepValue = 32;
    meta.params.push_back(inputSize);

    atp::ParamMeta threshold;
    threshold.key = QStringLiteral("confidence_threshold");
    threshold.name = QStringLiteral("置信度阈值");
    threshold.hint = QStringLiteral("Top1 分数低于阈值时提示不够可信。");
    threshold.editor = atp::ParamEditor::Double;
    threshold.defaultValue = 0.20;
    threshold.minValue = 0.0;
    threshold.maxValue = 1.0;
    threshold.stepValue = 0.05;
    meta.params.push_back(threshold);

    return meta;
}

bool DeepLearningPlugin::initialize(QString* errorMessage)
{
    if (errorMessage) {
        errorMessage->clear();
    }
    _initialized = true;
    return true;
}

atp::AlgorithmResult DeepLearningPlugin::run(const atp::AlgorithmRequest& request)
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

    const QString modelPath = resolveRuntimePath(
        request.params.value(QStringLiteral("model_path")).toString());
    const int inputSize = request.params.value(QStringLiteral("input_size"), 224).toInt();
    const double threshold = request.params.value(QStringLiteral("confidence_threshold"), 0.20).toDouble();

    if (!QFileInfo::exists(modelPath)) {
        result.message = QStringLiteral("模型文件不存在：%1").arg(modelPath);
        return result;
    }

#if ATP_ONNXRUNTIME_ENABLED
    int classIndex = -1;
    float score = 0.0f;
    QString errorMessage;
    if (!runOnnxClassification(modelPath, request.image, inputSize, classIndex, score, errorMessage)) {
        result.message = errorMessage;
        return result;
    }

    result.success = true;
    result.message = QStringLiteral("真实模型推理完成：Top1 类别 %1，分数 %2")
                         .arg(classIndex)
                         .arg(score, 0, 'f', 4);
    result.preview = atp::image::drawTextPanel(
        renderModelPreview(request.image, classIndex, score, threshold),
        {QStringLiteral("插件：MobileNet 模型推理"),
         QStringLiteral("模型：%1").arg(QFileInfo(modelPath).fileName()),
         QStringLiteral("Top1 类别：%1").arg(classIndex),
         QStringLiteral("Top1 分数：%1").arg(score, 0, 'f', 4),
         QStringLiteral("阈值：%1").arg(threshold, 0, 'f', 2)});
    result.notes.push_back(QStringLiteral("这次已经通过 ONNX Runtime 执行模型，不是模拟结果。"));
    result.notes.push_back(QStringLiteral("当前没有 ImageNet 标签表，所以先显示类别索引；后续可增加 labels.txt 显示类别名。"));
    result.values.insert(QStringLiteral("模型路径"), modelPath);
    result.values.insert(QStringLiteral("输入尺寸"), inputSize);
    result.values.insert(QStringLiteral("Top1类别"), classIndex);
    result.values.insert(QStringLiteral("Top1分数"), score);
    result.values.insert(QStringLiteral("是否高于阈值"), score >= threshold);
    return result;
#else
    result.message = QStringLiteral("当前构建没有启用 ONNX Runtime，无法运行真实模型");
    return result;
#endif
}
