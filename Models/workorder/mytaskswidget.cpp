#include "mytaskswidget.h"
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

MyTasksWidget::MyTasksWidget(AuthManager *authManager, const QString &currentUsername, QWidget *parent)
    : QWidget(parent)
    , m_authManager(authManager)
    , m_currentUsername(currentUsername)
    , m_workOrderManager(nullptr)
    , m_searchEdit(nullptr)
    , m_taskTable(nullptr)
{
    // 创建工单管理器
    if (m_authManager && m_authManager->getDatabaseManager()) {
        m_workOrderManager = new WorkOrderManager(m_authManager->getDatabaseManager());
    }
    
    // 设置为独立窗口
    setWindowFlags(Qt::Window);
    setWindowTitle("我的任务");
    setupUI();
    applyStyles();
    loadMyTasks();
}

MyTasksWidget::~MyTasksWidget()
{
    delete m_workOrderManager;
}

void MyTasksWidget::setupUI()
{
    // 创建顶部搜索栏
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("按工单标题搜索...");
    m_searchEdit->setObjectName("searchEdit");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MyTasksWidget::onSearchTextChanged);
    
    toolbarLayout->addWidget(m_searchEdit);
    
    // 创建任务表格（添加操作列）
    m_taskTable = new QTableWidget(this);
    m_taskTable->setColumnCount(7);
    QStringList headers;
    headers << "工单编号" << "工单类型" << "标题" << "状态" << "分配时间" << "创建人" << "操作";
    m_taskTable->setHorizontalHeaderLabels(headers);
    m_taskTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_taskTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 设置所有列宽度相同
    m_taskTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_taskTable->verticalHeader()->setDefaultSectionSize(50);  // 设置行高
    
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(toolbarLayout);
    mainLayout->addWidget(m_taskTable);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
}

void MyTasksWidget::applyStyles()
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
            "background-color: #E3F2FD;"
        "}"
        "QTableWidget QHeaderView::section:selected {"
            "background-color: #6CA6CD;"
        "}"
    );
}

void MyTasksWidget::loadMyTasks()
{
    if (!m_workOrderManager) {
        QMessageBox::warning(this, "错误", "工单管理器未初始化！");
        return;
    }
    
    // 如果是adminjmh管理员，加载所有工单；否则只加载分配给当前用户的工单
    bool isAdmin = (m_currentUsername == "adminjmh");
    if (isAdmin) {
        m_currentTasks = m_workOrderManager->getAllWorkOrders(m_currentUsername, true);
    } else {
        m_currentTasks = m_workOrderManager->getWorkOrdersByAssignee(m_currentUsername);
    }
    
    if (!m_workOrderManager->getLastError().isEmpty()) {
        QMessageBox::warning(this, "加载失败", QString("加载任务失败：%1").arg(m_workOrderManager->getLastError()));
        return;
    }
    
    // 显示任务数据
    displayTasks(m_currentTasks);
    
    qDebug() << "加载我的任务完成，当前用户：" << m_currentUsername << "，任务数量：" << m_currentTasks.size();
}

void MyTasksWidget::displayTasks(const QList<WorkOrderData> &tasks)
{
    // 清空表格
    m_taskTable->setRowCount(0);
    
    // 填充数据
    for (const WorkOrderData &data : tasks) {
        int row = m_taskTable->rowCount();
        m_taskTable->insertRow(row);
        
        // 工单编号
        QTableWidgetItem *orderIdItem = new QTableWidgetItem(data.orderId);
        m_taskTable->setItem(row, 0, orderIdItem);
        
        // 工单类型
        QTableWidgetItem *orderTypeItem = new QTableWidgetItem(data.orderType);
        m_taskTable->setItem(row, 1, orderTypeItem);
        
        // 标题
        QTableWidgetItem *titleItem = new QTableWidgetItem(data.title);
        m_taskTable->setItem(row, 2, titleItem);
        
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
        m_taskTable->setItem(row, 3, statusItem);
        
        // 分配时间
        QString assignTimeStr = data.assignTime.isValid() ? data.assignTime.toString("yyyy-MM-dd hh:mm:ss") : "未分配";
        QTableWidgetItem *assignTimeItem = new QTableWidgetItem(assignTimeStr);
        m_taskTable->setItem(row, 4, assignTimeItem);
        
        // 创建人
        QTableWidgetItem *creatorItem = new QTableWidgetItem(data.creatorId);
        m_taskTable->setItem(row, 5, creatorItem);
        
        // 操作按钮列
        QPushButton *completeButton = new QPushButton("完成", this);
        completeButton->setProperty("orderId", data.orderId);  // 存储工单ID
        completeButton->setProperty("row", row);  // 存储行号
        // 让按钮铺满单元格
        completeButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        
        // 只有状态为"处理中"的工单才显示完成按钮
        // adminjmh管理员可以操作所有状态的工单（但按钮文本根据状态变化）
        bool isAdmin = (m_currentUsername == "adminjmh");
        if (data.status == "处理中") {
            completeButton->setEnabled(true);
            completeButton->setText("完成");
            connect(completeButton, &QPushButton::clicked, this, &MyTasksWidget::onCompleteButtonClicked);
        } else if (isAdmin && (data.status == "待验收" || data.status == "已完成")) {
            // 管理员可以看到所有工单，对于已完成或待验收的工单，按钮显示相应状态但不可操作
            completeButton->setEnabled(false);
            completeButton->setText(data.status == "已完成" ? "已完成" : "待验收");
        } else {
            completeButton->setEnabled(false);
            completeButton->setText("已完成" == data.status ? "已完成" : "待验收");
        }
        
        m_taskTable->setCellWidget(row, 6, completeButton);
    }
    
    // 列宽已设置为均匀分布，无需调整
}

void MyTasksWidget::onSearchTextChanged(const QString &text)
{
    if (!m_workOrderManager) {
        return;
    }
    
    if (text.trimmed().isEmpty()) {
        // 如果搜索框为空，重新加载所有任务
        loadMyTasks();
        return;
    }
    
    // 执行搜索（在所有分配给当前用户的任务中搜索）
    QList<WorkOrderData> filteredTasks;
    for (const WorkOrderData &task : m_currentTasks) {
        if (task.title.contains(text.trimmed(), Qt::CaseInsensitive)) {
            filteredTasks.append(task);
        }
    }
    
    // 显示搜索结果
    displayTasks(filteredTasks);
}

void MyTasksWidget::onCompleteButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    QString orderId = button->property("orderId").toString();
    if (orderId.isEmpty()) return;
    
    // 弹出确认对话框
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认完成",
        QString("确定要完成工单 %1 吗？\n完成后工单状态将变为'待验收'。").arg(orderId),
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
    
    // 完成工单（传入当前用户名作为操作人）
    if (!m_workOrderManager->completeWorkOrder(orderId, m_currentUsername)) {
        QString errorMsg = m_workOrderManager->getLastError();
        QMessageBox::warning(this, "完成失败", QString("完成工单失败：%1").arg(errorMsg));
        return;
    }
    
    QMessageBox::information(this, "成功", "工单已完成，等待验收！");
    
    // 刷新列表
    loadMyTasks();
}

