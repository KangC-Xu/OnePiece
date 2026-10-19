#include <QObject>
#include <QtPlugin>

#include "algorithm_platform/plugin_contract.h"
#include "deep_learning_plugin.h"

class DeepLearningPluginEntry final : public QObject, public atp::IAlgorithmPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AlgorithmTeachingPluginFactory_iid)
    Q_INTERFACES(atp::IAlgorithmPluginFactory)

public:
    QString pluginId() const override
    {
        return QStringLiteral("deep_learning");
    }

    QString pluginName() const override
    {
        return QStringLiteral("深度学习流程插件");
    }

    atp::IAlgorithmPlugin* create() const override
    {
        return new DeepLearningPlugin();
    }
};

#include "deep_learning_plugin_entry.moc"
