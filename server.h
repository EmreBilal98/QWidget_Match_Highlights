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
#include <QMessageBox>

class Server:public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);

    void serverRequest(const int &user_id,const int &pitch_id,const QString timestamp);
    void loginRequest(const QString username,const QString password);
    void SignUpRequest(const QString username,const QString email,const int pitch_count, const QString password);

private slots:
    void serverReply(QNetworkReply *reply);
    void loginReply(QNetworkReply *reply);
    void SignUpReply(QNetworkReply *reply);

signals:
    void getTimeStamps(QStringList datetimelist,QList<int> teamIDList);
    void getVideoUrl(QString videoUrl);
    void getToken(QString token);
    void getName(QString name);
    void getTeamID(int tID);
    void getId(int id,int pitchCount);

private:
    QNetworkAccessManager manager,login,signUp;
    QStringList m_dateTimeList;
    QList<int> m_teamIDList;
    QString m_videoUrl;
    QString m_token;
};

#endif // SERVER_H
