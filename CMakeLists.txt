cmake_minimum_required(VERSION 2.8.12)
cmake_policy(VERSION 3.3)

project(myriad)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall -fexceptions")

find_package(Qt5 REQUIRED COMPONENTS
    Core
    Widgets
)

find_package(KF5 REQUIRED COMPONENTS
    CoreAddons
    I18n
    XmlGui
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(APP_NAME myriad)
set(SRC_SUBDIR src/)
set(UI_SUBDIR ui/)

set(Myriad_SRCS
    ${SRC_SUBDIR}deduplicator.cpp
    ${SRC_SUBDIR}deduplicatorthread.cpp
    ${SRC_SUBDIR}imageinfo.cpp
    ${SRC_SUBDIR}imageview.cpp
    ${SRC_SUBDIR}main.cpp
    ${SRC_SUBDIR}mainwindow.cpp
    ${SRC_SUBDIR}merger.cpp
    ${SRC_SUBDIR}processor.cpp
    ${SRC_SUBDIR}processorthread.cpp
    ${SRC_SUBDIR}queueitem.cpp
)

kconfig_add_kcfg_files(Myriad_SRCS
    ${SRC_SUBDIR}settings.kcfgc
)

qt5_wrap_ui(Myriad_SRCS
    ${UI_SUBDIR}mainwindow.ui
)

add_executable(${APP_NAME} ${Myriad_SRCS})
target_link_libraries(${APP_NAME}
    pHash
    KF5::I18n
    KF5::XmlGui
)

install(TARGETS ${APP_NAME} DESTINATION ${BIN_INSTALL_DIR})
install(FILES ${SRC_SUBDIR}myriadui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/${APP_NAME})
