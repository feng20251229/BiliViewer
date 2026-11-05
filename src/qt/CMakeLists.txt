cmake_minimum_required(VERSION 3.19)
project(qt LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(qt
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    ../connect.cpp
)

target_link_libraries(qt
    PRIVATE
        Qt::Core
        Qt::Widgets
)
# 1. 告诉编译器头文件在哪
target_include_directories(qt PRIVATE
    "E:/soft intallation/MSYS2/mingw64/include"
)

# 2. 告诉链接器库文件在哪
target_link_directories(qt PRIVATE
    "E:/soft intallation/MSYS2/mingw64/lib"
)

# 3. 链接具体库的名字（不用写 libcurl.dll，写 curl 就行）
target_link_libraries(qt PRIVATE
    curl
    cjson
)
install(TARGETS qt
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET qt
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
