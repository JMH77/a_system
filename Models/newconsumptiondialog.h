#ifndef NEWCONSUMPTIONDIALOG_H
#define NEWCONSUMPTIONDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QTableWidget>
#include "sparepartsmanager.h"

class SparePartsManager;
class AuthManager;

class NewConsumptionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewConsumptionDialog(SparePartsManager *sparePartsManager, AuthManager *authManager, 
                                  const QString &operatorId, QWidget *parent = nullptr);
    ~NewConsumptionDialog();

private slots:
    void onAddSpareClicked();
    void onRemoveSpareClicked();
    void onOkClicked();
    void onCancelClicked();

private:
    void setupUI();
    void loadRelatedWorkOrders();  // 加载当前用户相关的工单列表
    bool validateInput();

private:
    SparePartsManager *m_sparePartsManager;
    AuthManager *m_authManager;
    QString m_operatorId;
    
    QComboBox *m_orderIdCombo;     // 工单编号下拉框
    QTableWidget *m_spareTable;    // 备件消耗表格
    QPushButton *m_addSpareButton;
    QPushButton *m_removeSpareButton;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
};

#endif // NEWCONSUMPTIONDIALOG_H

