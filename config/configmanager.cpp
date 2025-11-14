#include "configmanager.h"
#include <QCoreApplication>
#include <QFile>

configmanager::configmanager()
   : m_settings(nullptr)
{
    QString configPath = getConfigPath();
    if (!configPath.isEmpty()) {
        initConfigManager(configPath);  
    }
}

configmanager::~configmanager() {
  if(m_settings){
    delete m_settings;
  }
  m_settings = nullptr;
}

//获取配置文件路径
QString  configmanager::getConfigPath(){
    QString Dir = QCoreApplication::applicationDirPath();
    QStringList PathList;
    PathList << Dir + "/../config.ini"
             << Dir + "/../../config.ini"
             << Dir + "/../../../config.ini";
             for(const QString &path : PathList){
              if(QFile::exists(path)){ 
                  return path;
                }
              }
              return QString();
}

//初始化配置管理器（向管理器中加载数据）
bool configmanager::initConfigManager(const QString &configPath){
    //检查路径是否有效
    if (!QFile::exists(configPath)) {
        qDebug() << "Config file does not exist:" << configPath;
        return false;
    }

    //初始化时检查是否清空缓存
    if(m_settings) {
        delete m_settings;
    }

    m_settings = new QSettings(configPath, QSettings::IniFormat);

    if(!m_settings){
        qDebug() << "Fail to create QSettings instance!";
        return false;
    }

    //加载配置文件数据
    loadLogPath();
    loadDatabase();
    loadSubsysPath();

    return true;
}

//私有成员函数，被初始化函数调用对配置文件进行读取并写进成员变量
void configmanager::loadLogPath() {
    m_settings->beginGroup("General");
    m_logPath = m_settings->value("LogPath", "./logs").toString();
    m_settings->endGroup();
}

bool configmanager::loadDatabase() {
    m_settings->beginGroup("Database");
    m_dbType = m_settings->value("Type", "DM").toString();

    if(m_dbType.toLower() == "dm"){
        m_dbConfig["Host"] = m_settings->value("Host", "localhost").toString();
        m_dbConfig["Port"] = m_settings->value("Port","5236").toString();
        m_dbConfig["DatabaseName"] = m_settings->value("DatabaseName", "subtest1").toString();
        m_dbConfig["UID"] = m_settings->value("UID", "SYSDBA").toString();
        m_dbConfig["Password"] = m_settings->value("Password", "Test1123").toString();
        m_settings->endGroup();
        return true;
    }
    else{
        qDebug() << "Fail to config the Database!";
        m_settings->endGroup();
        return false;
    }
  
}

void configmanager::loadSubsysPath(){
    for(int i = 1; i < 5; i++){
        QString GroupName = QString("Subsystem%1").arg(i);

        if(m_settings->childGroups().contains(GroupName)){
            m_settings->beginGroup(GroupName);
            m_subsysPath[i] = m_settings->value("Path", "").toString();
            m_subsysHost[i] = m_settings->value("Host", "").toString();
            m_subsysPort[i] = m_settings->value("Port", "").toString();
            m_settings->endGroup();
        }
    }
}


//下面是读取数据接口
QString configmanager::getLogPath() const{
    return m_logPath;
}


QString configmanager::getDbType() const{
    return m_dbType;
}

QMap<QString,QString> configmanager::getDbConfig() const{
    return m_dbConfig;
}

QString configmanager::getSubsysPath(int index) const{
    return m_subsysPath.value(index, "");
}

QString configmanager::getSubsysPort(int index) const{
    return m_subsysPort.value(index, "");
}

QString configmanager::getSubsysHost(int index) const{
    return m_subsysHost.value(index, "");
}

bool configmanager::isInitialized() const {
    return m_settings != nullptr;
}











