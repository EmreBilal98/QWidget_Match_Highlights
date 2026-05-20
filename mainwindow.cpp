#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // MainWindow içinde örnek kullanım:
    videocutterwidget *cutterWidget = new videocutterwidget(this);
    ui->videoLayout->addWidget(cutterWidget);

    // DB'den "00:15:30" (15. dakika 30. saniye) verisi geldiğini varsayalım
    QString orjinalVideo = "/home/bilal/bilal/qt_projects/match_highlights/match_highlights/halisaha_maci.mp4";
    //QString dbZamani = "00:15:30";

    // İşlemi başlat (Arka planda kesip bitince widget içinde oynatacaktır)
    //cutterWidget->playMainVideo(orjinalVideo);

    cutterWidget->setSourceVideoPath(orjinalVideo);

    // 2. Veritabanından geliyormuş gibi sahte bir liste oluştur
    QList<VideoRecord> dbVerileri;
    dbVerileri.append(VideoRecord{"1. Kayıt", "00:05:00"});
    dbVerileri.append(VideoRecord{"2. Kayıt", "00:12:30"});
    dbVerileri.append(VideoRecord{"3. Kayıt", "00:22:15"});
    dbVerileri.append(VideoRecord{"4. Kayıt", "00:45:00"});

    // 3. Modeli tabloya yükle
    cutterWidget->loadRecordsFromDb(dbVerileri);
}

MainWindow::~MainWindow()
{
    delete ui;
}
