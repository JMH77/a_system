#include "permissionmanagementwidget.h"
#include "../auth/authmanager.h"
#include "../auth/userinfo.h"
#include "../database/databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlDatabase>

PermissionManagementWidget::PermissionManagementWidget(AuthManager *authManager, QWidget *parent)
    : QDialog(parent)
    , m_authManager(authManager)
    , m_userTable(nullptr)
    , m_saveButton(nullptr)
    , m_cancelButton(nullptr)
{
    setWindowTitle("用户权限管理");
    setMinimumSize(700, 500);
    setupUI();
    loadUsers();
}

void PermissionManagementWidget::setupUI()
{
    // 创建表格
    m_userTable = new QTableWidget(this);
    m_userTable->setColumnCount(8);  // 增加了角色列
    QStringList headers;
    headers << "用户名" << "邮箱" << "角色" << "功能一" << "功能二" << "功能三" << "功能四" << "功能五";
    m_userTable->setHorizontalHeaderLabels(headers);
    m_userTable->setSelectionMode(QAbstractItemView::NoSelection);  // 禁用选择
    m_userTable->setSelectionBehavior(QAbstractItemView::SelectItems);  // 即使设置了NoSelection，这个也要设置
    m_userTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_userTable->setFocusPolicy(Qt::NoFocus);  // 禁用焦点，避免选中效果
    
    // 设置列宽调整策略
    m_userTable->horizontalHeader()->setSectionResizeMode(COL_USERNAME, QHeaderView::Fixed);
    m_userTable->horizontalHeader()->setSectionResizeMode(COL_EMAIL, QHeaderView::Fixed);
    m_userTable->horizontalHeader()->setSectionResizeMode(COL_ROLE, QHeaderView::Fixed);
    for (int i = COL_FUNC1; i <= COL_FUNC5; ++i) {
        m_userTable->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
    }
    
    // 创建按钮
    m_saveButton = new QPushButton("保存", this);
    m_cancelButton = new QPushButton("取消", this);
    
    connect(m_saveButton, &QPushButton::clicked, this, &PermissionManagementWidget::onSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &PermissionManagementWidget::onCancelClicked);
    
    // 布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_userTable);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
}

void PermissionManagementWidget::loadUsers()
{
    if (!m_authManager) {
        return;
    }
    
    // 获取数据库管理器
    databasemanager *dbManager = m_authManager->getDatabaseManager();
    if (!dbManager || !dbManager->isConnected()) {
        QMessageBox::warning(this, "错误", "数据库未连接！");
        return;
    }
    
    QSqlDatabase db = dbManager->getDatabase();
    
    // 查询所有用户（排除adminjmh，因为管理员权限不能修改），包括工单角色
    QSqlQuery query(db);
    query.prepare("SELECT userid, username, email, work_order_role FROM NowUsers WHERE username != ? ORDER BY username");
    query.addBindValue("adminjmh");
    
    QList<QPair<int, QString>> users;  // userid, username
    if (query.exec()) {
        while (query.next()) {
            int userId = query.value(0).toInt();
            QString username = query.value(1).toString();
            QString email = query.value(2).toString();
            QString workOrderRole = query.value(3).toString();  // 工单角色
            users.append(qMakePair(userId, username));
            
            // 添加到表格
            int row = m_userTable->rowCount();
            m_userTable->insertRow(row);
            
            // 用户名（只读，不可选择）
            QTableWidgetItem *usernameItem = new QTableWidgetItem(username);
            usernameItem->setFlags(usernameItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
            m_userTable->setItem(row, COL_USERNAME, usernameItem);
            
            // 邮箱（只读，不可选择）
            QTableWidgetItem *emailItem = new QTableWidgetItem(email);
            emailItem->setFlags(emailItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
            m_userTable->setItem(row, COL_EMAIL, emailItem);
            
            // 角色下拉框
            QComboBox *roleCombo = new QComboBox(this);
            roleCombo->addItem("无", "");
            roleCombo->addItem("分配人员", "分配人员");
            roleCombo->addItem("执行人员", "执行人员");
            roleCombo->addItem("检验人员", "检验人员");
            
            // 设置当前角色
            int roleIndex = roleCombo->findData(workOrderRole);
            if (roleIndex >= 0) {
                roleCombo->setCurrentIndex(roleIndex);
            } else {
                roleCombo->setCurrentIndex(0);  // 默认为"无"
            }
            
            // 连接角色变化信号，自动勾选对应的功能权限
            connect(roleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, row]() {
                updatePermissionsByRole(row);
            });
            
            m_userTable->setCellWidget(row, COL_ROLE, roleCombo);
            
            // 获取用户的功能权限
            QList<int> permissions = m_authManager->getUserFunctionPermissions(username);
            
            // 功能权限复选框（5个功能）
            for (int funcId = 1; funcId <= 5; ++funcId) {
                QCheckBox *checkBox = new QCheckBox(this);
                checkBox->setChecked(permissions.contains(funcId));
                m_userTable->setCellWidget(row, COL_FUNC1 + funcId - 1, checkBox);
            }
        }
    }
    
    // 设置列宽
    // 用户名、邮箱、角色列设置固定宽度（较宽）
    m_userTable->setColumnWidth(COL_USERNAME, 150);
    m_userTable->setColumnWidth(COL_EMAIL, 200);
    m_userTable->setColumnWidth(COL_ROLE, 120);
    
    // 功能一到功能五列使用Stretch模式，会自动均分剩余宽度
    // 已经在setupUI中设置了Stretch模式，这里不需要再设置宽度
}

// 根据角色自动勾选对应的功能权限
void PermissionManagementWidget::updatePermissionsByRole(int row)
{
    QComboBox *roleCombo = qobject_cast<QComboBox*>(m_userTable->cellWidget(row, COL_ROLE));
    if (!roleCombo) return;
    
    QString role = roleCombo->currentData().toString();
    
    // 根据角色自动勾选功能权限
    // 分配人员：功能一、功能四
    // 执行人员：功能二、功能四
    // 检验人员：功能三、功能五
    if (role == "分配人员") {
        QCheckBox *func1 = qobject_cast<QCheckBox*>(m_userTable->cellWidget(row, COL_FUNC1));
        QCheckBox *func4 = qobject_cast<QCheckBox*>(m_userTable->cellWidget(row, COL_FUNC4));
        if (func1) func1->setChecked(true);
        if (func4) func4->setChecked(true);
        QCheckBox *func2 = qobject_cast<QCheckBox*>(m_userTable->cellWidget(row, COL_FUNC2));
        QCheckBox *func3 = qobject_cast<QCheckBox*>(m_userTable->cellWidget(row, COL_FUNC3));
        QCheckBox *func5 = qobject_cast<QCheckBox*>(m_userTable->cellWidget(row, COL_FUNC5));
        if (func2) func2->setChecked(false);
        if (func3) func3->setChecked(false);
        if (func5) func5->setChecked(false);
    } else if (role == "执行人员") {
        QCheckBox *func2 = qobject_cast<QCheckBox*>(m_userTable->cellWidget(row, COL_FUNC2));
        QCheckBox *func4 = qobject_cast<QCheckBox*>(m_userTable->cellWidget(row, COL_FUNC4));
        if (func2) func2->setChecked(true);
        if (func4) func4->setChecked(true);
        QCheckBox *func1 = qobject_cast<QCheckBox*>(m_userTable->cellWidget(row, COL_FUNC1));
        QCheckBox *func3 = qobject_cast<QCheckBox*>(m_userTable->cellWidget(row, COL_FUNC3));
        QCheckBox *func5 = qobject_cast<QCheckBox*>(m_userTable->cellWidget(row, COL_FUNC5));
        if (func1) func1->setChecked(false);
        if (func3) func3->setChecked(false);
        if (func5) func5->setChecked(false);
    } else if (role == "检验人员") {
        QCheckBox *func3 = qobject_cast<QCheckBox*>(m_userTable->cellWidget(row, COL_FUNC3));
        QCheckBox *func5 = qobject_cast<QCheckBox*>(m_userTable->cellWidget(row, COL_FUNC5));
        if (func3) func3->setChecked(true);
        if (func5) func5->setChecked(true);
        QCheckBox *func1 = qobject_cast<QCheckBox*>(m_userTable->cellWidget(row, COL_FUNC1));
        QCheckBox *func2 = qobject_cast<QCheckBox*>(m_userTable->cellWidget(row, COL_FUNC2));
        QCheckBox *func4 = qobject_cast<QCheckBox*>(m_userTable->cellWidget(row, COL_FUNC4));
        if (func1) func1->setChecked(false);
        if (func2) func2->setChecked(false);
        if (func4) func4->setChecked(false);
    }
    // 如果选择"无"，不自动修改权限复选框，保持原状
}

void PermissionManagementWidget::onSaveClicked()
{
    if (!m_authManager) {
        QMessageBox::warning(this, "错误", "认证管理器未初始化！");
        return;
    }
    
    // 获取数据库管理器
    databasemanager *dbManager = m_authManager->getDatabaseManager();
    if (!dbManager || !dbManager->isConnected()) {
        QMessageBox::warning(this, "错误", "数据库未连接！");
        return;
    }
    
    QSqlDatabase db = dbManager->getDatabase();
    int successCount = 0;
    int failCount = 0;
    
    // 遍历所有行，保存权限
    for (int i = 0; i < m_userTable->rowCount(); ++i) {
        QTableWidgetItem *usernameItem = m_userTable->item(i, COL_USERNAME);
        if (!usernameItem) continue;
        
        QString username = usernameItem->text();
        
        // 获取userid
        QSqlQuery userQuery(db);
        userQuery.prepare("SELECT userid FROM NowUsers WHERE username = ?");
        userQuery.addBindValue(username);
        int userId = -1;
        if (userQuery.exec() && userQuery.next()) {
            userId = userQuery.value(0).toInt();
        }
        userQuery.finish();
        
        if (userId <= 0) continue;
        
        // 更新工单角色
        QComboBox *roleCombo = qobject_cast<QComboBox*>(m_userTable->cellWidget(i, COL_ROLE));
        if (roleCombo) {
            QString workOrderRole = roleCombo->currentData().toString();
            QSqlQuery roleUpdateQuery(db);
            roleUpdateQuery.prepare("UPDATE NowUsers SET work_order_role = ? WHERE userid = ?");
            if (workOrderRole.isEmpty()) {
                roleUpdateQuery.addBindValue(QVariant(QVariant::String));  // NULL
            } else {
                roleUpdateQuery.addBindValue(workOrderRole);
            }
            roleUpdateQuery.addBindValue(userId);
            if (roleUpdateQuery.exec()) {
                qDebug() << "更新用户角色成功:" << username << workOrderRole;
            } else {
                qDebug() << "更新用户角色失败:" << username << roleUpdateQuery.lastError().text();
            }
            roleUpdateQuery.finish();
        }
        
        // 更新功能权限
        for (int funcId = 1; funcId <= 5; ++funcId) {
            QCheckBox *checkBox = qobject_cast<QCheckBox*>(m_userTable->cellWidget(i, COL_FUNC1 + funcId - 1));
            if (checkBox) {
                bool enabled = checkBox->isChecked();
                
                // 先检查权限记录是否存在
                QSqlQuery checkQuery(db);
                checkQuery.prepare("SELECT COUNT(*) FROM NowUsersPermissions WHERE userid = ? AND function_id = ?");
                checkQuery.addBindValue(userId);
                checkQuery.addBindValue(funcId);
                bool exists = false;
                if (checkQuery.exec() && checkQuery.next()) {
                    exists = checkQuery.value(0).toInt() > 0;
                }
                checkQuery.finish();
                
                QSqlQuery updateQuery(db);
                if (exists) {
                    // 更新现有记录
                    updateQuery.prepare("UPDATE NowUsersPermissions SET enabled = ? WHERE userid = ? AND function_id = ?");
                    updateQuery.addBindValue(enabled ? 1 : 0);
                    updateQuery.addBindValue(userId);
                    updateQuery.addBindValue(funcId);
                } else {
                    // 插入新记录
                    updateQuery.prepare("INSERT INTO NowUsersPermissions (userid, function_id, enabled) VALUES (?, ?, ?)");
                    updateQuery.addBindValue(userId);
                    updateQuery.addBindValue(funcId);
                    updateQuery.addBindValue(enabled ? 1 : 0);
                }
                
                if (updateQuery.exec()) {
                    successCount++;
                } else {
                    failCount++;
                    qDebug() << "更新权限失败:" << username << "功能" << funcId << updateQuery.lastError().text();
                }
                updateQuery.finish();
            }
        }
    }
    
    // 提交事务
    if (!db.commit()) {
        QMessageBox::warning(this, "保存失败", QString("提交事务失败: %1").arg(db.lastError().text()));
        return;
    }
    
    if (failCount > 0) {
        QMessageBox::warning(this, "保存结果", 
            QString("保存完成，成功：%1项，失败：%2项").arg(successCount).arg(failCount));
    } else {
        QMessageBox::information(this, "保存成功", "所有权限已成功保存！");
        accept();  // 关闭对话框
    }
}

void PermissionManagementWidget::onCancelClicked()
{
    reject();  // 关闭对话框
}

