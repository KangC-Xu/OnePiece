#pragma once

#include <QImage>
#include <QList>
#include <QMap>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace atp
{

enum class ParamEditor
{
    Text,
    Int,
    Double,
    Bool,
    Choice,
    ImageFile
};

struct ParamChoice
{
    QString text;
    QVariant value;
};

struct ParamMeta
{
    QString key;
    QString name;
    QString hint;
    ParamEditor editor = ParamEditor::Text;
    QVariant defaultValue;
    QVariant minValue;
    QVariant maxValue;
    QVariant stepValue;
    QList<ParamChoice> choices;
};

struct AlgorithmMeta
{
    QString id;
    QString name;
    QString category;
    QString teachingPoint;
    QList<ParamMeta> params;
};

struct AlgorithmRequest
{
    QString algorithmId;
    QString imagePath;
    QImage image;
    QMap<QString, QVariant> params;
};

struct AlgorithmResult
{
    bool success = false;
    QString message;
    QImage preview;
    QStringList notes;
    QMap<QString, QVariant> values;
};

class IAlgorithmPlugin
{
public:
    virtual ~IAlgorithmPlugin() = default;

    virtual AlgorithmMeta meta() const = 0;
    virtual bool initialize(QString* errorMessage) = 0;
    virtual AlgorithmResult run(const AlgorithmRequest& request) = 0;
};

class IAlgorithmPluginFactory
{
public:
    virtual ~IAlgorithmPluginFactory() = default;

    virtual QString pluginId() const = 0;
    virtual QString pluginName() const = 0;
    virtual IAlgorithmPlugin* create() const = 0;
};

} // namespace atp

Q_DECLARE_METATYPE(atp::AlgorithmResult)

#define AlgorithmTeachingPluginFactory_iid "com.onepiece.algorithm_teaching_plugin_factory/1.0"
Q_DECLARE_INTERFACE(atp::IAlgorithmPluginFactory, AlgorithmTeachingPluginFactory_iid)
