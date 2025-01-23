#include "filesManager.h"

FilesManager::FilesManager() :
    supportedVids("*.mp4 *.wav *.mkv"), settings{SettingsManager()}
{
    m_defaultPath = settings.getSetting("VideosPath", "lastSelectedPath").toString().remove("file://");
}

FilesManager::~FilesManager()
{
    delete dialog;
}

// QString FilesManager::selectFile()
// {
//     m_defaultPath = settings.getSetting("VideosPath", "lastSelectedPath").toString().remove("file://");

//     QString file = m_dialog.getOpenFileName(nullptr, "Select video file", m_defaultPath, supportedVids);

// #ifdef Q_OS_LINUX
//     if (!file.isEmpty()) {
//         file = "file://" + file;
//         settings.saveSettings("VideosPath", "lastSelectedPath", file);
//         return file;
//     }
// #endif

//     if (!file.isEmpty()) {
//         settings.saveSettings("VideosPath", "lastSelectedPath", file);
//         return file;
//     }

//     //return last saved video (current running video)
//     return settings.getSetting("Video", "video").toString();
// }

QVector<QString> FilesManager::selectFiles()
{
    dialog = new QFileDialog();
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
