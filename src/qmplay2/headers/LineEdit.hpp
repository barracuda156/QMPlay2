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

#pragma once

#include <QMPlay2Lib.hpp>

#include <QLineEdit>

#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
    #include <QLabel>

class QMPLAY2SHAREDLIB_EXPORT LineEditButton final : public QLabel
{
    Q_OBJECT
public:
    LineEditButton();
private:
    void mousePressEvent(QMouseEvent *) override;
signals:
    void clicked();
};
#endif

class QMPLAY2SHAREDLIB_EXPORT LineEdit final : public QLineEdit
{
    Q_OBJECT

public:
    LineEdit(QWidget *parent = nullptr);
#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
private:
    void resizeEvent(QResizeEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;

    LineEditButton b;
private slots:
    void textChangedSlot(const QString &);
public slots:
#endif

    void clearText();

signals:
    void clearButtonClicked();
};
