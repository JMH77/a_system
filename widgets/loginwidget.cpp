#include "loginwidget.h"
#include "../auth/authmanager.h"

#include <QLabel>
#include <QLineEdit>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QPushButton>
#include <QSizePolicy>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QFileInfo>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QMessageBox>

LoginWidget::LoginWidget(QWidget *parent) 
    : QWidget(parent)
    , m_authManager(nullptr)
{
	setupUI();
	applyStyles();

	setBackgroundImage();

	//连接信号和槽
    //点击登录按钮的信号和槽函数
    connect(m_loginButton, &QPushButton::clicked, this, &LoginWidget::onLoginButtonClicked);
    //点击注册按钮发出信号(让主窗口中的堆叠窗口收到并切换界面)
    connect(m_registerButton, &QPushButton::clicked, this, &LoginWidget::onRegisterButtonClicked);
}

void LoginWidget::setAuthManager(AuthManager *authManager)
{
    m_authManager = authManager;
}

void LoginWidget::setupUI()
{
	// 创建控件
	m_usernameLabel = new QLabel(this);
	m_passwordLabel = new QLabel(this);
	m_usernameEdit = new QLineEdit(this);
	m_passwordEdit = new QLineEdit(this);
	m_loginButton = new QPushButton(this);
	m_registerButton = new QPushButton(this);

	// 基本属性
	m_usernameLabel->setText("用户名:");
	m_passwordLabel->setText("密码:");
	// 标签文本居中
	m_usernameLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_passwordLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_usernameEdit->setObjectName("usernameEdit");
	m_passwordEdit->setObjectName("passwordEdit");
	m_passwordEdit->setEchoMode(QLineEdit::Password);
	m_loginButton->setText("登录");
	m_registerButton->setText("注册");
	m_loginButton->setObjectName("loginButton");
	m_registerButton->setObjectName("registerButton");
	// 为两个标签设置对象名，便于定向样式
	m_usernameLabel->setObjectName("fieldLabel");
	m_passwordLabel->setObjectName("fieldLabel");

	// 表单布局（两行：用户名、密码）
	QFormLayout *formLayout = new QFormLayout();
	// 控制表单内左右控件之间的水平间距（标签 与 输入框）
	formLayout->setHorizontalSpacing(12);
	// 控制表单内上下相邻行之间的垂直间距（用户名行 与 密码行）
	formLayout->setVerticalSpacing(18);
	formLayout->setContentsMargins(0, 0, 0, 0);
	formLayout->addRow(m_usernameLabel, m_usernameEdit);
	formLayout->addRow(m_passwordLabel, m_passwordEdit);

	// 按钮行（登录、注册在一行，居中）
	QHBoxLayout *buttonsRow = new QHBoxLayout();
	buttonsRow->setContentsMargins(0, 0, 0, 0);
	buttonsRow->addStretch();
	// 放置“登录”按钮（按钮行内部）
	buttonsRow->addWidget(m_loginButton);
	// 控制两个按钮之间的水平间距（登录 与 注册）
	buttonsRow->addSpacing(20);
	buttonsRow->addWidget(m_registerButton);
	buttonsRow->addStretch();

	// 用水平布局包裹表单，并在两侧添加弹性伸展，确保水平居中
	QHBoxLayout *hCenter = new QHBoxLayout();
	hCenter->addStretch();
	// 将表单与按钮列成一列，保证按钮在表单下方
	QVBoxLayout *formAndButtons = new QVBoxLayout();
	formAndButtons->addLayout(formLayout);
	formAndButtons->addLayout(buttonsRow);
	// 避免默认 spacing 与 addSpacing 叠加导致距离偏大
	// 控制“表单区域 与 按钮区域”之间的垂直间距
	formAndButtons->setSpacing(18); 
	// 控制中心面板内部内容与其边框之间的内边距
	formAndButtons->setContentsMargins(12, 12, 12, 12); 

	// 中心面板：限制最大宽度，避免全屏时控件过长
	m_centerPanel = new QWidget(this);
	m_centerPanel->setObjectName("centerPanel");
	m_centerPanel->setLayout(formAndButtons);
	m_centerPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	// 限制中心卡片的最大宽度，避免输入框在大窗体下被拉得过长
	m_centerPanel->setMaximumWidth(360);
	// 去掉过高的最小高度，避免在表单与按钮之间被拉出额外空白
	m_centerPanel->setMinimumHeight(0);

	hCenter->addWidget(m_centerPanel);
	hCenter->addStretch();

	// 顶层垂直布局：上方伸展 + 中间表单 + 下方伸展，确保垂直居中
	QVBoxLayout *rootLayout = new QVBoxLayout();
	rootLayout->addStretch();
	rootLayout->addLayout(hCenter);
	rootLayout->addStretch();

	setLayout(rootLayout);
}



void LoginWidget::applyStyles()
{
	// 字号更大更饱满，并对关键元素加粗
	QFont labelFont = font();
    labelFont.setPointSize(18);
	labelFont.setBold(true);
	m_usernameLabel->setFont(labelFont);
	m_passwordLabel->setFont(labelFont);

	QFont editFont = font();
    editFont.setPointSize(14);
    editFont.setBold(true);
	m_usernameEdit->setFont(editFont);
	m_passwordEdit->setFont(editFont);

	QFont buttonFont = font();
    buttonFont.setPointSize(13);
	buttonFont.setBold(true);
	m_loginButton->setFont(buttonFont);
	m_registerButton->setFont(buttonFont);

	// 限制用户名输入框的最小宽度，保证不至于过窄
	m_usernameEdit->setMinimumWidth(220);
	// 限制用户名输入框的最大宽度，避免在大窗体下过长
	m_usernameEdit->setMaximumWidth(260);
	// 限制密码输入框的最小宽度
	m_passwordEdit->setMinimumWidth(220);
	// 限制密码输入框的最大宽度
	m_passwordEdit->setMaximumWidth(260);
	// 输入框与按钮高度统一在样式阶段设置
	// 提升用户名输入框的可触达高度
    m_usernameEdit->setMinimumHeight(60);
	// 提升密码输入框的可触达高度
    m_passwordEdit->setMinimumHeight(60);

	// 限制登录按钮的最小宽度，保证文字不被压缩
	m_loginButton->setMinimumWidth(96);
	// 限制注册按钮的最小宽度
	m_registerButton->setMinimumWidth(96);
	// 提升登录按钮的高度，便于点击
    m_loginButton->setMinimumHeight(50);
	// 提升注册按钮的高度，便于点击
    m_registerButton->setMinimumHeight(50);
    //设置标签高度
    m_usernameLabel->setMinimumHeight(60);
    m_passwordLabel->setMinimumHeight(60);

	if (layout()) {
		layout()->setContentsMargins(24, 24, 24, 24);
		layout()->setSpacing(12);
	}

	// 现代化外观样式（可迁移到外部 QSS）
	this->setStyleSheet(
		"#centerPanel { background: transparent; }"

		/* 仅给用户名/密码标签添加纯蓝圆角背景与白字 */
		"#fieldLabel {"
            "background:#6CA6CD;"
			"color:#ffffff;"
			"border-radius:10px;"
			"padding:2px 8px;"
		"}"

		/* 输入框：圆角、浅边框、聚焦高亮与阴影 */
		"QLineEdit {"
			"padding:6px 10px;"
            "border:1px solid #6CA6CD;"
			"border-radius:10px;"
			"background:#ffffff;"
		"}"
		"QLineEdit:focus {"
            "border-color:#6CA6CD;"
			"background:#ffffff;"
		"}"

		/* 按钮：统一白色、圆角、浅边框、hover/pressed 态 */
		"#loginButton, #registerButton {"
			"padding:0 14px;"
			"border-radius:10px;"
            "border:1px solid #6CA6CD;"
            "background:#6CA6CD;"
			"color:#ffffff;"
		"}"
        "#loginButton:hover, #registerButton:hover { background:#6CA6CD; }" /* LightSkyBlue hover 同色系稍亮 */
        "#loginButton:pressed, #registerButton:pressed { background:#6CA6CD; }"
	);
}

void LoginWidget::setBackgroundImage()
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
	} else {
	}
}

void LoginWidget::paintEvent(QPaintEvent *event)
{
    if (!m_bgPixmap.isNull()) {
        QPainter p(this);
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);
        const qreal opacity = 0.6;
		p.setOpacity(opacity);
        p.drawPixmap(rect(), m_bgPixmap);
    }
    QWidget::paintEvent(event);
}

bool LoginWidget::validateInput()
{
	QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text().trimmed();

    if(username.isEmpty() || username.length() < 3){
        return false;
    }
	for (const QChar &c : username) {
		ushort unicode = c.unicode();
        if (unicode >= 0x4E00 && unicode <= 0x9FFF) {
            return false;
        }
    }

    if(password.isEmpty() || password.length() < 6 || password.length() > 18){
        return false;
    }
	for (const QChar &c : password) {
		ushort unicode = c.unicode();
        if (unicode >= 0x4E00 && unicode <= 0x9FFF) {
            return false;
        }
	}
	return true;
}

void LoginWidget::onRegisterButtonClicked()
{
	emit changeToRegister();
}

void LoginWidget::onLoginButtonClicked()
{
	// 1. 验证输入格式
	if(!validateInput()){
        QMessageBox::information(this, "格式错误", "用户名或密码格式错误！");
        return;
	}
	
	// 2. 获取用户输入
	QString username = m_usernameEdit->text().trimmed();
	QString password = m_passwordEdit->text().trimmed();
	
	// 3. 检查认证管理器是否可用
	if (!m_authManager) {
		QMessageBox::warning(this, "错误", "认证管理器未设置！");
		emit loginFailed("系统错误：认证管理器未设置");
		return;
	}
	
	// 4. 调用认证管理器验证登录
	if (m_authManager->login(username, password)) {
		// 登录成功
		emit loginSuccess(username);
	} else {
		// 登录失败，获取错误信息
		QString errorMsg = m_authManager->getLastError();
		QMessageBox::warning(this, "登录失败", errorMsg);
		emit loginFailed(errorMsg);
	}
}













