#ifndef NEWWORKORDERDIALOG_H
#define NEWWORKORDERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QDoubleSpinBox>
#include "workordermanager.h"
#include "sparepartsmanager.h"

class WorkOrderManager;
struct SparePartsConsumptionData;

class NewWorkOrderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewWorkOrderDialog(WorkOrderManager *workOrderManager, const QString &creatorId, QWidget *parent = nullptr);
    ~NewWorkOrderDialog();
    
    // 获取工单数据（用户点击确定后）
    WorkOrderData getWorkOrderData() const;

private slots:
    void onOkClicked();
    void onCancelClicked();
    void onAddSpareClicked();
    void onRemoveSpareClicked();

private:
    void setupUI();
    void applyStyles();
    bool validateInput();
    void addSpareRow();
    void removeSpareRow(int row);

private:
    WorkOrderManager *m_workOrderManager;
    SparePartsManager *m_sparePartsManager;
    QString m_creatorId;
    
    QLineEdit *m_orderIdEdit;      // 工单编号（自动生成，只读）
    QComboBox *m_orderTypeCombo;   // 工单类型
    QLineEdit *m_titleEdit;        // 标题
    QTextEdit *m_descriptionEdit;  // 描述
    QLineEdit *m_equipIdEdit;      // 设备ID
    QLineEdit *m_shipIdEdit;       // 船舶ID
    QLineEdit *m_planIdEdit;       // 关联计划ID（可选）
    
    // 备件消耗表格
    QTableWidget *m_spareTable;    // 备件消耗表格
    QPushButton *m_addSpareButton; // 添加备件按钮
    QPushButton *m_removeSpareButton; // 删除备件按钮
    
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    
    WorkOrderData m_workOrderData;
    
    // 获取备件消耗数据列表
    QList<SparePartsConsumptionData> getSpareConsumptions() const;
};

#endif // NEWWORKORDERDIALOG_H

