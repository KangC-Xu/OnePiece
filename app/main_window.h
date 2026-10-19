#pragma once

#include <QComboBox>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMap>
#include <QPushButton>
#include <QScrollArea>
#include <QTextEdit>
#include <QVariant>
#include <QVBoxLayout>

#include "algorithm_runner.h"
#include "plugin_loader.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void reloadPlugins();
    void chooseImage();
    void runCurrentAlgorithm();
    void onAlgorithmChanged(int index);

private:
    QWidget* buildLeftPanel();
    QWidget* buildCenterPanel();
    QWidget* buildRightPanel();

    void rebuildAlgorithmList();
    void rebuildParamForm(const atp::AlgorithmMeta& meta);
    void setImagePreview(const QImage& image);
    void setResultPreview(const QImage& image);
    void appendLog(const QString& text);
    void appendNotes(const atp::AlgorithmResult& result);
    QMap<QString, QVariant> collectParams() const;

private:
    PluginLoader _pluginLoader;
    AlgorithmRunner _runner;

    QComboBox* _algorithmBox = nullptr;
    QLabel* _teachingPointLabel = nullptr;
    QLabel* _inputImageLabel = nullptr;
    QLabel* _resultImageLabel = nullptr;
    QTextEdit* _logEdit = nullptr;
    QScrollArea* _paramScrollArea = nullptr;
    QVBoxLayout* _paramLayout = nullptr;
    QWidget* _paramPanel = nullptr;
    QPushButton* _runButton = nullptr;

    QImage _currentImage;
    QString _currentImagePath;
    QMap<QString, QWidget*> _paramEditors;
};
