#ifndef WORKORDERMANAGER_H
#define WORKORDERMANAGER_H

#include <QString>
#include <QDateTime>
#include <QList>

class databasemanager;
class QSqlQuery;

// 工单数据结构
struct WorkOrderData
{
    QString orderId;
    QString orderType;  // "常规维护" 或 "紧急维修"
    QString title;
    QString description;
    QString relatedEquipId;
    QString shipId;
    QString status;  // "待分配"、"处理中"、"已完成"
    QDateTime createTime;
    QDateTime assignTime;
    QDateTime actualEndTime;
    QString creatorId;
    QString assigneeId;
    QString acceptorId;
    QString relatedPlanId;
};

class WorkOrderManager
{
public:
    WorkOrderManager(databasemanager *dbManager);
    
    // 创建工单
    bool createWorkOrder(const WorkOrderData &workOrder);
    
    // 获取所有工单（管理员和分配人员可以看到所有工单）
    QList<WorkOrderData> getAllWorkOrders(const QString &currentUsername, bool isAdmin = false);
    
    // 获取用户创建的工单
    QList<WorkOrderData> getWorkOrdersByCreator(const QString &creatorId);
    
    // 获取分配给用户的工单（待执行的任务）
    QList<WorkOrderData> getWorkOrdersByAssignee(const QString &assigneeId);
    
    // 获取待用户验收的工单
    QList<WorkOrderData> getWorkOrdersByAcceptor(const QString &acceptorId);
    
    // 根据标题搜索工单
    QList<WorkOrderData> searchWorkOrdersByTitle(const QString &keyword, const QString &currentUsername, bool isAdmin = false);
    
    // 根据工单ID获取工单
    WorkOrderData getWorkOrderById(const QString &orderId);
    
    // 更新工单状态
    bool updateWorkOrderStatus(const QString &orderId, const QString &status);
    
    // 执行人员完成工单（状态改为"待验收"，更新实际结束时间）
    bool completeWorkOrder(const QString &orderId, const QString &operatorId = "");
    
    // 检验人员验收工单（状态改为"已完成"）
    bool acceptWorkOrder(const QString &orderId, const QString &operatorId = "");
    
    // 分配工单给执行人和验收人
    bool assignWorkOrder(const QString &orderId, const QString &assigneeId, const QString &acceptorId, const QString &operatorId = "");
    
    // 更新工单信息（允许修改部分字段）
    bool updateWorkOrder(const QString &orderId, const WorkOrderData &workOrder);
    
    // 获取错误信息
    QString getLastError() const;
    
    // 生成工单编号
    QString generateOrderId();
    
    // 记录操作日志
    bool addLogRecord(const QString &orderId, const QString &operateType, const QString &operateContent, const QString &operatorId);
    
    // 获取数据库管理器（用于其他管理器）
    databasemanager* getDatabaseManager() const;

private:
    databasemanager *m_dbManager;
    QString m_lastError;
    
    // 从查询结果构建工单数据
    WorkOrderData buildWorkOrderFromQuery(const QSqlQuery &query);
    
    // 检查用户是否是管理员
    bool isAdmin(const QString &username) const;
};

#endif // WORKORDERMANAGER_H

