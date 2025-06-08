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

#include <OpenGL2.hpp>
#include <OpenGL2Writer.hpp>

OpenGL2::OpenGL2() :
    Module("OpenGL2")
{
    m_icon = QIcon(":/OpenGL2.svgz");

    init("Enabled", true);
    init("AllowPBO", true);
    init("HQScaling", false);
#ifdef VSYNC_SETTINGS
    init("VSync", true);
#endif
}

QList<OpenGL2::Info> OpenGL2::getModulesInfo(const bool showDisabled) const
{
    QList<Info> modulesInfo;
    if (showDisabled || getBool("Enabled"))
        modulesInfo += Info(OpenGL2WriterName, WRITER, QStringList{"video"});
    return modulesInfo;
}

void *OpenGL2::createInstance(const QString &name)
{
    if (name == OpenGL2WriterName && getBool("Enabled"))
        return new OpenGL2Writer(*this);
    return nullptr;
}

OpenGL2::SettingsWidget *OpenGL2::getSettingsWidget()
{
    return new ModuleSettingsWidget(*this);
}

QMPLAY2_EXPORT_MODULE(OpenGL2)

/**/

#include <QGridLayout>
#include <QCheckBox>

ModuleSettingsWidget::ModuleSettingsWidget(Module &module) :
    Module::SettingsWidget(module)
{
    enabledB = new QCheckBox(tr("Enabled"));
    enabledB->setChecked(sets().getBool("Enabled"));

    allowPboB = new QCheckBox(tr("Allow to use PBO (if available)"));
    allowPboB->setChecked(sets().getBool("AllowPBO"));

    hqScalingB = new QCheckBox(tr("High quality video scaling"));
    hqScalingB->setToolTip(tr("Trilinear filtering for minification and bicubic filtering for magnification."));
    hqScalingB->setChecked(sets().getBool("HQScaling"));
#ifdef VSYNC_SETTINGS
    vsyncB = new QCheckBox(tr("Vertical sync") +  " (VSync)");
    vsyncB->setChecked(sets().getBool("VSync"));
#endif
    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(enabledB);
    layout->addWidget(allowPboB);
    layout->addWidget(hqScalingB);
#ifdef VSYNC_SETTINGS
    layout->addWidget(vsyncB);
#endif
}

void ModuleSettingsWidget::saveSettings()
{
    sets().set("Enabled", enabledB->isChecked());
    sets().set("AllowPBO", allowPboB->isChecked());
    sets().set("HQScaling", hqScalingB->isChecked());
#ifdef VSYNC_SETTINGS
    sets().set("VSync", vsyncB->isChecked());
#endif
}
