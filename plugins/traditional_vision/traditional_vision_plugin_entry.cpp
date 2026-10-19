#include <QObject>
#include <QtPlugin>

#include "algorithm_platform/plugin_contract.h"
#include "traditional_vision_plugin.h"

class TraditionalVisionPluginEntry final : public QObject, public atp::IAlgorithmPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AlgorithmTeachingPluginFactory_iid)
    Q_INTERFACES(atp::IAlgorithmPluginFactory)

public:
    QString pluginId() const override
    {
        return QStringLiteral("traditional_vision");
    }

    QString pluginName() const override
    {
        return QStringLiteral("传统视觉基础插件");
    }

    atp::IAlgorithmPlugin* create() const override
    {
        return new TraditionalVisionPlugin();
    }
};

#include "traditional_vision_plugin_entry.moc"
