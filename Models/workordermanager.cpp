#include "workordermanager.h"
#include "../database/databasemanager.h"
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QDate>

WorkOrderManager::WorkOrderManager(databasemanager *dbManager)
    : m_dbManager(dbManager)
    , m_lastError("")
{
}

// 从查询结果构建工单数据（使用字段索引）
WorkOrderData WorkOrderManager::buildWorkOrderFromQuery(const QSqlQuery &query)
{
    WorkOrderData data;
    
    // 根据查询字段顺序使用索引访问（0-based）
    data.orderId = query.value(0).toString();          // ORDER_ID
    data.orderType = query.value(1).toString();        // ORDER_TYPE
    data.title = query.value(2).toString();            // TITLE
    data.description = query.value(3).toString();      // DESCRIPTION
    data.relatedEquipId = query.value(4).toString();   // RELATED_EQUIP_ID
    data.shipId = query.value(5).toString();           // SHIP_ID
    data.status = query.value(6).toString();           // STATUS
    data.createTime = query.value(7).toDateTime();     // CREATE_TIME
    
    if (!query.value(8).isNull()) {
        data.assignTime = query.value(8).toDateTime(); // ASSIGN_TIME
    }
    if (!query.value(9).isNull()) {
        data.actualEndTime = query.value(9).toDateTime(); // ACTUAL_END_TIME
    }
    
    data.creatorId = query.value(10).toString();       // CREATOR_ID
    data.assigneeId = query.value(11).toString();      // ASSIGNEE_ID
    data.acceptorId = query.value(12).toString();      // ACCEPTOR_ID
    data.relatedPlanId = query.value(13).toString();   // RELATED_PLAN_ID
    
    return data;
}

// 检查用户是否是管理员
bool WorkOrderManager::isAdmin(const QString &username) const
{
    return username == "adminjmh";
}

// 创建工单
bool WorkOrderManager::createWorkOrder(const WorkOrderData &workOrder)
{
    if (!m_dbManager || !m_dbManager->isConnected()) {
        m_lastError = "数据库未连接";
        return false;
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    
    // 确保数据库连接有效
    if (!db.isOpen()) {
        m_lastError = "数据库连接未打开";
        qDebug() << m_lastError;
        return false;
    }
    
    QSqlQuery query(db);
    
    query.prepare(
        "INSERT INTO WORK_ORDER ("
        "ORDER_ID, ORDER_TYPE, TITLE, DESCRIPTION, RELATED_EQUIP_ID, SHIP_ID, "
        "STATUS, CREATE_TIME, CREATOR_ID, RELATED_PLAN_ID"
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    );
    
    query.addBindValue(workOrder.orderId);
    query.addBindValue(workOrder.orderType);
    query.addBindValue(workOrder.title);
    query.addBindValue(workOrder.description);
    query.addBindValue(workOrder.relatedEquipId);
    query.addBindValue(workOrder.shipId);
    query.addBindValue(workOrder.status);
    query.addBindValue(workOrder.createTime);
    query.addBindValue(workOrder.creatorId);
    query.addBindValue(workOrder.relatedPlanId);
    
    if (!query.exec()) {
        m_lastError = QString("创建工单失败: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        query.finish();
        return false;
    }
    
    query.finish();
    
    // 达梦数据库需要显式提交事务
    if (!db.commit()) {
        m_lastError = QString("提交事务失败: %1").arg(db.lastError().text());
        qDebug() << m_lastError;
        return false;
    }
    
    // 记录创建工单日志
    QString logContent = QString("创建工单：%1（%2）").arg(workOrder.title).arg(workOrder.orderType);
    addLogRecord(workOrder.orderId, "创建工单", logContent, workOrder.creatorId);
    
    return true;
}

// 获取所有工单（根据用户角色过滤）
QList<WorkOrderData> WorkOrderManager::getAllWorkOrders(const QString &currentUsername, bool isAdmin)
{
    QList<WorkOrderData> workOrders;
    
    if (!m_dbManager || !m_dbManager->isConnected()) {
        m_lastError = "数据库未连接";
        return workOrders;
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    QSqlQuery query(db);
    
    QString sql;
    if (isAdmin || this->isAdmin(currentUsername)) {
        // 管理员可以看到所有工单
        sql = "SELECT ORDER_ID, ORDER_TYPE, TITLE, DESCRIPTION, RELATED_EQUIP_ID, SHIP_ID, STATUS, "
              "CREATE_TIME, ASSIGN_TIME, ACTUAL_END_TIME, CREATOR_ID, ASSIGNEE_ID, ACCEPTOR_ID, RELATED_PLAN_ID "
              "FROM WORK_ORDER ORDER BY CREATE_TIME DESC";
        query.prepare(sql);
    } else {
        // 普通用户只能看到和自己相关的工单（创建者、执行人、验收人）
        sql = "SELECT ORDER_ID, ORDER_TYPE, TITLE, DESCRIPTION, RELATED_EQUIP_ID, SHIP_ID, STATUS, "
              "CREATE_TIME, ASSIGN_TIME, ACTUAL_END_TIME, CREATOR_ID, ASSIGNEE_ID, ACCEPTOR_ID, RELATED_PLAN_ID "
              "FROM WORK_ORDER WHERE CREATOR_ID = ? OR ASSIGNEE_ID = ? OR ACCEPTOR_ID = ? ORDER BY CREATE_TIME DESC";
        query.prepare(sql);
        query.addBindValue(currentUsername);
        query.addBindValue(currentUsername);
        query.addBindValue(currentUsername);
    }
    
    if (!query.exec()) {
        m_lastError = QString("查询工单失败: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        query.finish();
        return workOrders;
    }
    
    while (query.next()) {
        WorkOrderData data = buildWorkOrderFromQuery(query);
        workOrders.append(data);
    }
    
    query.finish();
    return workOrders;
}

// 获取用户创建的工单
QList<WorkOrderData> WorkOrderManager::getWorkOrdersByCreator(const QString &creatorId)
{
    QList<WorkOrderData> workOrders;
    
    if (!m_dbManager || !m_dbManager->isConnected()) {
        m_lastError = "数据库未连接";
        return workOrders;
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    QSqlQuery query(db);
    
    query.prepare("SELECT ORDER_ID, ORDER_TYPE, TITLE, DESCRIPTION, RELATED_EQUIP_ID, SHIP_ID, STATUS, "
                  "CREATE_TIME, ASSIGN_TIME, ACTUAL_END_TIME, CREATOR_ID, ASSIGNEE_ID, ACCEPTOR_ID, RELATED_PLAN_ID "
                  "FROM WORK_ORDER WHERE CREATOR_ID = ? ORDER BY CREATE_TIME DESC");
    query.addBindValue(creatorId);
    
    if (!query.exec()) {
        m_lastError = QString("查询工单失败: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        query.finish();
        return workOrders;
    }
    
    while (query.next()) {
        WorkOrderData data = buildWorkOrderFromQuery(query);
        workOrders.append(data);
    }
    
    query.finish();
    return workOrders;
}

// 获取分配给用户的工单（待执行的任务）
QList<WorkOrderData> WorkOrderManager::getWorkOrdersByAssignee(const QString &assigneeId)
{
    QList<WorkOrderData> workOrders;
    
    if (!m_dbManager || !m_dbManager->isConnected()) {
        m_lastError = "数据库未连接";
        return workOrders;
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    QSqlQuery query(db);
    
    query.prepare("SELECT ORDER_ID, ORDER_TYPE, TITLE, DESCRIPTION, RELATED_EQUIP_ID, SHIP_ID, STATUS, "
                  "CREATE_TIME, ASSIGN_TIME, ACTUAL_END_TIME, CREATOR_ID, ASSIGNEE_ID, ACCEPTOR_ID, RELATED_PLAN_ID "
                  "FROM WORK_ORDER WHERE ASSIGNEE_ID = ? ORDER BY ASSIGN_TIME DESC");
    query.addBindValue(assigneeId);
    
    if (!query.exec()) {
        m_lastError = QString("查询工单失败: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        query.finish();
        return workOrders;
    }
    
    while (query.next()) {
        WorkOrderData data = buildWorkOrderFromQuery(query);
        workOrders.append(data);
    }
    
    query.finish();
    return workOrders;
}

// 获取待用户验收的工单
QList<WorkOrderData> WorkOrderManager::getWorkOrdersByAcceptor(const QString &acceptorId)
{
    QList<WorkOrderData> workOrders;
    
    if (!m_dbManager || !m_dbManager->isConnected()) {
        m_lastError = "数据库未连接";
        return workOrders;
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    QSqlQuery query(db);
    
    // 查询所有分配给当前用户验收的工单（不限制状态，因为可能处于不同阶段）
    query.prepare("SELECT ORDER_ID, ORDER_TYPE, TITLE, DESCRIPTION, RELATED_EQUIP_ID, SHIP_ID, STATUS, "
                  "CREATE_TIME, ASSIGN_TIME, ACTUAL_END_TIME, CREATOR_ID, ASSIGNEE_ID, ACCEPTOR_ID, RELATED_PLAN_ID "
                  "FROM WORK_ORDER WHERE ACCEPTOR_ID = ? ORDER BY CREATE_TIME DESC");
    query.addBindValue(acceptorId);
    
    if (!query.exec()) {
        m_lastError = QString("查询工单失败: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        query.finish();
        return workOrders;
    }
    
    while (query.next()) {
        WorkOrderData data = buildWorkOrderFromQuery(query);
        workOrders.append(data);
    }
    
    query.finish();
    return workOrders;
}

// 根据标题搜索工单
QList<WorkOrderData> WorkOrderManager::searchWorkOrdersByTitle(const QString &keyword, const QString &currentUsername, bool isAdmin)
{
    QList<WorkOrderData> workOrders;
    
    if (!m_dbManager || !m_dbManager->isConnected()) {
        m_lastError = "数据库未连接";
        return workOrders;
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    QSqlQuery query(db);
    
    QString sql;
    if (isAdmin || this->isAdmin(currentUsername)) {
        sql = "SELECT ORDER_ID, ORDER_TYPE, TITLE, DESCRIPTION, RELATED_EQUIP_ID, SHIP_ID, STATUS, "
              "CREATE_TIME, ASSIGN_TIME, ACTUAL_END_TIME, CREATOR_ID, ASSIGNEE_ID, ACCEPTOR_ID, RELATED_PLAN_ID "
              "FROM WORK_ORDER WHERE TITLE LIKE ? ORDER BY CREATE_TIME DESC";
        query.prepare(sql);
        query.addBindValue("%" + keyword + "%");
    } else {
        sql = "SELECT ORDER_ID, ORDER_TYPE, TITLE, DESCRIPTION, RELATED_EQUIP_ID, SHIP_ID, STATUS, "
              "CREATE_TIME, ASSIGN_TIME, ACTUAL_END_TIME, CREATOR_ID, ASSIGNEE_ID, ACCEPTOR_ID, RELATED_PLAN_ID "
              "FROM WORK_ORDER WHERE TITLE LIKE ? AND (CREATOR_ID = ? OR ASSIGNEE_ID = ? OR ACCEPTOR_ID = ?) ORDER BY CREATE_TIME DESC";
        query.prepare(sql);
        query.addBindValue("%" + keyword + "%");
        query.addBindValue(currentUsername);
        query.addBindValue(currentUsername);
        query.addBindValue(currentUsername);
    }
    
    if (!query.exec()) {
        m_lastError = QString("搜索工单失败: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        query.finish();
        return workOrders;
    }
    
    while (query.next()) {
        WorkOrderData data = buildWorkOrderFromQuery(query);
        workOrders.append(data);
    }
    
    query.finish();
    return workOrders;
}

// 根据工单ID获取工单
WorkOrderData WorkOrderManager::getWorkOrderById(const QString &orderId)
{
    WorkOrderData data;
    
    if (!m_dbManager || !m_dbManager->isConnected()) {
        m_lastError = "数据库未连接";
        return data;
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    QSqlQuery query(db);
    
    query.prepare("SELECT ORDER_ID, ORDER_TYPE, TITLE, DESCRIPTION, RELATED_EQUIP_ID, SHIP_ID, STATUS, "
                  "CREATE_TIME, ASSIGN_TIME, ACTUAL_END_TIME, CREATOR_ID, ASSIGNEE_ID, ACCEPTOR_ID, RELATED_PLAN_ID "
                  "FROM WORK_ORDER WHERE ORDER_ID = ?");
    query.addBindValue(orderId);
    
    if (!query.exec()) {
        m_lastError = QString("查询工单失败: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        query.finish();
        return data;
    }
    
    if (query.next()) {
        data = buildWorkOrderFromQuery(query);
    }
    
    query.finish();
    return data;
}

// 更新工单状态
bool WorkOrderManager::updateWorkOrderStatus(const QString &orderId, const QString &status)
{
    if (!m_dbManager || !m_dbManager->isConnected()) {
        m_lastError = "数据库未连接";
        return false;
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    QSqlQuery query(db);
    
    query.prepare("UPDATE WORK_ORDER SET STATUS = ? WHERE ORDER_ID = ?");
    query.addBindValue(status);
    query.addBindValue(orderId);
    
    if (!query.exec()) {
        m_lastError = QString("更新工单状态失败: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        query.finish();
        return false;
    }
    
    query.finish();
    
    if (!db.commit()) {
        m_lastError = QString("提交事务失败: %1").arg(db.lastError().text());
        qDebug() << m_lastError;
        return false;
    }
    
    return true;
}

// 执行人员完成工单
bool WorkOrderManager::completeWorkOrder(const QString &orderId, const QString &operatorId)
{
    if (!m_dbManager || !m_dbManager->isConnected()) {
        m_lastError = "数据库未连接";
        return false;
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    
    // 确保数据库连接有效
    if (!db.isOpen()) {
        m_lastError = "数据库连接未打开";
        qDebug() << m_lastError;
        return false;
    }
    
    QSqlQuery query(db);
    QDateTime currentTime = QDateTime::currentDateTime();
    
    // 更新状态为"待验收"，并更新实际结束时间
    query.prepare("UPDATE WORK_ORDER SET STATUS = ?, ACTUAL_END_TIME = ? WHERE ORDER_ID = ?");
    query.addBindValue("待验收");
    query.addBindValue(currentTime);
    query.addBindValue(orderId);
    
    if (!query.exec()) {
        m_lastError = QString("完成工单失败: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        query.finish();
        return false;
    }
    
    query.finish();
    
    // 达梦数据库需要显式提交事务
    if (!db.commit()) {
        m_lastError = QString("提交事务失败: %1").arg(db.lastError().text());
        qDebug() << m_lastError;
        return false;
    }
    
    // 记录完成工单日志
    QString actualOperator = operatorId;
    if (actualOperator.isEmpty()) {
        WorkOrderData orderData = getWorkOrderById(orderId);
        actualOperator = orderData.assigneeId;
    }
    QString logContent = QString("执行人员完成工单，状态变更为'待验收'");
    addLogRecord(orderId, "完成工单", logContent, actualOperator);
    
    return true;
}

// 检验人员验收工单
bool WorkOrderManager::acceptWorkOrder(const QString &orderId, const QString &operatorId)
{
    if (!m_dbManager || !m_dbManager->isConnected()) {
        m_lastError = "数据库未连接";
        return false;
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    
    // 确保数据库连接有效
    if (!db.isOpen()) {
        m_lastError = "数据库连接未打开";
        qDebug() << m_lastError;
        return false;
    }
    
    QSqlQuery query(db);
    
    // 更新状态为"已完成"
    query.prepare("UPDATE WORK_ORDER SET STATUS = ? WHERE ORDER_ID = ?");
    query.addBindValue("已完成");
    query.addBindValue(orderId);
    
    if (!query.exec()) {
        m_lastError = QString("验收工单失败: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        query.finish();
        return false;
    }
    
    query.finish();
    
    // 达梦数据库需要显式提交事务
    if (!db.commit()) {
        m_lastError = QString("提交事务失败: %1").arg(db.lastError().text());
        qDebug() << m_lastError;
        return false;
    }
    
    // 记录验收工单日志
    QString actualOperator = operatorId;
    if (actualOperator.isEmpty()) {
        WorkOrderData orderData = getWorkOrderById(orderId);
        actualOperator = orderData.acceptorId;
    }
    QString logContent = QString("检验人员验收工单，状态变更为'已完成'");
    addLogRecord(orderId, "验收工单", logContent, actualOperator);
    
    return true;
}

// 分配工单给执行人
bool WorkOrderManager::assignWorkOrder(const QString &orderId, const QString &assigneeId, const QString &acceptorId, const QString &operatorId)
{
    if (!m_dbManager || !m_dbManager->isConnected()) {
        m_lastError = "数据库未连接";
        return false;
    }
    
    // 获取工单当前状态，只有"待分配"状态的工单才能被分配
    WorkOrderData currentOrder = getWorkOrderById(orderId);
    if (currentOrder.orderId.isEmpty()) {
        m_lastError = "工单不存在";
        return false;
    }
    
    if (currentOrder.status != "待分配") {
        m_lastError = QString("工单当前状态为'%1'，只有'待分配'状态的工单才能进行分配！").arg(currentOrder.status);
        return false;
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    QSqlQuery query(db);
    
    QDateTime currentTime = QDateTime::currentDateTime();
    
    // 确定状态：如果有执行人员，状态改为"处理中"；否则保持"待分配"
    QString newStatus = assigneeId.isEmpty() ? "待分配" : "处理中";
    
    // 如果执行人员为空，不更新分配时间
    if (assigneeId.isEmpty()) {
        query.prepare("UPDATE WORK_ORDER SET ASSIGNEE_ID = ?, ACCEPTOR_ID = ? WHERE ORDER_ID = ?");
        query.addBindValue(assigneeId);
        query.addBindValue(acceptorId);
        query.addBindValue(orderId);
    } else {
        query.prepare("UPDATE WORK_ORDER SET ASSIGNEE_ID = ?, ACCEPTOR_ID = ?, ASSIGN_TIME = ?, STATUS = ? WHERE ORDER_ID = ?");
        query.addBindValue(assigneeId);
        query.addBindValue(acceptorId);
        query.addBindValue(currentTime);
        query.addBindValue(newStatus);
        query.addBindValue(orderId);
    }
    
    if (!query.exec()) {
        m_lastError = QString("分配工单失败: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        query.finish();
        return false;
    }
    
    query.finish();
    
    if (!db.commit()) {
        m_lastError = QString("提交事务失败: %1").arg(db.lastError().text());
        qDebug() << m_lastError;
        return false;
    }
    
    // 记录分配工单日志
    QString logContent = QString("分配工单：执行人员=%1，验收人员=%2")
                         .arg(assigneeId.isEmpty() ? "未分配" : assigneeId)
                         .arg(acceptorId.isEmpty() ? "未分配" : acceptorId);
    // 如果没有传入操作人，尝试获取创建者作为操作人
    QString actualOperator = operatorId;
    if (actualOperator.isEmpty()) {
        WorkOrderData orderData = getWorkOrderById(orderId);
        actualOperator = orderData.creatorId;
    }
    addLogRecord(orderId, "分配工单", logContent, actualOperator);
    
    return true;
}

// 更新工单信息
bool WorkOrderManager::updateWorkOrder(const QString &orderId, const WorkOrderData &workOrder)
{
    if (!m_dbManager || !m_dbManager->isConnected()) {
        m_lastError = "数据库未连接";
        return false;
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    
    // 确保数据库连接有效
    if (!db.isOpen()) {
        m_lastError = "数据库连接未打开";
        qDebug() << m_lastError;
        return false;
    }
    
    QSqlQuery query(db);
    
    // 更新可修改的字段：工单类型、标题、描述、设备ID、船舶ID、关联计划ID
    query.prepare(
        "UPDATE WORK_ORDER SET "
        "ORDER_TYPE = ?, TITLE = ?, DESCRIPTION = ?, "
        "RELATED_EQUIP_ID = ?, SHIP_ID = ?, RELATED_PLAN_ID = ? "
        "WHERE ORDER_ID = ?"
    );
    
    query.addBindValue(workOrder.orderType);
    query.addBindValue(workOrder.title);
    query.addBindValue(workOrder.description);
    query.addBindValue(workOrder.relatedEquipId);
    query.addBindValue(workOrder.shipId);
    query.addBindValue(workOrder.relatedPlanId);
    query.addBindValue(orderId);
    
    if (!query.exec()) {
        m_lastError = QString("更新工单失败: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        query.finish();
        return false;
    }
    
    query.finish();
    
    // 达梦数据库需要显式提交事务
    if (!db.commit()) {
        m_lastError = QString("提交事务失败: %1").arg(db.lastError().text());
        qDebug() << m_lastError;
        return false;
    }
    
    // 记录编辑工单日志（尝试从工单数据获取创建者，如果没有则使用空字符串）
    QString logContent = QString("编辑工单信息：%1").arg(workOrder.title);
    // 编辑操作的操作人应该是创建者，这里从原始数据获取
    WorkOrderData originalData = getWorkOrderById(orderId);
    addLogRecord(orderId, "编辑工单", logContent, originalData.creatorId);
    
    return true;
}

// 生成工单编号
QString WorkOrderManager::generateOrderId()
{
    // 格式：WO-YYYYMMDD-XXX（例如：WO-20250115-001）
    QString dateStr = QDate::currentDate().toString("yyyyMMdd");
    
    if (!m_dbManager || !m_dbManager->isConnected()) {
        return QString("WO-%1-001").arg(dateStr);
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    QSqlQuery query(db);
    
    // 查询当天已有的工单数量
    query.prepare("SELECT COUNT(*) FROM WORK_ORDER WHERE ORDER_ID LIKE ?");
    query.addBindValue(QString("WO-%1-%%").arg(dateStr));
    
    int count = 1;
    if (query.exec() && query.next()) {
        count = query.value(0).toInt() + 1;
    }
    query.finish();
    
    // 生成序号（3位数）
    QString seqNum = QString("%1").arg(count, 3, 10, QChar('0'));
    
    return QString("WO-%1-%2").arg(dateStr).arg(seqNum);
}

// 记录操作日志
bool WorkOrderManager::addLogRecord(const QString &orderId, const QString &operateType, const QString &operateContent, const QString &operatorId)
{
    if (!m_dbManager || !m_dbManager->isConnected()) {
        m_lastError = "数据库未连接";
        return false;
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    
    if (!db.isOpen()) {
        m_lastError = "数据库连接未打开";
        qDebug() << m_lastError;
        return false;
    }
    
    QSqlQuery query(db);
    QDateTime currentTime = QDateTime::currentDateTime();
    
    // 生成日志ID：LOG-YYYYMMDD-HHMMSS-XXX
    QString logId = QString("LOG-%1-%2")
                    .arg(currentTime.toString("yyyyMMdd"))
                    .arg(currentTime.toString("hhmmss"));
    
    // 查询当天已有的日志数量，生成序号
    query.prepare("SELECT COUNT(*) FROM WORK_ORDER_LOG WHERE LOG_ID LIKE ?");
    query.addBindValue(QString("LOG-%1-%%").arg(currentTime.toString("yyyyMMdd")));
    
    int count = 1;
    if (query.exec() && query.next()) {
        count = query.value(0).toInt() + 1;
    }
    query.finish();
    
    QString seqNum = QString("%1").arg(count, 3, 10, QChar('0'));
    logId = QString("LOG-%1-%2-%3")
            .arg(currentTime.toString("yyyyMMdd"))
            .arg(currentTime.toString("hhmmss"))
            .arg(seqNum);
    
    // 插入日志记录
    query.prepare(
        "INSERT INTO WORK_ORDER_LOG (LOG_ID, ORDER_ID, OPERATE_TYPE, OPERATE_CONTENT, OPERATOR_ID, OPERATE_TIME) "
        "VALUES (?, ?, ?, ?, ?, ?)"
    );
    
    query.addBindValue(logId);
    query.addBindValue(orderId);
    query.addBindValue(operateType);
    query.addBindValue(operateContent);
    query.addBindValue(operatorId);
    query.addBindValue(currentTime);
    
    if (!query.exec()) {
        m_lastError = QString("记录日志失败: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        query.finish();
        return false;
    }
    
    query.finish();
    
    // 达梦数据库需要显式提交事务
    if (!db.commit()) {
        m_lastError = QString("提交事务失败: %1").arg(db.lastError().text());
        qDebug() << m_lastError;
        return false;
    }
    
    return true;
}

// 获取错误信息
QString WorkOrderManager::getLastError() const
{
    return m_lastError;
}

// 获取数据库管理器
databasemanager* WorkOrderManager::getDatabaseManager() const
{
    return m_dbManager;
}

