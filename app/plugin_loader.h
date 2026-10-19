#pragma once

#include <memory>

#include <QDir>
#include <QList>
#include <QPluginLoader>
#include <QString>
#include <QStringList>

#include "algorithm_platform/plugin_contract.h"

class PluginLoader
{
public:
    struct LoadedPlugin
    {
        QString filePath;
        QString pluginId;
        QString pluginName;
        atp::AlgorithmMeta meta;
        atp::IAlgorithmPluginFactory* factory = nullptr;
    };

    int scan(const QString& pluginDir, QStringList* errors);
    const QList<LoadedPlugin>& plugins() const;
    const LoadedPlugin* findById(const QString& algorithmId) const;

    QStringList pluginNamesForLog() const;

private:
    static QStringList libraryNameFilters();

private:
    QList<LoadedPlugin> _plugins;
    QList<std::shared_ptr<QPluginLoader>> _loaders;
};
