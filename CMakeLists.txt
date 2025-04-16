cmake_minimum_required(VERSION 3.16)

project(OpenMedia VERSION 1.0 LANGUAGES CXX)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core Gui Quick QuickControls2 Widgets
    Multimedia MultimediaWidgets)
find_package(Qt6 REQUIRED COMPONENTS LinguistTools)

set(SOURCES
    client/main.cpp
    client/settings/settingsManager.cpp
    client/filesManager/filesManager.cpp
    client/model/listmodel.cpp
)

set(HEADERS
    client/settings/settingsManager.h
    client/filesManager/filesManager.h
    client/model/listmodel.h
)

qt_standard_project_setup()

qt6_add_resources(SOURCES "client/core/resources.qrc")

qt_add_executable(OpenMedia
    WIN32
    ${SOURCES}
    ${HEADERS}
)

qt_add_translations(OpenMedia TS_FILES client/core//translation/OpenMedia_en_US.ts)

set_target_properties(OpenMedia PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
)

target_link_libraries(OpenMedia PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)

include(GNUInstallDirs)
install(TARGETS OpenMedia
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
