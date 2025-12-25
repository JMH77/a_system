#include "assignworkorderdialog.h"
#include "workordermanager.h"
#include "../auth/authmanager.h"
#include "../auth/userinfo.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QDebug>

AssignWorkOrderDialog::AssignWorkOrderDialog(WorkOrderManager *workOrderManager, AuthManager *authManager,
                                             const QString &orderId, const QString &currentAssignee,
                                             const QString &currentAcceptor, const QString &operatorId,
                                             QWidget *parent)
    : QDialog(parent)
    , m_workOrderManager(workOrderManager)
    , m_authManager(authManager)
    , m_orderId(orderId)
    , m_operatorId(operatorId)
    , m_orderIdLabel(nullptr)
    , m_assigneeCombo(nullptr)
    , m_acceptorCombo(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
{
    setWindowTitle("分配工单");
    setMinimumSize(400, 200);
    setupUI();
    loadUsers();
    
    // 设置当前已分配的执行人员和验收人员
    if (!currentAssignee.isEmpty()) {
        int index = m_assigneeCombo->findText(currentAssignee);
        if (index >= 0) {
            m_assigneeCombo->setCurrentIndex(index);
        }
    }
    
    if (!currentAcceptor.isEmpty()) {
        int index = m_acceptorCombo->findText(currentAcceptor);
        if (index >= 0) {
            m_acceptorCombo->setCurrentIndex(index);
        }
    }
}

void AssignWorkOrderDialog::setupUI()
{
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(15);
    formLayout->setContentsMargins(20, 20, 20, 20);
    
    // 工单编号（只读显示）
    m_orderIdLabel = new QLabel(m_orderId, this);
    m_orderIdLabel->setObjectName("orderIdLabel");
    formLayout->addRow("工单编号:", m_orderIdLabel);
    
    // 执行人员（下拉框）
    m_assigneeCombo = new QComboBox(this);
    m_assigneeCombo->setObjectName("assigneeCombo");
    m_assigneeCombo->setEditable(false);
    m_assigneeCombo->addItem("未分配", "");  // 第一个选项是"未分配"
    formLayout->addRow("执行人员*:", m_assigneeCombo);
    
    // 验收人员（下拉框）
    m_acceptorCombo = new QComboBox(this);
    m_acceptorCombo->setObjectName("acceptorCombo");
    m_acceptorCombo->setEditable(false);
    m_acceptorCombo->addItem("未分配", "");  // 第一个选项是"未分配"
    formLayout->addRow("验收人员*:", m_acceptorCombo);
    
    // 按钮
    m_okButton = new QPushButton("确定", this);
    m_cancelButton = new QPushButton("取消", this);
    m_okButton->setObjectName("okButton");
    m_cancelButton->setObjectName("cancelButton");
    
    connect(m_okButton, &QPushButton::clicked, this, &AssignWorkOrderDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &AssignWorkOrderDialog::onCancelClicked);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);
    
    // 设置按钮大小和样式
    m_okButton->setMinimumSize(120, 50);
    m_cancelButton->setMinimumSize(120, 50);
    applyStyles();
}

void AssignWorkOrderDialog::applyStyles()
{
    // 设置按钮蓝色样式，与主系统一致
    this->setStyleSheet(
        "QPushButton#okButton {"
            "padding: 10px 20px;"
            "border-radius: 5px;"
            "border: none;"
            "background: #6CA6CD;"
            "color: #ffffff;"
            "font-size: 18px;"
        "}"
        "QPushButton#okButton:hover {"
            "background: #5B9BD5;"
        "}"
        "QPushButton#okButton:pressed {"
            "background: #4A8BC4;"
        "}"
        "QPushButton#cancelButton {"
            "padding: 10px 20px;"
            "border-radius: 5px;"
            "border: none;"
            "background: #CCCCCC;"
            "color: #333333;"
            "font-size: 18px;"
        "}"
        "QPushButton#cancelButton:hover {"
            "background: #BBBBBB;"
        "}"
        "QPushButton#cancelButton:pressed {"
            "background: #AAAAAA;"
        "}"
        "QLineEdit, QComboBox {"
            "font-size: 18px;"
        "}"
        "QLabel {"
            "font-size: 18px;"
        "}"
    );
}

void AssignWorkOrderDialog::loadUsers()
{
    if (!m_authManager) {
        return;
    }
    
    // 加载执行人员（work_order_role = '执行人员'）
    QList<userinfo> executors = m_authManager->getUsersByWorkOrderRole("执行人员");
    for (const userinfo &user : executors) {
        userinfodata data = user.getUserData();
        m_assigneeCombo->addItem(data.username, data.username);
    }
    
    // 加载验收人员（work_order_role = '检验人员'，数据库中存储的是'检验人员'）
    QList<userinfo> acceptors = m_authManager->getUsersByWorkOrderRole("检验人员");
    for (const userinfo &user : acceptors) {
        userinfodata data = user.getUserData();
        m_acceptorCombo->addItem(data.username, data.username);
    }
    
    // 如果下拉框为空（除了"未分配"），显示提示
    if (m_assigneeCombo->count() == 1) {
        m_assigneeCombo->addItem("暂无执行人员", "");
    }
    if (m_acceptorCombo->count() == 1) {
        m_acceptorCombo->addItem("暂无验收人员", "");
    }
}

bool AssignWorkOrderDialog::validateInput()
{
    // 必须同时指定执行人员和验收人员，否则工单无法完整流转
    QString assignee = m_assigneeCombo->currentData().toString();
    QString acceptor = m_acceptorCombo->currentData().toString();
    
    bool assigneeValid = !assignee.isEmpty() && 
                         m_assigneeCombo->currentText() != "未分配" && 
                         m_assigneeCombo->currentText() != "暂无执行人员";
    bool acceptorValid = !acceptor.isEmpty() && 
                         m_acceptorCombo->currentText() != "未分配" && 
                         m_acceptorCombo->currentText() != "暂无验收人员";
    
    if (!assigneeValid && !acceptorValid) {
        QMessageBox::warning(this, "验证失败", "请同时指定执行人员和验收人员！");
        return false;
    }
    if (!assigneeValid) {
        QMessageBox::warning(this, "验证失败", "请指定执行人员！");
        return false;
    }
    if (!acceptorValid) {
        QMessageBox::warning(this, "验证失败", "请指定验收人员！");
        return false;
    }
    
    return true;
}

void AssignWorkOrderDialog::onOkClicked()
{
    if (!validateInput()) {
        return;
    }
    
    if (!m_workOrderManager) {
        QMessageBox::warning(this, "错误", "工单管理器未初始化！");
        return;
    }
    
    QString assignee = m_assigneeCombo->currentData().toString();
    QString acceptor = m_acceptorCombo->currentData().toString();
    
    // 如果选择的是"未分配"或"暂无"，设置为空字符串
    if (assignee.isEmpty() || m_assigneeCombo->currentText() == "未分配" || m_assigneeCombo->currentText() == "暂无执行人员") {
        assignee = "";
    }
    if (acceptor.isEmpty() || m_acceptorCombo->currentText() == "未分配" || m_acceptorCombo->currentText() == "暂无验收人员") {
        acceptor = "";
    }
    
    // 分配工单（传入操作人ID）
    if (!m_workOrderManager->assignWorkOrder(m_orderId, assignee, acceptor, m_operatorId)) {
        QString errorMsg = m_workOrderManager->getLastError();
        QMessageBox::warning(this, "分配失败", QString("分配工单失败：%1").arg(errorMsg));
        return;
    }
    
    QMessageBox::information(this, "成功", "工单分配成功！");
    accept();  // 关闭对话框
}

void AssignWorkOrderDialog::onCancelClicked()
{
    reject();  // 关闭对话框
}

