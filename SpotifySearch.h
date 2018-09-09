#ifndef SPOTIFY_SEARCH_H
#define SPOTIFY_SEARCH_H

#include <QOAuth2AuthorizationCodeFlow>
#include <QList>
#include <QReadWriteLock>


class SpotifySearch : public QObject
{
    Q_OBJECT
public:

    explicit SpotifySearch(QObject *parent = nullptr);
    ~SpotifySearch();

    bool SearchTracksByName(QOAuth2AuthorizationCodeFlow *spotify, const QString trackName);

private:  

private:     


public slots:


signals:
     void newSearchTrack(QString trackID, QString trackName, QString strPreviewURL);

};


#endif // SPOTIFY_SEARCH_H
