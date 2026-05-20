#include "videocutterwidget.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QHeaderView>


videocutterwidget::videocutterwidget(QWidget *parent): QWidget(parent)
{
    // Oyuncu ve Video Alanı Kurulumu
    m_mediaPlayer = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_videoWidget = new QVideoWidget(this);

    m_mediaPlayer->setAudioOutput(m_audioOutput);
    m_mediaPlayer->setVideoOutput(m_videoWidget);

    // Tablo Kurulumu
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(3);
    m_tableWidget->setHorizontalHeaderLabels(QStringList() << "Seç" << "Kayıt ID" << "Zaman");
    m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 2. Buton Kurulumu
    m_cutButton = new QPushButton("Seçilenleri Kes ve Kaydet", this);
    connect(m_cutButton, &QPushButton::clicked, this, &videocutterwidget::onCutButtonClicked);

    // Layout Kurulumu
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_videoWidget);
    layout->addWidget(m_tableWidget);
    layout->addWidget(m_cutButton);

    // 3. Sigorta: Videonun tamamen kaybolmasını engellemek için minimum bir yükseklik ver
    m_videoWidget->setMinimumHeight(300); // Kafana göre değiştirebilirsin (örn: 250 veya 300)

    layout->setStretchFactor(m_videoWidget, 3); // Video dikey alanın %60'ını kaplasın
    layout->setStretchFactor(m_tableWidget, 2); // Tablo dikey alanın %40'ını kaplasın
    layout->setStretchFactor(m_cutButton, 0);   // Buton sadece kendi boyutu kadar yer kaplasın, hiç esnemesin
    setLayout(layout);

    // FFmpeg Süreç Kurulumu
    m_ffmpegProcess = new QProcess(this);
    connect(m_ffmpegProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onFFmpegFinished(int, QProcess::ExitStatus)));
}

videocutterwidget::~videocutterwidget(){}

void videocutterwidget::processVideoFromDb(const QString &videoPath, const QString &dbTimestamp)
{
    QString startTime = calculateStartTime(dbTimestamp);

    // Çıktı dosyası adı oluştur (Örn: kesilmis_video.mp4)
    QFileInfo fileInfo(videoPath);
    m_outputPath = fileInfo.absolutePath() + "/kesilmis_" + fileInfo.fileName();

    // FFmpeg Parametreleri
    // Komut mantığı: ffmpeg -ss [baslangic] -i [girdi] -t 00:04:00 -c copy -y [cikti]
    QStringList arguments;
    arguments << "-ss" << startTime          // Başlangıç zamanı (-2 dk)
              << "-i"  << videoPath          // Orijinal video yolu
              << "-t"  << "00:04:00"         // Süre: 4 dakika sabit (2 dk önce + 2 dk sonra)
              << "-c"  << "copy"             // Videoyu yeniden işleme, direkt kopyala (Çok hızlıdır!)
              << "-y"                        // Eğer dosya varsa üzerine yaz
              << m_outputPath;               // Yeni videonun kaydedileceği yer

    qDebug() << "FFmpeg başlatılıyor... Başlangıç:" << startTime;

    // FFmpeg'i çalıştır (Sisteminizde FFmpeg kurulu ve PATH'e ekli olmalıdır)
    m_ffmpegProcess->start("ffmpeg", arguments);
}

void videocutterwidget::loadRecordsFromDb(const QList<VideoRecord> &records)
{
    m_tableWidget->setRowCount(0); // Eski verileri temizle

    for (int i = 0; i < records.size(); ++i) {
        m_tableWidget->insertRow(i);

        // Sütun 0: Checkbox
        QTableWidgetItem *checkItem = new QTableWidgetItem();
        checkItem->setCheckState(Qt::Unchecked); // Varsayılan olarak seçili değil
        m_tableWidget->setItem(i, 0, checkItem);

        // Sütun 1: Kayıt IDsi
        QTableWidgetItem *idItem = new QTableWidgetItem(records[i].id);
        idItem->setFlags(idItem->flags() ^ Qt::ItemIsEditable); // Düzenlenemez yap
        m_tableWidget->setItem(i, 1, idItem);

        // Sütun 2: Zaman
        QTableWidgetItem *timeItem = new QTableWidgetItem(records[i].timestamp);
        timeItem->setFlags(timeItem->flags() ^ Qt::ItemIsEditable);
        m_tableWidget->setItem(i, 2, timeItem);
    }
}

void videocutterwidget::onFFmpegFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        qDebug() << "Video başarıyla kesildi! Oynatılıyor:" << m_outputPath;


        // Bir sonraki videoya geç (Kuyruk boşalana kadar kendi kendini çağırır)
        processNextInQueue();
        // Kesilen videoyu QVideoWidget alanında oynat
        //m_mediaPlayer->setSource(QUrl::fromLocalFile(m_outputPath));
        //m_mediaPlayer->play();
    } else {
        qDebug() << "FFmpeg hatası oluştu! Hata kodu:" << m_ffmpegProcess->readAllStandardError();
    }
}

void videocutterwidget::processNextInQueue()
{
    if (m_processingQueue.isEmpty()) {
        m_cutButton->setEnabled(true);
        QMessageBox::information(this, "Başarılı", "Seçilen tüm videolar başarıyla kesildi ve kaydedildi!");
        return;
    }

    // Sıradaki kaydı al (Kuyruktan çıkar)
    VideoRecord currentRecord = m_processingQueue.dequeue();
    QString startTime = calculateStartTime(currentRecord.timestamp);

    // Çıktı dosya adı formatı: örn. "Kayit_1_kesilmis.mp4"
    QFileInfo fileInfo(m_sourceVideoPath);
    QString cleanId = currentRecord.id;
    cleanId.replace(" ", "_").replace(".", "");
    QString outputPath = fileInfo.absolutePath() + "/" + cleanId + "_kesilmis.mp4";

    QStringList arguments;
    arguments << "-ss" << startTime
              << "-i"  << m_sourceVideoPath
              << "-t"  << "00:04:00"
              << "-c"  << "copy"
              << "-y"
              << outputPath;

    qDebug() << currentRecord.id << "işleniyor... Başlangıç:" << startTime;
    m_ffmpegProcess->start("ffmpeg", arguments);
}

QString videocutterwidget::calculateStartTime(const QString &dbTimestamp)
{
    QTime targetTime = QTime::fromString(dbTimestamp, "hh:mm:ss");
    if (!targetTime.isValid()) return "00:00:00";

    // 2 dakika (120 saniye) geriye git
    QTime startTime = targetTime.addSecs(-120);

    // Eğer video başından daha geriye düşerse 00:00:00 yap
    if (targetTime.hour() == 0 && targetTime.minute() < 2) {
        startTime = QTime(0, 0, 0);
    }

    return startTime.toString("hh:mm:ss");
}

void videocutterwidget::playMainVideo(QString Path){

    // Kesilen videoyu QVideoWidget alanında oynat
    m_mediaPlayer->setSource(QUrl::fromLocalFile(Path));
    m_mediaPlayer->play();
}

void videocutterwidget::onCutButtonClicked()
{
    if (m_sourceVideoPath.isEmpty()) {
        QMessageBox::warning(this, "Hata", "Lütfen önce kaynak bir video belirleyin!");
        return;
    }

    m_processingQueue.clear();

    // Tabloyu tara ve "Checked" olanları kuyruğa ekle
    for (int i = 0; i < m_tableWidget->rowCount(); ++i) {
        if (m_tableWidget->item(i, 0)->checkState() == Qt::Checked) {
            VideoRecord record;
            record.id = m_tableWidget->item(i, 1)->text();
            record.timestamp = m_tableWidget->item(i, 2)->text();
            m_processingQueue.enqueue(record);
        }
    }

    if (m_processingQueue.isEmpty()) {
        QMessageBox::information(this, "Bilgi", "Lütfen kesmek istediğiniz kayıtları seçin.");
        return;
    }

    // Butonu işlem bitene kadar devre dışı bırakalım
    m_cutButton->setEnabled(false);

    // İlk videoyu kesmeye başla
    processNextInQueue();
}
