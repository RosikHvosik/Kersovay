cmake_minimum_required(VERSION 3.19)
project(KYRSOVAYA LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(KYRSOVAYA
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    hashtabledialog.h hashtabledialog.cpp hashtabledialog.ui
    avltreedialog.h avltreedialog.cpp avltreedialog.ui
    types.h
    hashtable.hpp
    array.h
    linkedlist.cpp
    linkedlist.h
    avltree.hpp
    avltree.cpp
    avltree2.hpp
    avltree3.hpp




)

target_link_libraries(KYRSOVAYA
    PRIVATE
        Qt::Core
        Qt::Widgets
)
target_link_libraries(KYRSOVAYA PRIVATE Qt6::Widgets)

include(GNUInstallDirs)

install(TARGETS KYRSOVAYA
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET KYRSOVAYA
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
