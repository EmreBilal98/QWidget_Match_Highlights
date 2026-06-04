#include "recorddialog.h"
#include <QDateTime>

RecordDialog::RecordDialog(QWidget *parent) : QDialog(parent)
{
    // Pencere Başlığı
    setWindowTitle("Yeni Kayıt Ekle");
    setMinimumWidth(300);

    // Giriş Alanları
    m_idInput = new QLineEdit(this);
    m_idInput->setPlaceholderText("Örn: 2. Kayıt");

    m_timeInput = new QLineEdit(this);
    // Kolaylık olsun diye anlık zamanı varsayılan olarak yazdıralım
    m_timeInput->setText(QDateTime::currentDateTime().toString("mm:ss"));

    // Form Düzeni (Etiket - Input yan yana dursun diye)
    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow("Kayıt ID:", m_idInput);
    formLayout->addRow("Zaman (Timestamp):", m_timeInput);

    // Butonlar
    m_saveButton = new QPushButton("Kaydet", this);
    m_cancelButton = new QPushButton("İptal", this);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_cancelButton);

    // Ana Dikey Layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    // 🌟 SİHİRLİ BAĞLANTI: QDialog'un yerleşik accept ve reject mekanizması
    connect(m_saveButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}
