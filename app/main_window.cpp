#include "main_window.h"

#include <QApplication>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QImageReader>
#include <QMessageBox>
#include <QSpinBox>
#include <QSplitter>

namespace
{

QLabel* createTitle(const QString& text)
{
    auto* label = new QLabel(text);
    label->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 700;"));
    return label;
}

QLabel* createImageBox(const QString& placeholder)
{
    auto* label = new QLabel(placeholder);
    label->setMinimumSize(360, 260);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    label->setStyleSheet(QStringLiteral(
        "QLabel { background: #181c20; color: #c8d0d8; border: 1px solid #353b43; }"));
    return label;
}

QString pluginDirPath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("plugins"));
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , _runner(&_pluginLoader, this)
{
    resize(1280, 820);
    setWindowTitle(QStringLiteral("算法插件教学平台"));

    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(buildLeftPanel());
    splitter->addWidget(buildCenterPanel());
    splitter->addWidget(buildRightPanel());
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setStretchFactor(2, 1);
    splitter->setSizes({330, 520, 430});

    setCentralWidget(splitter);

    reloadPlugins();
}

QWidget* MainWindow::buildLeftPanel()
{
    auto* panel = new QWidget(this);
    auto* layout = new QVBoxLayout(panel);

    _algorithmBox = new QComboBox(panel);
    connect(_algorithmBox, &QComboBox::currentIndexChanged, this, &MainWindow::onAlgorithmChanged);

    auto* reloadButton = new QPushButton(QStringLiteral("重新扫描插件"), panel);
    connect(reloadButton, &QPushButton::clicked, this, &MainWindow::reloadPlugins);

    _teachingPointLabel = new QLabel(panel);
    _teachingPointLabel->setWordWrap(true);
    _teachingPointLabel->setStyleSheet(QStringLiteral("color: #425466;"));

    _paramScrollArea = new QScrollArea(panel);
    _paramScrollArea->setWidgetResizable(true);

    layout->addWidget(createTitle(QStringLiteral("1. 选择插件算法")));
    layout->addWidget(_algorithmBox);
    layout->addWidget(reloadButton);
    layout->addWidget(_teachingPointLabel);
    layout->addSpacing(12);
    layout->addWidget(createTitle(QStringLiteral("2. 修改参数")));
    layout->addWidget(_paramScrollArea, 1);

    return panel;
}

QWidget* MainWindow::buildCenterPanel()
{
    auto* panel = new QWidget(this);
    auto* layout = new QVBoxLayout(panel);

    _inputImageLabel = createImageBox(QStringLiteral("选择一张图片后在这里预览"));

    auto* chooseButton = new QPushButton(QStringLiteral("选择输入图片"), panel);
    connect(chooseButton, &QPushButton::clicked, this, &MainWindow::chooseImage);

    _runButton = new QPushButton(QStringLiteral("运行当前算法"), panel);
    _runButton->setMinimumHeight(42);
    connect(_runButton, &QPushButton::clicked, this, &MainWindow::runCurrentAlgorithm);

    layout->addWidget(createTitle(QStringLiteral("3. 输入图像")));
    layout->addWidget(_inputImageLabel, 1);
    layout->addWidget(chooseButton);
    layout->addSpacing(8);
    layout->addWidget(createTitle(QStringLiteral("4. 执行")));
    layout->addWidget(_runButton);

    return panel;
}

QWidget* MainWindow::buildRightPanel()
{
    auto* panel = new QWidget(this);
    auto* layout = new QVBoxLayout(panel);

    _resultImageLabel = createImageBox(QStringLiteral("算法结果会显示在这里"));

    _logEdit = new QTextEdit(panel);
    _logEdit->setReadOnly(true);
    _logEdit->document()->setMaximumBlockCount(1000);

    layout->addWidget(createTitle(QStringLiteral("5. 结果预览")));
    layout->addWidget(_resultImageLabel, 1);
    layout->addWidget(createTitle(QStringLiteral("6. 运行日志")));
    layout->addWidget(_logEdit, 1);

    return panel;
}

void MainWindow::reloadPlugins()
{
    QStringList errors;
    const int count = _pluginLoader.scan(pluginDirPath(), &errors);

    appendLog(QStringLiteral("扫描插件目录：%1").arg(pluginDirPath()));
    appendLog(QStringLiteral("已加载插件数量：%1").arg(count));
    for (const QString& line : _pluginLoader.pluginNamesForLog()) {
        appendLog(QStringLiteral("  - %1").arg(line));
    }
    for (const QString& error : errors) {
        appendLog(QStringLiteral("错误：%1").arg(error));
    }

    rebuildAlgorithmList();
}

void MainWindow::rebuildAlgorithmList()
{
    _algorithmBox->blockSignals(true);
    _algorithmBox->clear();

    for (const PluginLoader::LoadedPlugin& plugin : _pluginLoader.plugins()) {
        _algorithmBox->addItem(QStringLiteral("%1 - %2").arg(plugin.meta.category, plugin.meta.name),
                               plugin.pluginId);
    }

    _algorithmBox->blockSignals(false);
    onAlgorithmChanged(_algorithmBox->currentIndex());
}

void MainWindow::onAlgorithmChanged(int index)
{
    if (index < 0) {
        _teachingPointLabel->setText(QStringLiteral("没有可用插件。请先构建项目，确认插件 DLL 已输出到 build/app/plugins。"));
        rebuildParamForm({});
        return;
    }

    const QString algorithmId = _algorithmBox->itemData(index).toString();
    const PluginLoader::LoadedPlugin* plugin = _pluginLoader.findById(algorithmId);
    if (!plugin) {
        return;
    }

    _teachingPointLabel->setText(QStringLiteral("教学重点：%1").arg(plugin->meta.teachingPoint));
    rebuildParamForm(plugin->meta);
}

void MainWindow::rebuildParamForm(const atp::AlgorithmMeta& meta)
{
    _paramEditors.clear();

    QWidget* oldPanel = _paramScrollArea ? _paramScrollArea->takeWidget() : nullptr;
    if (oldPanel) {
        oldPanel->deleteLater();
    }

    _paramPanel = new QWidget(_paramScrollArea);
    _paramLayout = new QVBoxLayout(_paramPanel);
    _paramLayout->setContentsMargins(0, 0, 0, 0);

    auto* form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignLeft);

    for (const atp::ParamMeta& param : meta.params) {
        QWidget* editor = nullptr;

        switch (param.editor) {
        case atp::ParamEditor::Int:
        {
            auto* spin = new QSpinBox(_paramPanel);
            spin->setRange(param.minValue.isValid() ? param.minValue.toInt() : -999999,
                           param.maxValue.isValid() ? param.maxValue.toInt() : 999999);
            spin->setSingleStep(param.stepValue.isValid() ? param.stepValue.toInt() : 1);
            spin->setValue(param.defaultValue.toInt());
            editor = spin;
            break;
        }
        case atp::ParamEditor::Double:
        {
            auto* spin = new QDoubleSpinBox(_paramPanel);
            spin->setDecimals(3);
            spin->setRange(param.minValue.isValid() ? param.minValue.toDouble() : -999999.0,
                           param.maxValue.isValid() ? param.maxValue.toDouble() : 999999.0);
            spin->setSingleStep(param.stepValue.isValid() ? param.stepValue.toDouble() : 0.1);
            spin->setValue(param.defaultValue.toDouble());
            editor = spin;
            break;
        }
        case atp::ParamEditor::Bool:
        {
            auto* check = new QCheckBox(_paramPanel);
            check->setChecked(param.defaultValue.toBool());
            editor = check;
            break;
        }
        case atp::ParamEditor::Choice:
        {
            auto* combo = new QComboBox(_paramPanel);
            for (const atp::ParamChoice& choice : param.choices) {
                combo->addItem(choice.text, choice.value);
            }
            const int defaultIndex = combo->findData(param.defaultValue);
            if (defaultIndex >= 0) {
                combo->setCurrentIndex(defaultIndex);
            }
            editor = combo;
            break;
        }
        case atp::ParamEditor::ImageFile:
        case atp::ParamEditor::Text:
        default:
        {
            auto* line = new QLineEdit(_paramPanel);
            line->setText(param.defaultValue.toString());
            editor = line;
            break;
        }
        }

        editor->setToolTip(param.hint);
        _paramEditors.insert(param.key, editor);
        form->addRow(param.name, editor);
    }

    _paramLayout->addLayout(form);
    _paramLayout->addStretch();
    if (_paramScrollArea) {
        _paramScrollArea->setWidget(_paramPanel);
    }
}

void MainWindow::chooseImage()
{
    const QString path = QFileDialog::getOpenFileName(this,
                                                      QStringLiteral("选择输入图片"),
                                                      QString(),
                                                      QStringLiteral("图片 (*.bmp *.jpg *.jpeg *.png *.tif *.tiff)"));
    if (path.isEmpty()) {
        return;
    }

    QImageReader reader(path);
    reader.setAutoTransform(true);
    const QImage image = reader.read();
    if (image.isNull()) {
        QMessageBox::warning(this, QStringLiteral("读取失败"), QStringLiteral("图片读取失败：%1").arg(path));
        return;
    }

    _currentImagePath = path;
    _currentImage = image;
    setImagePreview(image);
    appendLog(QStringLiteral("输入图片：%1，尺寸：%2 x %3")
                  .arg(path)
                  .arg(image.width())
                  .arg(image.height()));
}

void MainWindow::runCurrentAlgorithm()
{
    if (_algorithmBox->currentIndex() < 0) {
        appendLog(QStringLiteral("没有可运行的插件"));
        return;
    }

    if (_currentImage.isNull()) {
        appendLog(QStringLiteral("请先选择输入图片"));
        return;
    }

    const QString algorithmId = _algorithmBox->currentData().toString();
    appendLog(QStringLiteral("开始运行算法：%1").arg(algorithmId));

    _runButton->setEnabled(false);
    QApplication::setOverrideCursor(Qt::BusyCursor);

    const atp::AlgorithmResult result =
        _runner.run(algorithmId, _currentImage, _currentImagePath, collectParams());

    QApplication::restoreOverrideCursor();
    _runButton->setEnabled(true);

    appendLog(result.success ? QStringLiteral("运行成功：%1").arg(result.message)
                             : QStringLiteral("运行失败：%1").arg(result.message));
    appendNotes(result);
    setResultPreview(result.preview);
}

void MainWindow::setImagePreview(const QImage& image)
{
    _inputImageLabel->setPixmap(QPixmap::fromImage(image).scaled(_inputImageLabel->size(),
                                                                 Qt::KeepAspectRatio,
                                                                 Qt::SmoothTransformation));
}

void MainWindow::setResultPreview(const QImage& image)
{
    if (image.isNull()) {
        _resultImageLabel->setText(QStringLiteral("没有结果图"));
        return;
    }

    _resultImageLabel->setPixmap(QPixmap::fromImage(image).scaled(_resultImageLabel->size(),
                                                                  Qt::KeepAspectRatio,
                                                                  Qt::SmoothTransformation));
}

void MainWindow::appendLog(const QString& text)
{
    _logEdit->append(text);
}

void MainWindow::appendNotes(const atp::AlgorithmResult& result)
{
    for (const QString& note : result.notes) {
        appendLog(QStringLiteral("说明：%1").arg(note));
    }

    for (auto it = result.values.begin(); it != result.values.end(); ++it) {
        appendLog(QStringLiteral("数据：%1 = %2").arg(it.key(), it.value().toString()));
    }
}

QMap<QString, QVariant> MainWindow::collectParams() const
{
    QMap<QString, QVariant> params;

    for (auto it = _paramEditors.begin(); it != _paramEditors.end(); ++it) {
        QWidget* editor = it.value();
        if (auto* spin = qobject_cast<QSpinBox*>(editor)) {
            params.insert(it.key(), spin->value());
        } else if (auto* spin = qobject_cast<QDoubleSpinBox*>(editor)) {
            params.insert(it.key(), spin->value());
        } else if (auto* check = qobject_cast<QCheckBox*>(editor)) {
            params.insert(it.key(), check->isChecked());
        } else if (auto* combo = qobject_cast<QComboBox*>(editor)) {
            params.insert(it.key(), combo->currentData());
        } else if (auto* line = qobject_cast<QLineEdit*>(editor)) {
            params.insert(it.key(), line->text());
        }
    }

    return params;
}
