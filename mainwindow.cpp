#include "mainwindow.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include "auth/userinfo.h"




MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_configManager(new configmanager())  // 创建配置管理器
    , dbManger(new databasemanager(m_configManager))  // 传入配置管理器
    , m_authManager(nullptr)
{
    this->setWindowTitle("登录");
    
    // 先初始化UI，确保界面能显示
    initUI();
    
    // 设置一个合理的默认窗口尺寸与最小尺寸，避免启动时过小
    this->resize(880, 640);
    this->setMinimumSize(880, 640);
    
    // 连接数据库
    bool dbConnected = false;
    if (!dbManger->connectDatabase()) {
        QString errorMsg = QString("数据库连接失败：%1\n\n请检查：\n1. 数据库服务是否运行\n2. 配置文件 config.ini 中的数据库配置是否正确").arg(dbManger->getLastError());
        QMessageBox::warning(this, "数据库连接失败", errorMsg);
    } else {
        // 初始化用户表
        if (!dbManger->initUserTable()) {
            QString errorMsg = QString("用户表初始化失败：%1").arg(dbManger->getLastError());
            QMessageBox::warning(this, "用户表初始化失败", errorMsg);
        } else {
            dbConnected = true;
        }
    }
    
    // 创建认证管理器（即使数据库未连接也创建，但功能会受限）
    m_authManager = new AuthManager(dbManger);
    
    // 设置认证管理器到登录界面
    m_loginWidget->setAuthManager(m_authManager);

    //调用建立槽函数连接
    connections();
}

MainWindow::~MainWindow()
{
    delete m_authManager;   // 删除认证管理器
    delete dbManger;        // 删除数据库管理器
    delete m_configManager; // 删除配置管理器
    delete m_stackedWidget;
}

void MainWindow::initUI()
{
    // 1. 创建堆叠窗口
    m_stackedWidget = new QStackedWidget(this);  // ← 创建，this作为父对象
    
    // 2. 创建三个页面
    m_loginWidget = new LoginWidget(m_stackedWidget);
    m_registerWidget = new RegisterWidget(m_stackedWidget);
    m_mainContentWidget = new MainContentWidget(m_stackedWidget);
    
    // 3. 将页面添加到堆叠窗口
    m_stackedWidget->addWidget(m_loginWidget);        // 索引0
    m_stackedWidget->addWidget(m_registerWidget);     // 索引1
    m_stackedWidget->addWidget(m_mainContentWidget);   // 索引2
    
    // 4. 设置堆叠窗口为中央部件（重要！）
    setCentralWidget(m_stackedWidget);  // ← 这样QStackedWidget才会显示
    
    // 5. 默认显示登录页面
    m_stackedWidget->setCurrentIndex(0);
}

void MainWindow::connections()
{
    //当收到登陆界面的注册按钮点击后发出的切换到注册界面信号
    connect(m_loginWidget, &LoginWidget::changeToRegister, this, [this](){
        m_stackedWidget->setCurrentIndex(1);
        this->setWindowTitle("注册");
    });

    //连接登录成功信号
    connect(m_loginWidget, &LoginWidget::loginSuccess, this, [this](const QString &username){
        // 根据用户权限更新主界面按钮状态
        m_mainContentWidget->updateButtonsByPermissions(m_authManager, username);
        
        // 切换到主内容页面
        m_stackedWidget->setCurrentIndex(2);  // 索引2是主内容页面
        this->setWindowTitle(QString("欢迎，%1").arg(username));
    });
    
    // 连接权限管理请求信号
    connect(m_mainContentWidget, &MainContentWidget::permissionManagementRequested, this, [this](){
        // 打开权限管理对话框
        PermissionManagementWidget *permWidget = new PermissionManagementWidget(m_authManager, this);
        permWidget->setAttribute(Qt::WA_DeleteOnClose);
        permWidget->exec();
        
        // 权限更新后，刷新主界面按钮显示
        // 重新获取当前用户名并更新按钮状态
        // 这里需要从登录信息中获取，暂时使用adminjmh（因为只有管理员能打开这个对话框）
        m_mainContentWidget->updateButtonsByPermissions(m_authManager, "adminjmh");
    });
    
    // 连接退出登录信号
    connect(m_mainContentWidget, &MainContentWidget::logoutRequested, this, [this](){
        // 清空登录界面的输入框
        m_loginWidget->clearInputFields();
        // 切换到登录页面
        m_stackedWidget->setCurrentIndex(0);
        this->setWindowTitle("登录");
    });

    //连接登录失败信号
    connect(m_loginWidget, &LoginWidget::loginFailed, this, [](const QString &errorMessage){
        // LoginWidget 内部已经显示了错误信息，这里不再重复显示
    });

    //当收到注册界面的返回按钮点击后发出的切换到登录界面信号
    connect(m_registerWidget, &RegisterWidget::backToLogin, this, [this](){
        m_stackedWidget->setCurrentIndex(0);
        this->setWindowTitle("登录");
    });

    //连接注册请求信号
    connect(m_registerWidget, &RegisterWidget::registerRequest, this, [this](const userinfodata &data){
        // 检查认证管理器是否可用
        if (!m_authManager) {
            QMessageBox::warning(this, "错误", "认证管理器未初始化！");
            return;
        }
        
        // 创建 UserInfo 对象
        userinfo user;
        user.setUserData(data);
        
        // 检查用户名是否已存在
        if (m_authManager->userExists(user.getUserData().username)) {
            QMessageBox::warning(this, "注册失败", "用户名已存在，请选择其他用户名！");
            return;
        }
        
        // 尝试注册用户
        if (m_authManager->registerUser(user)) {
            QMessageBox::information(this, "注册成功", "注册成功，请返回登录！");
            // 清空注册界面的输入框
            m_registerWidget->clearInputFields();
            // 切换到登录页面
            m_stackedWidget->setCurrentIndex(0);
            this->setWindowTitle("登录");
        } else {
            // 显示错误信息
            QString errorMsg = m_authManager->getLastError();
            QMessageBox::warning(this, "注册失败", errorMsg);
        }
    });

}





