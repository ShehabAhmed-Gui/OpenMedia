cmake_minimum_required(VERSION 3.16)

project(OpenMedia VERSION 1.0 LANGUAGES C CXX)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell Qt to use opengl for backend
set(ENV{QSG_RHI_BACKEND} opengl)

set(FFMPEG_DIR ${CMAKE_CURRENT_LIST_DIR}/3rdparty/ffmpeg)

message("FFMPEG_DIR = ${FFMPEG_DIR}")

find_package(Qt6 REQUIRED COMPONENTS Core
    Gui Quick QuickControls2 Widgets
    Multimedia Core5Compat OpenGL OpenGLWidgets)

set(HEADERS
    client/core/corecontroller.h
    client/core/settings.h
    client/core/settingsloader.h
    client/core/filesmanager.h
    client/core/videomanager.h
    client/core/frameprovider.h
    client/core/thumbnailsextractor.h
    client/core/foldermonitor.h

    client/core/models/metadatamodel.h
    client/core/models/playlistmodel.h
    client/core/models/metadatafilterproxymodel.h

    client/core/controllers/filescontroller.h
    client/core/controllers/settingscontroller.h
    client/core/controllers/videocontroller.h
    client/core/mediaplayer.h
)

set(SOURCES
    client/core/main.cpp
    client/core/corecontroller.cpp
    client/core/settings.cpp
    client/core/settingsloader.cpp
    client/core/filesmanager.cpp
    client/core/videomanager.cpp
    client/core/frameprovider.cpp
    client/core/thumbnailsextractor.cpp
    client/core/foldermonitor.cpp

    client/core/models/metadatamodel.cpp
    client/core/models/playlistmodel.cpp
    client/core/models/metadatafilterproxymodel.cpp

    client/core/controllers/filescontroller.cpp
    client/core/controllers/settingscontroller.cpp
    client/core/controllers/videocontroller.cpp
    client/core/mediaplayer.cpp
)

set(BACKEND_HEADERS

    client/core/backend/demuxer.h

    client/core/backend/decoder.h
)

set(BACKEND_SOURCES

    client/core/backend/demuxer.cpp

    client/core/backend/decoder.cpp

)

qt_standard_project_setup()

qt6_add_resources(SOURCES "client/core/resources.qrc")

# Point pkg-config to your FFmpeg .pc files
set(ENV{PKG_CONFIG_PATH} ${CMAKE_CURRENT_LIST_DIR}/3rdparty/ffmpeg/lib/pkgconfig)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    #libavdevice
    #libavfilter
    libavformat
    libavcodec
    libswresample
    libswscale
    libavutil
)

# This FFmpeg build ships the avfilter import library under bin/, not lib/.
find_library(AVFILTER_LIBRARY
    NAMES avfilter
    PATHS ${FFMPEG_DIR}/lib ${FFMPEG_DIR}/bin
    NO_DEFAULT_PATH
    REQUIRED)

qt_add_executable(OpenMedia
    #WIN32
    ${SOURCES}
    ${HEADERS}
    ${BACKEND_HEADERS}
    ${BACKEND_SOURCES}
    client/core/backend/playbacklogger.h client/core/backend/playbacklogger.cpp
    client/core/controllers/mediaplayercontroller.h client/core/controllers/mediaplayercontroller.cpp
    client/core/audioplayer.h client/core/audioplayer.cpp

    client/core/audiobufferdevice.h client/core/audiobufferdevice.cpp
    client/core/audiolimiter.h client/core/audiolimiter.cpp
    client/core/defs.h

    client/core/backend/frameconverter.h client/core/backend/frameconverter.cpp
    client/core/backend/artworkframe.h client/core/backend/artworkframe.cpp
    client/core/backend/videodecoder.h client/core/backend/videodecoder.cpp
    client/core/backend/audiodecoder.h client/core/backend/audiodecoder.cpp
    client/core/videorenderer.h client/core/videorenderer.cpp
    client/core/videoitem.h client/core/videoitem.cpp
)

# Gives the executable itself the app icon in Explorer and the taskbar.
if (WIN32)
    target_sources(OpenMedia PRIVATE client/core/app.rc)
endif()

set_target_properties(OpenMedia PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
)

include_directories(client/core/backend)

include_directories(${FFMPEG_DIR}/include)
link_directories(${FFMPEG_DIR}/lib)
link_directories(${FFMPEG_DIR}/bin)

target_link_libraries(OpenMedia PRIVATE Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::Core5Compat
    Qt6::OpenGL
    Qt6::OpenGLWidgets

    PkgConfig::LIBAV
    ${AVFILTER_LIBRARY}
)

include(GNUInstallDirs)
install(TARGETS OpenMedia
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
