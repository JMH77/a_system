#include "sparepartsconsumptionwidget.h"
#include "../workorder/workordermanager.h"
#include "sparepartsmanager.h"
#include "newconsumptiondialog.h"
#include "../auth/authmanager.h"
#include "../database/databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QTableWidgetItem>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>

SparePartsConsumptionWidget::SparePartsConsumptionWidget(AuthManager *authManager, const QString &currentUsername, QWidget *parent)
    : QWidget(parent)
    , m_authManager(authManager)
    , m_currentUsername(currentUsername)
    , m_dbManager(nullptr)
    , m_sparePartsManager(nullptr)
    , m_newButton(nullptr)
    , m_searchEdit(nullptr)
    , m_consumptionTable(nullptr)
{
    // 获取数据库管理器并创建备件管理器
    if (m_authManager) {
        m_dbManager = m_authManager->getDatabaseManager();
        if (m_dbManager) {
            m_sparePartsManager = new SparePartsManager(m_dbManager);
        }
    }
    
    // 设置为独立窗口
    setWindowFlags(Qt::Window);
    setWindowTitle("备件消耗");
    setupUI();
    applyStyles();
    loadConsumptions();
}

SparePartsConsumptionWidget::~SparePartsConsumptionWidget()
{
    delete m_sparePartsManager;
}

void SparePartsConsumptionWidget::setupUI()
{
    // 创建顶部工具栏
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    
    m_newButton = new QPushButton("新建消耗记录", this);
    m_newButton->setObjectName("newButton");
    connect(m_newButton, &QPushButton::clicked, this, &SparePartsConsumptionWidget::onNewConsumptionClicked);
    
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("按工单编号或备件ID搜索...");
    m_searchEdit->setObjectName("searchEdit");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &SparePartsConsumptionWidget::onSearchTextChanged);
    
    toolbarLayout->addWidget(m_newButton);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(m_searchEdit);
    
    // 创建消耗记录表格
    m_consumptionTable = new QTableWidget(this);
    m_consumptionTable->setColumnCount(6);
    QStringList headers;
    headers << "消耗ID" << "工单编号" << "备件ID" << "消耗数量" << "消耗时间" << "操作人";
    m_consumptionTable->setHorizontalHeaderLabels(headers);
    m_consumptionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_consumptionTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_consumptionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 设置所有列宽度相同
    m_consumptionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(toolbarLayout);
    mainLayout->addWidget(m_consumptionTable);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
}

void SparePartsConsumptionWidget::applyStyles()
{
    m_newButton->setMinimumHeight(35);
    m_searchEdit->setMinimumHeight(35);
    
    // 设置按钮蓝色样式，与主系统一致
    this->setStyleSheet(
        "QPushButton#newButton {"
            "padding: 8px 16px;"
            "border-radius: 5px;"
            "border: none;"
            "background: #6CA6CD;"
            "color: #ffffff;"
            "font-size: 12px;"
        "}"
        "QPushButton#newButton:hover {"
            "background: #5B9BD5;"
        "}"
        "QPushButton#newButton:pressed {"
            "background: #4A8BC4;"
        "}"
        "QPushButton#newButton:disabled {"
            "background: #CCCCCC;"
            "color: #888888;"
        "}"
    );
}

void SparePartsConsumptionWidget::loadConsumptions()
{
    if (!m_dbManager || !m_dbManager->isConnected()) {
        QMessageBox::warning(this, "错误", "数据库未连接！");
        return;
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    QSqlQuery query(db);
    
    // 先获取当前用户相关的工单ID列表
    // 管理员可以看到所有记录，普通用户只能看到自己相关的工单
    QString sql;
    bool isAdmin = (m_currentUsername == "adminjmh");
    
    if (isAdmin) {
        // 管理员可以看到所有备件消耗记录
        sql = "SELECT CONSUME_ID, ORDER_ID, SPARE_ID, CONSUME_QUANTITY, CONSUME_TIME, OPERATOR_ID "
              "FROM WORK_ORDER_SPARE ORDER BY CONSUME_TIME DESC";
        query.prepare(sql);
    } else {
        // 普通用户只能看到和自己相关的工单的备件消耗记录
        sql = "SELECT DISTINCT s.CONSUME_ID, s.ORDER_ID, s.SPARE_ID, s.CONSUME_QUANTITY, "
              "s.CONSUME_TIME, s.OPERATOR_ID "
              "FROM WORK_ORDER_SPARE s "
              "INNER JOIN WORK_ORDER o ON s.ORDER_ID = o.ORDER_ID "
              "WHERE o.CREATOR_ID = ? OR o.ASSIGNEE_ID = ? OR o.ACCEPTOR_ID = ? "
              "ORDER BY s.CONSUME_TIME DESC";
        query.prepare(sql);
        query.addBindValue(m_currentUsername);
        query.addBindValue(m_currentUsername);
        query.addBindValue(m_currentUsername);
    }
    
    QList<QStringList> consumptions;
    
    if (query.exec()) {
        while (query.next()) {
            QStringList row;
            row << query.value(0).toString();  // CONSUME_ID
            row << query.value(1).toString();  // ORDER_ID
            row << query.value(2).toString();  // SPARE_ID
            row << query.value(3).toString();  // CONSUME_QUANTITY
            row << query.value(4).toDateTime().toString("yyyy-MM-dd hh:mm:ss");  // CONSUME_TIME
            row << query.value(5).toString();  // OPERATOR_ID
            consumptions.append(row);
        }
    } else {
        QMessageBox::warning(this, "加载失败", QString("加载备件消耗记录失败：%1").arg(query.lastError().text()));
        return;
    }
    
    query.finish();
    
    m_currentConsumptions = consumptions;
    displayConsumptions(consumptions);
    
    qDebug() << "加载备件消耗记录完成，当前用户：" << m_currentUsername << "，记录数量：" << consumptions.size();
}

void SparePartsConsumptionWidget::displayConsumptions(const QList<QStringList> &consumptions)
{
    // 清空表格
    m_consumptionTable->setRowCount(0);
    
    // 填充数据
    for (const QStringList &row : consumptions) {
        if (row.size() < 6) continue;
        
        int tableRow = m_consumptionTable->rowCount();
        m_consumptionTable->insertRow(tableRow);
        
        m_consumptionTable->setItem(tableRow, 0, new QTableWidgetItem(row[0]));  // 消耗ID
        m_consumptionTable->setItem(tableRow, 1, new QTableWidgetItem(row[1]));  // 工单编号
        m_consumptionTable->setItem(tableRow, 2, new QTableWidgetItem(row[2]));  // 备件ID
        m_consumptionTable->setItem(tableRow, 3, new QTableWidgetItem(row[3]));  // 消耗数量
        m_consumptionTable->setItem(tableRow, 4, new QTableWidgetItem(row[4]));  // 消耗时间
        m_consumptionTable->setItem(tableRow, 5, new QTableWidgetItem(row[5]));  // 操作人
    }
    
    // 列宽已设置为均匀分布，无需调整
}

void SparePartsConsumptionWidget::onNewConsumptionClicked()
{
    if (!m_sparePartsManager || !m_authManager) {
        QMessageBox::warning(this, "错误", "系统未初始化！");
        return;
    }
    
    // 打开新建消耗记录对话框
    NewConsumptionDialog dialog(m_sparePartsManager, m_authManager, m_currentUsername, this);
    
    if (dialog.exec() == QDialog::Accepted) {
        // 对话框成功关闭，刷新消耗记录列表
        loadConsumptions();
    }
}

void SparePartsConsumptionWidget::onSearchTextChanged(const QString &text)
{
    if (text.trimmed().isEmpty()) {
        // 如果搜索框为空，重新加载所有记录
        loadConsumptions();
        return;
    }
    
    // 执行搜索
    QList<QStringList> filteredConsumptions;
    QString searchText = text.trimmed().toLower();
    
    for (const QStringList &consumption : m_currentConsumptions) {
        // 搜索工单编号或备件ID
        if (consumption.size() >= 3) {
            if (consumption[1].toLower().contains(searchText) ||  // ORDER_ID
                consumption[2].toLower().contains(searchText)) {  // SPARE_ID
                filteredConsumptions.append(consumption);
            }
        }
    }
    
    // 显示搜索结果
    displayConsumptions(filteredConsumptions);
}

