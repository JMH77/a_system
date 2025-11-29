#include "editworkorderdialog.h"
#include "workordermanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QDebug>

EditWorkOrderDialog::EditWorkOrderDialog(WorkOrderManager *workOrderManager, const WorkOrderData &workOrderData, QWidget *parent)
    : QDialog(parent)
    , m_workOrderManager(workOrderManager)
    , m_originalData(workOrderData)
    , m_orderIdEdit(nullptr)
    , m_orderTypeCombo(nullptr)
    , m_titleEdit(nullptr)
    , m_descriptionEdit(nullptr)
    , m_equipIdEdit(nullptr)
    , m_shipIdEdit(nullptr)
    , m_planIdEdit(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
{
    setWindowTitle("编辑工单");
    setMinimumSize(500, 400);
    setupUI();
    applyStyles();
    loadWorkOrderData();
}

void EditWorkOrderDialog::setupUI()
{
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(15);
    formLayout->setContentsMargins(20, 20, 20, 20);
    
    // 工单编号（只读）
    m_orderIdEdit = new QLineEdit(this);
    m_orderIdEdit->setReadOnly(true);
    m_orderIdEdit->setObjectName("orderIdEdit");
    formLayout->addRow("工单编号:", m_orderIdEdit);
    
    // 工单类型（下拉框）
    m_orderTypeCombo = new QComboBox(this);
    m_orderTypeCombo->addItem("常规维护");
    m_orderTypeCombo->addItem("紧急维修");
    m_orderTypeCombo->setObjectName("orderTypeCombo");
    formLayout->addRow("工单类型:", m_orderTypeCombo);
    
    // 标题（可编辑）
    m_titleEdit = new QLineEdit(this);
    m_titleEdit->setObjectName("titleEdit");
    m_titleEdit->setPlaceholderText("请输入工单标题");
    formLayout->addRow("标题*:", m_titleEdit);
    
    // 描述（多行文本）
    m_descriptionEdit = new QTextEdit(this);
    m_descriptionEdit->setObjectName("descriptionEdit");
    m_descriptionEdit->setPlaceholderText("请输入详细描述");
    m_descriptionEdit->setMaximumHeight(100);
    formLayout->addRow("描述:", m_descriptionEdit);
    
    // 设备ID
    m_equipIdEdit = new QLineEdit(this);
    m_equipIdEdit->setObjectName("equipIdEdit");
    m_equipIdEdit->setPlaceholderText("例如：EQP-1001");
    formLayout->addRow("设备ID:", m_equipIdEdit);
    
    // 船舶ID
    m_shipIdEdit = new QLineEdit(this);
    m_shipIdEdit->setObjectName("shipIdEdit");
    m_shipIdEdit->setPlaceholderText("例如：SHIP-01");
    formLayout->addRow("船舶ID:", m_shipIdEdit);
    
    // 关联计划ID（可选）
    m_planIdEdit = new QLineEdit(this);
    m_planIdEdit->setObjectName("planIdEdit");
    m_planIdEdit->setPlaceholderText("例如：PLAN-001（可选）");
    formLayout->addRow("关联计划ID:", m_planIdEdit);
    
    // 按钮
    m_okButton = new QPushButton("确定", this);
    m_cancelButton = new QPushButton("取消", this);
    m_okButton->setObjectName("okButton");
    m_cancelButton->setObjectName("cancelButton");
    
    connect(m_okButton, &QPushButton::clicked, this, &EditWorkOrderDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &EditWorkOrderDialog::onCancelClicked);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);
}

void EditWorkOrderDialog::applyStyles()
{
    m_titleEdit->setMinimumHeight(35);
    m_equipIdEdit->setMinimumHeight(35);
    m_shipIdEdit->setMinimumHeight(35);
    m_planIdEdit->setMinimumHeight(35);
    m_okButton->setMinimumSize(80, 35);
    m_cancelButton->setMinimumSize(80, 35);
}

void EditWorkOrderDialog::loadWorkOrderData()
{
    // 加载原始工单数据到界面
    m_orderIdEdit->setText(m_originalData.orderId);
    
    // 设置工单类型
    int typeIndex = m_orderTypeCombo->findText(m_originalData.orderType);
    if (typeIndex >= 0) {
        m_orderTypeCombo->setCurrentIndex(typeIndex);
    }
    
    m_titleEdit->setText(m_originalData.title);
    m_descriptionEdit->setPlainText(m_originalData.description);
    m_equipIdEdit->setText(m_originalData.relatedEquipId);
    m_shipIdEdit->setText(m_originalData.shipId);
    m_planIdEdit->setText(m_originalData.relatedPlanId);
}

bool EditWorkOrderDialog::validateInput()
{
    if (m_titleEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "标题不能为空！");
        return false;
    }
    
    if (m_titleEdit->text().trimmed().length() > 200) {
        QMessageBox::warning(this, "输入错误", "标题长度不能超过200个字符！");
        return false;
    }
    
    return true;
}

void EditWorkOrderDialog::onOkClicked()
{
    if (!validateInput()) {
        return;
    }
    
    if (!m_workOrderManager) {
        QMessageBox::warning(this, "错误", "工单管理器未初始化！");
        return;
    }
    
    // 构建更新后的工单数据
    WorkOrderData updatedData = m_originalData;
    updatedData.orderType = m_orderTypeCombo->currentText();
    updatedData.title = m_titleEdit->text().trimmed();
    updatedData.description = m_descriptionEdit->toPlainText().trimmed();
    updatedData.relatedEquipId = m_equipIdEdit->text().trimmed();
    updatedData.shipId = m_shipIdEdit->text().trimmed();
    updatedData.relatedPlanId = m_planIdEdit->text().trimmed();
    
    // 更新工单
    if (!m_workOrderManager->updateWorkOrder(m_originalData.orderId, updatedData)) {
        QString errorMsg = m_workOrderManager->getLastError();
        QMessageBox::warning(this, "更新失败", QString("更新工单失败：%1").arg(errorMsg));
        return;
    }
    
    QMessageBox::information(this, "成功", "工单更新成功！");
    accept();
}

void EditWorkOrderDialog::onCancelClicked()
{
    reject();
}

