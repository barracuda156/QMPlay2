cmake_minimum_required(VERSION 3.1)
project(OpenGL2)

set(OpenGL2_HDR
    OpenGL2.hpp
    OpenGL2Writer.hpp
    OpenGL2Common.hpp
    OpenGL2Widget.hpp
    Sphere.hpp
    Vertices.hpp
)

set(OpenGL2_SRC
    OpenGL2.cpp
    OpenGL2Writer.cpp
    OpenGL2Common.cpp
    OpenGL2Widget.cpp
    Sphere.cpp
)

set(OpenGL2_RESOURCES
    res.qrc
)

function(gles1Error)
    message(SEND_ERROR "OpenGL|ES 1.0 is not supported!")
endfunction()

find_package(Qt4 REQUIRED QtOpenGL)
include(${QT_USE_FILE})
if(${QT_QCONFIG} MATCHES "opengles2")
    set(QT_USES_OPENGLES ON)
    add_definitions(-DOPENGL_ES2)
elseif(${QT_QCONFIG} MATCHES "opengles1")
    gles1Error()
endif()

if(NOT QT_USES_OPENGLES)
    add_definitions(-DVSYNC_SETTINGS)
endif()

qt4_add_resources(OpenGL2_RESOURCES_RCC ${OpenGL2_RESOURCES})

include_directories(../../qmplay2/headers)

add_library(${PROJECT_NAME} ${QMPLAY2_MODULE}
    ${OpenGL2_HDR}
    ${OpenGL2_SRC}
    ${OpenGL2_RESOURCES}
)

target_link_libraries(${PROJECT_NAME} Qt4::QtCore Qt4::QtGui Qt4::QtOpenGL)

target_link_libraries(${PROJECT_NAME}
    libqmplay2
)

if(APPLE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
endif()

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${MODULES_INSTALL_PATH})
