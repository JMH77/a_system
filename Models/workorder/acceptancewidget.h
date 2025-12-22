#ifndef ACCEPTANCEWIDGET_H
#define ACCEPTANCEWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QTableWidget>

class AuthManager;
class WorkOrderManager;
struct WorkOrderData;

class AcceptanceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AcceptanceWidget(AuthManager *authManager, const QString &currentUsername, QWidget *parent = nullptr);
    ~AcceptanceWidget();

private slots:
    void onSearchTextChanged(const QString &text);
    void onAcceptButtonClicked();

private:
    void setupUI();
    void applyStyles();
    void loadAcceptanceTasks();
    void displayTasks(const QList<WorkOrderData> &tasks);

private:
    AuthManager *m_authManager;
    QString m_currentUsername;
    WorkOrderManager *m_workOrderManager;
    
    QLineEdit *m_searchEdit;
    QTableWidget *m_acceptanceTable;
    
    QList<WorkOrderData> m_currentTasks;
};

#endif // ACCEPTANCEWIDGET_H

