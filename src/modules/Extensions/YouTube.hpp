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

#include <QMPlay2Extensions.hpp>

#include <NetworkAccess.hpp>

#include <QTreeWidget>
#include <QPointer>
#include <QMutex>

class NetworkReply;
class QProgressBar;
class QActionGroup;
class QToolButton;
class QCompleter;
class YouTubeDL;
class QSpinBox;
class LineEdit;
class QLabel;
class QMenu;

/**/

class ResultsYoutube : public QTreeWidget
{
    Q_OBJECT
public:
    ResultsYoutube();
    ~ResultsYoutube() final;

private:
    void playOrEnqueue(const QString &param, QTreeWidgetItem *tWI, const QString &addrParam = QString());

    QMenu *menu;

private slots:
    void playEntry(QTreeWidgetItem *tWI);

    void openPage();
    void copyPageURL();

    void contextMenu(const QPoint &p);
};

/**/

class PageSwitcher : public QWidget
{
    Q_OBJECT
public:
    PageSwitcher(QWidget *youTubeW);

    QToolButton *prevB, *nextB;
    QSpinBox *currPageB;
};

/**/

using ItagNames = QPair<QStringList, QList<int>>;

class YouTube : public QWidget, public QMPlay2Extensions
{
    Q_OBJECT

public:
    static const QStringList getQualityPresets();

public:
    YouTube(Module &module);
    ~YouTube() final;

    bool set() override final;

    DockWidget *getDockWidget() override final;

    bool canConvertAddress() const override final;

    QString matchAddress(const QString &url) const override final;
    QList<AddressPrefix> addressPrefixList(bool) const override final;
    void convertAddress(const QString &, const QString &, const QString &, QString *, QString *, QIcon *, QString *, IOController<> *ioCtrl) override final;

    QVector<QAction *> getActions(const QString &, double, const QString &, const QString &, const QString &) override final;

private slots:
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    void showSettings();
    void setQualityFromMenu();
#endif
    void next();
    void prev();
    void chPage();

    void searchTextEdited(const QString &text);
    void search();

    void netFinished(NetworkReply *reply);

    void searchMenu();

private:
// #if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
//     void setItags(int qualityIdx);
// #else
    void setItags();
// #endif

    void deleteReplies();

    void setAutocomplete(const QByteArray &data);
    void setSearchResults(QString data);

    QStringList getYouTubeVideo(const QString &param, const QString &url, IOController<YouTubeDL> &youTubeDL);

    void preparePlaylist(const QString &data, QTreeWidgetItem *tWI);

    DockWidget *dw;

    QIcon youtubeIcon, videoIcon;

    LineEdit *searchE;
    QToolButton *searchB;
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QMenu *qualityMenu;
#endif
    ResultsYoutube *resultsW;
    QProgressBar *progressB;
    PageSwitcher *pageSwitcher;

    QString lastTitle;
    QCompleter *completer;
    int currPage;

    QPointer<NetworkReply> autocompleteReply, searchReply;
    QList<NetworkReply *> linkReplies, imageReplies;
    NetworkAccess net;

    bool m_allowSubtitles;

// #if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
//     QActionGroup *m_qualityGroup = nullptr, *m_sortByGroup = nullptr;
// 
//     int m_sortByIdx = 0;
// #endif

    QMutex m_itagsMutex;
    QList<int> m_videoItags, m_audioItags, m_hlsItags, m_singleUrlItags;
};

#define YouTubeName "YouTube Browser"
