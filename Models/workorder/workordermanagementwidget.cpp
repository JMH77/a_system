#include "workordermanagementwidget.h"
#include "workordermanager.h"
#include "newworkorderdialog.h"
#include "assignworkorderdialog.h"
#include "editworkorderdialog.h"
#include "../auth/authmanager.h"
#include "../database/databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QBrush>
#include <QColor>
#include <QMessageBox>
#include <QDebug>

WorkOrderManagementWidget::WorkOrderManagementWidget(AuthManager *authManager, const QString &currentUsername, QWidget *parent)
    : QWidget(parent)
    , m_authManager(authManager)
    , m_currentUsername(currentUsername)
    , m_workOrderManager(nullptr)
    , m_newButton(nullptr)
    , m_assignButton(nullptr)
    , m_editButton(nullptr)
    , m_searchEdit(nullptr)
    , m_workOrderTable(nullptr)
{
    // 创建工单管理器
    if (m_authManager && m_authManager->getDatabaseManager()) {
        m_workOrderManager = new WorkOrderManager(m_authManager->getDatabaseManager());
    }
    
    // 设置为独立窗口
    setWindowFlags(Qt::Window);
    setWindowTitle("工单管理");
    setupUI();
    applyStyles();
    loadWorkOrders();
}

WorkOrderManagementWidget::~WorkOrderManagementWidget()
{
    delete m_workOrderManager;
}

void WorkOrderManagementWidget::setupUI()
{
    // 创建顶部工具栏
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    
    m_newButton = new QPushButton("新建工单", this);
    m_newButton->setObjectName("newButton");
    connect(m_newButton, &QPushButton::clicked, this, &WorkOrderManagementWidget::onNewWorkOrderClicked);
    
    m_assignButton = new QPushButton("分配工单", this);
    m_assignButton->setObjectName("assignButton");
    m_assignButton->setEnabled(false);  // 默认禁用，选中行后启用
    connect(m_assignButton, &QPushButton::clicked, this, &WorkOrderManagementWidget::onAssignButtonClicked);
    
    m_editButton = new QPushButton("编辑工单", this);
    m_editButton->setObjectName("editButton");
    m_editButton->setEnabled(false);  // 默认禁用，选中行后启用
    connect(m_editButton, &QPushButton::clicked, this, &WorkOrderManagementWidget::onEditButtonClicked);
    
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("按工单标题搜索...");
    m_searchEdit->setObjectName("searchEdit");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &WorkOrderManagementWidget::onSearchTextChanged);
    
    toolbarLayout->addWidget(m_newButton);
    toolbarLayout->addWidget(m_assignButton);
    toolbarLayout->addWidget(m_editButton);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(m_searchEdit);
    
    // 创建工单表格
    m_workOrderTable = new QTableWidget(this);
    m_workOrderTable->setColumnCount(6);
    QStringList headers;
    headers << "工单编号" << "工单类型" << "标题" << "状态" << "创建时间" << "执行人";
    m_workOrderTable->setHorizontalHeaderLabels(headers);
    m_workOrderTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_workOrderTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_workOrderTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 设置所有列宽度相同
    m_workOrderTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_workOrderTable->verticalHeader()->setDefaultSectionSize(50);  // 设置行高
    
    // 设置标题行样式，避免受选中状态影响
    m_workOrderTable->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
            "background-color: #6CA6CD;"
            "color: white;"
            "font-size: 18px;"
            "padding: 10px;"
            "border: none;"
        "}"
    );
    
    // 连接表格选择变化信号，启用/禁用按钮
    connect(m_workOrderTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        bool hasSelection = m_workOrderTable->currentRow() >= 0;
        
        // 分配按钮的启用状态：只有"待分配"状态的工单才能被分配
        bool canAssign = hasSelection;
        if (hasSelection) {
            WorkOrderData selectedOrder = getSelectedWorkOrder();
            // 只有"待分配"状态的工单才能被分配
            if (selectedOrder.status != "待分配") {
                canAssign = false;
            }
        }
        m_assignButton->setEnabled(canAssign);
        
        // 编辑按钮的启用状态需要考虑权限：如果工单已分配，只有adminjmh可以编辑
        bool canEdit = hasSelection;
        if (hasSelection) {
            WorkOrderData selectedOrder = getSelectedWorkOrder();
            // 如果工单已分配且当前用户不是adminjmh，禁用编辑按钮
            if (!selectedOrder.assigneeId.isEmpty() && m_currentUsername != "adminjmh") {
                canEdit = false;
            }
        }
        m_editButton->setEnabled(canEdit);
    });
    
    // 连接双击事件
    connect(m_workOrderTable, &QTableWidget::cellDoubleClicked, this, &WorkOrderManagementWidget::onTableDoubleClicked);
    
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(toolbarLayout);
    mainLayout->addWidget(m_workOrderTable);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
}

void WorkOrderManagementWidget::applyStyles()
{
    // 设置按钮高度
    m_newButton->setMinimumHeight(50);
    m_assignButton->setMinimumHeight(50);
    m_editButton->setMinimumHeight(50);
    m_searchEdit->setMinimumHeight(45);
    
    // 设置按钮蓝色样式，与主系统一致
    this->setStyleSheet(
        "QPushButton#newButton, QPushButton#assignButton, QPushButton#editButton {"
            "padding: 10px 20px;"
            "border-radius: 5px;"
            "border: none;"
            "background: #6CA6CD;"
            "color: #ffffff;"
            "font-size: 18px;"
        "}"
        "QPushButton#newButton:hover, QPushButton#assignButton:hover, QPushButton#editButton:hover {"
            "background: #5B9BD5;"
        "}"
        "QPushButton#newButton:pressed, QPushButton#assignButton:pressed, QPushButton#editButton:pressed {"
            "background: #4A8BC4;"
        "}"
        "QPushButton#newButton:disabled, QPushButton#assignButton:disabled, QPushButton#editButton:disabled {"
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
        "QTableWidget QHeaderView::section {"
            "font-size: 18px;"
            "padding: 10px;"
            "background-color: #6CA6CD;"
            "color: white;"
            "border: none;"
        "}"
        "QTableWidget::item:selected {"
            "background-color:  #6CA6CD;"
        "}"
    );
}

bool WorkOrderManagementWidget::isAdmin() const
{
    return m_currentUsername == "adminjmh";
}

void WorkOrderManagementWidget::loadWorkOrders()
{
    if (!m_workOrderManager) {
        QMessageBox::warning(this, "错误", "工单管理器未初始化！");
        return;
    }
    
    // 加载所有工单（根据用户角色自动过滤）
    m_currentWorkOrders = m_workOrderManager->getAllWorkOrders(m_currentUsername, isAdmin());
    
    if (!m_workOrderManager->getLastError().isEmpty()) {
        QMessageBox::warning(this, "加载失败", QString("加载工单数据失败：%1").arg(m_workOrderManager->getLastError()));
        return;
    }
    
    // 显示工单数据
    displayWorkOrders(m_currentWorkOrders);
    
    qDebug() << "加载工单数据完成，当前用户：" << m_currentUsername << "，工单数量：" << m_currentWorkOrders.size();
}

void WorkOrderManagementWidget::displayWorkOrders(const QList<WorkOrderData> &workOrders)
{
    // 清空表格
    m_workOrderTable->setRowCount(0);
    
    // 填充数据
    for (const WorkOrderData &data : workOrders) {
        int row = m_workOrderTable->rowCount();
        m_workOrderTable->insertRow(row);
        
        // 工单编号
        QTableWidgetItem *orderIdItem = new QTableWidgetItem(data.orderId);
        m_workOrderTable->setItem(row, 0, orderIdItem);
        
        // 工单类型
        QTableWidgetItem *orderTypeItem = new QTableWidgetItem(data.orderType);
        m_workOrderTable->setItem(row, 1, orderTypeItem);
        
        // 标题
        QTableWidgetItem *titleItem = new QTableWidgetItem(data.title);
        m_workOrderTable->setItem(row, 2, titleItem);
        
        // 状态（带颜色）
        QTableWidgetItem *statusItem = new QTableWidgetItem(data.status);
        if (data.status == "待分配") {
            statusItem->setForeground(QBrush(QColor(255, 165, 0)));  // 橙色
        } else if (data.status == "处理中") {
            statusItem->setForeground(QBrush(QColor(0, 100, 255)));  // 蓝色
        } else if (data.status == "已完成") {
            statusItem->setForeground(QBrush(QColor(0, 150, 0)));  // 绿色
        }
        m_workOrderTable->setItem(row, 3, statusItem);
        
        // 创建时间
        QString createTimeStr = data.createTime.toString("yyyy-MM-dd hh:mm:ss");
        QTableWidgetItem *createTimeItem = new QTableWidgetItem(createTimeStr);
        m_workOrderTable->setItem(row, 4, createTimeItem);
        
        // 执行人
        QTableWidgetItem *assigneeItem = new QTableWidgetItem(data.assigneeId.isEmpty() ? "未分配" : data.assigneeId);
        m_workOrderTable->setItem(row, 5, assigneeItem);
    }
    
    // 列宽已设置为均匀分布，无需调整
}

void WorkOrderManagementWidget::onNewWorkOrderClicked()
{
    if (!m_workOrderManager) {
        QMessageBox::warning(this, "错误", "工单管理器未初始化！");
        return;
    }
    
    // 打开新建工单对话框
    NewWorkOrderDialog dialog(m_workOrderManager, m_currentUsername, this);
    
    if (dialog.exec() == QDialog::Accepted) {
        // 对话框成功关闭，刷新工单列表
        loadWorkOrders();
    }
}

void WorkOrderManagementWidget::onSearchTextChanged(const QString &text)
{
    if (!m_workOrderManager) {
        return;
    }
    
    if (text.trimmed().isEmpty()) {
        // 如果搜索框为空，重新加载所有工单
        loadWorkOrders();
        return;
    }
    
    // 执行搜索
    m_currentWorkOrders = m_workOrderManager->searchWorkOrdersByTitle(text.trimmed(), m_currentUsername, isAdmin());
    
    if (!m_workOrderManager->getLastError().isEmpty()) {
        qDebug() << "搜索失败：" << m_workOrderManager->getLastError();
        return;
    }
    
    // 显示搜索结果
    displayWorkOrders(m_currentWorkOrders);
}

int WorkOrderManagementWidget::getSelectedRow() const
{
    return m_workOrderTable->currentRow();
}

WorkOrderData WorkOrderManagementWidget::getSelectedWorkOrder() const
{
    WorkOrderData emptyData;
    int row = getSelectedRow();
    
    if (row < 0 || row >= m_currentWorkOrders.size()) {
        return emptyData;
    }
    
    return m_currentWorkOrders[row];
}

void WorkOrderManagementWidget::onAssignButtonClicked()
{
    WorkOrderData selectedOrder = getSelectedWorkOrder();
    if (selectedOrder.orderId.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择一个工单！");
        return;
    }
    
    if (!m_workOrderManager || !m_authManager) {
        QMessageBox::warning(this, "错误", "系统未初始化！");
        return;
    }
    
    // 状态检查：只有"待分配"状态的工单才能被分配，即使管理员也不能分配已进入其他状态的工单
    if (selectedOrder.status != "待分配") {
        QMessageBox::warning(this, "无法分配", QString("该工单当前状态为'%1'，只有'待分配'状态的工单才能进行分配！").arg(selectedOrder.status));
        return;
    }
    
    // 打开分配工单对话框（传入当前用户名作为操作人）
    AssignWorkOrderDialog dialog(m_workOrderManager, m_authManager, 
                                 selectedOrder.orderId, 
                                 selectedOrder.assigneeId, 
                                 selectedOrder.acceptorId,
                                 m_currentUsername,  // 操作人ID
                                 this);
    
    if (dialog.exec() == QDialog::Accepted) {
        // 分配成功，刷新列表
        loadWorkOrders();
    }
}

void WorkOrderManagementWidget::onEditButtonClicked()
{
    WorkOrderData selectedOrder = getSelectedWorkOrder();
    if (selectedOrder.orderId.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择一个工单！");
        return;
    }
    
    if (!m_workOrderManager) {
        QMessageBox::warning(this, "错误", "工单管理器未初始化！");
        return;
    }
    
    // 权限检查：如果工单已分配（有执行人员），只有adminjmh可以编辑
    if (!selectedOrder.assigneeId.isEmpty() && m_currentUsername != "adminjmh") {
        QMessageBox::warning(this, "权限不足", "该工单已分配，只有管理员可以编辑！");
        return;
    }
    
    // 打开编辑工单对话框
    EditWorkOrderDialog dialog(m_workOrderManager, selectedOrder, this);
    
    if (dialog.exec() == QDialog::Accepted) {
        // 编辑成功，刷新列表
        loadWorkOrders();
    }
}

void WorkOrderManagementWidget::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    
    if (row < 0 || row >= m_currentWorkOrders.size()) {
        return;
    }
    
    // 双击时打开编辑对话框
    onEditButtonClicked();
}

