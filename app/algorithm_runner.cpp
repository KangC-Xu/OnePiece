#include "algorithm_runner.h"

AlgorithmRunner::AlgorithmRunner(PluginLoader* loader, QObject* parent)
    : QObject(parent)
    , _loader(loader)
{
}

atp::AlgorithmResult AlgorithmRunner::run(const QString& algorithmId,
                                          const QImage& image,
                                          const QString& imagePath,
                                          const QMap<QString, QVariant>& params) const
{
    atp::AlgorithmResult result;

    if (!_loader) {
        result.message = QStringLiteral("运行器没有绑定插件加载器");
        return result;
    }

    const PluginLoader::LoadedPlugin* pluginInfo = _loader->findById(algorithmId);
    if (!pluginInfo || !pluginInfo->factory) {
        result.message = QStringLiteral("没有找到算法插件：%1").arg(algorithmId);
        return result;
    }

    std::unique_ptr<atp::IAlgorithmPlugin> plugin(pluginInfo->factory->create());
    if (!plugin) {
        result.message = QStringLiteral("工厂创建算法对象失败：%1").arg(algorithmId);
        return result;
    }

    QString initError;
    if (!plugin->initialize(&initError)) {
        result.message = QStringLiteral("算法初始化失败：%1").arg(initError);
        return result;
    }

    atp::AlgorithmRequest request;
    request.algorithmId = algorithmId;
    request.image = image;
    request.imagePath = imagePath;
    request.params = params;

    return plugin->run(request);
}
