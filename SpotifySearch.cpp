#include <QtDebug>
#include <QtNetworkAuth>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>
#include <QList>

#include "SpotifySearch.h"

SpotifySearch::SpotifySearch(QObject *parent) :
    QObject(parent)
{

}

SpotifySearch::~SpotifySearch()
{
}

bool SpotifySearch::SearchTracksByName(QOAuth2AuthorizationCodeFlow *spotify, const QString trackName)
{
    if( spotify == Q_NULLPTR)
    {
        qDebug ("SearchTracksByName received a Null pointer.");
        return false;
    }

    QString lTrackName = trackName;
    lTrackName.replace("\"", " ");
    lTrackName = "\"" + lTrackName + "\"";
    QUrl u ("https://api.spotify.com/v1/search?q=%" + lTrackName + "&type=track&limit=10&offset=0");

    auto reply = spotify->get(u);
    QObject::connect(reply, &QNetworkReply::finished,[=]()
    {
        if (reply->error() != QNetworkReply::NoError) {          
            return false;
        }

        const auto data = reply->readAll();

        // release buffer
        reply->deleteLater();

        const auto document = QJsonDocument::fromJson(data);
        const auto root = document.object();
        QString rootValue = root.value("id").toString();

        QVariantMap root_map = root.toVariantMap();
        QVariantMap tracks_map = root_map["tracks"].toMap();
        QVariantList track_list = tracks_map["items"].toList();

        for(int i =0; i< track_list.count();i++)
        {
            QVariantMap itemMap = track_list[i].toMap();
            QString ID  = itemMap["id"].toString();
            QString name = itemMap["name"].toString();
            QString previewUrl = itemMap["preview_url"].toString();

            emit newSearchTrack(ID, name, previewUrl);
        }

        return true;
    });

    return true;
}

