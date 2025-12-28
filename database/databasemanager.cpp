#include "databasemanager.h"
#include "../config/configmanager.h"
#include <QCryptographicHash>
#include <QDateTime>

databasemanager::databasemanager(configmanager *config)
    :m_configManager(config),
    m_lastError("")
{
    
}

databasemanager::~databasemanager() {
    if(m_db.isOpen()) {
        m_db.close();
    }
}

//建立数据库连接
bool databasemanager::connectDatabase(){
    // 检查配置管理器
    if (!m_configManager) {
        m_lastError = "配置管理器未设置";
        qDebug() << m_lastError;
        return false;
    }

    if (!m_configManager->isInitialized()) {
        m_lastError = "配置管理器未初始化";
        qDebug() << m_lastError;
        return false;
    }

    // 检查是否已经连接
    if (m_db.isOpen()) {
        qDebug() << "数据库已连接";
        return true;
    }

    // 从配置管理器读取数据库配置
    QString dbType = m_configManager->getDbType();
    QMap<QString, QString> dbConfig = m_configManager->getDbConfig();

    // 使用QODBC驱动连接达梦数据库
    QString driverName = "QODBC";
    
    // 创建数据库连接
    m_db = QSqlDatabase::addDatabase(driverName);

    // 设置达梦数据库连接参数
    m_db.setHostName(dbConfig.value("Host", "localhost"));
    m_db.setPort(dbConfig.value("Port", "5236").toInt());
    m_db.setDatabaseName(dbConfig.value("DatabaseName", ""));
    m_db.setUserName(dbConfig.value("UID", ""));
    m_db.setPassword(dbConfig.value("Password", ""));

    // 打开连接
    if (!m_db.open()) {
        m_lastError = QString("数据库连接失败: %1").arg(m_db.lastError().text());
        qDebug() << m_lastError;
        return false;
    }

    qDebug() << "数据库连接成功";
    return true;
}



//判断数据库是否连接
bool databasemanager::isConnected() const{
    return m_db.isOpen();
}


//返回错误信息
QString databasemanager::getLastError() const{
    return m_lastError;
}


//断开连接
bool databasemanager::disconnected(){
    if (m_db.isOpen()) {
        m_db.close();
        qDebug() << "数据库连接已断开";
        return true;
    } else {
        m_lastError = "数据库未连接";
        qDebug() << m_lastError;
        return false;
    }
}

//初始化用户表
bool databasemanager::initUserTable()
{
    if (!m_db.isOpen()) {
        m_lastError = "数据库未连接";
        qDebug() << m_lastError;
        return false;
    }
    
    // 创建用户表（包含role_type字段）
    QString createTableSQL = 
        "CREATE TABLE IF NOT EXISTS NowUsers ("
        "userid INT PRIMARY KEY IDENTITY, "
        "username VARCHAR(100) UNIQUE NOT NULL, "
        "password VARCHAR(255) NOT NULL, "
        "email VARCHAR(255), "
        "name VARCHAR(100), "
        "role_type INT DEFAULT 2"
        ")";
    
    QSqlQuery query(m_db);
    if (!query.exec(createTableSQL)) {
        QString errorText = query.lastError().text();
        // 如果表已存在，视为成功
        if (errorText.contains("已存在") || errorText.contains("already exists")) {
            qDebug() << "用户表已存在，跳过创建";
        } else {
            m_lastError = QString("创建用户表失败: %1").arg(errorText);
            qDebug() << m_lastError;
            return false;
        }
    } else {
        qDebug() << "用户表创建成功";
    }
    query.finish();
    
    // 初始化超级管理员 adminjmh
    // 先检查是否存在
    QSqlQuery checkAdminQuery(m_db);
    checkAdminQuery.prepare("SELECT COUNT(*) FROM NowUsers WHERE username = ?");
    checkAdminQuery.addBindValue("adminjmh");
    bool adminExists = false;
    if (checkAdminQuery.exec() && checkAdminQuery.next()) {
        int count = checkAdminQuery.value(0).toInt();
        adminExists = (count > 0);
    }
    checkAdminQuery.finish();
    
    if (!adminExists) {
        // 创建超级管理员，密码为adminjmh123（MD5加密）
        QSqlQuery insertAdminQuery(m_db);
        QString password = "adminjmh123";
        QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5);
        QString passwordHash = hash.toHex();
        
        insertAdminQuery.prepare("INSERT INTO NowUsers (username, password, email, name, role_type) VALUES (?, ?, ?, ?, ?)");
        insertAdminQuery.addBindValue("adminjmh");
        insertAdminQuery.addBindValue(passwordHash);
        insertAdminQuery.addBindValue("");
        insertAdminQuery.addBindValue("超级管理员");
        insertAdminQuery.addBindValue(1);  // role_type = 1 (admin)
        
        if (insertAdminQuery.exec()) {
            qDebug() << "超级管理员adminjmh创建成功";
            // 提交事务
            if (!m_db.commit()) {
                qDebug() << "提交事务失败:" << m_db.lastError().text();
            }
        } else {
            qDebug() << "创建超级管理员失败:" << insertAdminQuery.lastError().text();
        }
        insertAdminQuery.finish();
    } else {
        qDebug() << "超级管理员adminjmh已存在";
    }
    
    qDebug() << "用户表初始化成功";
    return true;
}

//初始化用户权限表
bool databasemanager::initUserPermissionsTable()
{
    if (!m_db.isOpen()) {
        m_lastError = "数据库未连接";
        qDebug() << m_lastError;
        return false;
    }
    
    // 创建用户权限表
    QString createTableSQL = 
        "CREATE TABLE IF NOT EXISTS NowUsersPermissions ("
        "permissionid INT PRIMARY KEY IDENTITY, "
        "userid INT NOT NULL, "
        "function_id INT NOT NULL, "
        "enabled INT DEFAULT 0, "
        "FOREIGN KEY (userid) REFERENCES NowUsers(userid) ON DELETE CASCADE, "
        "UNIQUE(userid, function_id)"
        ")";
    
    QSqlQuery query(m_db);
    if (!query.exec(createTableSQL)) {
        QString errorText = query.lastError().text();
        // 如果表已存在，视为成功
        if (errorText.contains("已存在") || errorText.contains("already exists")) {
            qDebug() << "用户权限表已存在，跳过创建";
        } else {
            m_lastError = QString("创建用户权限表失败: %1").arg(errorText);
            qDebug() << m_lastError;
            return false;
        }
    } else {
        qDebug() << "用户权限表创建成功";
    }
    query.finish();
    
    // 为超级管理员adminjmh初始化所有权限（5个功能全部启用）
    QSqlQuery adminQuery(m_db);
    adminQuery.prepare("SELECT userid FROM NowUsers WHERE username = ?");
    adminQuery.addBindValue("adminjmh");
    int adminUserId = -1;
    if (adminQuery.exec() && adminQuery.next()) {
        adminUserId = adminQuery.value(0).toInt();
    }
    adminQuery.finish();
    
    if (adminUserId > 0) {
        // 为adminjmh添加5个功能的权限（全部启用）
        for (int funcId = 1; funcId <= 5; ++funcId) {
            // 先检查权限是否已存在
            QSqlQuery checkPermQuery(m_db);
            checkPermQuery.prepare("SELECT COUNT(*) FROM NowUsersPermissions WHERE userid = ? AND function_id = ?");
            checkPermQuery.addBindValue(adminUserId);
            checkPermQuery.addBindValue(funcId);
            
            bool permExists = false;
            if (checkPermQuery.exec() && checkPermQuery.next()) {
                int count = checkPermQuery.value(0).toInt();
                permExists = (count > 0);
            }
            checkPermQuery.finish();
            
            // 如果权限不存在，才插入
            if (!permExists) {
                QSqlQuery permQuery(m_db);
                permQuery.prepare("INSERT INTO NowUsersPermissions (userid, function_id, enabled) VALUES (?, ?, ?)");
                permQuery.addBindValue(adminUserId);
                permQuery.addBindValue(funcId);
                permQuery.addBindValue(1);  // 启用
                
                if (!permQuery.exec()) {
                    QString errorText = permQuery.lastError().text();
                    qDebug() << "为adminjmh添加功能" << funcId << "权限失败:" << errorText;
                }
                permQuery.finish();
            }
        }
        
        // 提交事务
        if (!m_db.commit()) {
            qDebug() << "提交权限初始化事务失败:" << m_db.lastError().text();
        } else {
            qDebug() << "超级管理员权限初始化成功";
        }
    } else {
        qDebug() << "未找到adminjmh用户，跳过权限初始化";
    }
    
    qDebug() << "用户权限表初始化成功";
    return true;
}

//获取数据库连接
QSqlDatabase databasemanager::getDatabase() const
{
    return m_db;
}

//获取数据库类型（只支持达梦数据库）
QString databasemanager::getDbType() const
{
    return "DM";
}

//扩展用户表，添加工单角色字段
bool databasemanager::addWorkOrderRoleToUserTable()
{
    if (!m_db.isOpen()) {
        m_lastError = "数据库未连接";
        qDebug() << m_lastError;
        return false;
    }
    
    // 检查字段是否已存在（达梦数据库）
    QSqlQuery checkQuery(m_db);
    QString checkColumnSQL = "SELECT COUNT(*) FROM USER_TAB_COLUMNS WHERE TABLE_NAME = 'NOWUSERS' AND COLUMN_NAME = 'WORK_ORDER_ROLE'";
    
    bool columnExists = false;
    if (checkQuery.exec(checkColumnSQL)) {
        if (checkQuery.next()) {
            columnExists = checkQuery.value(0).toInt() > 0;
        }
    }
    checkQuery.finish();
    
    if (columnExists) {
        qDebug() << "工单角色字段已存在，跳过添加";
        return true;
    }
    
    // 添加字段
    QSqlQuery addColumnQuery(m_db);
    QString addColumnSQL = "ALTER TABLE NowUsers ADD work_order_role VARCHAR(50)";
    
    if (!addColumnQuery.exec(addColumnSQL)) {
        m_lastError = QString("添加工单角色字段失败: %1").arg(addColumnQuery.lastError().text());
        qDebug() << m_lastError;
        return false;
    }
    
    qDebug() << "工单角色字段添加成功";
    return true;
}

//初始化工单主表
bool databasemanager::initWorkOrderTable()
{
    if (!m_db.isOpen()) {
        m_lastError = "数据库未连接";
        qDebug() << m_lastError;
        return false;
    }
    
    // 达梦数据库语法
    QString createTableSQL = 
        "CREATE TABLE IF NOT EXISTS WORK_ORDER ("
        "ORDER_ID VARCHAR(50) PRIMARY KEY, "
        "ORDER_TYPE VARCHAR(50) NOT NULL, "
        "TITLE VARCHAR(200) NOT NULL, "
        "DESCRIPTION CLOB, "
        "RELATED_EQUIP_ID VARCHAR(50), "
        "SHIP_ID VARCHAR(50), "
        "STATUS VARCHAR(50) NOT NULL, "
        "CREATE_TIME TIMESTAMP(6) NOT NULL, "
        "ASSIGN_TIME TIMESTAMP(6), "
        "ACTUAL_END_TIME TIMESTAMP(6), "
        "CREATOR_ID VARCHAR(50) NOT NULL, "
        "ASSIGNEE_ID VARCHAR(50), "
        "ACCEPTOR_ID VARCHAR(50), "
        "RELATED_PLAN_ID VARCHAR(50)"
        ")";
    
    QSqlQuery query(m_db);
    if (!query.exec(createTableSQL)) {
        QString errorText = query.lastError().text();
        if (errorText.contains("已存在") || errorText.contains("already exists")) {
            qDebug() << "工单主表已存在，跳过创建";
        } else {
            m_lastError = QString("创建工单主表失败: %1").arg(errorText);
            qDebug() << m_lastError;
            return false;
        }
    } else {
        qDebug() << "工单主表创建成功";
    }
    query.finish();
    
    return true;
}

//初始化工单备件消耗表
bool databasemanager::initWorkOrderSpareTable()
{
    if (!m_db.isOpen()) {
        m_lastError = "数据库未连接";
        qDebug() << m_lastError;
        return false;
    }
    
    // 达梦数据库语法
    QString createTableSQL = 
        "CREATE TABLE IF NOT EXISTS WORK_ORDER_SPARE ("
        "CONSUME_ID VARCHAR(50) PRIMARY KEY, "
        "ORDER_ID VARCHAR(50) NOT NULL, "
        "SPARE_ID VARCHAR(50) NOT NULL, "
        "CONSUME_QUANTITY NUMBER(18,4) NOT NULL, "
        "CONSUME_TIME TIMESTAMP(6) NOT NULL, "
        "OPERATOR_ID VARCHAR(50), "
        "FOREIGN KEY (ORDER_ID) REFERENCES WORK_ORDER(ORDER_ID) ON DELETE CASCADE"
        ")";
    
    QSqlQuery query(m_db);
    if (!query.exec(createTableSQL)) {
        QString errorText = query.lastError().text();
        if (errorText.contains("已存在") || errorText.contains("already exists")) {
            qDebug() << "工单备件消耗表已存在，跳过创建";
        } else {
            m_lastError = QString("创建工单备件消耗表失败: %1").arg(errorText);
            qDebug() << m_lastError;
            return false;
        }
    } else {
        qDebug() << "工单备件消耗表创建成功";
    }
    query.finish();
    
    return true;
}

//初始化工单检测报表表
bool databasemanager::initWorkOrderReportTable()
{
    if (!m_db.isOpen()) {
        m_lastError = "数据库未连接";
        qDebug() << m_lastError;
        return false;
    }
    
    // 达梦数据库语法
    QString createTableSQL = 
        "CREATE TABLE IF NOT EXISTS WORK_ORDER_REPORT ("
        "REPORT_ID VARCHAR(50) PRIMARY KEY, "
        "ORDER_ID VARCHAR(50) NOT NULL, "
        "REPORT_TYPE VARCHAR(50), "
        "REPORT_CONTENT CLOB, "
        "ATTACHMENT_IDS VARCHAR(4000), "
        "REPORTER_ID VARCHAR(50), "
        "REPORT_TIME TIMESTAMP(6) NOT NULL, "
        "FOREIGN KEY (ORDER_ID) REFERENCES WORK_ORDER(ORDER_ID) ON DELETE CASCADE"
        ")";
    
    QSqlQuery query(m_db);
    if (!query.exec(createTableSQL)) {
        QString errorText = query.lastError().text();
        if (errorText.contains("已存在") || errorText.contains("already exists")) {
            qDebug() << "工单检测报表表已存在，跳过创建";
        } else {
            m_lastError = QString("创建工单检测报表表失败: %1").arg(errorText);
            qDebug() << m_lastError;
            return false;
        }
    } else {
        qDebug() << "工单检测报表表创建成功";
    }
    query.finish();
    
    return true;
}

//初始化工单操作日志表
bool databasemanager::initWorkOrderLogTable()
{
    if (!m_db.isOpen()) {
        m_lastError = "数据库未连接";
        qDebug() << m_lastError;
        return false;
    }
    
    // 达梦数据库语法
    QString createTableSQL = 
        "CREATE TABLE IF NOT EXISTS WORK_ORDER_LOG ("
        "LOG_ID VARCHAR(50) PRIMARY KEY, "
        "ORDER_ID VARCHAR(50) NOT NULL, "
        "OPERATE_TYPE VARCHAR(50) NOT NULL, "
        "OPERATE_CONTENT VARCHAR(1000), "
        "OPERATOR_ID VARCHAR(50), "
        "OPERATE_TIME TIMESTAMP(6) NOT NULL, "
        "FOREIGN KEY (ORDER_ID) REFERENCES WORK_ORDER(ORDER_ID) ON DELETE CASCADE"
        ")";
    
    QSqlQuery query(m_db);
    if (!query.exec(createTableSQL)) {
        QString errorText = query.lastError().text();
        if (errorText.contains("已存在") || errorText.contains("already exists")) {
            qDebug() << "工单操作日志表已存在，跳过创建";
        } else {
            m_lastError = QString("创建工单操作日志表失败: %1").arg(errorText);
            qDebug() << m_lastError;
            return false;
        }
    } else {
        qDebug() << "工单操作日志表创建成功";
    }
    query.finish();
    
    return true;
}

//初始化工单数据库触发器（监测用户角色变化）
bool databasemanager::initWorkOrderTriggers()
{
    if (!m_db.isOpen()) {
        m_lastError = "数据库未连接";
        qDebug() << m_lastError;
        return false;
    }
    
    QSqlQuery query(m_db);
    
    // 首先创建一个系统工单用于记录用户角色变化（如果不存在）
    // 这个工单ID用于记录系统操作日志
    QString systemOrderId = "SYS-USER-ROLE";
    QSqlQuery checkOrderQuery(m_db);
    checkOrderQuery.prepare("SELECT COUNT(*) FROM WORK_ORDER WHERE ORDER_ID = ?");
    checkOrderQuery.addBindValue(systemOrderId);
    bool systemOrderExists = false;
    if (checkOrderQuery.exec() && checkOrderQuery.next()) {
        systemOrderExists = checkOrderQuery.value(0).toInt() > 0;
    }
    checkOrderQuery.finish();
    
    if (!systemOrderExists) {
        // 创建系统工单用于记录用户角色变化
        QSqlQuery createSystemOrderQuery(m_db);
        createSystemOrderQuery.prepare(
            "INSERT INTO WORK_ORDER ("
            "ORDER_ID, ORDER_TYPE, TITLE, DESCRIPTION, STATUS, CREATE_TIME, CREATOR_ID"
            ") VALUES (?, ?, ?, ?, ?, ?, ?)"
        );
        createSystemOrderQuery.addBindValue(systemOrderId);
        createSystemOrderQuery.addBindValue("系统操作");
        createSystemOrderQuery.addBindValue("系统用户角色管理");
        createSystemOrderQuery.addBindValue("用于记录用户工单角色变化的系统工单");
        createSystemOrderQuery.addBindValue("已完成");
        createSystemOrderQuery.addBindValue(QDateTime::currentDateTime());
        createSystemOrderQuery.addBindValue("system");
        
        if (!createSystemOrderQuery.exec()) {
            qDebug() << "创建系统工单失败（可能已存在）:" << createSystemOrderQuery.lastError().text();
        } else {
            m_db.commit();
            qDebug() << "系统工单创建成功，用于记录用户角色变化";
        }
        createSystemOrderQuery.finish();
    }
    
    // 监测触发器：监测用户工单角色变化并自动记录日志
    // 当NowUsers表的work_order_role字段发生变化时，自动在WORK_ORDER_LOG表中插入一条监测日志
    QString triggerSQL = 
        "CREATE OR REPLACE TRIGGER TRG_USER_ROLE_CHANGE_MONITOR\n"
        "AFTER UPDATE OF work_order_role ON NowUsers\n"
        "FOR EACH ROW\n"
        "WHEN (NVL(OLD.work_order_role, '') != NVL(NEW.work_order_role, ''))\n"
        "DECLARE\n"
        "    v_log_id VARCHAR(50);\n"
        "    v_seq_num INT := 1;\n"
        "    v_old_role VARCHAR(50);\n"
        "    v_new_role VARCHAR(50);\n"
        "BEGIN\n"
        "    -- 处理NULL值\n"
        "    v_old_role := NVL(:OLD.work_order_role, '无');\n"
        "    v_new_role := NVL(:NEW.work_order_role, '无');\n"
        "    \n"
        "    -- 生成日志ID：LOG-YYYYMMDD-HHMMSS-序号\n"
        "    BEGIN\n"
        "        SELECT COUNT(*) + 1 INTO v_seq_num\n"
        "        FROM WORK_ORDER_LOG\n"
        "        WHERE LOG_ID LIKE 'LOG-' || TO_CHAR(SYSDATE, 'YYYYMMDD') || '-%';\n"
        "    EXCEPTION\n"
        "        WHEN OTHERS THEN\n"
        "            v_seq_num := 1;\n"
        "    END;\n"
        "    \n"
        "    IF v_seq_num IS NULL THEN\n"
        "        v_seq_num := 1;\n"
        "    END IF;\n"
        "    \n"
        "    v_log_id := 'LOG-' || TO_CHAR(SYSDATE, 'YYYYMMDD') || '-' || \n"
        "                TO_CHAR(SYSDATE, 'HH24MISS') || '-' || \n"
        "                LPAD(TO_CHAR(v_seq_num), 3, '0');\n"
        "    \n"
        "    -- 插入用户角色变化监测日志\n"
        "    INSERT INTO WORK_ORDER_LOG (\n"
        "        LOG_ID, ORDER_ID, OPERATE_TYPE, OPERATE_CONTENT, OPERATOR_ID, OPERATE_TIME\n"
        "    ) VALUES (\n"
        "        v_log_id,\n"
        "        'SYS-USER-ROLE',\n"
        "        '角色变化监测',\n"
        "        '用户 [' || :NEW.username || '] 的工单角色从 [' || v_old_role || '] 变更为 [' || v_new_role || ']',\n"
        "        :NEW.username,\n"
        "        SYSDATE\n"
        "    );\n"
        "END;";
    
    if (!query.exec(triggerSQL)) {
        QString errorText = query.lastError().text();
        // 触发器已存在时，删除后重新创建
        if (errorText.contains("已存在") || errorText.contains("already exists")) {
            qDebug() << "触发器TRG_USER_ROLE_CHANGE_MONITOR已存在，重新创建";
            query.exec("DROP TRIGGER TRG_USER_ROLE_CHANGE_MONITOR");
            query.finish();
            if (!query.exec(triggerSQL)) {
                m_lastError = QString("创建用户角色变化监测触发器失败: %1").arg(query.lastError().text());
                qDebug() << m_lastError;
                query.finish();
                return false;
            }
        } else {
            m_lastError = QString("创建用户角色变化监测触发器失败: %1").arg(errorText);
            qDebug() << m_lastError;
            query.finish();
            return false;
        }
    }
    query.finish();
    qDebug() << "用户角色变化监测触发器创建成功（自动监测并记录work_order_role字段变化）";
    
    return true;
}






















