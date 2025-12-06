#include "filesmanager.h"

FilesManager::FilesManager(const QSharedPointer<Settings> settings,
                           QObject *parent)
    : m_supportedFormats("*.mp3 *.mp4 *.wav *.mkv *.webm")
    , m_settings(settings)
{
    m_defaultPath = m_settings->getSetting("VideosPath", "lastSelectedPath").toString().remove("file://");
    dialog = new QFileDialog();
}

FilesManager::~FilesManager()
{
    delete dialog;
}

QVector<QString> FilesManager::selectFiles()
{
    dialog->setOptions(QFileDialog::ReadOnly);
    m_loadedFiles = dialog->getOpenFileNames(nullptr, "Select A Bunch Of Videos", m_defaultPath, m_supportedFormats);

    if (!m_loadedFiles.isEmpty()) {
        const QString &videosPath = m_loadedFiles.last();
        m_settings->saveSetting("VideosPath", "lastSelectedPath", videosPath);
        m_defaultPath = videosPath;
    }

#ifdef Q_OS_LINUX
        QVector<QString> linuxFiles;
        for (QString &file : selectedFiles) {
            linuxFiles.append("file://" + file);
        }
        return linuxFiles;
#endif

    return m_loadedFiles;
}

#ifdef Q_OS_LINUX
void FilesManager::setupDesktopFile()
{
    const QString appFile = QCoreApplication::applicationDirPath() + "/OpenMedia.sh";
    const QString appIcon = QCoreApplication::applicationDirPath() + "/../icon/OpenMedia.png";
    const QString targetDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/applications";
    const QString targetFile = targetDir + "/OpenMedia.desktop";

    QDir().mkpath(targetDir);

    QFile desktopFile(targetFile);

    if (desktopFile.open(QIODevice::WriteOnly)) { 
        QString fileContent;
        QTextStream ts(&fileContent);

        ts <<
            "[Desktop Entry]\n"
            "Name[en_US]=OpenMedia\n"
            "Comment=Play videos with OpenMedia\n"
            "Exec=\"%1\"\n"
            "Icon=%2\n"
            "Terminal=false\n"
            "Type=Application\n"
            "MimeType=video/mp4;audio/wav\n"
            "Categories=AudioVideo;Video;\n";

        fileContent = fileContent.arg(appFile, appIcon);

        desktopFile.write(fileContent.toUtf8());
        desktopFile.setPermissions(targetFile, QFile::WriteUser | QFile::ReadUser);
        desktopFile.close();

        // Update desktop database
        QProcess process;
        process.start("update-desktop-database", QStringList() << targetDir);
        process.waitForFinished();
    }
}
#endif
