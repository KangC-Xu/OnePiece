#include "plugin_loader.h"

#include <QFileInfo>

QStringList PluginLoader::libraryNameFilters()
{
#ifdef Q_OS_WIN
    return {"*.dll"};
#elif defined(Q_OS_MACOS)
    return {"*.dylib"};
#else
    return {"*.so"};
#endif
}

int PluginLoader::scan(const QString& pluginDir, QStringList* errors)
{
    _plugins.clear();
    _loaders.clear();

    QDir dir(pluginDir);
    if (!dir.exists()) {
        if (errors) {
            errors->push_back(QStringLiteral("插件目录不存在：%1").arg(pluginDir));
        }
        return 0;
    }

    const QFileInfoList files = dir.entryInfoList(libraryNameFilters(),
                                                  QDir::Files | QDir::Readable,
                                                  QDir::Name);

    for (const QFileInfo& file : files) {
        auto loader = std::make_shared<QPluginLoader>(file.absoluteFilePath());
        QObject* object = loader->instance();
        if (!object) {
            if (errors) {
                errors->push_back(QStringLiteral("加载失败：%1，原因：%2")
                                      .arg(file.fileName(), loader->errorString()));
            }
            continue;
        }

        auto* factory = qobject_cast<atp::IAlgorithmPluginFactory*>(object);
        if (!factory) {
            if (errors) {
                errors->push_back(QStringLiteral("不是教学平台插件：%1").arg(file.fileName()));
            }
            loader->unload();
            continue;
        }

        std::unique_ptr<atp::IAlgorithmPlugin> plugin(factory->create());
        if (!plugin) {
            if (errors) {
                errors->push_back(QStringLiteral("工厂创建插件失败：%1").arg(file.fileName()));
            }
            loader->unload();
            continue;
        }

        QString initError;
        if (!plugin->initialize(&initError)) {
            if (errors) {
                errors->push_back(QStringLiteral("插件初始化失败：%1，原因：%2")
                                      .arg(file.fileName(), initError));
            }
            loader->unload();
            continue;
        }

        LoadedPlugin info;
        info.filePath = file.absoluteFilePath();
        info.pluginId = factory->pluginId();
        info.pluginName = factory->pluginName();
        info.meta = plugin->meta();
        info.factory = factory;

        _plugins.push_back(info);
        _loaders.push_back(loader);
    }

    return _plugins.size();
}

const QList<PluginLoader::LoadedPlugin>& PluginLoader::plugins() const
{
    return _plugins;
}

const PluginLoader::LoadedPlugin* PluginLoader::findById(const QString& algorithmId) const
{
    const QString id = algorithmId.trimmed();
    for (const LoadedPlugin& plugin : _plugins) {
        if (plugin.pluginId == id || plugin.meta.id == id) {
            return &plugin;
        }
    }
    return nullptr;
}

QStringList PluginLoader::pluginNamesForLog() const
{
    QStringList lines;
    for (const LoadedPlugin& plugin : _plugins) {
        lines.push_back(QStringLiteral("%1：%2（%3）")
                            .arg(plugin.meta.category, plugin.meta.name, plugin.pluginId));
    }
    return lines;
}
