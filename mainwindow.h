#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include "videocutterwidget.h"
#include "server.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void searchClicked();
    void loginClicked();
    void SignUpClicked();
    void suLoginClicked();
    void suSignUpClicked();
    void getTimeStamps(QStringList datetimelist);
    void getVideoUrl(QString videoUrl);
    void getToken(QString token);
    void getName(QString name);
    void getId(int id,int pitchCount);

private:
    Ui::MainWindow *ui;
    QList<VideoRecord> dbVerileri;
    videocutterwidget *cutterWidget;
    Server *m_server;
    QString m_token;
    int m_id;
};
#endif // MAINWINDOW_H
