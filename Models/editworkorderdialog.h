#ifndef EDITWORKORDERDIALOG_H
#define EDITWORKORDERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include "workordermanager.h"

class WorkOrderManager;

class EditWorkOrderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditWorkOrderDialog(WorkOrderManager *workOrderManager, const WorkOrderData &workOrderData, QWidget *parent = nullptr);

private slots:
    void onOkClicked();
    void onCancelClicked();

private:
    void setupUI();
    void applyStyles();
    bool validateInput();
    void loadWorkOrderData();

private:
    WorkOrderManager *m_workOrderManager;
    WorkOrderData m_originalData;
    
    QLineEdit *m_orderIdEdit;      // 工单编号（只读）
    QComboBox *m_orderTypeCombo;   // 工单类型
    QLineEdit *m_titleEdit;        // 标题
    QTextEdit *m_descriptionEdit;  // 描述
    QLineEdit *m_equipIdEdit;      // 设备ID
    QLineEdit *m_shipIdEdit;       // 船舶ID
    QLineEdit *m_planIdEdit;       // 关联计划ID
    
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
};

#endif // EDITWORKORDERDIALOG_H

