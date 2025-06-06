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

#pragma once

#include <QVector>

extern "C"
{
	#include <libavutil/pixfmt.h>
}

enum class QMPlay2PixelFormat //Compatible with FFmpeg
{
	YUV420P = AV_PIX_FMT_YUV420P,
	YUV422P = AV_PIX_FMT_YUV422P,
	YUV444P = AV_PIX_FMT_YUV444P,
	YUV410P = AV_PIX_FMT_YUV410P,
	YUV411P = AV_PIX_FMT_YUV411P,
	YUV440P = AV_PIX_FMT_YUV440P,

	Count   =  6
};
using QMPlay2PixelFormats = QVector<QMPlay2PixelFormat>;
