/*
    QMPlay2 is a video and audio player.
    Copyright (C) 2010-2025  Błażej Szczygieł

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

#include <OpenThr.hpp>

#include <ChapterProgramInfo.hpp>
#include <StreamInfo.hpp>

#include <QCoreApplication>

extern "C"
{
    #include <libavformat/version.h>
}

struct AVFormatContext;
struct AVDictionary;
struct AVStream;
struct AVPacket;
class OggHelper;
class Packet;
#ifdef Q_OS_ANDROID
class QFile;
#endif

class FormatContext
{
    Q_DECLARE_TR_FUNCTIONS(FormatContext)
public:
    FormatContext(bool reconnectNetwork = false, bool allowExperimental = false);
    ~FormatContext();

    bool metadataChanged() const;

    bool isStillImage() const;

    QList<ProgramInfo> getPrograms() const;
    QList<ChapterInfo> getChapters() const;

    QString name() const;
    QString title() const;
    QList<QMPlay2Tag> tags() const;
    bool getReplayGain(bool album, float &gain_db, float &peak) const;
    qint64 size() const;
    double length() const;
    int bitrate() const;
    QByteArray image(bool forceCopy) const;

    bool seek(double pos, bool backward);
    bool read(Packet &encoded, int &idx);
    void pause();
    void abort();

    bool open(const QString &_url, const QString &param = QString());

    void setStreamOffset(double offset);

    void selectStreams(const QSet<int> &selectedStreams);

    bool isLocal, isStreamed, isError, m_allDiscarded = false;
    QList<StreamInfo *> streamsInfo;
    double currPos;
private:
    StreamInfo *getStreamInfo(AVStream *stream) const;
    AVDictionary *getMetadata() const;

    std::shared_ptr<AbortContext> abortCtx;

    QVector<int> index_map;
    QVector<AVStream *> streams;
    QVector<double> streamsTS;
    QVector<double> streamsOffset;
    QVector<double> nextDts;
    AVFormatContext *formatCtx;
    AVPacket *packet;

    OggHelper *oggHelper;

    const bool m_reconnectNetwork;
    const bool m_allowExperimental;
    bool isPaused, fixMkvAss;
    mutable bool isMetadataChanged;
    double lastTime, startTime;
    bool isOneStreamOgg;
    bool forceCopy;

    int m_retErrCount, errFromSeek;
    bool maybeHasFrame;

    bool artistWithTitle;
    bool stillImage;

    double lengthToPlay;

#ifdef Q_OS_ANDROID
    std::unique_ptr<QFile> m_file;
#endif
};
