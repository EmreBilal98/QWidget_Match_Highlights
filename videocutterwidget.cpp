#include "videocutterwidget.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QHeaderView>

#define TIMESTAMP_COL 2
#define VIDEO_DURATION 4


videocutterwidget::videocutterwidget(QWidget *parent): QWidget(parent)
{

    // --- Yeni Grafik Sahnesi Oynatıcı Kurulumu ---
    m_graphicsScene = new QGraphicsScene(this);
    m_graphicsView = new QGraphicsView(m_graphicsScene, this);
    m_videoItem = new QGraphicsVideoItem();

    m_graphicsScene->addItem(m_videoItem);

    // Arka planı simsiyah mühürle ve çerçeveyi kaldır
    m_graphicsView->setBackgroundBrush(Qt::black);
    m_graphicsView->setFrameShape(QFrame::NoFrame);
    m_graphicsView->setMinimumHeight(300);

    // Oynatıcı çıkışlarını ve ses kartını bağla
    m_mediaPlayer = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);

    m_mediaPlayer->setAudioOutput(m_audioOutput);
    m_mediaPlayer->setVideoOutput(m_videoItem); // Çıkış artık videoItem!

    // Ekran en boy oranını pencereye göre esnetmek istiyorsan (IgnoreAspectRatio alternatifi):
    m_videoItem->setAspectRatioMode(Qt::IgnoreAspectRatio);

    // Tablo Kurulumu
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(3);
    m_tableWidget->setHorizontalHeaderLabels(QStringList() << "Seç" << "Kayıt ID" << "Zaman");
    m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // main video Buton Kurulumu
    m_mainVideoButton = new QPushButton("back to full match", this);
    connect(m_mainVideoButton, &QPushButton::clicked, this, &videocutterwidget::onMainVideoButtonClicked);

    // cut Buton Kurulumu
    m_cutButton = new QPushButton("Cut and save the choosens", this);
    connect(m_cutButton, &QPushButton::clicked, this, &videocutterwidget::onCutButtonClicked);

    //play pause buton kurulumu
    m_playPauseButton=new QPushButton("Pause", this);
    connect(m_playPauseButton, &QPushButton::clicked, this, &videocutterwidget::onPlayPauseButtonClicked);

    //kayıt ekleme buton kurulumu
    m_addRecordButton=new QPushButton("Add new record", this);
    connect(m_addRecordButton, &QPushButton::clicked, this, &videocutterwidget::onAddRecordButtonClicked);
    m_addRecordButton->setEnabled(false);
    //slider kurulumu
    m_videoSlider=new QSlider(Qt::Horizontal,this);

    m_currentTimeLabel = new QLabel("00:00", this);
    m_totalTimeLabel = new QLabel("00:00", this);

    m_currentTimeLabel->setAlignment(Qt::AlignCenter);
    m_totalTimeLabel->setAlignment(Qt::AlignCenter);
    m_currentTimeLabel->setMinimumWidth(50);
    m_totalTimeLabel->setMinimumWidth(50);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_mainVideoButton);
    buttonLayout->addWidget(m_addRecordButton);

    QHBoxLayout *sliderLayout = new QHBoxLayout();
    sliderLayout->addWidget(m_playPauseButton);
    sliderLayout->addWidget(m_currentTimeLabel); // En solda anlık süre
    sliderLayout->addWidget(m_videoSlider);       // Ortada  slider
    sliderLayout->addWidget(m_totalTimeLabel);     // En sağda toplam süre



    // 1. Toplam süre değiştiğinde slider aralığını güncelle
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, [this](qint64 duration) {
        m_videoSlider->setRange(0, duration);
        m_totalTimeLabel->setText(formatTime(duration));
    });


    connect(m_videoSlider, &QSlider::sliderMoved, this, [this](int position) {
        m_endPositionMs = 0;
        m_currentTimeLabel->setText(formatTime(position)); // Sürüklerken sol etiket canlansın
    });

    // 2. Video oynarken slider'ı yürüt (Sadece kullanıcı slider'ı TUTMUYORSA)
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
        if (!m_videoSlider->isSliderDown()) {
            // Sinyal çakışmasını engellemek için slider'ın sinyallerini geçici olarak engelliyoruz
            m_videoSlider->blockSignals(true);
            m_videoSlider->setValue(position);
            m_videoSlider->blockSignals(false);
        }

        m_currentTimeLabel->setText(formatTime(position));
    });

    // Bu yöntem Nginx'e saniyede 100 tane istek atmak yerine, sadece tek bir kesin istek atar ve donmayı önler.
    connect(m_videoSlider, &QSlider::sliderReleased, this, [this]() {
        qint64 targetPos = m_videoSlider->value();
        qInfo() << "Kullanıcı slider'ı bıraktı, zıplanıyor:" << targetPos;
        m_mediaPlayer->setPosition(targetPos);
    });

    // Layout Kurulumu
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_graphicsView);
    layout->addLayout(sliderLayout);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_tableWidget);
    layout->addWidget(m_cutButton);

    m_graphicsView->setMinimumHeight(300); // Kafana göre değiştirebilirsin (örn: 250 veya 300)

    layout->setStretchFactor(m_graphicsView, 3); // Video dikey alanın %60'ını kaplasın
    layout->setStretchFactor(sliderLayout, 0);
    layout->setStretchFactor(m_mainVideoButton, 0);   // Buton sadece kendi boyutu kadar yer kaplasın, hiç esnemesin
    layout->setStretchFactor(m_tableWidget, 2); // Tablo dikey alanın %40'ını kaplasın
    layout->setStretchFactor(m_cutButton, 0);   // Buton sadece kendi boyutu kadar yer kaplasın, hiç esnemesin
    setLayout(layout);

    // FFmpeg Süreç Kurulumu
    m_ffmpegProcess = new QProcess(this);
    connect(m_ffmpegProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onFFmpegFinished(int, QProcess::ExitStatus)));

    connect(m_tableWidget, &QTableWidget::cellClicked, this, &videocutterwidget::onTableBlockClicked);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &videocutterwidget::handlePositionChanged);

    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        // Media yüklendiğinde veya tampon bellek dolduğunda (BufferedMedia / LoadedMedia)
        if (status == QMediaPlayer::BufferedMedia || status == QMediaPlayer::LoadedMedia) {
            if (m_startPositionMs > 0) {
                qInfo() << "Video hazır! Hedef saniyeye zıplanıyor:" << m_startPositionMs;
                m_mediaPlayer->setPosition(m_startPositionMs);

                // Tekrar tekrar tetiklenmesin diye hedefi sıfırlıyoruz
                //m_startPositionMs = 0;
            }
        }
    });


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

void videocutterwidget::handlePositionChanged(qint64 position)
{
    if (position >= m_endPositionMs && m_endPositionMs!=0) {
        m_mediaPlayer->pause();
        qDebug() << "Hedef süreye ulaşıldı, video durduruldu.";
    }
}

void videocutterwidget::onTableBlockClicked(int row, int column)
{

    QTableWidgetItem *item = m_tableWidget->item(row, TIMESTAMP_COL);

    if (!item) {
        qWarning() << "Tıklanan satırda zaman bulunamadı!";
        return;
    }

    QString modified_time = calculateStartTime(item->text());
    qInfo() << "Seçilen blok oynatılıyor:" << modified_time;

    QStringList times=modified_time.split(":");
    int time_second=(times.at(1).toInt()*60)+times.at(2).toInt();
    qInfo() <<time_second;

    playVideoSegment(time_second);
}

void videocutterwidget::getDate(QString date)
{
    m_currentdate=date;
}

QString videocutterwidget::formatTime(qint64 milliseconds)
{
    qint64 totalSeconds = milliseconds / 1000;
    int seconds = totalSeconds % 60;
    int minutes = (totalSeconds / 60) % 60;
    int hours = totalSeconds / 3600;

    // Sayıların başına sıfır eklemek için QString::arg kullanıyoruz (Örn: 5 saniye -> "05")
    QString timeString;
    if (hours > 0) {
        // Eğer video 1 saatten uzunsa saati de gösterir: "01:23:45"
        timeString = QString("%1:%2:%3")
                         .arg(hours, 2, 10, QChar('0'))
                         .arg(minutes, 2, 10, QChar('0'))
                         .arg(seconds, 2, 10, QChar('0'));
    } else {
        // 1 saatten kısaysa sadece dakika ve saniye: "23:45"
        timeString = QString("%1:%2")
                         .arg(minutes, 2, 10, QChar('0'))
                         .arg(seconds, 2, 10, QChar('0'));
    }
    return timeString;
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
    qInfo()<<"tiimestamp::"<<currentRecord.timestamp;
    QString startTime = calculateStartTime(currentRecord.timestamp);


    qInfo()<<"mmy path:"<<m_sourceVideoPath;
    QString cleanId = currentRecord.id;
    cleanId.replace(" ", "_").replace(".", "");

    qInfo()<<cleanId<<"-"<<currentRecord.timestamp;
    QDateTime dateTime = QDateTime::fromString(currentRecord.timestamp, "d/M/yy hh:mm:ss");
    if (!dateTime.isValid()) {
        qWarning() << "Hatalı zaman formatı:" << currentRecord.timestamp;
    }
    QString newFormat = dateTime.toString("ddMMyy_hhmmss");

    QString Path=m_sourceVideoPath.replace("http://127.0.0.1:8085","/var/www/matchrecord");
    qInfo()<<"path:"<<Path;
    QFileInfo pathInfo(Path);
    QString outputPath = pathInfo.absolutePath()+"/trim/" + cleanId +"_"+ newFormat +"_kesilmis.mp4";
    qInfo()<<"outputpath:"<<outputPath;

    QStringList arguments;
    arguments << "-ss" << startTime
              << "-i"  << m_sourceVideoPath
              << "-t"  << "00:0"+QString::number(VIDEO_DURATION)+":00"
              << "-c"  << "copy"
              << "-y"
              << outputPath;

    qDebug()<<"ffmpeg komut:"<<arguments;
    qDebug() << currentRecord.id << "işleniyor... Başlangıç:" << startTime;
    m_ffmpegProcess->start("ffmpeg", arguments);
}

QString videocutterwidget::calculateStartTime(const QString &dbTimestamp)
{
    QDateTime targetDateTime = QDateTime::fromString(dbTimestamp, "d/M/yy hh:mm:ss");
    if (!targetDateTime.isValid()) return "00:00:00";

    QTime targetTime = targetDateTime.time();

    // 2 dakika (120 saniye) geriye git
    QTime startTime = targetTime.addSecs(-120);

    // Eğer video başından daha geriye düşerse 00:00:00 yap
    if (targetTime.hour() == 0 && targetTime.minute() < 2) {
        startTime = QTime(0, 0, 0);
    }

    return startTime.toString("0:mm:ss");
}

void videocutterwidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);


    if (m_graphicsScene && m_graphicsView && m_videoItem) {

        m_graphicsScene->setSceneRect(m_graphicsView->rect());


        m_videoItem->setSize(m_graphicsView->size());
    }
}

void videocutterwidget::playMainVideo(QString Path){

    // Kesilen videoyu QVideoWidget alanında oynat
    m_mediaPlayer->setSource(QUrl::fromLocalFile(Path));
    m_mediaPlayer->play();
    m_endPositionMs=0;
    m_startPositionMs=0;
    m_addRecordButton->setEnabled(true);


}

void videocutterwidget::playVideoSegment(int startSecond)
{
    // Qt milisaniye (ms) çalıştığı için saniyeleri 1000 ile çarpıyoruz
    m_startPositionMs = startSecond * 1000;
    m_endPositionMs = (startSecond+(VIDEO_DURATION*60)) * 1000;

    if (m_mediaPlayer->source() == QUrl(m_sourceVideoPath)) {
        m_mediaPlayer->setPosition(m_startPositionMs);
        m_mediaPlayer->play();
        m_playPauseButton->setText("Pause");
    } else {
        m_mediaPlayer->setSource(QUrl(m_sourceVideoPath));
        m_mediaPlayer->play();
        m_playPauseButton->setText("Pause");
    }
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

void videocutterwidget::onPlayPauseButtonClicked()
{
    if(m_playPauseButton->text()=="Pause"){
        m_playPauseButton->setText("Play");
        m_mediaPlayer->pause();
    }else if(m_playPauseButton->text()=="Play"){
        m_playPauseButton->setText("Pause");
        m_mediaPlayer->play();
    }
}

void videocutterwidget::onMainVideoButtonClicked()
{
    playMainVideo(m_sourceVideoPath);
}

void videocutterwidget::onAddRecordButtonClicked()
{
    // 1. Dialog nesnesini oluşturuyoruz
    RecordDialog dialog(this);

    // 2. Dialogu "Modal" (Ana ekranı kilitleyecek şekilde) açıyoruz
    // exec() fonksiyonu, kullanıcı Kaydet veya İptal diyene kadar kodu burada bekletir.
    if (dialog.exec() == QDialog::Accepted) {
        // Kullanıcı "Kaydet" butonuna bastıysa buraya girer
        QString newId = dialog.getId();
        QString newTimestamp = dialog.getTimestamp();

        if (newId.isEmpty() || newTimestamp.isEmpty()) {
            QMessageBox::warning(this, "Hata", "Alanlar boş bırakılamaz!");
            return;
        }

        // 3. Mevcut Tabloya (`m_tableWidget`) Yeni Satır Ekleme Mantığı
        int currentRowCount = m_tableWidget->rowCount();
        m_tableWidget->insertRow(currentRowCount);

        // Sütun 0: Checkbox (Kesme işlemine dahil edilebilsin diye varsayılan Unchecked yapıyoruz)
        QTableWidgetItem *checkItem = new QTableWidgetItem();
        checkItem->setCheckState(Qt::Unchecked);
        m_tableWidget->setItem(currentRowCount, 0, checkItem);

        // Sütun 1: Kayıt ID
        QTableWidgetItem *idItem = new QTableWidgetItem(newId);
        idItem->setFlags(idItem->flags() ^ Qt::ItemIsEditable); // Düzenlenemez yap
        m_tableWidget->setItem(currentRowCount, 1, idItem);

        // Sütun 2: Zaman
        QTableWidgetItem *timeItem = new QTableWidgetItem(m_currentdate+":"+newTimestamp);
        timeItem->setFlags(timeItem->flags() ^ Qt::ItemIsEditable);
        m_tableWidget->setItem(currentRowCount, 2, timeItem);

        qInfo() << "Tabloya elle yeni kayıt eklendi:" << newId << "-" << newTimestamp;
    } else {
        qInfo() << "Kullanıcı yeni kayıt ekleme işlemini iptal etti.";
    }
}
