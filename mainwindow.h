#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QOAuth2AuthorizationCodeFlow>
#include <QStandardItemModel>
#include <QMediaPlayer>
#include <QMediaPlaylist>

#include "SpotifySearch.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QStandardItemModel *m_viewModelPlaylist;
    QStandardItemModel *m_viewModelSearch;

private slots:

    void granted();

    void authStatusChanged (QAbstractOAuth::Status status);

    void on_pbSearch_clicked();

    void newTrackSearchFound(QString trackID, QString trackName, QString previewURL);

    void on_pbAdd_clicked();

    void on_pbRemove_clicked();

    void on_pbAddMusic_clicked();

    void on_pbRemoveMusic_clicked();

    void on_actionConectar_triggered();

    void on_pbPlay_clicked();

    void on_pbPause_clicked();

    void on_pbPlaylistSave_clicked();

    void on_pbPlaylisttLoad_clicked();

private:

    void restartTrackSearch();

    void restartPlaylist();

private:
    Ui::MainWindow *ui;

    QOAuth2AuthorizationCodeFlow m_s2aSpotify;

    bool isGranted;
    QString userName;

    SpotifySearch *m_instSpotifySearch;

    QMediaPlayer *m_playerPreview;
    QMediaPlaylist *m_playlistPreview;


    enum TWMusicSearch
    {
        TW_MUSIC, TW_ID
    };

    enum TWPLAYLIST_COLUMN
    {
        TW_PL_NAME = 0,
        TW_PL_ID = 1,
        TW_PL_URL = 2
    };

};

#endif // MAINWINDOW_H
