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

#include <YouTube.hpp>

#include <YouTubeDL.hpp>
#include <LineEdit.hpp>

#include <QStringListModel>
#include <QDesktopServices>
#include <QTextDocument>
#include <QProgressBar>
#include <QApplication>
#include <QHeaderView>
#include <QGridLayout>
#include <QToolButton>
#include <QCompleter>
#include <QClipboard>
#include <QMimeData>
#include <QSpinBox>
#include <QAction>
#include <QMenu>
#include <QUrl>
#include <QSignalMapper>
#include <QDebug>

#include <QJsonParseError.h>
#include <QJsonObject.h>
#include <QJsonArray.h>

#define YOUTUBE_URL "https://www.youtube.com"

static inline QString toPercentEncoding(const QString &txt)
{
    return txt.toUtf8().toPercentEncoding();
}

static inline QString getYtUrl(const QString &title, const int page, const int sortByIdx)
{
    static const char *sortBy[4] {
        "",             // Relevance ("&sp=CAA%253D")
        "&sp=CAI%253D", // Upload date
        "&sp=CAM%253D", // View count
        "&sp=CAE%253D", // Rating
    };
    Q_ASSERT(sortByIdx >= 0 && sortByIdx <= 3);
    return QString(YOUTUBE_URL "/results?search_query=%1%2&page=%3").arg(toPercentEncoding(title), sortBy[sortByIdx]).arg(page);
}
static inline QString getAutocompleteUrl(const QString &text)
{
    return QString("http://suggestqueries.google.com/complete/search?client=firefox&ds=yt&q=%1").arg(toPercentEncoding(text));
}

static inline bool isPlaylist(QTreeWidgetItem *tWI)
{
    return tWI->data(1, Qt::UserRole).toBool();
}

/**/

ResultsYoutube::ResultsYoutube()
    : menu(new QMenu(this))
{
    setAnimated(true);
    setIndentation(12);
    setIconSize({100, 100});
    setExpandsOnDoubleClick(false);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    headerItem()->setText(0, tr("Title"));
    headerItem()->setText(1, tr("Length"));
    headerItem()->setText(2, tr("User"));

    header()->setStretchLastSection(false);
    header()->setResizeMode(0, QHeaderView::Stretch);
    header()->setResizeMode(1, QHeaderView::ResizeToContents);

    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(playEntry(QTreeWidgetItem *)));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenu(const QPoint &)));
    setContextMenuPolicy(Qt::CustomContextMenu);
}
ResultsYoutube::~ResultsYoutube()
{}

void ResultsYoutube::playOrEnqueue(const QString &param, QTreeWidgetItem *tWI, const QString &addrParam)
{
    if (!tWI)
        return;
    if (!isPlaylist(tWI))
    {
        emit QMPlay2Core.processParam(param, "YouTube://{" + tWI->data(0, Qt::UserRole).toString() + "}" + addrParam);
    }
    else
    {
        const QStringList ytPlaylist = tWI->data(0, Qt::UserRole + 1).toStringList();
        QMPlay2CoreClass::GroupEntries entries;
        for (int i = 0; i < ytPlaylist.count() ; i += 2)
            entries += {ytPlaylist[i+1], "YouTube://{" YOUTUBE_URL "/watch?v=" + ytPlaylist[i+0] + "}" + addrParam};
        if (!entries.isEmpty())
        {
            const bool enqueue = (param == "enqueue");
            QMPlay2Core.loadPlaylistGroup(YouTubeName "/" + QString(tWI->text(0)).replace('/', '_'), entries, enqueue);
        }
    }
}

void ResultsYoutube::playEntry(QTreeWidgetItem *tWI)
{
    playOrEnqueue("open", tWI);
}

void ResultsYoutube::playCurrentItem()
{
    QTreeWidgetItem *tWI = currentItem();
    if (!tWI)
        return;

    playOrEnqueue("open", tWI, currentParam);
}

void ResultsYoutube::enqueueCurrentItem()
{
    QTreeWidgetItem *tWI = currentItem();
    if (!tWI)
        return;

    playOrEnqueue("enqueue", tWI, currentParam);
}

void ResultsYoutube::openPage()
{
    QTreeWidgetItem *tWI = currentItem();
    if (tWI)
        QDesktopServices::openUrl(tWI->data(0, Qt::UserRole).toString());
}
void ResultsYoutube::copyPageURL()
{
    QTreeWidgetItem *tWI = currentItem();
    if (tWI)
    {
        QMimeData *mimeData = new QMimeData;
        mimeData->setText(tWI->data(0, Qt::UserRole).toString());
        QApplication::clipboard()->setMimeData(mimeData);
    }
}

void ResultsYoutube::contextMenu(const QPoint &point)
{
    menu->clear();
    QTreeWidgetItem *tWI = currentItem();
    if (!tWI)
        return;

    const QString name = tWI->text(0);
    const QString url = tWI->data(0, Qt::UserRole).toString();

    for (int i = 0; i < 2; ++i)
    {
        QAction *section = new QAction(i == 0 ? tr("Audio and video") : tr("Audio only"), menu);
        section->setEnabled(false); // Make it non-interactive
        menu->addAction(section);

        if (!tWI->isDisabled())
        {
            currentParam = (i == 0 ? QString() : QString("audio"));
            QAction *playAction = menu->addAction(tr("Play"));
            QAction *enqueueAction = menu->addAction(tr("Enqueue"));

            connect(playAction, SIGNAL(triggered()), this, SLOT(playCurrentItem()));
            connect(enqueueAction, SIGNAL(triggered()), this, SLOT(enqueueCurrentItem()));

            menu->addSeparator();
        }

        if (i == 0)
        {
            menu->addAction(tr("Open the page in the browser"), this, SLOT(openPage()));
            menu->addAction(tr("Copy page address"), this, SLOT(copyPageURL()));
            menu->addSeparator();
        }

        if (isPlaylist(tWI))
            continue;

        for (QMPlay2Extensions *QMPlay2Ext : QMPlay2Extensions::QMPlay2ExtensionsList())
        {
            if (dynamic_cast<YouTube *>(QMPlay2Ext))
                continue;

            for (QAction *act : QMPlay2Ext->getActions(name, -2, url, "YouTube", i == 0 ? QString() : QString("audio")))
            {
                act->setParent(menu);
                menu->addAction(act);
            }
        }
    }

    menu->popup(viewport()->mapToGlobal(point));
}

/**/

PageSwitcher::PageSwitcher(QWidget *youTubeW)
{
    prevB = new QToolButton;
    connect(prevB, SIGNAL(clicked()), youTubeW, SLOT(prev()));
    prevB->setAutoRaise(true);
    prevB->setArrowType(Qt::LeftArrow);

    currPageB = new QSpinBox;
    connect(currPageB, SIGNAL(editingFinished()), youTubeW, SLOT(chPage()));
    currPageB->setMinimum(1);
    currPageB->setMaximum(50); //1000 wyników, po 20 wyników na stronę

    nextB = new QToolButton;
    connect(nextB, SIGNAL(clicked()), youTubeW, SLOT(next()));
    nextB->setAutoRaise(true);
    nextB->setArrowType(Qt::RightArrow);

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(prevB);
    hLayout->addWidget(currPageB);
    hLayout->addWidget(nextB);
}

/**/

const QStringList YouTube::getQualityPresets()
{
    return {
        "4320p 60FPS",
        "2160p 60FPS",
        "1440p 60FPS",
        "1080p 60FPS",
        "720p 60FPS",
        "2160p",
        "1440p",
        "1080p",
        "720p",
        "480p",
    };
}

void YouTube::onQualityPresetChanged(const QString &preset) {
    sets().set("YouTube/QualityPreset", preset);
}

void YouTube::onQualityToggled() {
    QAction *act = qobject_cast<QAction *>(sender());
    if (act && act->isChecked()) {
        int qualityIdx = m_qualityGroup->actions().indexOf(act);
        setItags(qualityIdx);
    }
}

YouTube::YouTube(Module &module) :
    completer(new QCompleter(new QStringListModel(this), this)),
    currPage(1),
    net(this)
{
    youtubeIcon = QIcon(":/youtube.svgz");
    videoIcon = QIcon(":/video.svgz");

    dw = new DockWidget;
    connect(dw, SIGNAL(visibilityChanged(bool)), this, SLOT(setEnabled(bool)));
    dw->setWindowTitle("YouTube");
    dw->setObjectName(YouTubeName);
    dw->setWidget(this);

    completer->setCaseSensitivity(Qt::CaseInsensitive);

    searchE = new LineEdit;
#ifndef Q_OS_ANDROID
    connect(searchE, SIGNAL(textEdited(const QString &)), this, SLOT(searchTextEdited(const QString &)));
#endif
    connect(searchE, SIGNAL(clearButtonClicked()), this, SLOT(search()));
    connect(searchE, SIGNAL(returnPressed()), this, SLOT(search()));
    searchE->setCompleter(completer);

    searchB = new QToolButton;
    connect(searchB, SIGNAL(clicked()), this, SLOT(search()));
    searchB->setIcon(QMPlay2Core.getIconFromTheme("edit-find"));
    searchB->setToolTip(tr("Search"));
    searchB->setAutoRaise(true);

    QToolButton *showSettingsB = new QToolButton;
    connect(showSettingsB, SIGNAL(clicked()), this, SLOT(onShowSettingsClicked()));

    showSettingsB->setIcon(QMPlay2Core.getIconFromTheme("configure"));
    showSettingsB->setToolTip(tr("Settings"));
    showSettingsB->setAutoRaise(true);

    m_qualityGroup = new QActionGroup(this);
    for (auto &&qualityPreset : getQualityPresets())
        m_qualityGroup->addAction(qualityPreset);

    QMenu *qualityMenu = new QMenu(this);
    int qualityIdx = 0;
    QSignalMapper *qualitySignalMapper = new QSignalMapper(this);

    for (int i = 0; i < m_qualityGroup->actions().size(); ++i) {
        QAction *act = m_qualityGroup->actions().at(i);

        qualitySignalMapper->setMapping(act, act->text());
        connect(act, SIGNAL(triggered()), qualitySignalMapper, SLOT(map()));
        connect(act, SIGNAL(toggled(bool)), this, SLOT(onQualityToggled()));

        act->setCheckable(true);
        qualityMenu->addAction(act);
        ++qualityIdx;
    }
    connect(qualitySignalMapper, SIGNAL(mapped(QString)), this, SLOT(onQualityPresetChanged(QString)));
    if (qualityMenu->actions().size() > 5) {
        qualityMenu->insertSeparator(qualityMenu->actions().at(5));
    }

    QToolButton *qualityB = new QToolButton;
    qualityB->setPopupMode(QToolButton::InstantPopup);
    qualityB->setToolTip(tr("Preferred quality"));
    qualityB->setIcon(QMPlay2Core.getIconFromTheme("video-display"));
    qualityB->setMenu(qualityMenu);
    qualityB->setAutoRaise(true);

    m_sortByGroup = new QActionGroup(this);
    m_sortByGroup->addAction(tr("Relevance"));
    m_sortByGroup->addAction(tr("Upload date"));
    m_sortByGroup->addAction(tr("View count"));
    m_sortByGroup->addAction(tr("Rating"));

    QMenu *sortByMenu = new QMenu(this);
    QSignalMapper *sortBySignalMapper = new QSignalMapper(this);

    for (int i = 0; i < m_sortByGroup->actions().size(); ++i) {
        QAction *act = m_sortByGroup->actions().at(i);

        sortBySignalMapper->setMapping(act, i);
        connect(act, SIGNAL(triggered()), sortBySignalMapper, SLOT(map()));

        act->setCheckable(true);
        sortByMenu->addAction(act);
    }
    connect(sortBySignalMapper, SIGNAL(mapped(int)), this, SLOT(onSortByChanged(int)));

    QToolButton *sortByB = new QToolButton;
    sortByB->setPopupMode(QToolButton::InstantPopup);
    sortByB->setToolTip(tr("Sort search results by ..."));
    {
        // FIXME: Add icon
        QFont f(sortByB->font());
        f.setBold(true);
        f.setPointSize(qMax(7, f.pointSize() - 1));
        sortByB->setFont(f);
        sortByB->setText("A-z");
    }
    sortByB->setMenu(sortByMenu);
    sortByB->setAutoRaise(true);

    resultsW = new ResultsYoutube;

    progressB = new QProgressBar;
    progressB->hide();

    pageSwitcher = new PageSwitcher(this);
    pageSwitcher->hide();

    connect(&net, SIGNAL(finished(NetworkReply *)), this, SLOT(netFinished(NetworkReply *)));

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(showSettingsB, 0, 0, 1, 1);
    layout->addWidget(qualityB, 0, 1, 1, 1);
    layout->addWidget(sortByB, 0, 2, 1, 1);
    layout->addWidget(searchE, 0, 3, 1, 1);
    layout->addWidget(searchB, 0, 4, 1, 1);
    layout->addWidget(pageSwitcher, 0, 5, 1, 1);
    layout->addWidget(resultsW, 1, 0, 1, 6);
    layout->addWidget(progressB, 2, 0, 1, 6);
    layout->setSpacing(3);
    setLayout(layout);

    SetModule(module);
}
YouTube::~YouTube()
{}

bool YouTube::set()
{
    const auto preferredCodec = sets().getString("YouTube/PreferredCodec");
    const auto oldPpreferredCodec = m_preferredCodec;

    if (preferredCodec == "H.264")
        m_preferredCodec = PreferredCodec::H264;
    else if (preferredCodec == "AV1")
        m_preferredCodec = PreferredCodec::AV1;
    else
        m_preferredCodec = PreferredCodec::VP9;

    const auto qualityActions = m_qualityGroup->actions();
    const auto qualityText = sets().getString("YouTube/QualityPreset");
    bool qualityActionChecked = false;
    if (!qualityText.isEmpty())
    {
        for (auto &&qualityAction : qualityActions)
        {
            if (qualityAction->text() == qualityText)
            {
                if (oldPpreferredCodec != m_preferredCodec && qualityAction->isChecked())
                    qualityAction->setChecked(false); // Force "toggled" signal
                qualityAction->setChecked(true);
                qualityActionChecked = true;
                break;
            }
        }
    }
    if (!qualityActionChecked)
    {
        if (oldPpreferredCodec != m_preferredCodec && qualityActions[3]->isChecked())
            qualityActions[3]->setChecked(false); // Force "toggled" signal
        qualityActions[3]->setChecked(true);
    }

    resultsW->setColumnCount(sets().getBool("YouTube/ShowUserName") ? 3 : 2);
    m_allowSubtitles = sets().getBool("YouTube/Subtitles");
    m_sortByIdx = qBound(0, sets().getInt("YouTube/SortBy"), 3);
    m_sortByGroup->actions().at(m_sortByIdx)->setChecked(true);
    return true;
}

DockWidget *YouTube::getDockWidget()
{
    return dw;
}

bool YouTube::canConvertAddress() const
{
    return true;
}

QString YouTube::matchAddress(const QString &url) const
{
    const QUrl qurl(url);
    if (qurl.scheme().startsWith("http") && (qurl.host().contains("youtube.") || qurl.host().contains("youtu.be")))
        return "YouTube";
    return QString();
}
QList<YouTube::AddressPrefix> YouTube::addressPrefixList(bool img) const
{
    return {
        AddressPrefix("YouTube", img ? youtubeIcon : QIcon()),
        AddressPrefix("youtube-dl", img ? videoIcon : QIcon())
    };
}
void YouTube::convertAddress(const QString &prefix, const QString &url, const QString &param, QString *stream_url, QString *name, QIcon *icon, QString *extension, IOController<> *ioCtrl)
{
    if (!stream_url && !name && !icon)
        return;
    if (prefix == "YouTube")
    {
        if (icon)
            *icon = youtubeIcon;
        if (ioCtrl && (stream_url || name))
        {
            auto &youTubeDl = ioCtrl->toRef<YouTubeDL>();
            const QStringList youTubeVideo = getYouTubeVideo(param, url, youTubeDl);
            if (youTubeVideo.count() == 3)
            {
                if (stream_url)
                    *stream_url = youTubeVideo[0];
                if (name && !youTubeVideo[2].isEmpty())
                    *name = youTubeVideo[2];
                if (extension)
                    *extension = youTubeVideo[1];
            }
            youTubeDl.reset();
        }
    }
    else if (prefix == "youtube-dl")
    {
        if (icon)
            *icon = videoIcon;
        if (ioCtrl)
        {
            IOController<YouTubeDL> &youTubeDL = ioCtrl->toRef<YouTubeDL>();
            if (ioCtrl->assign(new YouTubeDL))
            {
                youTubeDL->addr(url, param, stream_url, name, extension);
                ioCtrl->reset();
            }
        }
    }
}

QVector<QAction *> YouTube::getActions(const QString &name, double, const QString &url, const QString &, const QString &)
{
    if (name != url)
    {
        QAction *act = new QAction(YouTube::tr("Search on YouTube"), nullptr);
        act->connect(act, SIGNAL(triggered()), this, SLOT(searchMenu()));
        act->setIcon(youtubeIcon);
        act->setProperty("name", name);
        return {act};
    }
    return {};
}

void YouTube::next()
{
    pageSwitcher->currPageB->setValue(pageSwitcher->currPageB->value() + 1);
    chPage();
}
void YouTube::prev()
{
    pageSwitcher->currPageB->setValue(pageSwitcher->currPageB->value() - 1);
    chPage();
}
void YouTube::chPage()
{
    if (currPage != pageSwitcher->currPageB->value())
    {
        currPage = pageSwitcher->currPageB->value();
        search();
    }
}

void YouTube::searchTextEdited(const QString &text)
{
    if (autocompleteReply)
        autocompleteReply->deleteLater();
    if (text.isEmpty())
        ((QStringListModel *)completer->model())->setStringList({});
    else
        autocompleteReply = net.start(getAutocompleteUrl(text));
}
void YouTube::search()
{
    const QString title = searchE->text();
    deleteReplies();
    if (autocompleteReply)
        autocompleteReply->deleteLater();
    if (searchReply)
        searchReply->deleteLater();
    resultsW->clear();
    if (!title.isEmpty())
    {
        if (lastTitle != title || sender() == searchE || sender() == searchB || qobject_cast<QAction *>(sender()))
            currPage = 1;
        searchReply = net.start(getYtUrl(title, currPage, m_sortByIdx));
        progressB->setRange(0, 0);
        progressB->show();
    }
    else
    {
        pageSwitcher->hide();
        progressB->hide();
    }
    lastTitle = title;
}

void YouTube::netFinished(NetworkReply *reply)
{
    if (reply->hasError())
    {
        if (reply == searchReply)
        {
            deleteReplies();
            resultsW->clear();
            lastTitle.clear();
            progressB->hide();
            pageSwitcher->hide();
            emit QMPlay2Core.sendMessage(tr("Connection error"), YouTubeName, 3);
        }
    }
    else
    {
        QTreeWidgetItem *tWI = ((QTreeWidgetItem *)reply->property("tWI").value<void *>());
        const QByteArray replyData = reply->readAll();
        if (reply == autocompleteReply)
        {
            setAutocomplete(replyData);
        }
        else if (reply == searchReply)
        {
            setSearchResults(replyData);
        }
        else if (linkReplies.contains(reply))
        {
            if (isPlaylist(tWI))
                preparePlaylist(replyData, tWI);
        }
        else if (imageReplies.contains(reply))
        {
            QPixmap p;
            if (p.loadFromData(replyData))
                tWI->setIcon(0, p);
        }
    }

    if (linkReplies.contains(reply))
    {
        linkReplies.removeOne(reply);
        progressB->setValue(progressB->value() + 1);
    }
    else if (imageReplies.contains(reply))
    {
        imageReplies.removeOne(reply);
        progressB->setValue(progressB->value() + 1);
    }

    if (progressB->isVisible() && linkReplies.isEmpty() && imageReplies.isEmpty())
        progressB->hide();

    reply->deleteLater();
}

void YouTube::searchMenu()
{
    const QString name = sender()->property("name").toString();
    if (!name.isEmpty())
    {
        if (!dw->isVisible())
            dw->show();
        dw->raise();
        searchE->setText(name);
        search();
    }
}

void YouTube::setItags(int qualityIdx)
{
    // Itag info: https://gist.github.com/AgentOak/34d47c65b1d28829bb17c24c04a0096f
    enum
    {
        // Video
        H264_144p = 160,
        H264_240p = 133,
        H264_360p = 134,
        H264_480p = 135,
        H264_720p = 136,
        H264_1080p = 137,
        H264_1440p = 264,
        H264_2160p = 266,

        H264_720p60 = 298,
        H264_1080p60 = 299,
        H264_1440p60 = 304,
        H264_2160p60 = 305,

        VP9_144p = 278,
        VP9_240p = 242,
        VP9_360p = 243,
        VP9_480p = 244,
        VP9_720p = 247,
        VP9_1080p = 248,
        VP9_1440p = 271,
        VP9_2160p = 313,

        VP9_720p60 = 302,
        VP9_1080p60 = 303,
        VP9_1440p60 = 308,
        VP9_2160p60 = 315,
        VP9_4320p60 = 272,

        AV1_480p = 397,
        AV1_360p = 396,
        AV1_240p = 395,
        AV1_144p = 394,

        AV1_HFR_4320p_1 = 571,
        AV1_HFR_4320p_2 = 402,
        AV1_HFR_2160p = 401,
        AV1_HFR_1440p = 400,
        AV1_HFR_1080p = 399,
        AV1_HFR_720p = 398,

        AV1_HFR_HIGH_2160p = 701,
        AV1_HFR_HIGH_1440p = 700,
        AV1_HFR_HIGH_1080p = 699,
        AV1_HFR_HIGH_720p = 698,

        // Live video
        H264_144p_AAC_48 = 91,
        H264_240p_AAC_48 = 92,
        H264_360p_AAC_128 = 93,
        H264_480p_AAC_128 = 94,
        H264_720p_AAC_256 = 95,
        H264_1080p_AAC_256 = 96,
        H264_720p60_AAC_128 = 300,
        H264_1080p60_AAC_128 = 301,

        // Audio
        Opus_160 = 251,
        AAC_128 = 140,

        // Video + Audio
        H264_360P_AAC_128 = 18,
        H264_720P_AAC_128 = 22,
    };

    enum
    {
        Preset_4320p60,
        Preset_2160p60,
        Preset_1440p60,
        Preset_1080p60,
        Preset_720p60,
        Preset_2160p,
        Preset_1440p,
        Preset_1080p,
        Preset_720p,
        Preset_480p,

        PresetCount,
    };

    QVector<int> qualityPresets[PresetCount];
    {
        if (m_preferredCodec == PreferredCodec::VP9)
        {
            qualityPresets[Preset_480p]  << VP9_480p << H264_480p << VP9_360p << H264_360p << H264_360P_AAC_128 << VP9_240p << H264_240p << VP9_144p << H264_144p;
            qualityPresets[Preset_720p]  << VP9_720p << H264_720p << H264_720P_AAC_128 << qualityPresets[Preset_480p];
            qualityPresets[Preset_1080p] << VP9_1080p << H264_1080p << qualityPresets[Preset_720p];
            qualityPresets[Preset_1440p] << VP9_1440p << H264_1440p << qualityPresets[Preset_1080p];
            qualityPresets[Preset_2160p] << VP9_2160p << H264_2160p << qualityPresets[Preset_1440p];

            qualityPresets[Preset_720p60]  << VP9_720p60 << H264_720p60;
            qualityPresets[Preset_1080p60] << VP9_1080p60 << H264_1080p60 << qualityPresets[Preset_720p60];
            qualityPresets[Preset_1440p60] << VP9_1440p60 << H264_1440p60 << qualityPresets[Preset_1080p60];
            qualityPresets[Preset_2160p60] << VP9_2160p60 << H264_2160p60 << qualityPresets[Preset_1440p60];
            qualityPresets[Preset_4320p60] << VP9_4320p60 << qualityPresets[Preset_2160p60];
        }
        else if (m_preferredCodec == PreferredCodec::H264)
        {
            qualityPresets[Preset_480p]  << H264_480p << VP9_480p << H264_360p << VP9_360p << H264_360P_AAC_128 << H264_240p << VP9_240p << H264_144p << VP9_144p;
            qualityPresets[Preset_720p]  << H264_720p << VP9_720p << H264_720P_AAC_128 << qualityPresets[Preset_480p];
            qualityPresets[Preset_1080p] << H264_1080p << VP9_1080p << qualityPresets[Preset_720p];
            qualityPresets[Preset_1440p] << H264_1440p << VP9_1440p << qualityPresets[Preset_1080p];
            qualityPresets[Preset_2160p] << H264_2160p << VP9_2160p << qualityPresets[Preset_1440p];

            qualityPresets[Preset_720p60]  << H264_720p60 << VP9_720p60;
            qualityPresets[Preset_1080p60] << H264_1080p60 << VP9_1080p60 << qualityPresets[Preset_720p60];
            qualityPresets[Preset_1440p60] << H264_1440p60 << VP9_1440p60 << qualityPresets[Preset_1080p60];
            qualityPresets[Preset_2160p60] << H264_2160p60 << VP9_2160p60 << qualityPresets[Preset_1440p60];
            qualityPresets[Preset_4320p60] << VP9_4320p60 << qualityPresets[Preset_2160p60];
        }
        else if (m_preferredCodec == PreferredCodec::AV1)
        {
            qualityPresets[Preset_480p]  << AV1_480p << AV1_360p << VP9_480p << H264_480p << VP9_360p << H264_360p << H264_360P_AAC_128 << AV1_240p << VP9_240p << H264_240p << AV1_144p << VP9_144p << H264_144p;
            qualityPresets[Preset_720p]  << VP9_720p << H264_720p << H264_720P_AAC_128 << qualityPresets[Preset_480p];
            qualityPresets[Preset_1080p] << VP9_1080p << H264_1080p << qualityPresets[Preset_720p];
            qualityPresets[Preset_1440p] << VP9_1440p << H264_1440p << qualityPresets[Preset_1080p];
            qualityPresets[Preset_2160p] << VP9_2160p << H264_2160p << qualityPresets[Preset_1440p];

            qualityPresets[Preset_720p60]  << AV1_HFR_HIGH_720p << AV1_HFR_720p << VP9_720p60 << H264_720p60;
            qualityPresets[Preset_1080p60] << AV1_HFR_HIGH_1080p << AV1_HFR_1080p << VP9_1080p60 << H264_1080p60 << qualityPresets[Preset_720p60];
            qualityPresets[Preset_1440p60] << AV1_HFR_HIGH_1440p << AV1_HFR_1440p << VP9_1440p60 << H264_1440p60 << qualityPresets[Preset_1080p60];
            qualityPresets[Preset_2160p60] << AV1_HFR_HIGH_2160p << AV1_HFR_2160p << VP9_2160p60 << H264_2160p60 << qualityPresets[Preset_1440p60];
            qualityPresets[Preset_4320p60] << AV1_HFR_4320p_1 << AV1_HFR_4320p_2 << VP9_4320p60 << qualityPresets[Preset_2160p60];
        }

        // Append also non-60 FPS itags to 60 FPS itags
        qualityPresets[Preset_720p60]  << qualityPresets[Preset_720p];
        qualityPresets[Preset_1080p60] << qualityPresets[Preset_1080p];
        qualityPresets[Preset_1440p60] << qualityPresets[Preset_1440p];
        qualityPresets[Preset_2160p60] << qualityPresets[Preset_2160p];
        qualityPresets[Preset_4320p60] << qualityPresets[Preset_2160p];
    }

    QVector<int> liveQualityPresets[PresetCount];
    {
        liveQualityPresets[Preset_480p]  << H264_480p_AAC_128 << H264_360p_AAC_128 << H264_240p_AAC_48 << H264_144p_AAC_48;
        liveQualityPresets[Preset_720p]  << H264_720p_AAC_256 << liveQualityPresets[Preset_480p];
        liveQualityPresets[Preset_1080p] << H264_1080p_AAC_256 << liveQualityPresets[Preset_720p];
        liveQualityPresets[Preset_1440p] << liveQualityPresets[Preset_1080p];
        liveQualityPresets[Preset_2160p] << liveQualityPresets[Preset_1440p];

        liveQualityPresets[Preset_720p60]  << H264_720p60_AAC_128;
        liveQualityPresets[Preset_1080p60] << H264_1080p60_AAC_128 << liveQualityPresets[Preset_720p60];
        liveQualityPresets[Preset_1440p60] << liveQualityPresets[Preset_1080p60];
        liveQualityPresets[Preset_2160p60] << liveQualityPresets[Preset_1440p60];
        liveQualityPresets[Preset_4320p60] << liveQualityPresets[Preset_2160p60];

        // Append also non-60 FPS itags to 60 FPS itags
        liveQualityPresets[Preset_720p60]  += liveQualityPresets[Preset_720p];
        liveQualityPresets[Preset_1080p60] += liveQualityPresets[Preset_1080p];
        liveQualityPresets[Preset_1440p60] += liveQualityPresets[Preset_1440p];
        liveQualityPresets[Preset_2160p60] += liveQualityPresets[Preset_2160p];
        liveQualityPresets[Preset_4320p60] += liveQualityPresets[Preset_2160p];
    }

    QMutexLocker locker(&m_itagsMutex);
    m_videoItags = qualityPresets[qualityIdx];
    m_audioItags = {Opus_160, AAC_128};
    m_hlsItags = liveQualityPresets[qualityIdx];
}

void YouTube::deleteReplies()
{
    while (!linkReplies.isEmpty())
        linkReplies.takeFirst()->deleteLater();
    while (!imageReplies.isEmpty())
        imageReplies.takeFirst()->deleteLater();
}

void YouTube::setAutocomplete(const QByteArray &data)
{
    QJsonParseError jsonErr;
    const QJsonDocument json = QJsonDocument::fromJson(data, &jsonErr);
    if (jsonErr.error != QJsonParseError::NoError)
    {
        qWarning() << "Cannot parse autocomplete JSON:" << jsonErr.errorString();
        return;
    }
    const QJsonArray mainArr = json.array();
    if (mainArr.count() < 2)
    {
        qWarning() << "Invalid autocomplete JSON array";
        return;
    }
    const QJsonArray arr = mainArr.at(1).toArray();
    if (arr.isEmpty())
        return;
    QStringList list;
    list.reserve(arr.count());
    for (const QJsonValue &val : arr)
        list += val.toString();
    ((QStringListModel *)completer->model())->setStringList(list);
    if (searchE->hasFocus())
        completer->complete();
}

void YouTube::setSearchResults(const QByteArray &data)
{
    const auto json = getYtInitialData(data);

    const auto sectionListRendererContents = json.object()
        ["contents"].toObject()
        ["twoColumnSearchResultsRenderer"].toObject()
        ["primaryContents"].toObject()
        ["sectionListRenderer"].toObject()
        ["contents"].toArray()
    ;

    for (auto &&obj : sectionListRendererContents)
    {
        const auto contents = obj.toObject()
            ["itemSectionRenderer"].toObject()
            ["contents"].toArray()
        ;

        for (auto &&obj : contents)
        {
            const auto videoRenderer = obj.toObject()["videoRenderer"].toObject();
            const auto playlistRenderer = obj.toObject()["playlistRenderer"].toObject();

            const bool isVideo = !videoRenderer.isEmpty() && playlistRenderer.isEmpty();

            QString title, contentId, length, user, publishTime, viewCount, thumbnail, url;

            if (isVideo)
            {
                title = videoRenderer["title"].toObject()["runs"].toArray().at(0).toObject()["text"].toString();
                contentId = videoRenderer["videoId"].toString();
                if (title.isEmpty() || contentId.isEmpty())
                    continue;

                length = videoRenderer["lengthText"].toObject()["simpleText"].toString();
                user = videoRenderer["ownerText"].toObject()["runs"].toArray().at(0).toObject()["text"].toString();
                publishTime = videoRenderer["publishedTimeText"].toObject()["simpleText"].toString();
                viewCount = videoRenderer["shortViewCountText"].toObject()["simpleText"].toString();
                thumbnail = videoRenderer["thumbnail"].toObject()["thumbnails"].toArray().at(0).toObject()["url"].toString();

                url = YOUTUBE_URL "/watch?v=" + contentId;
            }
            else
            {
                title = playlistRenderer["title"].toObject()["simpleText"].toString();
                contentId = playlistRenderer["playlistId"].toString();
                if (title.isEmpty() || contentId.isEmpty())
                    continue;

                user = playlistRenderer["longBylineText"].toObject()["runs"].toArray().at(0).toObject()["text"].toString();
                thumbnail = playlistRenderer
                    ["thumbnailRenderer"].toObject()
                    ["playlistVideoThumbnailRenderer"].toObject()
                    ["thumbnail"].toObject()
                    ["thumbnails"].toArray().at(0).toObject()
                    ["url"].toString()
                ;

                url = YOUTUBE_URL "/playlist?list=" + contentId;
            }

            auto tWI = new QTreeWidgetItem(resultsW);

            tWI->setText(0, title);
            tWI->setText(1, isVideo ? length : tr("Playlist"));
            tWI->setText(2, user);

            QString tooltip;
            tooltip += QString("%1: %2\n").arg(resultsW->headerItem()->text(0), tWI->text(0));
            tooltip += QString("%1: %2\n").arg(isVideo ? resultsW->headerItem()->text(1) : tr("Playlist"), isVideo ? tWI->text(1) : tr("yes"));
            tooltip += QString("%1: %2").arg(resultsW->headerItem()->text(2), tWI->text(2));
            if (isVideo)
            {
                tooltip += QString("\n%1: %2\n").arg(tr("Publish time"), publishTime);
                tooltip += QString("%1: %2").arg(tr("View count"), viewCount);
            }
            tWI->setToolTip(0, tooltip);

            tWI->setData(0, Qt::UserRole, url);
            tWI->setData(1, Qt::UserRole, !isVideo);

            if (!isVideo)
            {
                tWI->setDisabled(true);

                auto linkReply = net.start(url);
                linkReply->setProperty("tWI", QVariant::fromValue((void *)tWI));
                linkReplies += linkReply;
            }

            if (!thumbnail.isEmpty())
            {
                auto imageReply = net.start(thumbnail);
                imageReply->setProperty("tWI", QVariant::fromValue((void *)tWI));
                imageReplies += imageReply;
            }
        }
    }

    if (resultsW->topLevelItemCount() > 0)
    {
        pageSwitcher->currPageB->setValue(currPage);
        pageSwitcher->show();

        progressB->setMaximum(linkReplies.count() + imageReplies.count());
        progressB->setValue(0);
    }
}

QStringList YouTube::getYouTubeVideo(const QString &param, const QString &url, IOController<YouTubeDL> &youTubeDL)
{
    if (!youTubeDL.assign(new YouTubeDL))
        return {};

    const auto rawOutputs = youTubeDL->exec(url, {"--flat-playlist", "--write-sub", "-J"}, nullptr, true);
    if (rawOutputs.count() != 2)
        return {};

    const auto rawOutput = rawOutputs[0].toUtf8();
    if (rawOutput.isEmpty())
        return {};

    const auto &rawErrOutput = rawOutputs[1];

    const auto o = QJsonDocument::fromJson(rawOutput).object();
    if (o.isEmpty())
        return {};

    const auto formats = o["formats"].toArray();
    if (formats.isEmpty())
        return {};

    const bool hasTitle = !rawErrOutput.contains("Unable to extract video title", Qt::CaseInsensitive);
    const auto title = hasTitle ? o["title"].toString() : QString();

    const bool audioOnly = (param.compare("audio", Qt::CaseInsensitive) == 0);
    const bool isLive = o["is_live"].toBool();

    QStringList urls;
    QStringList exts;

    m_itagsMutex.lock();
    const auto videoItags = m_videoItags;
    const auto audioItags = m_audioItags;
    const auto hlsItags = m_hlsItags;
    m_itagsMutex.unlock();

    QHash<int, QPair<QString, QString>> itagsData;

    for (auto &&formatVal : formats)
    {
        const auto format = formatVal.toObject();
        if (format.isEmpty())
            continue;

        const auto protocol = format["protocol"].toString();
        if (protocol.contains("dash", Qt::CaseInsensitive))
        {
            if (format.contains("fragment_base_url"))
                continue; // Skip DASH, because it doesn't work
        }

        const auto itag = format["format_id"].toString().toInt();
        const auto url = format["url"].toString();
        const auto ext = format["ext"].toString();
        if (itag != 0 && !url.isEmpty() && !ext.isEmpty())
            itagsData[itag] = {url, "." + ext};
    }

    auto appendUrl = [&](const QVector<int> &itags) {
        for (auto &&itag : itags)
        {
            auto it = itagsData.constFind(itag);
            if (it != itagsData.constEnd())
            {
                urls += it->first;
                exts += it->second;
                break;
            }
        }
    };

    if (isLive)
    {
        appendUrl(hlsItags);
    }
    else
    {
        appendUrl(audioItags);
        if (!audioOnly)
            appendUrl(videoItags);
    }

    if (urls.count() != 1 + (audioOnly ? 0 : 1))
    {
        urls.clear();
        exts.clear();
    }

    if (urls.isEmpty())
    {
        qCritical() << "YouTube :: Can't find desired format, available:" << itagsData.keys();
        return {};
    }

    const auto subtitles = o["subtitles"].toObject();
    QString lang = QMPlay2Core.getSettings().getString("SubtitlesLanguage");
    if (lang.isEmpty()) // Default language
        lang = QLocale::languageToString(QLocale::system().language());
    if (!audioOnly && m_allowSubtitles && !subtitles.isEmpty() && !lang.isEmpty())
    {
        // Try to convert full language name into short language code
        for (int i = QLocale::C + 1; i <= QLocale::LastLanguage; ++i)
        {
            const QLocale::Language ll = (QLocale::Language)i;
            if (lang == QLocale::languageToString(ll))
            {
                lang = QLocale(ll).name();
                const int idx = lang.indexOf('_');
                if (idx > -1)
                    lang.remove(idx, lang.length() - idx);
                break;
            }
        }

        auto subtitlesForLang = subtitles[lang].toArray();
        if (subtitlesForLang.isEmpty())
            subtitlesForLang = subtitles[QMPlay2Core.getLanguage()].toArray();

        for (auto &&subtitlesFmtVal : asConst(subtitlesForLang))
        {
            const auto subtitlesFmt = subtitlesFmtVal.toObject();
            if (subtitlesFmt.isEmpty())
                continue;

            const auto ext = subtitlesFmt["ext"].toString();
            if (ext != "vtt")
                continue;

            const auto url = subtitlesFmt["url"].toString();
            if (url.isEmpty())
                continue;

            urls += url;
            exts += ".vtt";
            break;
        }
    }

    Q_ASSERT(!urls.isEmpty());
    Q_ASSERT(urls.count() == exts.count());

    QStringList result;
    if (urls.count() == 1)
    {
        result += urls.at(0);
        result += exts.at(0);
    }
    else
    {
        QString url = "FFmpeg://{";
        for (auto &&urlPart : asConst(urls))
            url += "[" + urlPart + "]";
        url += "}";

        QString ext;
        for (auto &&extPart : asConst(exts))
            ext += "[" + extPart + "]";

        result += url;
        result += ext;
    }
    result += title;

    return result;
}

void YouTube::preparePlaylist(const QByteArray &data, QTreeWidgetItem *tWI)
{
    QStringList playlist;

    const auto json = getYtInitialData(data);

    const auto contents = json.object()
        ["contents"].toObject()
        ["twoColumnBrowseResultsRenderer"].toObject()
        ["tabs"].toArray().at(0).toObject()
        ["tabRenderer"].toObject()
        ["content"].toObject()
        ["sectionListRenderer"].toObject()
        ["contents"].toArray().at(0).toObject()
        ["itemSectionRenderer"].toObject()
        ["contents"].toArray().at(0).toObject()
        ["playlistVideoListRenderer"].toObject()
        ["contents"].toArray()
    ;

    for (auto &&obj : contents)
    {
        const auto playlistRenderer = obj.toObject()["playlistVideoRenderer"].toObject();

        const auto title = playlistRenderer["title"].toObject()["runs"].toArray().at(0).toObject()["text"].toString();
        const auto videoId = playlistRenderer["videoId"].toString();
        if (title.isEmpty() || videoId.isEmpty())
            continue;

        playlist += {
            videoId,
            title,
        };
    }

    if (!playlist.isEmpty())
    {
        tWI->setData(0, Qt::UserRole + 1, playlist);
        tWI->setDisabled(false);
    }
}

void YouTube::onShowSettingsClicked() {
    emit QMPlay2Core.showSettings("Extensions");
}

void YouTube::onSortByChanged(int index) {
    if (m_sortByIdx != index) {
        m_sortByIdx = index;
        sets().set("YouTube/SortBy", m_sortByIdx);
        search();
    }
}

QJsonDocument YouTube::getYtInitialData(const QByteArray &data)
{
    int idx = data.indexOf("ytInitialData");
    if (idx < 0)
        return QJsonDocument();

    idx = data.indexOf("{", idx);
    if (idx < 0)
        return QJsonDocument();

    QJsonParseError e = {};
    auto jsonDoc = QJsonDocument::fromJson(data.mid(idx), &e);

    if (Q_UNLIKELY(e.error == QJsonParseError::NoError))
        return jsonDoc;

    if (e.error == QJsonParseError::GarbageAtEnd && e.offset > 0)
        return QJsonDocument::fromJson(data.mid(idx, e.offset));

    return QJsonDocument();
}
