#include "newconsumptiondialog.h"
#include "sparepartsmanager.h"
#include "../auth/authmanager.h"
#include "../database/databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDateTime>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDoubleSpinBox>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QDebug>

NewConsumptionDialog::NewConsumptionDialog(SparePartsManager *sparePartsManager, AuthManager *authManager,
                                          const QString &operatorId, QWidget *parent)
    : QDialog(parent)
    , m_sparePartsManager(sparePartsManager)
    , m_authManager(authManager)
    , m_operatorId(operatorId)
    , m_orderIdCombo(nullptr)
    , m_spareTable(nullptr)
    , m_addSpareButton(nullptr)
    , m_removeSpareButton(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
{
    setWindowTitle("新建备件消耗记录");
    setMinimumSize(600, 500);
    setupUI();
    loadRelatedWorkOrders();
}

NewConsumptionDialog::~NewConsumptionDialog()
{
}

void NewConsumptionDialog::setupUI()
{
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(15);
    formLayout->setContentsMargins(20, 20, 20, 20);
    
    // 工单编号选择
    m_orderIdCombo = new QComboBox(this);
    m_orderIdCombo->setObjectName("orderIdCombo");
    m_orderIdCombo->setEditable(false);
    formLayout->addRow("选择工单*:", m_orderIdCombo);
    
    // 备件消耗表格
    m_spareTable = new QTableWidget(this);
    m_spareTable->setColumnCount(2);
    QStringList spareHeaders;
    spareHeaders << "备件ID" << "消耗数量";
    m_spareTable->setHorizontalHeaderLabels(spareHeaders);
    m_spareTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_spareTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_spareTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    m_spareTable->horizontalHeader()->setStretchLastSection(true);
    m_spareTable->setMaximumHeight(200);
    
    // 备件操作按钮
    QHBoxLayout *spareButtonLayout = new QHBoxLayout();
    m_addSpareButton = new QPushButton("添加备件", this);
    m_removeSpareButton = new QPushButton("删除选中", this);
    m_removeSpareButton->setEnabled(false);
    connect(m_addSpareButton, &QPushButton::clicked, this, &NewConsumptionDialog::onAddSpareClicked);
    connect(m_removeSpareButton, &QPushButton::clicked, this, &NewConsumptionDialog::onRemoveSpareClicked);
    connect(m_spareTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        m_removeSpareButton->setEnabled(m_spareTable->currentRow() >= 0);
    });
    
    spareButtonLayout->addWidget(m_addSpareButton);
    spareButtonLayout->addWidget(m_removeSpareButton);
    spareButtonLayout->addStretch();
    
    QWidget *spareWidget = new QWidget(this);
    QVBoxLayout *spareLayout = new QVBoxLayout(spareWidget);
    spareLayout->setContentsMargins(0, 0, 0, 0);
    spareLayout->addWidget(m_spareTable);
    spareLayout->addLayout(spareButtonLayout);
    
    formLayout->addRow("备件消耗*:", spareWidget);
    
    // 按钮
    m_okButton = new QPushButton("确定", this);
    m_cancelButton = new QPushButton("取消", this);
    m_okButton->setObjectName("okButton");
    m_cancelButton->setObjectName("cancelButton");
    
    connect(m_okButton, &QPushButton::clicked, this, &NewConsumptionDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &NewConsumptionDialog::onCancelClicked);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);
}

void NewConsumptionDialog::loadRelatedWorkOrders()
{
    if (!m_authManager || !m_authManager->getDatabaseManager()) {
        return;
    }
    
    QString currentUsername = m_operatorId;
    bool isAdmin = (currentUsername == "adminjmh");
    
    databasemanager *dbManager = m_authManager->getDatabaseManager();
    if (!dbManager || !dbManager->isConnected()) {
        return;
    }
    
    QSqlDatabase db = dbManager->getDatabase();
    QSqlQuery query(db);
    
    QString sql;
    if (isAdmin) {
        // 管理员可以看到所有工单
        sql = "SELECT ORDER_ID, TITLE FROM WORK_ORDER ORDER BY CREATE_TIME DESC";
        query.prepare(sql);
    } else {
        // 普通用户只能看到和自己相关的工单
        sql = "SELECT ORDER_ID, TITLE FROM WORK_ORDER WHERE CREATOR_ID = ? OR ASSIGNEE_ID = ? OR ACCEPTOR_ID = ? ORDER BY CREATE_TIME DESC";
        query.prepare(sql);
        query.addBindValue(currentUsername);
        query.addBindValue(currentUsername);
        query.addBindValue(currentUsername);
    }
    
    if (query.exec()) {
        while (query.next()) {
            QString orderId = query.value(0).toString();
            QString title = query.value(1).toString();
            QString displayText = QString("%1 - %2").arg(orderId).arg(title);
            m_orderIdCombo->addItem(displayText, orderId);
        }
    }
    query.finish();
}

void NewConsumptionDialog::onAddSpareClicked()
{
    int row = m_spareTable->rowCount();
    m_spareTable->insertRow(row);
    
    // 备件ID
    QTableWidgetItem *spareIdItem = new QTableWidgetItem();
    spareIdItem->setText("");
    m_spareTable->setItem(row, 0, spareIdItem);
    
    // 消耗数量
    QDoubleSpinBox *quantitySpinBox = new QDoubleSpinBox(this);
    quantitySpinBox->setMinimum(0.0);
    quantitySpinBox->setMaximum(999999.0);
    quantitySpinBox->setDecimals(4);
    quantitySpinBox->setValue(0.0);
    m_spareTable->setCellWidget(row, 1, quantitySpinBox);
}

void NewConsumptionDialog::onRemoveSpareClicked()
{
    int currentRow = m_spareTable->currentRow();
    if (currentRow >= 0) {
        m_spareTable->removeRow(currentRow);
    }
}

bool NewConsumptionDialog::validateInput()
{
    if (m_orderIdCombo->currentIndex() < 0) {
        QMessageBox::warning(this, "输入错误", "请选择工单！");
        return false;
    }
    
    if (m_spareTable->rowCount() == 0) {
        QMessageBox::warning(this, "输入错误", "请至少添加一条备件消耗记录！");
        return false;
    }
    
    // 验证每一行都有有效的备件ID和数量
    for (int row = 0; row < m_spareTable->rowCount(); ++row) {
        QTableWidgetItem *spareIdItem = m_spareTable->item(row, 0);
        if (!spareIdItem || spareIdItem->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "输入错误", QString("第%1行的备件ID不能为空！").arg(row + 1));
            return false;
        }
        
        QDoubleSpinBox *quantitySpinBox = qobject_cast<QDoubleSpinBox*>(m_spareTable->cellWidget(row, 1));
        if (!quantitySpinBox || quantitySpinBox->value() <= 0) {
            QMessageBox::warning(this, "输入错误", QString("第%1行的消耗数量必须大于0！").arg(row + 1));
            return false;
        }
    }
    
    return true;
}

void NewConsumptionDialog::onOkClicked()
{
    if (!validateInput()) {
        return;
    }
    
    if (!m_sparePartsManager) {
        QMessageBox::warning(this, "错误", "备件管理器未初始化！");
        return;
    }
    
    QString orderId = m_orderIdCombo->currentData().toString();
    if (orderId.isEmpty()) {
        QMessageBox::warning(this, "错误", "工单编号为空！");
        return;
    }
    
    QDateTime currentTime = QDateTime::currentDateTime();
    QList<SparePartsConsumptionData> consumptions;
    
    // 从表格中读取备件消耗数据
    for (int row = 0; row < m_spareTable->rowCount(); ++row) {
        QTableWidgetItem *spareIdItem = m_spareTable->item(row, 0);
        if (!spareIdItem) continue;
        
        QString spareId = spareIdItem->text().trimmed();
        if (spareId.isEmpty()) continue;
        
        QDoubleSpinBox *quantitySpinBox = qobject_cast<QDoubleSpinBox*>(m_spareTable->cellWidget(row, 1));
        if (!quantitySpinBox) continue;
        
        double quantity = quantitySpinBox->value();
        if (quantity <= 0) continue;
        
        SparePartsConsumptionData consumption;
        consumption.consumeId = m_sparePartsManager->generateConsumeId();
        consumption.orderId = orderId;
        consumption.spareId = spareId;
        consumption.consumeQuantity = quantity;
        consumption.consumeTime = currentTime;
        consumption.operatorId = m_operatorId;
        
        consumptions.append(consumption);
    }
    
    if (consumptions.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "没有有效的备件消耗记录！");
        return;
    }
    
    // 批量创建备件消耗记录
    if (!m_sparePartsManager->createConsumptions(consumptions)) {
        QString errorMsg = m_sparePartsManager->getLastError();
        QMessageBox::warning(this, "创建失败", QString("创建备件消耗记录失败：%1").arg(errorMsg));
        return;
    }
    
    QMessageBox::information(this, "成功", QString("成功创建%1条备件消耗记录！").arg(consumptions.size()));
    accept();
}

void NewConsumptionDialog::onCancelClicked()
{
    reject();
}

