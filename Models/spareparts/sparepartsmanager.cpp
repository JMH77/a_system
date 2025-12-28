#include "sparepartsmanager.h"
#include "../database/databasemanager.h"
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QDate>
#include <QTime>

SparePartsManager::SparePartsManager(databasemanager *dbManager)
    : m_dbManager(dbManager)
    , m_lastError("")
{
}

// 生成消耗ID
QString SparePartsManager::generateConsumeId()
{
    // 格式：CS-YYYYMMDD-XXX（例如：CS-20250115-001）
    QString dateStr = QDate::currentDate().toString("yyyyMMdd");
    
    if (!m_dbManager || !m_dbManager->isConnected()) {
        return QString("CS-%1-001").arg(dateStr);
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    QSqlQuery query(db);
    
    // 查询当天已有的消耗记录数量
    query.prepare("SELECT COUNT(*) FROM WORK_ORDER_SPARE WHERE CONSUME_ID LIKE ?");
    query.addBindValue(QString("CS-%1-%%").arg(dateStr));
    
    int count = 1;
    if (query.exec() && query.next()) {
        count = query.value(0).toInt() + 1;
    }
    query.finish();
    
    // 生成序号（3位数）
    QString seqNum = QString("%1").arg(count, 3, 10, QChar('0'));
    
    return QString("CS-%1-%2").arg(dateStr).arg(seqNum);
}

// 创建备件消耗记录
bool SparePartsManager::createConsumption(const SparePartsConsumptionData &consumption)
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
    
    query.prepare(
        "INSERT INTO WORK_ORDER_SPARE ("
        "CONSUME_ID, ORDER_ID, SPARE_ID, CONSUME_QUANTITY, CONSUME_TIME, OPERATOR_ID"
        ") VALUES (?, ?, ?, ?, ?, ?)"
    );
    
    query.addBindValue(consumption.consumeId);
    query.addBindValue(consumption.orderId);
    query.addBindValue(consumption.spareId);
    query.addBindValue(consumption.consumeQuantity);
    query.addBindValue(consumption.consumeTime);
    query.addBindValue(consumption.operatorId);
    
    if (!query.exec()) {
        m_lastError = QString("创建备件消耗记录失败: %1").arg(query.lastError().text());
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

// 批量创建备件消耗记录
bool SparePartsManager::createConsumptions(const QList<SparePartsConsumptionData> &consumptions)
{
    if (consumptions.isEmpty()) {
        return true;  // 如果没有消耗记录，直接返回成功
    }
    
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
    
    // 查询已有记录数量，用于生成唯一ID
    QSqlQuery countQuery(db);
    QString dateStr = QDate::currentDate().toString("yyyyMMdd");
    countQuery.prepare("SELECT COUNT(*) FROM WORK_ORDER_SPARE WHERE CONSUME_ID LIKE ?");
    countQuery.addBindValue(QString("CS-%1-%%").arg(dateStr));
    
    int startCount = 0;
    if (countQuery.exec() && countQuery.next()) {
        startCount = countQuery.value(0).toInt();
    }
    countQuery.finish();
    
    QSqlQuery query(db);
    int index = 0;
    
    // 批量插入
    for (const SparePartsConsumptionData &consumption : consumptions) {
        // 如果ID为空，重新生成
        QString consumeId = consumption.consumeId;
        if (consumeId.isEmpty()) {
            // 生成唯一ID，使用序号避免重复
            int seqNum = startCount + index + 1;
            consumeId = QString("CS-%1-%2").arg(dateStr).arg(seqNum, 3, 10, QChar('0'));
        }
        
        query.prepare(
            "INSERT INTO WORK_ORDER_SPARE ("
            "CONSUME_ID, ORDER_ID, SPARE_ID, CONSUME_QUANTITY, CONSUME_TIME, OPERATOR_ID"
            ") VALUES (?, ?, ?, ?, ?, ?)"
        );
        
        query.addBindValue(consumeId);
        query.addBindValue(consumption.orderId);
        query.addBindValue(consumption.spareId);
        query.addBindValue(consumption.consumeQuantity);
        query.addBindValue(consumption.consumeTime);
        query.addBindValue(consumption.operatorId);
        
        if (!query.exec()) {
            m_lastError = QString("创建备件消耗记录失败: %1").arg(query.lastError().text());
            qDebug() << m_lastError;
            query.finish();
            return false;
        }
        query.finish();
        
        index++;  // 增加索引
    }
    
    // 达梦数据库需要显式提交事务
    if (!db.commit()) {
        m_lastError = QString("提交事务失败: %1").arg(db.lastError().text());
        qDebug() << m_lastError;
        return false;
    }
    
    return true;
}

// 获取工单的所有备件消耗记录
QList<SparePartsConsumptionData> SparePartsManager::getConsumptionsByOrderId(const QString &orderId)
{
    QList<SparePartsConsumptionData> consumptions;
    
    if (!m_dbManager || !m_dbManager->isConnected()) {
        m_lastError = "数据库未连接";
        return consumptions;
    }
    
    QSqlDatabase db = m_dbManager->getDatabase();
    QSqlQuery query(db);
    
    query.prepare(
        "SELECT CONSUME_ID, ORDER_ID, SPARE_ID, CONSUME_QUANTITY, CONSUME_TIME, OPERATOR_ID "
        "FROM WORK_ORDER_SPARE WHERE ORDER_ID = ? ORDER BY CONSUME_TIME DESC"
    );
    query.addBindValue(orderId);
    
    if (!query.exec()) {
        m_lastError = QString("查询备件消耗记录失败: %1").arg(query.lastError().text());
        qDebug() << m_lastError;
        query.finish();
        return consumptions;
    }
    
    while (query.next()) {
        SparePartsConsumptionData data;
        data.consumeId = query.value(0).toString();
        data.orderId = query.value(1).toString();
        data.spareId = query.value(2).toString();
        data.consumeQuantity = query.value(3).toDouble();
        data.consumeTime = query.value(4).toDateTime();
        data.operatorId = query.value(5).toString();
        consumptions.append(data);
    }
    
    query.finish();
    return consumptions;
}

// 获取错误信息
QString SparePartsManager::getLastError() const
{
    return m_lastError;
}

// 获取数据库管理器
databasemanager* SparePartsManager::getDatabaseManager() const
{
    return m_dbManager;
}

