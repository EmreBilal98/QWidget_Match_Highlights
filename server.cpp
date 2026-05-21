#include "server.h"


Server::Server(QObject *parent): QObject{parent}
{
    connect(&manager, &QNetworkAccessManager::finished, this, &Server::serverReply);
}

void Server::serverRequest(const int &user_id,const int &pitch_id,const QString timestamp)
{
    QUrl url(QString("http://127.0.0.1:8000/api/records/%1/%2").arg(user_id).arg(pitch_id));//url bul
    QUrlQuery query;
    query.addQueryItem("time_stamp", timestamp);
    url.setQuery(query);
    qInfo()<<url;
    QNetworkRequest request(url);
    manager.get(request);
}

void Server::serverReply(QNetworkReply *reply)
{
    m_dateTimeList.clear();
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

        if (error.error != QJsonParseError::NoError) {
            qWarning() << "JSON parse hatası:" << error.errorString();
            reply->deleteLater();
            return;
        }


        if (jsonDoc.isArray()) {
            QJsonArray jsonArray = jsonDoc.array();

            for (const QJsonValue &value : jsonArray) {
                // Her bir elemanı objeye çevir
                QJsonObject obj = value.toObject();

                // "datetime_from_st" anahtarını kontrol et ve listeye ekle
                if (obj.contains("datetime_from_st") && obj["datetime_from_st"].isString()) {
                    QString timestamp = obj["datetime_from_st"].toString();
                    m_dateTimeList.append(timestamp);
                }
            }
        }

        if(!m_dateTimeList.isEmpty()){
            emit getTimeStamps(m_dateTimeList);
        }
        else{
            qWarning() << "bu saate ait kayıt yok!";
        }

    } else {
        qWarning() << "Hata:" << reply->errorString();
    }
    reply->deleteLater();
}
