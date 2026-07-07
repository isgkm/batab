cmake_minimum_required(VERSION 3.19)
project(batab LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

set(CMAKE_AUTORCC ON)

qt_add_executable(batab
    WIN32 MACOSX_BUNDLE
    main.cpp
    batab.cpp
    batab.h
    batab.ui
    assets.qrc
    appswitcher.h appswitcher.cpp appswitcher.ui
    winprocs.h winprocs.cpp
    trackedwindows.h trackedwindows.cpp
    util.h util.cpp
)

target_link_libraries(batab
    PRIVATE
        Qt::Core
        Qt::Widgets
        dwmapi
)

install(TARGETS batab
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET batab
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
