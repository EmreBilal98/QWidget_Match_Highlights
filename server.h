#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QStringList>
#include <QJsonArray>
#include <QJsonValue>
#include <QUrlQuery>

class Server:public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);

    void serverRequest(const int &user_id,const int &pitch_id,const QString timestamp);

private slots:
    void serverReply(QNetworkReply *reply);

signals:
    void getTimeStamps(QStringList datetimelist);

private:
    QNetworkAccessManager manager;
    QStringList m_dateTimeList;
};

#endif // SERVER_H
