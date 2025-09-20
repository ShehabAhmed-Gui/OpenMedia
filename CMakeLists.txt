cmake_minimum_required(VERSION 3.16)

project(OpenMedia VERSION 1.0 LANGUAGES CXX)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core Gui Quick QuickControls2 Widgets
    Multimedia MultimediaWidgets)

find_package(Qt6 REQUIRED COMPONENTS LinguistTools)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)

set(SOURCES
    client/core/main.cpp
    client/core/settings.cpp
    client/core/filesmanager.cpp
    client/core/models/playlistmodel.cpp
)

set(HEADERS
    client/core/settings.h
    client/core/filesmanager.h
    client/core/models/playlistmodel.h
)

qt_standard_project_setup()

qt6_add_resources(SOURCES "client/core/resources.qrc")

qt_add_executable(OpenMedia
    # WIN32
    ${SOURCES}
    ${HEADERS}
    client/core/controllers/settingscontroller.h client/core/controllers/settingscontroller.cpp
    client/core/corecontroller.h client/core/corecontroller.cpp
    client/core/controllers/filescontroller.h client/core/controllers/filescontroller.cpp
    client/core/models/metadatamodel.h client/core/models/metadatamodel.cpp
    client/core/models/metadatafilterproxymodel.h client/core/models/metadatafilterproxymodel.cpp
)

# qt_add_translations(OpenMedia TS_FILES client/core//translation/OpenMedia_en_US.ts)

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
target_link_libraries(OpenMedia PRIVATE Qt6::Core)
target_link_libraries(OpenMedia PRIVATE Qt6::Core)
target_link_libraries(OpenMedia PRIVATE Qt6::Core)
target_link_libraries(OpenMedia PRIVATE Qt6::Core)
target_link_libraries(OpenMedia PRIVATE Qt6::Core)

include(GNUInstallDirs)
install(TARGETS OpenMedia
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
