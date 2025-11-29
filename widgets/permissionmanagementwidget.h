#ifndef PERMISSIONMANAGEMENTWIDGET_H
#define PERMISSIONMANAGEMENTWIDGET_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>

class AuthManager;

class PermissionManagementWidget : public QDialog
{
    Q_OBJECT

public:
    explicit PermissionManagementWidget(AuthManager *authManager, QWidget *parent = nullptr);

private slots:
    void onSaveClicked();
    void onCancelClicked();

private:
    void setupUI();
    void loadUsers();
    void updatePermissionsByRole(int row);  // 根据角色自动更新功能权限
    
    AuthManager *m_authManager;
    QTableWidget *m_userTable;
    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;
    
    // 列索引
    static const int COL_USERNAME = 0;
    static const int COL_EMAIL = 1;
    static const int COL_ROLE = 2;      // 角色列
    static const int COL_FUNC1 = 3;
    static const int COL_FUNC2 = 4;
    static const int COL_FUNC3 = 5;
    static const int COL_FUNC4 = 6;
    static const int COL_FUNC5 = 7;
};

#endif // PERMISSIONMANAGEMENTWIDGET_H

