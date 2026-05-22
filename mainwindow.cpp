#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    cutterWidget = new videocutterwidget(this);
    m_server  = new  Server(this);
    ui->videoLayout->addWidget(cutterWidget);

    connect(ui->search_button,&QPushButton::clicked,this,&MainWindow::searchClicked);
    connect(m_server,&Server::getTimeStamps,this,&MainWindow::getTimeStamps);
    connect(m_server,&Server::getVideoUrl,this,&MainWindow::getVideoUrl);
    ui->date_lineEdit->setInputMask("99/99/99;_");




}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::searchClicked()
{
    // DB'den "00:15:30" (15. dakika 30. saniye) verisi geldiğini varsayalım
    //QString orjinalVideo = "/home/bilal/bilal/qt_projects/match_highlights/match_highlights/halisaha_maci.mp4";
    //QString dbZamani = "00:15:30";

    // İşlemi başlat (Arka planda kesip bitince widget içinde oynatacaktır)
    //cutterWidget->playMainVideo(orjinalVideo);

    //cutterWidget->setSourceVideoPath(orjinalVideo);

    QDate tarihObjesi = QDate::fromString(ui->date_lineEdit->text(), "dd/MM/yy");

    if (!tarihObjesi.isValid()) {
        qWarning() << "Geçersiz tarih formatı!";
        return;
    }

    QString temizTarih = tarihObjesi.toString("d/M/yy");

    QString match_datetime = QString("%1 %2")
                                 .arg(temizTarih)
                                 .arg(ui->time_spinbox->value());

    qInfo() << match_datetime;
    m_server->serverRequest(1,ui->pitch_spinbox->value(),match_datetime);

}

void MainWindow::getTimeStamps(QStringList datetimelist)
{
    dbVerileri.clear();
    int i=0;
    foreach (QString datetime, datetimelist) {
        dbVerileri.append(VideoRecord{QString("%1. Kayıt").arg(++i), datetime});
    }

    cutterWidget->loadRecordsFromDb(dbVerileri);
}

void MainWindow::getVideoUrl(QString videoUrl)
{
    qInfo()<<"url:"<<videoUrl;
    cutterWidget->setSourceVideoPath(videoUrl);
}


