cmake_minimum_required(VERSION 3.14)

project(signalmonitor LANGUAGES CXX C VERSION 1.0.0.0)

configure_file(config.h.in config.h)

set(signalmonitor_VERSION ${PROJECT_VERSION})

string(TIMESTAMP signalmonitor_BUILD_TIME "%Y-%m-%d %H:%M:%S" UTC)

set(STATIC_BUILD ON)
set(CMAKE_PREFIX_PATH C:/ENV/qt/6.5.1/msvc2019_64/lib/cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets SerialPort Charts PrintSupport)

include_directories(App/Inc)

# all app resources
file(GLOB_RECURSE app_RES_FILES "App/Assets/*.qrc")
qt_add_resources(qrc_FILES ${app_RES_FILES})

# app icon for windows
set(app_icon_resource_windows "App/Assets/icon.rc")

# all source files
file(GLOB_RECURSE signalmonitor_SRCS
  App/Src/*.cpp
  App/Src/*.ui
  App/Inc/*.hpp
  App/Inc/*.h
)

add_executable(signalmonitors
  ${signalmonitor_SRCS}
  ${app_icon_resource_windows}
  ${qrc_FILES}
)

target_compile_definitions(signalmonitors PRIVATE QCUSTOMPLOT_USE_OPENGL)

target_link_libraries(signalmonitors Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(signalmonitors Qt${QT_VERSION_MAJOR}::Gui)
target_link_libraries(signalmonitors Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(signalmonitors Qt${QT_VERSION_MAJOR}::SerialPort)
target_link_libraries(signalmonitors Qt${QT_VERSION_MAJOR}::Charts)
target_link_libraries(signalmonitors Qt${QT_VERSION_MAJOR}::PrintSupport)

target_link_libraries(signalmonitors OpenGL32.lib)
