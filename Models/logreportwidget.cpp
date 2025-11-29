#include "logreportwidget.h"
#include "../auth/authmanager.h"
#include "../database/databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>

LogReportWidget::LogReportWidget(AuthManager *authManager, const QString &currentUsername, QWidget *parent)
    : QWidget(parent)
    , m_authManager(authManager)
    , m_currentUsername(currentUsername)
    , m_dbManager(nullptr)
    , m_newButton(nullptr)
    , m_searchEdit(nullptr)
    , m_reportTable(nullptr)
{
    // 获取数据库管理器
    if (m_authManager) {
        m_dbManager = m_authManager->getDatabaseManager();
    }
    
    // 设置为独立窗口
    setWindowFlags(Qt::Window);
    setWindowTitle("日志报告");
    setupUI();
    applyStyles();
    loadReports();
}

void LogReportWidget::setupUI()
{
    // 创建顶部工具栏
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    
    m_newButton = new QPushButton("新建报告", this);
    m_newButton->setObjectName("newButton");
    connect(m_newButton, &QPushButton::clicked, this, &LogReportWidget::onNewReportClicked);
    
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("按工单编号搜索...");
    m_searchEdit->setObjectName("searchEdit");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &LogReportWidget::onSearchTextChanged);
    
    toolbarLayout->addWidget(m_newButton);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(m_searchEdit);
    
    // 创建日志表格
    m_reportTable = new QTableWidget(this);
    m_reportTable->setColumnCount(6);
    QStringList headers;
    headers << "日志ID" << "工单编号" << "操作类型" << "操作内容" << "操作时间" << "操作人";
    m_reportTable->setHorizontalHeaderLabels(headers);
    m_reportTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_reportTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_reportTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_reportTable->horizontalHeader()->setStretchLastSection(true);
    
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(toolbarLayout);
    mainLayout->addWidget(m_reportTable);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
}

void LogReportWidget::applyStyles()
{
    m_newButton->setMinimumHeight(35);
    m_searchEdit->setMinimumHeight(35);
}

void LogReportWidget::loadReports()
{
    if (!m_dbManager || !m_dbManager->isConnected()) {
        QMessageBox::warning(this, "错误", "数据库未连接！");
        return;
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    QSqlQuery query(db);
    
    // 查询所有日志记录（根据用户角色过滤：只显示与当前用户相关的工单的日志）
    // 如果用户是管理员，显示所有日志；否则只显示相关工单的日志
    QString sql;
    bool isAdmin = (m_currentUsername == "adminjmh");
    
    if (isAdmin) {
        // 管理员可以看到所有日志
        sql = "SELECT LOG_ID, ORDER_ID, OPERATE_TYPE, OPERATE_CONTENT, OPERATE_TIME, OPERATOR_ID "
              "FROM WORK_ORDER_LOG ORDER BY OPERATE_TIME DESC";
        query.prepare(sql);
    } else {
        // 普通用户只能看到和自己相关的工单的日志（创建者、执行人、验收人）
        sql = "SELECT l.LOG_ID, l.ORDER_ID, l.OPERATE_TYPE, l.OPERATE_CONTENT, l.OPERATE_TIME, l.OPERATOR_ID "
              "FROM WORK_ORDER_LOG l "
              "INNER JOIN WORK_ORDER o ON l.ORDER_ID = o.ORDER_ID "
              "WHERE o.CREATOR_ID = ? OR o.ASSIGNEE_ID = ? OR o.ACCEPTOR_ID = ? "
              "ORDER BY l.OPERATE_TIME DESC";
        query.prepare(sql);
        query.addBindValue(m_currentUsername);
        query.addBindValue(m_currentUsername);
        query.addBindValue(m_currentUsername);
    }
    
    QList<QStringList> reports;
    
    if (query.exec()) {
        while (query.next()) {
            QStringList row;
            row << query.value(0).toString();  // LOG_ID
            row << query.value(1).toString();  // ORDER_ID
            row << query.value(2).toString();  // OPERATE_TYPE
            row << query.value(3).toString();  // OPERATE_CONTENT
            row << query.value(4).toDateTime().toString("yyyy-MM-dd hh:mm:ss");  // OPERATE_TIME
            row << query.value(5).toString();  // OPERATOR_ID
            reports.append(row);
        }
    } else {
        QMessageBox::warning(this, "加载失败", QString("加载日志记录失败：%1").arg(query.lastError().text()));
        return;
    }
    
    query.finish();
    
    m_currentReports = reports;
    displayReports(reports);
    
    qDebug() << "加载日志记录完成，当前用户：" << m_currentUsername << "，记录数量：" << reports.size();
}

void LogReportWidget::displayReports(const QList<QStringList> &reports)
{
    // 清空表格
    m_reportTable->setRowCount(0);
    
    // 填充数据
    for (const QStringList &row : reports) {
        if (row.size() < 6) continue;
        
        int tableRow = m_reportTable->rowCount();
        m_reportTable->insertRow(tableRow);
        
        m_reportTable->setItem(tableRow, 0, new QTableWidgetItem(row[0]));  // 日志ID
        m_reportTable->setItem(tableRow, 1, new QTableWidgetItem(row[1]));  // 工单编号
        m_reportTable->setItem(tableRow, 2, new QTableWidgetItem(row[2]));  // 操作类型
        m_reportTable->setItem(tableRow, 3, new QTableWidgetItem(row[3]));  // 操作内容
        m_reportTable->setItem(tableRow, 4, new QTableWidgetItem(row[4]));  // 操作时间
        m_reportTable->setItem(tableRow, 5, new QTableWidgetItem(row[5]));  // 操作人
    }
    
    // 自动调整列宽
    m_reportTable->resizeColumnsToContents();
}

void LogReportWidget::onNewReportClicked()
{
    // TODO: 打开新建报告对话框（如果需要手动创建报告的话）
    qDebug() << "点击新建报告按钮";
    QMessageBox::information(this, "提示", "新建报告功能待实现");
}

void LogReportWidget::onSearchTextChanged(const QString &text)
{
    if (text.trimmed().isEmpty()) {
        // 如果搜索框为空，重新加载所有记录
        loadReports();
        return;
    }
    
    // 执行搜索（按工单编号搜索）
    QList<QStringList> filteredReports;
    QString searchText = text.trimmed().toLower();
    
    for (const QStringList &report : m_currentReports) {
        if (report.size() >= 2) {
            // 搜索工单编号
            if (report[1].toLower().contains(searchText)) {
                filteredReports.append(report);
            }
        }
    }
    
    // 显示搜索结果
    displayReports(filteredReports);
}

