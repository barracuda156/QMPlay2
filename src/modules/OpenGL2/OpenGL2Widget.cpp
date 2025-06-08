/*
    QMPlay2 is a video and audio player.
    Copyright (C) 2010-2017  Błażej Szczygieł

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "OpenGL2Widget.hpp"

#include <QMPlay2Core.hpp>

OpenGL2Widget::OpenGL2Widget()
{
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(updateGL())); // updateGL() from the base class
}

OpenGL2Widget::~OpenGL2Widget()
{
    makeCurrent();
}

QWidget *OpenGL2Widget::widget()
{
    return this;
}

bool OpenGL2Widget::testGL()
{
    makeCurrent();
    if ((isOK = isValid()))
        testGLInternal();
    doneCurrent();
    return isOK;
}

bool OpenGL2Widget::setVSync(bool enable)
{
#ifdef VSYNC_SETTINGS
    bool doDoneCurrent = false;
    if (QGLContext::currentContext() != context())
    {
        makeCurrent();
        doDoneCurrent = true;
    }
    using SwapInterval = int (APIENTRY *)(int); // BOOL is just normal int in Windows, APIENTRY declares nothing on non-Windows platforms
    SwapInterval swapInterval = NULL;
    swapInterval = (SwapInterval)context()->getProcAddress("glXSwapIntervalMESA");
    if (!swapInterval)
        swapInterval = (SwapInterval)context()->getProcAddress("glXSwapIntervalSGI");
    if (swapInterval)
        swapInterval(enable);
    if (doDoneCurrent)
        doneCurrent();
    vSync = enable;
#else
    Q_UNUSED(enable)
#endif
    return true;
}

void OpenGL2Widget::updateGL(bool requestDelayed)
{
    if (requestDelayed)
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest), Qt::LowEventPriority);
    else
        QGLWidget::updateGL();
}

void OpenGL2Widget::initializeGL()
{
    OpenGL2Common::initializeGL();
}

void OpenGL2Widget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    OpenGL2Common::paintGL();
}

void OpenGL2Widget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

bool OpenGL2Widget::event(QEvent *e)
{
    dispatchEvent(e, parent());
    return QGLWidget::event(e);
}
