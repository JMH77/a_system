#include "acceptancewidget.h"
#include "workordermanager.h"
#include "../auth/authmanager.h"
#include "../database/databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QBrush>
#include <QColor>
#include <QMessageBox>
#include <QPushButton>
#include <QDebug>

AcceptanceWidget::AcceptanceWidget(AuthManager *authManager, const QString &currentUsername, QWidget *parent)
    : QWidget(parent)
    , m_authManager(authManager)
    , m_currentUsername(currentUsername)
    , m_workOrderManager(nullptr)
    , m_searchEdit(nullptr)
    , m_acceptanceTable(nullptr)
{
    // 创建工单管理器
    if (m_authManager && m_authManager->getDatabaseManager()) {
        m_workOrderManager = new WorkOrderManager(m_authManager->getDatabaseManager());
    }
    
    // 设置为独立窗口
    setWindowFlags(Qt::Window);
    setWindowTitle("验收任务");
    setupUI();
    applyStyles();
    loadAcceptanceTasks();
}

AcceptanceWidget::~AcceptanceWidget()
{
    delete m_workOrderManager;
}

void AcceptanceWidget::setupUI()
{
    // 创建顶部搜索栏
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("按工单标题搜索...");
    m_searchEdit->setObjectName("searchEdit");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &AcceptanceWidget::onSearchTextChanged);
    
    toolbarLayout->addWidget(m_searchEdit);
    
    // 创建验收任务表格（添加操作列）
    m_acceptanceTable = new QTableWidget(this);
    m_acceptanceTable->setColumnCount(7);
    QStringList headers;
    headers << "工单编号" << "工单类型" << "标题" << "状态" << "完成时间" << "执行人" << "操作";
    m_acceptanceTable->setHorizontalHeaderLabels(headers);
    m_acceptanceTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_acceptanceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 设置所有列宽度相同
    m_acceptanceTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_acceptanceTable->verticalHeader()->setDefaultSectionSize(50);  // 设置行高
    
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(toolbarLayout);
    mainLayout->addWidget(m_acceptanceTable);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
}

void AcceptanceWidget::applyStyles()
{
    m_searchEdit->setMinimumHeight(45);
    
    // 设置按钮蓝色样式，与主系统一致
    this->setStyleSheet(
        "QPushButton {"
            "padding: 10px 20px;"
            "border-radius: 5px;"
            "border: none;"
            "background: #6CA6CD;"
            "color: #ffffff;"
            "font-size: 18px;"
        "}"
        "QPushButton:hover {"
            "background: #5B9BD5;"
        "}"
        "QPushButton:pressed {"
            "background: #4A8BC4;"
        "}"
        "QPushButton:disabled {"
            "background: #CCCCCC;"
            "color: #888888;"
        "}"
        "QLineEdit {"
            "font-size: 18px;"
        "}"
        "QTableWidget {"
            "font-size: 18px;"
        "}"
        "QTableWidget::item {"
            "padding: 8px;"
        "}"
        "QHeaderView::section {"
            "font-size: 18px;"
            "padding: 10px;"
            "background-color: #6CA6CD;"
            "color: white;"
        "}"
        "QTableWidget QPushButton {"
            "width: 100%;"
            "height: 100%;"
            "margin: 1px;"
            "padding: 5px;"
        "}"
        "QTableWidget::item:selected {"
            "background-color:rgb(39, 144, 219);"
        "}"
        "QTableWidget QHeaderView::section:selected {"
            "background-color: #6CA6CD;"
        "}"
    );
}

void AcceptanceWidget::loadAcceptanceTasks()
{
    if (!m_workOrderManager) {
        QMessageBox::warning(this, "错误", "工单管理器未初始化！");
        return;
    }
    
    // 如果是adminjmh管理员，加载所有工单；否则只加载分配给当前用户验收的工单
    bool isAdmin = (m_currentUsername == "adminjmh");
    if (isAdmin) {
        m_currentTasks = m_workOrderManager->getAllWorkOrders(m_currentUsername, true);
    } else {
        m_currentTasks = m_workOrderManager->getWorkOrdersByAcceptor(m_currentUsername);
    }
    
    if (!m_workOrderManager->getLastError().isEmpty()) {
        QMessageBox::warning(this, "加载失败", QString("加载验收任务失败：%1").arg(m_workOrderManager->getLastError()));
        return;
    }
    
    // 显示任务数据
    displayTasks(m_currentTasks);
    
    qDebug() << "加载验收任务完成，当前用户：" << m_currentUsername << "，任务数量：" << m_currentTasks.size();
}

void AcceptanceWidget::displayTasks(const QList<WorkOrderData> &tasks)
{
    // 清空表格
    m_acceptanceTable->setRowCount(0);
    
    // 填充数据
    for (const WorkOrderData &data : tasks) {
        int row = m_acceptanceTable->rowCount();
        m_acceptanceTable->insertRow(row);
        
        // 工单编号
        QTableWidgetItem *orderIdItem = new QTableWidgetItem(data.orderId);
        m_acceptanceTable->setItem(row, 0, orderIdItem);
        
        // 工单类型
        QTableWidgetItem *orderTypeItem = new QTableWidgetItem(data.orderType);
        m_acceptanceTable->setItem(row, 1, orderTypeItem);
        
        // 标题
        QTableWidgetItem *titleItem = new QTableWidgetItem(data.title);
        m_acceptanceTable->setItem(row, 2, titleItem);
        
        // 状态（带颜色）
        QTableWidgetItem *statusItem = new QTableWidgetItem(data.status);
        if (data.status == "待分配") {
            statusItem->setForeground(QBrush(QColor(255, 165, 0)));  // 橙色
        } else if (data.status == "处理中") {
            statusItem->setForeground(QBrush(QColor(0, 100, 255)));  // 蓝色
        } else if (data.status == "待验收") {
            statusItem->setForeground(QBrush(QColor(255, 200, 0)));  // 金黄色
        } else if (data.status == "已完成") {
            statusItem->setForeground(QBrush(QColor(0, 150, 0)));  // 绿色
        }
        m_acceptanceTable->setItem(row, 3, statusItem);
        
        // 完成时间（实际结束时间）
        QString endTimeStr = data.actualEndTime.isValid() ? data.actualEndTime.toString("yyyy-MM-dd hh:mm:ss") : "未完成";
        QTableWidgetItem *endTimeItem = new QTableWidgetItem(endTimeStr);
        m_acceptanceTable->setItem(row, 4, endTimeItem);
        
        // 执行人
        QTableWidgetItem *executorItem = new QTableWidgetItem(data.assigneeId.isEmpty() ? "未分配" : data.assigneeId);
        m_acceptanceTable->setItem(row, 5, executorItem);
        
        // 操作按钮列
        QPushButton *acceptButton = new QPushButton("验收完毕", this);
        acceptButton->setProperty("orderId", data.orderId);  // 存储工单ID
        acceptButton->setProperty("row", row);  // 存储行号
        // 让按钮铺满单元格
        acceptButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        
        // 只有状态为"待验收"的工单才显示可点击的验收按钮
        if (data.status == "待验收") {
            acceptButton->setEnabled(true);
            connect(acceptButton, &QPushButton::clicked, this, &AcceptanceWidget::onAcceptButtonClicked);
        } else {
            acceptButton->setEnabled(false);
            if (data.status == "已完成") {
                acceptButton->setText("已完成");
            } else {
                acceptButton->setText("未完成");
            }
        }
        
        m_acceptanceTable->setCellWidget(row, 6, acceptButton);
    }
    
    // 列宽已设置为均匀分布，无需调整
}

void AcceptanceWidget::onSearchTextChanged(const QString &text)
{
    if (text.trimmed().isEmpty()) {
        // 如果搜索框为空，重新加载所有任务
        loadAcceptanceTasks();
        return;
    }
    
    // 执行搜索（在所有待验收的任务中搜索）
    QList<WorkOrderData> filteredTasks;
    for (const WorkOrderData &task : m_currentTasks) {
        if (task.title.contains(text.trimmed(), Qt::CaseInsensitive)) {
            filteredTasks.append(task);
        }
    }
    
    // 显示搜索结果
    displayTasks(filteredTasks);
}

void AcceptanceWidget::onAcceptButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    QString orderId = button->property("orderId").toString();
    if (orderId.isEmpty()) return;
    
    // 弹出确认对话框
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认验收",
        QString("确定要验收工单 %1 吗？\n验收后工单状态将变为'已完成'。").arg(orderId),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    if (!m_workOrderManager) {
        QMessageBox::warning(this, "错误", "工单管理器未初始化！");
        return;
    }
    
    // 验收工单（传入当前用户名作为操作人）
    if (!m_workOrderManager->acceptWorkOrder(orderId, m_currentUsername)) {
        QString errorMsg = m_workOrderManager->getLastError();
        QMessageBox::warning(this, "验收失败", QString("验收工单失败：%1").arg(errorMsg));
        return;
    }
    
    QMessageBox::information(this, "成功", "工单已验收完毕！");
    
    // 刷新列表
    loadAcceptanceTasks();
}

