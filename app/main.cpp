#ifdef _WIN32
#include <windows.h>
#endif

#include <cstdio>

#include <QApplication>
#include <QMetaType>
#include <QString>

#include "algorithm_platform/plugin_contract.h"
#include "main_window.h"

namespace
{

QString messageLevelText(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return QStringLiteral("调试");
    case QtInfoMsg:
        return QStringLiteral("信息");
    case QtWarningMsg:
        return QStringLiteral("警告");
    case QtCriticalMsg:
        return QStringLiteral("错误");
    case QtFatalMsg:
        return QStringLiteral("致命");
    }
    return QStringLiteral("日志");
}

void outputUtf8Line(const QString& line)
{
#ifdef _WIN32
    HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
    DWORD consoleMode = 0;
    if (handle && handle != INVALID_HANDLE_VALUE && GetConsoleMode(handle, &consoleMode)) {
        const std::wstring wideLine = line.toStdWString();
        DWORD written = 0;
        WriteConsoleW(handle, wideLine.c_str(), static_cast<DWORD>(wideLine.size()), &written, nullptr);
        return;
    }
#endif

    const QByteArray bytes = line.toUtf8();
    std::fwrite(bytes.constData(), 1, static_cast<size_t>(bytes.size()), stderr);
    std::fflush(stderr);
}

void qtMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    Q_UNUSED(context);
    outputUtf8Line(QStringLiteral("[%1] %2\n").arg(messageLevelText(type), message));

    if (type == QtFatalMsg) {
        abort();
    }
}

} // namespace

int main(int argc, char* argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    qInstallMessageHandler(qtMessageHandler);

    QApplication app(argc, argv);
    qRegisterMetaType<atp::AlgorithmResult>("atp::AlgorithmResult");

    MainWindow window;
    window.show();

    return app.exec();
}
