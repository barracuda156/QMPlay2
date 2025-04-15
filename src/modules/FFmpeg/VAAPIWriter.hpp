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

#include <VideoWriter.hpp>
#include <VAAPI.hpp>

#include <QWidget>
#include <QTimer>

class VAAPIWriter : public QWidget, public VideoWriter
{
    Q_OBJECT
public:
    VAAPIWriter(Module &module, VAAPI *vaapi);
    ~VAAPIWriter() final;

    bool set() override final;

    bool readyWrite() const override final;

    bool processParams(bool *paramsCorrected) override final;
    void writeVideo(const VideoFrame &videoFrame) override final;
    void writeOSD(const QList<const QMPlay2OSD *> &osd) override final;
    void pause() override final;

    bool hwAccelGetImg(const VideoFrame &videoFrame, void *dest, ImgScaler *nv12ToRGB32) const override final;

    QString name() const override final;

    bool open() override final;

    /**/

    inline VAAPI *getVAAPI() const
    {
        return vaapi;
    }

private:
    void draw(VASurfaceID id = -1, int field = -1);

    void resizeEvent(QResizeEvent *) override final;
    void paintEvent(QPaintEvent *) override final;
    bool event(QEvent *) override final;

    QPaintEngine *paintEngine() const override final;

    void clearVaImage();

    VAAPI *vaapi;

    bool paused;

    static constexpr int drawTimeout = 40;
    QList<const QMPlay2OSD *> osd_list;
    bool m_subpictDestIsScreenCoord = false;
    QVector<quint64> osd_ids;
    VASubpictureID m_vaSubpicID = VA_INVALID_ID;
    VAImageFormat m_rgbImgFmt = {};
    QMutex osd_mutex;
    QTimer drawTim;
    VAImage m_vaImg = {};

    QRect dstQRect, srcQRect;
    double aspect_ratio = 0.0, zoom = 0.0;
    VASurfaceID m_id = VA_INVALID_SURFACE;
    int m_field = -1, X = 0, Y = 0, W = 0, H = 0, deinterlace, Hue = 0, Saturation = 0, Brightness = 0, Contrast = 0;

    QHash<VASurfaceID, VideoFrame> m_frames;
};
