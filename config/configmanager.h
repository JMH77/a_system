#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H
#include <QSettings>
#include <QString>
#include <QMap>
#include <QDebug>

class configmanager
{
public:
    configmanager();
    ~configmanager();

    //获取到配置文件的路径
    QString  getConfigPath();

    //配置管理器初始化函数
    bool initConfigManager(const QString &configPath);

    //检查配置管理器是否已成功初始化
    bool isInitialized() const;

    //读取配置数据接口
    QString getLogPath() const;
    QString getDbType() const;
    QMap<QString,QString> getDbConfig() const;
    QString getSubsysPath(int index) const;
    QString getSubsysPort(int index) const;
    QString getSubsysHost(int index) const;


private:
    //设置QSettings变量  -> 用来读取的一个实例
    QSettings *m_settings;

    //普通配置管理器成员变量 -> 存储配置文件中读取到的信息
    QString m_logPath;
    QString m_dbType;
    QMap<QString,QString> m_dbConfig;
    QMap<int, QString> m_subsysPath;
    QMap<int, QString> m_subsysPort;
    QMap<int, QString> m_subsysHost;

    //加载，通过m_settings将配置文件内容读取到成员变量中
    void loadLogPath();
    bool loadDatabase();
    void loadSubsysPath();

};

#endif // CONFIGMANAGER_H
