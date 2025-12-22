#ifndef ASSIGNWORKORDERDIALOG_H
#define ASSIGNWORKORDERDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include "workordermanager.h"

class AuthManager;
class WorkOrderManager;

class AssignWorkOrderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AssignWorkOrderDialog(WorkOrderManager *workOrderManager, AuthManager *authManager, 
                                   const QString &orderId, const QString &currentAssignee, 
                                   const QString &currentAcceptor, const QString &operatorId = "", 
                                   QWidget *parent = nullptr);

private slots:
    void onOkClicked();
    void onCancelClicked();

private:
    void setupUI();
    void applyStyles();
    void loadUsers();
    bool validateInput();

private:
    WorkOrderManager *m_workOrderManager;
    AuthManager *m_authManager;
    QString m_orderId;
    QString m_operatorId;  // 操作人ID（分配工单的用户）
    
    QLabel *m_orderIdLabel;
    QComboBox *m_assigneeCombo;    // 执行人员
    QComboBox *m_acceptorCombo;    // 验收人员
    
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
};

#endif // ASSIGNWORKORDERDIALOG_H

