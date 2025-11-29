#ifndef MAINCONTENTWIDGET_H
#define MAINCONTENTWIDGET_H
#include <QWidget>
#include <QPixmap>

class QPushButton;

class MainContentWidget : public QWidget
{
public:
    MainContentWidget(QWidget *parent = nullptr);

protected:
    // 重写paintEvent事件，绘制背景图
    void paintEvent(class QPaintEvent *event) override;

private:
    void setupUI();
    void applyStyles();
    void setBackgroundImage();

private:
    QPushButton *m_functionButton1;
    QPushButton *m_functionButton2;
    QPushButton *m_functionButton3;
    QPushButton *m_functionButton4;
    QPushButton *m_functionButton5;
    QPixmap m_bgPixmap;
};

#endif // MAINCONTENTWIDGET_H
