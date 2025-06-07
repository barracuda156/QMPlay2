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

#include <QVector>

extern "C"
{
    #include <libavutil/pixfmt.h>
}

enum class QMPlay2PixelFormat
{
    None = -1,

    YUV420P = AV_PIX_FMT_YUV420P,
    YUVJ420P = AV_PIX_FMT_YUVJ420P,
    YUV422P = AV_PIX_FMT_YUV422P,
    YUVJ422P = AV_PIX_FMT_YUVJ422P,
    YUV444P = AV_PIX_FMT_YUV444P,
    YUVJ444P = AV_PIX_FMT_YUVJ444P,

    YUV410P = AV_PIX_FMT_YUV410P,
    YUV411P = AV_PIX_FMT_YUV411P,
    YUVJ411P = AV_PIX_FMT_YUVJ411P,
    YUV440P = AV_PIX_FMT_YUV440P,
    YUVJ440P = AV_PIX_FMT_YUVJ440P,

    Count,
};
using QMPlay2PixelFormats = QVector<QMPlay2PixelFormat>;

enum class QMPlay2ColorSpace
{
    Unknown = -1,

    BT709,
    BT470BG,
    SMPTE170M,
    SMPTE240M,
    BT2020,
};

struct LumaCoefficients
{
    float cR, cG, cB;
};

namespace QMPlay2PixelFormatConvert {

QMPLAY2SHAREDLIB_EXPORT int toFFmpeg(QMPlay2PixelFormat pixFmt);
QMPLAY2SHAREDLIB_EXPORT QMPlay2PixelFormat fromFFmpeg(int pixFmt);

QMPLAY2SHAREDLIB_EXPORT QMPlay2ColorSpace fromFFmpegColorSpace(int colorSpace, int h);
QMPLAY2SHAREDLIB_EXPORT LumaCoefficients getLumaCoeff(QMPlay2ColorSpace colorSpace);

}
