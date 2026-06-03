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
#include <QLabel>

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

    // Videoyu kesmeden sadece belirli aralıkta oynatacak fonksiyon
    void playVideoSegment(int startSecond);

private slots:
    void onCutButtonClicked();
    void onMainVideoButtonClicked();
    void onFFmpegFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handlePositionChanged(qint64 position);
    void onTableBlockClicked(int row, int column);

private:
    // UI Elemanları
    QVideoWidget *m_videoWidget;
    QMediaPlayer *m_mediaPlayer;
    QAudioOutput *m_audioOutput;

    QTableWidget *m_tableWidget;
    QPushButton *m_mainVideoButton;
    QPushButton *m_cutButton;
    QSlider *m_videoSlider;
    QLabel *m_currentTimeLabel;
    QLabel *m_totalTimeLabel;

    QString formatTime(qint64 milliseconds);

    // İşlem Elemanları
    QProcess *m_ffmpegProcess;
    QString m_outputPath;
    QString m_sourceVideoPath;

    // İşaretlenen videoları sırayla işlemek için kuyruk
    QQueue<VideoRecord> m_processingQueue;

    // Yardımcı Fonksiyon
    void processNextInQueue();
    QString calculateStartTime(const QString &dbTimestamp);

    quint64 m_endPositionMs;
    quint64 m_startPositionMs;
};

#endif // VIDEOCUTTERWIDGET_H
