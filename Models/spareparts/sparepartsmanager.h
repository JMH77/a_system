#ifndef SPAREPARTSMANAGER_H
#define SPAREPARTSMANAGER_H

#include <QString>
#include <QDateTime>
#include <QList>

class databasemanager;

// 备件消耗数据结构
struct SparePartsConsumptionData
{
    QString consumeId;
    QString orderId;
    QString spareId;
    double consumeQuantity;
    QDateTime consumeTime;
    QString operatorId;
};

class SparePartsManager
{
public:
    SparePartsManager(databasemanager *dbManager);
    
    // 创建备件消耗记录
    bool createConsumption(const SparePartsConsumptionData &consumption);
    
    // 批量创建备件消耗记录
    bool createConsumptions(const QList<SparePartsConsumptionData> &consumptions);
    
    // 获取工单的所有备件消耗记录
    QList<SparePartsConsumptionData> getConsumptionsByOrderId(const QString &orderId);
    
    // 获取错误信息
    QString getLastError() const;
    
    // 生成消耗ID
    QString generateConsumeId();
    
    // 获取数据库管理器（用于查询等）
    databasemanager* getDatabaseManager() const;

private:
    databasemanager *m_dbManager;
    QString m_lastError;
};

#endif // SPAREPARTSMANAGER_H

