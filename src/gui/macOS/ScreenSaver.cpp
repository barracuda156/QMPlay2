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

#include <ScreenSaver.hpp>

#include <IOKit/pwr_mgt/IOPMLib.h>

#ifdef __APPLE__
	#include <AvailabilityMacros.h>
#endif

#define QMPLAY2_MEDIA_PLAYBACK CFSTR("QMPlay2 media playback")

class ScreenSaverPriv
{
public:
	inline ~ScreenSaverPriv()
	{
		unInhibit();
	}

	inline void inhibit()
	{
#if defined(__APPLE__) && (MAC_OS_X_VERSION_MAX_ALLOWED < 1060)
		m_okDisp = (IOPMAssertionCreate(kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, &m_idDisp) == kIOReturnSuccess);
		m_okSys  = (IOPMAssertionCreate(kIOPMAssertionTypeNoIdleSleep, kIOPMAssertionLevelOn, &m_idSys)  == kIOReturnSuccess);
#else
		m_okDisp = (IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, QMPLAY2_MEDIA_PLAYBACK, &m_idDisp) == kIOReturnSuccess);
		m_okSys  = (IOPMAssertionCreateWithName(kIOPMAssertionTypeNoIdleSleep, kIOPMAssertionLevelOn, QMPLAY2_MEDIA_PLAYBACK, &m_idSys) == kIOReturnSuccess);
#endif
	}
	inline void unInhibit()
	{
		if (m_okDisp)
		{
			IOPMAssertionRelease(m_idDisp);
			m_okDisp = false;
		}
		if (m_okSys)
		{
			IOPMAssertionRelease(m_idSys);
			m_okSys = false;
		}
	}

private:
	IOPMAssertionID m_idDisp;
	IOPMAssertionID m_idSys;
	bool m_okDisp = false;
	bool m_okSys  = false;
};

/**/

ScreenSaver::ScreenSaver() :
	m_priv(new ScreenSaverPriv)
{}
ScreenSaver::~ScreenSaver()
{
	delete m_priv;
}

void ScreenSaver::inhibit(int context)
{
	if (inhibitHelper(context))
		m_priv->inhibit();
}
void ScreenSaver::unInhibit(int context)
{
	if (unInhibitHelper(context))
		m_priv->unInhibit();
}
