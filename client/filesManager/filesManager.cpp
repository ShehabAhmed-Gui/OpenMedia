#include "filesManager.h"

FilesManager::FilesManager(QObject *parent) :
    supportedVids("*.mp4 *.wav *.mkv *.webm"), settings{SettingsManager()}
{
    m_defaultPath = settings.getSetting("VideosPath", "lastSelectedPath").toString().remove("file://");
}

FilesManager::~FilesManager()
{
    delete dialog;
}

void FilesManager::addFile(QString path)
{
    #ifdef Q_OS_LINUX
        path = "file://" + path;
    #endif

    emit videoPassedAsArg(path);
}

QVector<QString> FilesManager::selectFiles()
{
    dialog = new QFileDialog();
    dialog->setOptions(QFileDialog::ReadOnly);
    selectedFiles = dialog->getOpenFileNames(nullptr, "Select A Bunch Of Videos", m_defaultPath, supportedVids);

    if (!selectedFiles.isEmpty()) {
        const QString &videosPath = selectedFiles.last();
        settings.saveSettings("VideosPath", "lastSelectedPath", videosPath);
        m_defaultPath = videosPath;
    }

    #ifdef Q_OS_LINUX
        QVector<QString> linuxFiles;
        for (QString &file : selectedFiles) {
            linuxFiles.append("file://" + file);
        }
        return linuxFiles;
    #endif

    return selectedFiles;
}

void FilesManager::setupDesktopFile()
{
    const QString appFile = QCoreApplication::applicationFilePath();
    const QString targetDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/applications";
    const QString targetFile = targetDir + "/OpenMedia.desktop";

    QDir().mkpath(targetDir);

    // Copy the .desktop file
    QFile desktopFile(targetFile);

    if (desktopFile.open(QIODevice::WriteOnly)) {
        // Write desktop file

        const QString fileContent = QString(
            "[Desktop Entry]\n"
            "Name=OpenMedia\n"
            "Comment=Play videos with OpenMedia\n"
            "Exec=%1 %%f\n"
            "Terminal=false\n"
            "Type=Application\n"
            "MimeType=video/mp4;video/wav;\n"
            "Categories=AudioVideo;Video;\n"
            "Name[en_US]=OpenMedia\n"
        ).arg(appFile);

        desktopFile.write(fileContent.toUtf8());
        // Set permissions
        desktopFile.setPermissions(targetFile, QFile::ReadUser | QFile::WriteUser);
        desktopFile.close();

        // Update desktop database
        QProcess process;
        process.start("update-desktop-database", QStringList() << targetDir);
        process.waitForFinished();
    }
}
