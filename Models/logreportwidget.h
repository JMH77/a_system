#ifndef LOGREPORTWIDGET_H
#define LOGREPORTWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>

class AuthManager;
class databasemanager;

class LogReportWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LogReportWidget(AuthManager *authManager, const QString &currentUsername, QWidget *parent = nullptr);

private slots:
    void onNewReportClicked();
    void onSearchTextChanged(const QString &text);

private:
    void setupUI();
    void applyStyles();
    void loadReports();
    void displayReports(const QList<QStringList> &reports);

private:
    AuthManager *m_authManager;
    QString m_currentUsername;
    databasemanager *m_dbManager;
    
    QPushButton *m_newButton;
    QLineEdit *m_searchEdit;
    QTableWidget *m_reportTable;
    
    QList<QStringList> m_currentReports;  // 存储当前显示的日志记录
};

#endif // LOGREPORTWIDGET_H

