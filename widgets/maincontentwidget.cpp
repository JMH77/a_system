#include "maincontentwidget.h"
#include "../auth/authmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QFileInfo>
#include <QFile>
#include <QCoreApplication>
#include <QDir>
#include <QFont>
#include <QDebug>
#include <QMessageBox>

MainContentWidget::MainContentWidget(QWidget *parent)
    : QWidget(parent)
    , m_authManager(nullptr)
    , m_functionButton1(nullptr)
    , m_functionButton2(nullptr)
    , m_functionButton3(nullptr)
    , m_functionButton4(nullptr)
    , m_functionButton5(nullptr)
    , m_permissionButton(nullptr)
    , m_logoutButton(nullptr)
{
    setupUI();
    applyStyles();
    setBackgroundImage();
}

void MainContentWidget::setupUI()
{
    // 创建5个功能按钮
    m_functionButton1 = new QPushButton("工单管理", this);
    m_functionButton2 = new QPushButton("我的任务", this);
    m_functionButton3 = new QPushButton("验收任务", this);
    m_functionButton4 = new QPushButton("备件消耗", this);
    m_functionButton5 = new QPushButton("日志报告", this);
    
    // 创建权限管理按钮（初始隐藏，仅管理员可见）
    m_permissionButton = new QPushButton("权限管理", this);
    m_permissionButton->setObjectName("permissionButton");
    m_permissionButton->hide();
    connect(m_permissionButton, &QPushButton::clicked, this, &MainContentWidget::onPermissionManagementClicked);
    
    // 创建退出登录按钮（右上角）
    m_logoutButton = new QPushButton("退出登录", this);
    m_logoutButton->setObjectName("logoutButton");
    connect(m_logoutButton, &QPushButton::clicked, this, &MainContentWidget::onLogoutButtonClicked);

    // 设置按钮对象名，便于样式设置和权限控制
    m_functionButton1->setObjectName("functionButton1");
    m_functionButton2->setObjectName("functionButton2");
    m_functionButton3->setObjectName("functionButton3");
    m_functionButton4->setObjectName("functionButton4");
    m_functionButton5->setObjectName("functionButton5");
    
    // 连接功能按钮的点击事件
    connect(m_functionButton1, &QPushButton::clicked, this, &MainContentWidget::onFunction1Clicked);
    connect(m_functionButton2, &QPushButton::clicked, this, &MainContentWidget::onFunction2Clicked);
    connect(m_functionButton3, &QPushButton::clicked, this, &MainContentWidget::onFunction3Clicked);
    connect(m_functionButton4, &QPushButton::clicked, this, &MainContentWidget::onFunction4Clicked);
    connect(m_functionButton5, &QPushButton::clicked, this, &MainContentWidget::onFunction5Clicked);

    // 创建垂直布局放置按钮
    QVBoxLayout *buttonsLayout = new QVBoxLayout();
    buttonsLayout->setSpacing(50);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    buttonsLayout->addWidget(m_functionButton1);
    buttonsLayout->addWidget(m_functionButton2);
    buttonsLayout->addWidget(m_functionButton3);
    buttonsLayout->addWidget(m_functionButton4);
    buttonsLayout->addWidget(m_functionButton5);

    // 创建中心面板，限制最大宽度
    QWidget *centerPanel = new QWidget(this);
    centerPanel->setObjectName("centerPanel");
    centerPanel->setLayout(buttonsLayout);
    centerPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    centerPanel->setMaximumWidth(400);

    // 用水平布局包裹中心面板，并在两侧添加弹性伸展以实现水平居中
    QHBoxLayout *hCenter = new QHBoxLayout();
    hCenter->addStretch();
    hCenter->addWidget(centerPanel);
    hCenter->addStretch();

    // 顶层水平布局：左侧伸展 + 右上角退出登录按钮
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addStretch();
    topLayout->addWidget(m_logoutButton);
    
    // 顶层垂直布局：顶部退出登录按钮 + 上方伸展 + 中间按钮 + 下方伸展 + 权限管理按钮（左下角）
    QVBoxLayout *rootLayout = new QVBoxLayout();
    rootLayout->addLayout(topLayout);
    rootLayout->addStretch();
    rootLayout->addLayout(hCenter);
    rootLayout->addStretch();
    
    // 底部水平布局：左侧权限管理按钮 + 右侧伸展
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(m_permissionButton);
    bottomLayout->addStretch();
    rootLayout->addLayout(bottomLayout);
    rootLayout->setContentsMargins(20, 20, 20, 20);

    setLayout(rootLayout);
}

void MainContentWidget::applyStyles()
{
    // 设置按钮字体
    QFont buttonFont = font();
    buttonFont.setPointSize(16);  // 比登录界面的按钮字体稍大
    buttonFont.setBold(true);
    m_functionButton1->setFont(buttonFont);
    m_functionButton2->setFont(buttonFont);
    m_functionButton3->setFont(buttonFont);
    m_functionButton4->setFont(buttonFont);
    m_functionButton5->setFont(buttonFont);
    
    // 设置退出登录按钮字体
    QFont logoutButtonFont = font();
    logoutButtonFont.setPointSize(14);
    logoutButtonFont.setBold(true);
    if (m_logoutButton) {
        m_logoutButton->setFont(logoutButtonFont);
    }
    
    // 设置权限管理按钮字体
    QFont permissionButtonFont = font();
    permissionButtonFont.setPointSize(14);
    permissionButtonFont.setBold(true);
    if (m_permissionButton) {
        m_permissionButton->setFont(permissionButtonFont);
    }

    // 设置按钮尺寸 - 做得更大一些
    int buttonWidth = 380;
    int buttonHeight = 88;
    m_functionButton1->setMinimumSize(buttonWidth, buttonHeight);
    m_functionButton2->setMinimumSize(buttonWidth, buttonHeight);
    m_functionButton3->setMinimumSize(buttonWidth, buttonHeight);
    m_functionButton4->setMinimumSize(buttonWidth, buttonHeight);
    m_functionButton5->setMinimumSize(buttonWidth, buttonHeight);
    
    // 设置权限管理按钮尺寸 - 增大
    if (m_permissionButton) {
        m_permissionButton->setMinimumSize(140, 45);
    }
    
    // 设置退出登录按钮尺寸 - 增大
    if (m_logoutButton) {
        m_logoutButton->setMinimumSize(120, 45);
    }

    // 设置样式表 - 与登录界面按钮风格一致
    this->setStyleSheet(
        "#centerPanel { background: transparent; }"

        /* 功能按钮：与登录界面按钮相同的样式 */
        "QPushButton[objectName^=\"functionButton\"] {"
            "padding: 0 20px;"
            "border-radius: 10px;"
            "border: none;"
            "background: #6CA6CD;"
            "color: #ffffff;"
        "}"
        "QPushButton[objectName^=\"functionButton\"]:hover {"
            "background: #5B9BD5;"
        "}"
        "QPushButton[objectName^=\"functionButton\"]:pressed {"
            "background: #4A8BC4;"
        "}"
        "QPushButton[objectName^=\"functionButton\"]:focus {"
            "outline: none;"
            "border: none;"
        "}"
        /* 禁用状态的按钮样式（灰色） */
        "QPushButton[objectName^=\"functionButton\"]:disabled {"
            "background: #CCCCCC;"
            "color: #888888;"
        "}"
        /* 权限管理按钮样式 */
        "#permissionButton {"
            "padding: 10px 20px;"
            "border-radius: 5px;"
            "border: none;"
            "background: #8B7355;"
            "color: #ffffff;"
            "font-size: 14px;"
        "}"
        "#permissionButton:hover {"
            "background: #7A6344;"
        "}"
        "#permissionButton:pressed {"
            "background: #6A5334;"
        "}"
        /* 退出登录按钮样式 */
        "#logoutButton {"
            "padding: 10px 20px;"
            "border-radius: 5px;"
            "border: none;"
            "background: #DC143C;"
            "color: #ffffff;"
            "font-size: 14px;"
        "}"
        "#logoutButton:hover {"
            "background: #B22222;"
        "}"
        "#logoutButton:pressed {"
            "background: #8B0000;"
        "}"
    );
}

void MainContentWidget::updateButtonsByPermissions(AuthManager *authManager, const QString &username)
{
    if (!authManager) {
        qDebug() << "AuthManager为空，无法更新按钮权限";
        return;
    }
    
    m_authManager = authManager;
    m_currentUsername = username;
    
    // 检查是否是管理员（adminjmh）
    bool isAdmin = (username == "adminjmh");
    
    // 管理员显示权限管理按钮
    if (m_permissionButton) {
        m_permissionButton->setVisible(isAdmin);
    }
    
    // 获取用户的功能权限列表
    QList<int> permissions = authManager->getUserFunctionPermissions(username);
    qDebug() << "用户" << username << "的权限列表:" << permissions;
    
    // 更新每个按钮的状态
    updateButtonState(m_functionButton1, permissions.contains(1));
    updateButtonState(m_functionButton2, permissions.contains(2));
    updateButtonState(m_functionButton3, permissions.contains(3));
    updateButtonState(m_functionButton4, permissions.contains(4));
    updateButtonState(m_functionButton5, permissions.contains(5));
}

void MainContentWidget::onPermissionManagementClicked()
{
    emit permissionManagementRequested();
}

void MainContentWidget::onLogoutButtonClicked()
{
    // 显示确认对话框
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认退出",
        "确定要退出登录吗？",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    // 如果用户选择"是"，则发出退出登录信号
    if (reply == QMessageBox::Yes) {
        emit logoutRequested();
    }
}

void MainContentWidget::onFunction1Clicked()
{
    emit function1Requested();
}

void MainContentWidget::onFunction2Clicked()
{
    emit function2Requested();
}

void MainContentWidget::onFunction3Clicked()
{
    emit function3Requested();
}

void MainContentWidget::onFunction4Clicked()
{
    emit function4Requested();
}

void MainContentWidget::onFunction5Clicked()
{
    emit function5Requested();
}

void MainContentWidget::updateButtonState(QPushButton *button, bool enabled)
{
    if (!button) return;
    
    // 直接设置按钮的启用/禁用状态，样式表会自动应用
    button->setEnabled(enabled);
}

void MainContentWidget::setBackgroundImage()
{
    QString startDir = QCoreApplication::applicationDirPath();

    QString path;
    QString curDir = startDir;
    for (int level = 0; level < 6 && path.isEmpty(); ++level) {
        const QString base = curDir + "/resources/";
        const QString jpg = base + "loginWidget.jpg";
        const QString png = base + "loginWidget.png";
        const bool jpgExists = QFile::exists(jpg);
        const bool pngExists = QFile::exists(png);
        if (jpgExists) {
            path = jpg;
            break;
        }
        if (pngExists) {
            path = png;
            break;
        }
        // 上移一层
        curDir = QDir::cleanPath(curDir + "/..");
    }

    // 使用样式表 border-image 自适应填充，随窗口变化自动缩放
    if (!path.isEmpty()) {
        QPixmap probe(path); 
        if (!probe.isNull()) {
            m_bgPixmap = probe;
            update(); // 触发重绘
        }

        // 叠加样式，避免覆盖先前的控件样式
        const QString prev = this->styleSheet();
        this->setStyleSheet(prev + "\n" + QString(
            "border-image: url(%1) 0 0 0 0 stretch stretch;"
        ).arg(path));
    }
}

void MainContentWidget::paintEvent(QPaintEvent *event)
{
    if (!m_bgPixmap.isNull()) {
        QPainter p(this);
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);
        const qreal opacity = 0.6;  // 与登录界面相同的透明度
        p.setOpacity(opacity);
        p.drawPixmap(rect(), m_bgPixmap);
    }
    QWidget::paintEvent(event);
}
