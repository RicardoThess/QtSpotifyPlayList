#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtCore>
#include <QDebug>
#include <QtNetworkAuth>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>
#include <QtXml>
#include <QTreeView>
#include <QMessageBox>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QReadWriteLock>

#include "clientid.h"
#include "SpotifySearch.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    isGranted(false)
{
    ui->setupUi(this);

    auto replyHandler = new QOAuthHttpServerReplyHandler( 8080, this);
    replyHandler->setCallbackPath(QString("/cb/")); // necessary to set the correct redirect url (http:://localhost:8080/cb) configured on the Spotify

    m_s2aSpotify.setReplyHandler(replyHandler);
    m_s2aSpotify.setAuthorizationUrl(QUrl("https://accounts.spotify.com/authorize"));
    m_s2aSpotify.setAccessTokenUrl(QUrl("https://accounts.spotify.com/api/token"));
    m_s2aSpotify.setClientIdentifier(clientId); // came from the spotify dashboard. Necessary create a count and add a new application to get it.
    m_s2aSpotify.setClientIdentifierSharedKey(clientSecret); // provided by spotify after get the clientId.
    m_s2aSpotify.setScope("user-read-private user-top-read playlist-read-private playlist-modify-public playlist-modify-private");
        //oauth2.setScope("identity read");

    connect(&m_s2aSpotify, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, &QDesktopServices::openUrl);

    connect(&m_s2aSpotify, &QOAuth2AuthorizationCodeFlow::statusChanged, this, &MainWindow::authStatusChanged);

    connect(&m_s2aSpotify, &QOAuth2AuthorizationCodeFlow::granted, this, &MainWindow::granted);


    m_instSpotifySearch = new SpotifySearch();
    connect(m_instSpotifySearch, &SpotifySearch::newSearchTrack, this, &MainWindow::newTrackSearchFound);


    // Table With the track search result
    m_viewModelSearch = new QStandardItemModel(0, 3, this);
    ui->twMusicSearchResults->setModel(m_viewModelSearch);


    m_viewModelPlaylist = new QStandardItemModel(0, 3, this);    
    ui->treeViewPlaylist->setModel(m_viewModelPlaylist);    

    restartTrackSearch();
    restartPlaylist();

    m_playerPreview = new QMediaPlayer;
    m_playlistPreview = new QMediaPlaylist();

    ui->pteStatus->appendPlainText("Desconectado.");
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::granted ()
{
    isGranted = true;
}


void MainWindow::on_actionConectar_triggered()
{
    m_s2aSpotify.grant();
}

void MainWindow::authStatusChanged(QAbstractOAuth::Status status)
{
     switch(status) {
        case QAbstractOAuth::Status::Granted :
        {
            ui->pteStatus->clear();
            ui->pteStatus->appendPlainText("Status update: Connected.");
            break;
        }
        case QAbstractOAuth::Status::TemporaryCredentialsReceived :
        {
            ui->pteStatus->clear();
            ui->pteStatus->appendPlainText("Status update: Getting credentials.");
            break;
        }
        default:
            break;
    }
}


// --------------------------------------   Music Search ---------------------


void MainWindow::on_pbSearch_clicked()
{
    QString strMusic = ui->leMusicSearchName->text();


    if( m_instSpotifySearch == Q_NULLPTR )
    {
        qDebug("m_instSpotifySearch null pointer.");
        return;
    }

    restartTrackSearch();

    m_instSpotifySearch->SearchTracksByName(&m_s2aSpotify ,strMusic);
}

void MainWindow::newTrackSearchFound(QString trackID, QString trackName, QString previewURL)
{    
    lockModelSearch.lockForRead();

    int row = m_viewModelSearch->rowCount();

    QStandardItem *siName = new QStandardItem(trackName);
    QStandardItem *siID = new QStandardItem(trackID);
    QStandardItem *siPreviewURL = new QStandardItem(previewURL);

    m_viewModelSearch->setItem(row,0, siName);
    m_viewModelSearch->setItem(row,1, siID);
    m_viewModelSearch->setItem(row,2, siPreviewURL);

    lockModelSearch.unlock();
}


// ------------------------- PLAYLIST ---------------------------------------------
void MainWindow::on_pbAdd_clicked()
{
    QString strPlayListName = ui->inputPlaylistName->text();

    lockModelPlaylist.lockForRead();

    QList<QStandardItem*> otherPlaylist;
    otherPlaylist = m_viewModelPlaylist->findItems(strPlayListName, Qt::MatchFlag::MatchExactly, 0);
    if(!otherPlaylist.isEmpty() )
    {
        QMessageBox msgBox;
        msgBox.setText("Nome da playlist já existe.");
        msgBox.setInformativeText("Favor escolher outro nome.");
        msgBox.setStandardButtons(QMessageBox::Close);
        msgBox.setDefaultButton(QMessageBox::Close);
        msgBox.exec();

        lockModelPlaylist.unlock();

        return;
    }

    int row = m_viewModelPlaylist->rowCount();
    QStandardItem *siPlaylist = new QStandardItem(strPlayListName);
    m_viewModelPlaylist->setItem(row, 0, siPlaylist);

    lockModelPlaylist.unlock();
}

void MainWindow::on_pbAddMusic_clicked()
{
    try {

        QModelIndexList selectionPlaylist = ui->treeViewPlaylist->selectionModel()->selectedRows();
        if(selectionPlaylist.isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText("Inclusão de música na playlist.");
            msgBox.setInformativeText("Para incluir uma música, selecione antes uma playlist de destino.");
            msgBox.setStandardButtons(QMessageBox::Close);
            msgBox.setDefaultButton(QMessageBox::Close);
            msgBox.exec();

            return;
        }

        QModelIndexList selectionTracks = ui->twMusicSearchResults->selectionModel()->selectedRows();
        if(selectionTracks.isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText("Inclusão de música na playlist.");
            msgBox.setInformativeText("Selecione uma música para ser adicionada.");
            msgBox.setStandardButtons(QMessageBox::Close);
            msgBox.setDefaultButton(QMessageBox::Close);
            msgBox.exec();

            return;
        }

        // get the track name and id from search

        lockModelSearch.lockForRead();

        int trackRow = selectionTracks[0].row();
        QModelIndex index = m_viewModelSearch->index(trackRow,TW_PL_NAME,QModelIndex());
        QStandardItem *siTrackName = m_viewModelSearch->itemFromIndex(index)->clone();
        index = m_viewModelSearch->index(trackRow,TW_PL_ID,QModelIndex());
        QStandardItem *siTrackID = m_viewModelSearch->itemFromIndex(index)->clone();
        index = m_viewModelSearch->index(trackRow,TW_PL_URL,QModelIndex());
        QStandardItem *siTrackPreviewURL = m_viewModelSearch->itemFromIndex(index)->clone();

        lockModelSearch.unlock();

        // find the playlist selected.
        lockModelPlaylist.lockForRead();

        int playlistRow = selectionPlaylist[0].row();
        index = m_viewModelPlaylist->index(playlistRow,0,QModelIndex());
        qDebug() << m_viewModelPlaylist->data(index).toString();
        QStandardItem *siPlaylist = m_viewModelPlaylist->itemFromIndex(index);

        qDebug() << siPlaylist->text();

        int iTrackOnPlaylist = siPlaylist->rowCount();
        qDebug() << "Track on the current playlist: " << iTrackOnPlaylist;

        // test to check if the current track alread was included
        QStandardItem *siChildID;
        if( iTrackOnPlaylist>0 )
        {
            for( int iRow = 0; iRow < iTrackOnPlaylist;++iRow )
            {
                siChildID = siPlaylist->child(iRow, TW_PL_ID);
                if( siChildID == Q_NULLPTR)
                {
                    qDebug() << "siChildID is NULL.";
                    return ;
                }
                qDebug() << "Track ID on the list: " << siChildID->text();
                qDebug() << "Track ID on the search list: " << siTrackID->text();
                qDebug() << "Track Preview URL: " << siTrackPreviewURL->text();

                if(siChildID->text() == siTrackID->text())
                {
                    delete(siTrackName);
                    delete(siTrackID);
                    delete(siTrackPreviewURL);

                    QMessageBox msgBox;
                    msgBox.setText("Inclusão de música na playlist.");
                    msgBox.setInformativeText("Só é possível incluir uma música uma unica vez na playlist.");
                    msgBox.setStandardButtons(QMessageBox::Close);
                    msgBox.setDefaultButton(QMessageBox::Close);
                    msgBox.exec();

                    return;
                }
            }
        }

        siTrackName->setCheckable(true);

        siPlaylist->setChild(iTrackOnPlaylist, TW_PL_NAME, siTrackName);
        siPlaylist->setChild(iTrackOnPlaylist, TW_PL_ID, siTrackID);
        siPlaylist->setChild(iTrackOnPlaylist, TW_PL_URL, siTrackPreviewURL);

        lockModelPlaylist.unlock();
    } catch (...) {
       qDebug() << "Exception on on_pbAddMusic_clicked.";
       lockModelPlaylist.unlock();
    }
}


void MainWindow::on_pbRemove_clicked()
{
    QModelIndexList selection = ui->treeViewPlaylist->selectionModel()->selectedRows();

    QList<QStandardItem*> items ;
    int row;

    if(!selection.isEmpty())
    {
        row = selection[0].row();

        lockModelPlaylist.lockForRead();

        items = m_viewModelPlaylist->takeRow(row);

        foreach(QStandardItem *item, items)
        {
            delete(item);
        }

        lockModelPlaylist.unlock();
    }
}


void MainWindow::on_pbRemoveMusic_clicked()
{
    try {

        QModelIndexList selectionPlaylist = ui->treeViewPlaylist->selectionModel()->selectedRows();
        if(selectionPlaylist.isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText("Remover música na playlist.");
            msgBox.setInformativeText("Selecione a música desejada para ser removida.");
            msgBox.setStandardButtons(QMessageBox::Close);
            msgBox.setDefaultButton(QMessageBox::Close);
            msgBox.exec();

            return;
        }

        // remove all tracks checked on the current playlist.
        lockModelPlaylist.lockForRead();

        int playlistRow = selectionPlaylist[0].row();
        QModelIndex index = m_viewModelPlaylist->index(playlistRow,0,QModelIndex());
        qDebug() << m_viewModelPlaylist->data(index).toString();


        QStandardItem *siPlaylist = m_viewModelPlaylist->itemFromIndex(index);

        int iTrackOnPlaylist = siPlaylist->rowCount();
        for( int iRowChild = iTrackOnPlaylist-1; iRowChild >= 0;--iRowChild )
        {
            QStandardItem *siChildID;
            siChildID = siPlaylist->child(iRowChild, TW_PL_NAME);
            qDebug() << siChildID->text();

            Qt::CheckState currentState = siChildID->checkState();
            if( currentState == Qt::CheckState::Checked )
            {
                QList<QStandardItem*> items ;
                items = siPlaylist->takeRow(iRowChild);

                foreach(QStandardItem *item, items)
                {
                    delete(item);
                }
            }
        }

        lockModelPlaylist.unlock();
    } catch (...) {
       qDebug() << "Exception on on_pbRemoveMusic_clicked.";
       lockModelPlaylist.unlock();
    }
}


void MainWindow::on_pbPlay_clicked()
{
    ui->pteStatus->clear();
    ui->pteStatus->appendPlainText("Carregando Música.");

    if(m_playerPreview->state() != QMediaPlayer::PausedState)
        m_playerPreview->stop();

    QModelIndexList selection = ui->treeViewPlaylist->selectionModel()->selectedRows();
    if(selection.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("Reproduzir preview das mpusicas.");
        msgBox.setInformativeText("Necessário selecione a playlist que deseja ouvir.");
        msgBox.setStandardButtons(QMessageBox::Close);
        msgBox.setDefaultButton(QMessageBox::Close);
        msgBox.exec();

        return;
    }

    m_playlistPreview = new QMediaPlaylist();

    QList<QStandardItem*> items ;
    int playlistRow = selection[0].row();

    lockModelPlaylist.lockForRead();

    QModelIndex index = m_viewModelPlaylist->index(playlistRow,0,QModelIndex());
    QStandardItem *siPlaylist = m_viewModelPlaylist->itemFromIndex(index);
    qDebug() << "siPlaylist: " << siPlaylist->text();

    int iTracksOnPlaylist = siPlaylist->rowCount();
    for( int iRowChild = 0; iRowChild < iTracksOnPlaylist;++iRowChild )
    {
        QStandardItem *siChildPreviewURL;
        siChildPreviewURL = siPlaylist->child(iRowChild, TW_PL_URL);
        qDebug() << siChildPreviewURL->text();

        m_playlistPreview->addMedia(QUrl(siChildPreviewURL->text()));
    }

    lockModelPlaylist.unlock();

    m_playerPreview->setPlaylist(m_playlistPreview);

    ui->pteStatus->appendPlainText("Iniciando preview.");
    m_playerPreview->play();

    ui->pbPause->setText("Pausar");
}

void MainWindow::on_pbPause_clicked()
{
    if(m_playerPreview->state() != QMediaPlayer::PausedState)
    {
        m_playerPreview->pause();

        ui->pbPause->setText("Continuar");
    }
    else
    {
        m_playerPreview->play();

        ui->pbPause->setText("Pausar");
    }
}

void MainWindow::on_pbPlaylistSave_clicked()
{
    /*  XML File format
        <PlayLists>
            <Playlist name="Nome">
                <Track id="jjjj" name="jjdsldsj" preview_url="ddd"/>
                <Track id="jttt" name="aaaaaa" preview_url="ddd"/>
                <Track id="yyy" name="bbbbbb" preview_url="ddd"/>
            </Playlist>
            <Playlist name="Nome2">
                <Track id="jjjj2" name="jjdsldsj" preview_url="ddd"/>
            </Playlist>
        </PlayLists>
     */

    try
    {
        // Open the saved playlists
        QFile file("Playlists.xml");
        if(!file.open(QIODevice::ReadWrite | QIODevice::Text) )
        {
            qDebug() << "Fail to open the Playlist.xml file.";
            ui->pteStatus->clear();
            ui->pteStatus->appendPlainText("Salvar o arquivo falhou. Não foi possível criar o arquivo para salvar a lista.");
        }
        else
        {
            QXmlStreamWriter xmlWriter(&file);
            xmlWriter.setAutoFormatting(true);
            xmlWriter.writeStartDocument();

            xmlWriter.writeStartElement("PlayLists");

            // Add  playlists
            lockModelPlaylist.lockForRead();

            int iPlaylistRows = m_viewModelPlaylist->rowCount();
            for( int iPlaylistRow = 0; iPlaylistRow < iPlaylistRows; ++iPlaylistRow)
            {
                QModelIndex indexPlaylist = m_viewModelPlaylist->index(iPlaylistRow,0,QModelIndex());
                QStandardItem *siPlaylist = m_viewModelPlaylist->itemFromIndex(indexPlaylist);
                qDebug() << "siPlaylist: " << siPlaylist->text();

                xmlWriter.writeStartElement("Playlist");
                xmlWriter.writeAttribute("name",siPlaylist->text() );

                // save all tracks
                int iTrackRows = siPlaylist->rowCount();
                for( int iTrackRow = 0; iTrackRow < iTrackRows; ++iTrackRow)
                {
                    QStandardItem *siTrackName;
                    QStandardItem *siTrackID;
                    QStandardItem *siTrackPreviewURL;

                    siTrackName = siPlaylist->child(iTrackRow, TW_PL_NAME);
                    qDebug() << "Track name: " << siTrackName->text();
                    siTrackID = siPlaylist->child(iTrackRow, TW_PL_ID);
                    qDebug() << "Track ID: " << siTrackID->text();
                    siTrackPreviewURL = siPlaylist->child(iTrackRow, TW_PL_URL);
                    qDebug() << "Track preview url: " << siTrackPreviewURL->text();


                    xmlWriter.writeStartElement("Track");

                    xmlWriter.writeAttribute("id", siTrackID->text());
                    xmlWriter.writeAttribute("name", siTrackName->text());
                    xmlWriter.writeAttribute("preview_url", siTrackPreviewURL->text());

                    xmlWriter.writeEndElement();

                }

                xmlWriter.writeEndElement();
            }

            xmlWriter.writeEndElement();

            file.close();

            qDebug() << "Playlist.xml saved with success.";
            ui->pteStatus->clear();
            ui->pteStatus->appendPlainText("Playlists salvas com sucesso.");

            lockModelPlaylist.unlock();
        }

    } catch (...) {
        qDebug() << "Fail to open the Playlist.xml file.";
        ui->pteStatus->clear();
        ui->pteStatus->appendPlainText("Salvar o arquivo falhou. Não foi possível criar o arquivo para salvar a lista.");

        lockModelPlaylist.unlock();
    }
}

void MainWindow::on_pbPlaylisttLoad_clicked()
{
    /*  XML File format
        <PlayLists>
            <Playlist name="Nome">
                <Track id="jjjj" name="jjdsldsj" preview_url="ddd"/>
                <Track id="jttt" name="aaaaaa" preview_url="ddd"/>
                <Track id="yyy" name="bbbbbb" preview_url="ddd"/>
            </Playlist>
            <Playlist name="Nome2">
                <Track id="jjjj2" name="jjdsldsj" preview_url="ddd"/>
            </Playlist>
        </PlayLists>
     */

    try
    {
        // Open the saved playlists
        QFile file("Playlists.xml");
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text) )
        {
            qDebug() << "Fail to open the Playlist.xml file.";
            ui->pteStatus->clear();
            ui->pteStatus->appendPlainText("Carregar playlist falhou. Não foi possível abrir o arquivo.");
        }
        else
        {
            restartPlaylist();

            QDomDocument doc;
            if(!doc.setContent(&file))
            {
                file.close();
                qDebug() << "Configuration file is no a valid XML.";
                ui->pteStatus->clear();
                ui->pteStatus->appendPlainText("Arquivo de playlist corrompido.");

                return;
            }

            lockModelPlaylist.lockForRead();

            QDomNodeList nodeListPlaylist = doc.elementsByTagName("Playlist");

            for (int iPlaylist = 0; iPlaylist < nodeListPlaylist.size(); iPlaylist++)
            {
                qDebug() <<nodeListPlaylist.item(0).nodeName();
                QDomNode nodePlaylist = nodeListPlaylist.item(iPlaylist);

                QString strPlayListName = nodePlaylist.attributes().namedItem("name").nodeValue();

                // add playlist on model                
                QStandardItem *siPlaylist = new QStandardItem(strPlayListName);
                m_viewModelPlaylist->setItem(iPlaylist, 0, siPlaylist);

                QDomNodeList nodeListTracks = nodePlaylist.childNodes();
                for (int iTrack = 0; iTrack < nodeListTracks.size(); iTrack++)
                {
                    QDomNode nodeTrack = nodeListTracks.item(iTrack);
                    QString strTrackID = nodeTrack.attributes().namedItem("id").nodeValue();
                    QString strTrackName = nodeTrack.attributes().namedItem("name").nodeValue();
                    QString strTrackURL = nodeTrack.attributes().namedItem("preview_url").nodeValue();


                    QStandardItem *siTrackID = new QStandardItem(strTrackID);
                    QStandardItem *siTrackName = new QStandardItem(strTrackName);
                    QStandardItem *siTrackPreviewURL = new QStandardItem(strTrackURL);

                    siTrackName->setCheckable(true);

                    siPlaylist->setChild(iTrack, TW_PL_NAME, siTrackName);
                    siPlaylist->setChild(iTrack, TW_PL_ID, siTrackID);
                    siPlaylist->setChild(iTrack, TW_PL_URL, siTrackPreviewURL);
                }
            }

            file.close();

            qDebug() << "Playlist.xml load with success.";
            ui->pteStatus->clear();
            ui->pteStatus->appendPlainText("Playlists carregada com sucesso.");

            lockModelPlaylist.unlock();
        }

    } catch (...) {
        qDebug() << "Fail to open the Playlist.xml file.";
        ui->pteStatus->clear();
        ui->pteStatus->appendPlainText("Carregar playlist falhou. Não foi possível abrir o arquivo.");

        lockModelPlaylist.unlock();
    }
}

void MainWindow::restartTrackSearch()
{
    lockModelSearch.lockForRead();

    QStringList strMusicHeadLables;
    strMusicHeadLables << "Musica" << "ID";
    m_viewModelSearch->clear();
    m_viewModelSearch->setHorizontalHeaderLabels(strMusicHeadLables);
    ui->twMusicSearchResults->resizeColumnsToContents();
    ui->twMusicSearchResults->hideColumn(2);

    lockModelSearch.unlock();
}

void MainWindow::restartPlaylist()
{
    lockModelPlaylist.lockForRead();

    QStringList strPlaylistHeadLables;
    strPlaylistHeadLables << "Nome" << "ID";
    m_viewModelPlaylist->clear();
    m_viewModelPlaylist->setHorizontalHeaderLabels(strPlaylistHeadLables);
    ui->treeViewPlaylist->hideColumn(2);

    lockModelPlaylist.unlock();
}
