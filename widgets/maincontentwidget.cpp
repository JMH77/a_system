#include "maincontentwidget.h"
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

MainContentWidget::MainContentWidget(QWidget *parent)
    : QWidget(parent)
    , m_functionButton1(nullptr)
    , m_functionButton2(nullptr)
    , m_functionButton3(nullptr)
    , m_functionButton4(nullptr)
    , m_functionButton5(nullptr)
{
    setupUI();
    applyStyles();
    setBackgroundImage();
}

void MainContentWidget::setupUI()
{
    // 创建5个功能按钮
    m_functionButton1 = new QPushButton("功能一", this);
    m_functionButton2 = new QPushButton("功能二", this);
    m_functionButton3 = new QPushButton("功能三", this);
    m_functionButton4 = new QPushButton("功能四", this);
    m_functionButton5 = new QPushButton("功能五", this);

    // 设置按钮对象名，便于样式设置
    m_functionButton1->setObjectName("functionButton");
    m_functionButton2->setObjectName("functionButton");
    m_functionButton3->setObjectName("functionButton");
    m_functionButton4->setObjectName("functionButton");
    m_functionButton5->setObjectName("functionButton");

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

    // 用水平布局包裹中心面板，并在两侧添加弹性伸展，确保水平居中
    QHBoxLayout *hCenter = new QHBoxLayout();
    hCenter->addStretch();
    hCenter->addWidget(centerPanel);
    hCenter->addStretch();

    // 顶层垂直布局：上方伸展 + 中间按钮 + 下方伸展，确保垂直居中
    QVBoxLayout *rootLayout = new QVBoxLayout();
    rootLayout->addStretch();
    rootLayout->addLayout(hCenter);
    rootLayout->addStretch();

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

    // 设置按钮尺寸 - 做得更大一些
    int buttonWidth = 380;
    int buttonHeight = 88;
    m_functionButton1->setMinimumSize(buttonWidth, buttonHeight);
    m_functionButton2->setMinimumSize(buttonWidth, buttonHeight);
    m_functionButton3->setMinimumSize(buttonWidth, buttonHeight);
    m_functionButton4->setMinimumSize(buttonWidth, buttonHeight);
    m_functionButton5->setMinimumSize(buttonWidth, buttonHeight);

    // 设置样式表 - 与登录界面按钮风格一致
    this->setStyleSheet(
        "#centerPanel { background: transparent; }"

        /* 功能按钮：与登录界面按钮相同的样式 */
        "#functionButton {"
            "padding: 0 20px;"
            "border-radius: 10px;"
            "border: none;"
            "background: #6CA6CD;"
            "color: #ffffff;"
        "}"
        "#functionButton:hover {"
            "background: #5B9BD5;"
        "}"
        "#functionButton:pressed {"
            "background: #4A8BC4;"
        "}"
        "#functionButton:focus {"
            "outline: none;"
            "border: none;"
        "}"
    );
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
