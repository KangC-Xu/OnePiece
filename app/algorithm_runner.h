#pragma once

#include <QImage>
#include <QMap>
#include <QObject>
#include <QVariant>

#include "plugin_loader.h"

class AlgorithmRunner : public QObject
{
    Q_OBJECT

public:
    explicit AlgorithmRunner(PluginLoader* loader, QObject* parent = nullptr);

    atp::AlgorithmResult run(const QString& algorithmId,
                             const QImage& image,
                             const QString& imagePath,
                             const QMap<QString, QVariant>& params) const;

private:
    PluginLoader* _loader = nullptr;
};
