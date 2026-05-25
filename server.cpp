#include "server.h"


Server::Server(QObject *parent): QObject{parent}
{
    connect(&manager, &QNetworkAccessManager::finished, this, &Server::serverReply);
    connect(&login, &QNetworkAccessManager::finished, this, &Server::loginReply);
    connect(&signUp, &QNetworkAccessManager::finished, this, &Server::SignUpReply);
}

void Server::serverRequest(const int &user_id,const int &pitch_id,const QString timestamp)
{
    QUrl url(QString("http://127.0.0.1:8000/api/records/%1/%2").arg(user_id).arg(pitch_id));//url bul
    QUrlQuery query;
    query.addQueryItem("time_stamp", timestamp);
    url.setQuery(query);
    qInfo()<<url;
    QNetworkRequest request(url);

    qInfo()<<"token:"<<m_token;
    QByteArray bearerHeader = "Bearer " + m_token.toUtf8();
    request.setRawHeader("Authorization", bearerHeader);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    manager.get(request);
}

void Server::loginRequest(const QString username, const QString password)
{
    QUrl url(QString("http://127.0.0.1:8000/api/auth/login"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("accept", "application/json");
    qInfo()<<url;

    QJsonObject json;
    json["username"] =username;
    json["password"] =password;

    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson();

    login.post(request,jsonData);
}

void Server::SignUpRequest(const QString username, const QString email, const int pitch_count, const QString password)
{
    QUrl url(QString("http://127.0.0.1:8000/api/users"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("accept", "application/json");
    qInfo()<<url;

    QJsonObject json;
    json["username"] =username;
    json["email"] =email;
    json["pitch_count"] =QString::number(pitch_count);
    json["password"] =password;

    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson();

    signUp.post(request,jsonData);
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

                if (obj.contains("video_url") && obj["video_url"].isString()) {
                    m_videoUrl = obj["video_url"].toString();
                }
            }
        }

        if(!m_dateTimeList.isEmpty()){
            emit getTimeStamps(m_dateTimeList);
        }
        else{
            qWarning() << "bu saate ait kayıt yok!";
        }

        if(!m_videoUrl.isEmpty()){
            emit getVideoUrl(m_videoUrl);
        }
        else{
            qWarning() << "bu saate ait video kaydı yok!";
        }

    } else {
        qWarning() << "Hata:" << reply->errorString();
    }
    reply->deleteLater();
}

void Server::loginReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
        QJsonObject responseObj = responseDoc.object();

        m_token = responseObj["access_token"].toString();
        qDebug() << "Giriş Başarılı! Alınan Token:" << m_token;

        if(!m_token.isEmpty()){
            emit getToken(m_token);
        }
        else{
            qWarning() << "token yok!";
        }

    } else {
        qDebug() << "Giriş Başarısız! Hata kodu:" << reply->error();
        qDebug() << "Sunucu Yanıtı:" << reply->readAll();
    }
    reply->deleteLater();
}

void Server::SignUpReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
        QJsonObject responseObj = responseDoc.object();

        QString name = responseObj["username"].toString();

        if(!name.isEmpty()){
            emit getName(name);
        }
        else{
            qWarning() << "giriş başarısız isim yok!";
        }

    } else {
        qDebug() << "Giriş Başarısız! Hata kodu:" << reply->error();
        qDebug() << "Sunucu Yanıtı:" << reply->readAll();
    }
    reply->deleteLater();
}
