/*
    QMPlay2 is a video and audio player.
    Copyright (C) 2010-2019  Błażej Szczygieł

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

#include <LineEdit.hpp>

#include <QMPlay2Core.hpp>

#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
    #include <QAction>
#else
    #include <Functions.hpp>

    #include <QResizeEvent>

LineEditButton::LineEditButton()
{
    const QSize iconSize(16, 16);
    setToolTip(tr("Clear"));
    setPixmap(Functions::getPixmapFromIcon(QMPlay2Core.getIconFromTheme("edit-clear"), iconSize, this));
    resize(iconSize);
    setCursor(Qt::ArrowCursor);
}

void LineEditButton::mousePressEvent(QMouseEvent *e)
{
    if (e->button() & Qt::LeftButton)
        emit clicked();
}
#endif

LineEdit::LineEdit(QWidget *parent)
    : QLineEdit(parent)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
    QAction *clearAct = addAction(QMPlay2Core.getIconFromTheme("edit-clear"), TrailingPosition);
    connect(clearAct, &QAction::triggered, this, &LineEdit::clearText);
    connect(this, &LineEdit::textChanged, this, [=](const QString &text) {
        clearAct->setVisible(!text.isEmpty());
    });
    clearAct->setToolTip(tr("Clear"));
    clearAct->setVisible(false);
#else
    connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(textChangedSlot(const QString &)));
    connect(&b, SIGNAL(clicked()), this, SLOT(clearText()));
    setMinimumWidth(b.width() * 2.5);
    setTextMargins(0, 0, b.width() * 1.5, 0);
    b.setParent(this);
    b.hide();
#endif
}

#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
void LineEdit::resizeEvent(QResizeEvent *e)
{
    b.move(e->size().width() - b.width() * 1.5, e->size().height() / 2 - b.height() / 2);
}
void LineEdit::mousePressEvent(QMouseEvent *e)
{
    if (!b.underMouse())
        QLineEdit::mousePressEvent(e);
}
void LineEdit::mouseMoveEvent(QMouseEvent *e)
{
    if (!b.underMouse())
        QLineEdit::mouseMoveEvent(e);
}
void LineEdit::textChangedSlot(const QString &str)
{
    b.setVisible(!str.isEmpty());
}
#endif

void LineEdit::clearText()
{
    clear();
    emit clearButtonClicked();
}
