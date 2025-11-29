#ifndef MYTASKSWIDGET_H
#define MYTASKSWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QTableWidget>

class AuthManager;
class WorkOrderManager;
struct WorkOrderData;

class MyTasksWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MyTasksWidget(AuthManager *authManager, const QString &currentUsername, QWidget *parent = nullptr);
    ~MyTasksWidget();

private slots:
    void onSearchTextChanged(const QString &text);
    void onCompleteButtonClicked();

private:
    void setupUI();
    void applyStyles();
    void loadMyTasks();
    void displayTasks(const QList<WorkOrderData> &tasks);

private:
    AuthManager *m_authManager;
    QString m_currentUsername;
    WorkOrderManager *m_workOrderManager;
    
    QLineEdit *m_searchEdit;
    QTableWidget *m_taskTable;
    
    QList<WorkOrderData> m_currentTasks;
};

#endif // MYTASKSWIDGET_H

