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
    connect(m_server,&Server::getId,this,&MainWindow::getId);
    ui->date_lineEdit->setInputMask("99/99/99;_");


    connect(ui->date_lineEdit,&QLineEdit::textChanged,this,&MainWindow::makeDate);
    connect(ui->time_spinbox,&QSpinBox::valueChanged,this,&MainWindow::makeDate);
    connect(this,&MainWindow::getDate,cutterWidget,&videocutterwidget::getDate);


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
    m_server->serverRequest(m_id,ui->pitch_spinbox->value(),match_datetime);//güncellenecek

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

void MainWindow::getTimeStamps(QStringList datetimelist,QList<int> teamIDList)
{
    dbVerileri.clear();
    int i=0;
    foreach (QString datetime, datetimelist) {
        dbVerileri.append(VideoRecord{QString("%1. Takım").arg(teamIDList.at(i)),QString("%1. Kayıt").arg(++i), datetime});
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

    m_id=token.split("_").at(3).toInt();

    qDebug() << "Ayıklanan User ID:" << m_id; // Çıktı: 14
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

void MainWindow::getTeamID(int tID)
{

}

void MainWindow::getId(int id,int pitchCount)
{
    QString basePath = "/var/www/matchrecord/matches/user_"+QString::number(id)+"/";
    qInfo()<<basePath;

    QDir dir;
    if (!dir.exists(basePath)) {
        dir.mkpath(basePath);
    }

    for(int i=0;i<pitchCount;i++){

        QString pitchPath=basePath+"pitch_"+QString::number(i+1)+"/";
        if (!dir.exists(pitchPath)) {
            dir.mkpath(pitchPath);
        }

        QString trimPath=pitchPath+"trim/";
        if (!dir.exists(trimPath)) {
            dir.mkpath(trimPath);
        }

        qDebug() << "Klasör zinciri başarıyla oluşturuldu:" << trimPath;
    }

}

void MainWindow::makeDate()
{
    emit getDate(ui->date_lineEdit->text()+" "+QString::number(ui->time_spinbox->value()));
}


