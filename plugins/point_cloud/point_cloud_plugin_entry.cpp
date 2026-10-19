#include <QObject>
#include <QtPlugin>

#include "algorithm_platform/plugin_contract.h"
#include "point_cloud_plugin.h"

class PointCloudPluginEntry final : public QObject, public atp::IAlgorithmPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AlgorithmTeachingPluginFactory_iid)
    Q_INTERFACES(atp::IAlgorithmPluginFactory)

public:
    QString pluginId() const override
    {
        return QStringLiteral("point_cloud");
    }

    QString pluginName() const override
    {
        return QStringLiteral("点云基础插件");
    }

    atp::IAlgorithmPlugin* create() const override
    {
        return new PointCloudPlugin();
    }
};

#include "point_cloud_plugin_entry.moc"
