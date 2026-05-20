#ifndef VIDEOCUTTERWIDGET_H
#define VIDEOCUTTERWIDGET_H

#include <QObject>
#include <QWidget>
#include <QProcess>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTime>
#include <QTableWidget>
#include <QPushButton>
#include <QQueue>

// Her bir DB kaydını temsil eden yapı
struct VideoRecord {
    QString id;
    QString timestamp;
};

class videocutterwidget : public QWidget
{
    Q_OBJECT
public:
    explicit videocutterwidget(QWidget *parent = nullptr);
    ~videocutterwidget();

    // Dışarıdan tetiklenecek fonksiyon (DB'den veri geldiğinde çağrılır)
    void processVideoFromDb(const QString &videoPath, const QString &dbTimestamp);

    // Veritabanından gelen verileri tabloya yüklemek için fonksiyon
    void loadRecordsFromDb(const QList<VideoRecord> &records);

    void setSourceVideoPath(const QString &path) { m_sourceVideoPath = path;playMainVideo(m_sourceVideoPath); }

    //tam maç videosunu oynatmak için kullanılır
    void playMainVideo(QString Path);

private slots:
    void onCutButtonClicked();
    void onFFmpegFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    // UI Elemanları
    QVideoWidget *m_videoWidget;
    QMediaPlayer *m_mediaPlayer;
    QAudioOutput *m_audioOutput;

    QTableWidget *m_tableWidget;
    QPushButton *m_cutButton;

    // İşlem Elemanları
    QProcess *m_ffmpegProcess;
    QString m_outputPath;
    QString m_sourceVideoPath;

    // İşaretlenen videoları sırayla işlemek için kuyruk
    QQueue<VideoRecord> m_processingQueue;

    // Yardımcı Fonksiyon
    void processNextInQueue();
    QString calculateStartTime(const QString &dbTimestamp);
};

#endif // VIDEOCUTTERWIDGET_H
