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

#include <VideoAdjustmentW.hpp>

#include <ModuleParams.hpp>
#include <Settings.hpp>
#include <Slider.hpp>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    #include <ShortcutHandler.hpp>
    #include <Main.hpp>
    #include <QAction>
#else
    #include <QVariant>
#endif

#include <QGridLayout>
#include <QPushButton>
#include <QLabel>

enum CONTROLS
{
    BRIGHTNESS,
    CONTRAST,
    SATURATION,
    HUE,
    SHARPNESS,

    CONTROLS_COUNT
};
constexpr const char *g_controlsNames[CONTROLS_COUNT] = {
    QT_TRANSLATE_NOOP("VideoAdjustmentW", "Brightness"),
    QT_TRANSLATE_NOOP("VideoAdjustmentW", "Contrast"),
    QT_TRANSLATE_NOOP("VideoAdjustmentW", "Saturation"),
    QT_TRANSLATE_NOOP("VideoAdjustmentW", "Hue"),
    QT_TRANSLATE_NOOP("VideoAdjustmentW", "Sharpness")
};
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    constexpr int g_step = 5;
#endif

VideoAdjustmentW::VideoAdjustmentW()
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
{
    QGridLayout *layout = new QGridLayout;

    m_sliders.reserve(CONTROLS_COUNT);
    m_actions.reserve(CONTROLS_COUNT);

    for (int i = 0; i < CONTROLS_COUNT; ++i)
    {
        QLabel *titleL = new QLabel(tr(g_controlsNames[i]) + ": ");
        titleL->setAlignment(Qt::AlignRight);

        QLabel *valueL = new QLabel("0");

        Slider *slider = new Slider;
        slider->setTickPosition(QSlider::TicksBelow);
        slider->setMinimumWidth(50);
        slider->setTickInterval(25);
        slider->setRange(-100, 100);
        slider->setWheelStep(1);
        slider->setValue(0);
        connect(slider, &Slider::valueChanged, this, [=](int v) {
            valueL->setText(QString::number(v));
            emit videoAdjustmentChanged(titleL->text() + QString::number(v));
        });
        m_sliders.push_back(slider);

        QAction *actionDown = new QAction(this);
        connect(actionDown, &QAction::triggered, this, [=] {
            slider->setValue(slider->value() - g_step);
        });

        QAction *actionUp = new QAction(this);
        connect(actionUp, &QAction::triggered, this, [=] {
            slider->setValue(slider->value() + g_step);
        });

        m_actions.push_back({actionDown, actionUp});

        layout->addWidget(titleL, i, 0);
        layout->addWidget(slider, i, 1);
        layout->addWidget(valueL, i, 2);
    }

    QPushButton *resetB = new QPushButton(tr("Reset"));
    connect(resetB, &QPushButton::clicked, this, [this] {
        for (int i = 0; i < CONTROLS_COUNT; ++i)
            m_sliders[i]->setValue(0);
    });

    m_resetAction = new QAction(tr("Reset video adjustments"), this);
    connect(m_resetAction, &QAction::triggered, resetB, &QPushButton::click);

    layout->addWidget(resetB, layout->rowCount(), 0, 1, 3);
    layout->addItem(new QSpacerItem(40, 0, QSizePolicy::Maximum, QSizePolicy::Minimum), layout->rowCount(), 2);

    setLayout(layout);
}
#else
    : m_sliders(new Slider[CONTROLS_COUNT])
{
    QGridLayout *layout = new QGridLayout;
    int i;
    for (i = 0; i < CONTROLS_COUNT; ++i)
    {

        QLabel *titleL = new QLabel(tr(g_controlsNames[i]) + ": ");
        titleL->setAlignment(Qt::AlignRight);

        QLabel *valueL = new QLabel("0");

        Slider *slider = &m_sliders[i];
        slider->setProperty("valueL", qVariantFromValue((void *)valueL));
        slider->setTickPosition(QSlider::TicksBelow);
        slider->setMinimumWidth(50);
        slider->setTickInterval(25);
        slider->setRange(-100, 100);
        slider->setWheelStep(1);
        slider->setValue(0);
        connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));

        layout->addWidget(titleL, i, 0);
        layout->addWidget(slider, i, 1);
        layout->addWidget(valueL, i, 2);
    }

    QPushButton *resetB = new QPushButton(tr("Reset"));
    connect(resetB, SIGNAL(clicked()), this, SLOT(reset()));

    layout->addWidget(resetB, i++, 0, 1, 3);
    layout->addItem(new QSpacerItem(40, 0, QSizePolicy::Maximum, QSizePolicy::Minimum), i, 2);

    setLayout(layout);
}
#endif

VideoAdjustmentW::~VideoAdjustmentW()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    delete[] m_sliders;
#endif
}

void VideoAdjustmentW::restoreValues()
{
    for (int i = 0; i < CONTROLS_COUNT; ++i)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        m_sliders[i]->setValue(QMPlay2Core.getSettings().getInt(QString("VideoAdjustment/") + g_controlsNames[i]));
#else
        m_sliders[i].setValue(QMPlay2Core.getSettings().getInt(QString("VideoAdjustment/") + g_controlsNames[i]));
#endif
}
void VideoAdjustmentW::saveValues()
{
    for (int i = 0; i < CONTROLS_COUNT; ++i)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        QMPlay2Core.getSettings().set(QString("VideoAdjustment/") + g_controlsNames[i], m_sliders[i]->value());
#else
        QMPlay2Core.getSettings().set(QString("VideoAdjustment/") + g_controlsNames[i], m_sliders[i].value());
#endif
}

void VideoAdjustmentW::setModuleParam(ModuleParams *writer)
{
    for (int i = 0; i < CONTROLS_COUNT; ++i)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        m_sliders[i]->setEnabled(writer->hasParam(g_controlsNames[i]));
        writer->modParam(g_controlsNames[i], m_sliders[i]->value());
#else
        m_sliders[i].setEnabled(writer->hasParam(g_controlsNames[i]));
        writer->modParam(g_controlsNames[i], m_sliders[i].value());
#endif
    }
}

void VideoAdjustmentW::enableControls()
{
    for (int i = 0; i < CONTROLS_COUNT; ++i)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        m_sliders[i]->setEnabled(true);
#else
        m_sliders[i].setEnabled(true);
#endif
}

void VideoAdjustmentW::setValue(int v)
{
    ((QLabel *)sender()->property("valueL").value<void *>())->setText(QString::number(v));
    emit videoAdjustmentChanged();
}
void VideoAdjustmentW::reset()
{
    for (int i = 0; i < CONTROLS_COUNT; ++i)
        m_sliders[i].setValue(0);
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
void VideoAdjustmentW::setKeyShortcuts()
{
    ShortcutHandler *shortcuts = QMPlay2GUI.shortcutHandler;

    const auto appendAction = [&](QAction *action, const QString &text, const QString &settingsName, const QString &key) {
        if (!text.isEmpty())
            action->setText(text);
        shortcuts->appendAction(action, "KeyBindings/VideoAdjustment-" + settingsName, key.isEmpty() ? QString() : "Shift+" + key);
    };

    appendAction(m_actions[BRIGHTNESS][0], tr("Brightness down"), "brightnessDown", "1");
    appendAction(m_actions[BRIGHTNESS][1], tr("Brightness up"), "brightnessUp", "2");

    appendAction(m_actions[CONTRAST][0], tr("Contrast down"), "contrastDown", "3");
    appendAction(m_actions[CONTRAST][1], tr("Contrast up"), "contrastUp", "4");

    appendAction(m_actions[SATURATION][0], tr("Saturation down"), "saturationDown", "5");
    appendAction(m_actions[SATURATION][1], tr("Saturation up"), "saturationUp", "6");

    appendAction(m_actions[HUE][0], tr("Hue down"), "hueDown", QString());
    appendAction(m_actions[HUE][1], tr("Hue up"), "hueUp", QString());

    appendAction(m_actions[SHARPNESS][0], tr("Sharpness down"), "sharpnessDown", "7");
    appendAction(m_actions[SHARPNESS][1], tr("Sharpness up"), "sharpnessUp", "9");

    appendAction(m_resetAction, QString(), "reset", "0");
}
void VideoAdjustmentW::addActionsToWidget(QWidget *w)
{
    for (int i = 0; i < CONTROLS_COUNT; ++i)
    {
        for (int j = 0; j < 2; ++j)
            w->addAction(m_actions[i][j]);
    }
    w->addAction(m_resetAction);
}
#endif
