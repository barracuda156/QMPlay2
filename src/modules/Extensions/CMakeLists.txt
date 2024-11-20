cmake_minimum_required(VERSION 3.1)
project(Extensions)

set(Extensions_HDR
    Extensions.hpp
    YouTube.hpp
    Downloader.hpp
    Radio.hpp
    Radio/RadioBrowserModel.hpp
)

set(Extensions_SRC
    Extensions.cpp
    YouTube.cpp
    Downloader.cpp
    Radio.cpp
    Radio/RadioBrowserModel.cpp
)

set(Extensions_FORMS
    Radio/Radio.ui
)

set(Extensions_RESOURCES
    icons.qrc
    js.qrc
)

if(USE_MPRIS2)
    list(APPEND Extensions_HDR MPRIS2.hpp)
    list(APPEND Extensions_SRC MPRIS2.cpp)
    add_definitions(-DUSE_MPRIS2)
    set(DBUS Qt5::DBus)
endif()

if(USE_LASTFM)
    list(APPEND Extensions_HDR LastFM.hpp)
    list(APPEND Extensions_SRC LastFM.cpp)
    add_definitions(-DUSE_LASTFM)
endif()

if(USE_LYRICS)
    list(APPEND Extensions_HDR Lyrics.hpp)
    list(APPEND Extensions_SRC Lyrics.cpp)
    add_definitions(-DUSE_LYRICS)
endif()

if(USE_MEDIABROWSER)
    list(APPEND Extensions_HDR MediaBrowser.hpp MediaBrowserJS.hpp)
    list(APPEND Extensions_SRC MediaBrowser.cpp MediaBrowserJS.cpp)
    add_definitions(-DUSE_MEDIABROWSER)
    if(JS_SKIP_SSE2_CHECK)
        add_definitions(-DJS_SKIP_SSE2_CHECK)
    endif()
    set(QML Qt5::Qml)
endif()

if(USE_QT4)
    qt4_wrap_ui(Extensions_FORM_HDR ${Extensions_FORMS})
else()
    qt5_wrap_ui(Extensions_FORM_HDR ${Extensions_FORMS})
endif()
set_property(SOURCE ${Extensions_FORM_HDR} PROPERTY SKIP_AUTOMOC ON)

include_directories(../../qmplay2/headers
    ${LIBAVUTIL_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME} ${QMPLAY2_MODULE}
    ${Extensions_HDR}
    ${Extensions_SRC}
    ${Extensions_FORM_HDR}
    ${Extensions_RESOURCES}
)

if(USE_QT4)
    add_definitions(-I/opt/local/include/QJson4)
    target_link_libraries(${PROJECT_NAME} Qt4::QtCore Qt4::QtGui QJson4)
else()
    target_link_libraries(${PROJECT_NAME} ${QML})
endif()

target_link_libraries(${PROJECT_NAME}
    ${DBUS}
    libqmplay2
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${MODULES_INSTALL_PATH})
