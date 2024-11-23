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

#include <QWidget>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    #include <vector>
    #include <array>
#endif

class ModuleParams;
class QAction;
class Slider;

class VideoAdjustmentW : public QWidget
{
    Q_OBJECT

public:
    VideoAdjustmentW();
    ~VideoAdjustmentW() final;

    void restoreValues();
    void saveValues();

    void setModuleParam(ModuleParams *writer);
    void enableControls();

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    void setKeyShortcuts();
    void addActionsToWidget(QWidget *w);
#endif

signals:
// MOC is stupid.
// #if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
//     void videoAdjustmentChanged(const QString &osdText);
// #else
    void videoAdjustmentChanged();
// #endif

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
private slots:
	void setValue(int);
	void reset();
#endif

private:
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    std::vector<Slider *> m_sliders;
    std::vector<std::array<QAction *, 2>> m_actions;
    QAction *m_resetAction = nullptr;
#else
    Slider *m_sliders;
#endif
};
