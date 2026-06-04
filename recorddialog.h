#ifndef RECORDDIALOG_H
#define RECORDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>

    class RecordDialog : public QDialog {
    Q_OBJECT
public:
    explicit RecordDialog(QWidget *parent = nullptr);

    QString getId() const { return m_idInput->text(); }
    QString getTimestamp() const { return m_timeInput->text(); }

private:
    QLineEdit *m_idInput;
    QLineEdit *m_timeInput;
    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;
};
#endif // RECORDDIALOG_H
