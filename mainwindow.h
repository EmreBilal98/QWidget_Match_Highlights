#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
    void getTimeStamps(QStringList datetimelist);

private:
    Ui::MainWindow *ui;
    QList<VideoRecord> dbVerileri;
    videocutterwidget *cutterWidget;
    Server *m_server;
};
#endif // MAINWINDOW_H
