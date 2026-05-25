#include "mainwindow.h"
#include "ui_mainwindow.h"

#define RecordPage 0
#define LoginPage  1
#define SignUpPage 2

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    cutterWidget = new videocutterwidget(this);
    m_server  = new  Server(this);
    ui->videoLayout->addWidget(cutterWidget);

    connect(ui->search_button,&QPushButton::clicked,this,&MainWindow::searchClicked);
    connect(ui->btnLogIn,&QPushButton::clicked,this,&MainWindow::loginClicked);
    connect(ui->btnSignUp,&QPushButton::clicked,this,&MainWindow::SignUpClicked);
    connect(ui->btnSuSignUp,&QPushButton::clicked,this,&MainWindow::suSignUpClicked);
    connect(ui->btnsuLogIn,&QPushButton::clicked,this,&MainWindow::suLoginClicked);
    connect(m_server,&Server::getTimeStamps,this,&MainWindow::getTimeStamps);
    connect(m_server,&Server::getVideoUrl,this,&MainWindow::getVideoUrl);
    connect(m_server,&Server::getToken,this,&MainWindow::getToken);
    connect(m_server,&Server::getName,this,&MainWindow::getName);
    ui->date_lineEdit->setInputMask("99/99/99;_");




}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::searchClicked()
{
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

void MainWindow::loginClicked()
{
    m_server->loginRequest(ui->lineUsername->text(),ui->linePassword->text());
}

void MainWindow::SignUpClicked()
{
    ui->stackedWidget->setCurrentIndex(SignUpPage);
}

void MainWindow::suLoginClicked()
{
    ui->stackedWidget->setCurrentIndex(LoginPage);
}

void MainWindow::suSignUpClicked()
{
    if(ui->lineEdit_suPass->text() == ui->lineEdit_suPassVerify->text()){
        m_server->SignUpRequest(ui->lineEdit_suUsername->text(),ui->lineEdit_suEmail->text(),
                                ui->spinBox_pCount->value(),ui->lineEdit_suPass->text());
    }
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

void MainWindow::getToken(QString token)
{
    m_token=token;
    qInfo()<<"giriş başarılı";
    ui->stackedWidget->setCurrentIndex(RecordPage);
}

void MainWindow::getName(QString name)
{
    qInfo()<<"isim"<<name;
    if(name==ui->lineEdit_suUsername->text()){
        qInfo()<<"kayıt başarılı";
        ui->stackedWidget->setCurrentIndex(LoginPage);
    }
}


