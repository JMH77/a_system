#include "databasemanager.h"
#include "../config/configmanager.h"
#include <QCryptographicHash>

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
    // 步骤1：检查配置管理器
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

    // 步骤2：检查是否已经连接
    if (m_db.isOpen()) {
        qDebug() << "数据库已连接";
        return true;
    }

    // 步骤3：从配置管理器读取数据库配置
    QString dbType = m_configManager->getDbType();
    QMap<QString, QString> dbConfig = m_configManager->getDbConfig();

    //步骤4：根据数据库类型创建连接
    QString driverName;
    if(dbType.toUpper() == "SQLITE" || dbType.toUpper() == "QSQLITE"){
        driverName = "QSQLITE";
    } else if(dbType.toUpper() == "DM"){
        driverName = "QODBC";
    } else{
        m_lastError = QString("不支持的数据库类型: %1").arg(dbType);
        qDebug() << m_lastError;
        return false;
    }

    // 步骤5：创建数据库连接
    m_db = QSqlDatabase::addDatabase(driverName);

    // 步骤6：设置连接参数
    if (driverName == "QSQLITE") {
        // SQLite 只需要数据库文件路径
        QString dbName = dbConfig.value("DatabaseName", "learn1.db");
        m_db.setDatabaseName(dbName);
    } else {
        // 其他数据库需要更多参数
        m_db.setHostName(dbConfig.value("Host", "localhost"));
        m_db.setPort(dbConfig.value("Port", "5236").toInt());
        m_db.setDatabaseName(dbConfig.value("DatabaseName", ""));
        m_db.setUserName(dbConfig.value("UID", ""));
        m_db.setPassword(dbConfig.value("Password", ""));
    }

    // 步骤7：打开连接
    if (!m_db.open()) {
        m_lastError = QString("数据库连接失败: %1").arg(m_db.lastError().text());
        qDebug() << m_lastError;
        return false;
    }


    // 步骤8：连接成功
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
            QSqlQuery permQuery(m_db);
            permQuery.prepare("INSERT INTO NowUsersPermissions (userid, function_id, enabled) VALUES (?, ?, ?)");
            permQuery.addBindValue(adminUserId);
            permQuery.addBindValue(funcId);
            permQuery.addBindValue(1);  // 启用
            
            if (!permQuery.exec()) {
                QString errorText = permQuery.lastError().text();
                // 如果已存在，忽略错误
                if (errorText.contains("唯一") || errorText.contains("UNIQUE") || 
                    errorText.contains("重复") || errorText.contains("duplicate")) {
                    // 权限已存在，跳过
                } else {
                    qDebug() << "为adminjmh添加功能" << funcId << "权限失败:" << errorText;

                }
            }
            permQuery.finish();
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






















