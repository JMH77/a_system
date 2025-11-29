#ifndef WORKORDERMANAGEMENTWIDGET_H
#define WORKORDERMANAGEMENTWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>

class AuthManager;
class WorkOrderManager;
struct WorkOrderData;

class WorkOrderManagementWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WorkOrderManagementWidget(AuthManager *authManager, const QString &currentUsername, QWidget *parent = nullptr);
    ~WorkOrderManagementWidget();

private slots:
    void onNewWorkOrderClicked();
    void onSearchTextChanged(const QString &text);
    void onAssignButtonClicked();
    void onEditButtonClicked();
    void onTableDoubleClicked(int row, int column);

private:
    void setupUI();
    void applyStyles();
    void loadWorkOrders();
    void displayWorkOrders(const QList<WorkOrderData> &workOrders);
    bool isAdmin() const;
    WorkOrderData getSelectedWorkOrder() const;
    int getSelectedRow() const;

private:
    AuthManager *m_authManager;
    QString m_currentUsername;
    WorkOrderManager *m_workOrderManager;
    
    QPushButton *m_newButton;
    QPushButton *m_assignButton;
    QPushButton *m_editButton;
    QLineEdit *m_searchEdit;
    QTableWidget *m_workOrderTable;
    
    QList<WorkOrderData> m_currentWorkOrders;  // 存储当前显示的工单列表
};

#endif // WORKORDERMANAGEMENTWIDGET_H

