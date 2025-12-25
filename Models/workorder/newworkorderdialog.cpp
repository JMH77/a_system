#include "newworkorderdialog.h"
#include "workordermanager.h"
#include "../spareparts/sparepartsmanager.h"
#include "../database/databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDateTime>
#include <QDate>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QDebug>

NewWorkOrderDialog::NewWorkOrderDialog(WorkOrderManager *workOrderManager, const QString &creatorId, QWidget *parent)
    : QDialog(parent)
    , m_workOrderManager(workOrderManager)
    , m_sparePartsManager(nullptr)
    , m_creatorId(creatorId)
    , m_orderIdEdit(nullptr)
    , m_orderTypeCombo(nullptr)
    , m_titleEdit(nullptr)
    , m_descriptionEdit(nullptr)
    , m_equipIdEdit(nullptr)
    , m_shipIdEdit(nullptr)
    , m_planIdEdit(nullptr)
    , m_spareTable(nullptr)
    , m_addSpareButton(nullptr)
    , m_removeSpareButton(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
{
    setWindowTitle("新建工单");
    setMinimumSize(600, 600);
    setupUI();
    applyStyles();
    
    // 自动生成工单编号
    if (m_workOrderManager) {
        QString orderId = m_workOrderManager->generateOrderId();
        m_orderIdEdit->setText(orderId);
        
        // 创建备件管理器
        if (m_workOrderManager->getDatabaseManager()) {
            m_sparePartsManager = new SparePartsManager(m_workOrderManager->getDatabaseManager());
        }
    }
}

NewWorkOrderDialog::~NewWorkOrderDialog()
{
    delete m_sparePartsManager;
}

void NewWorkOrderDialog::setupUI()
{
    // 创建表单布局
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(15);
    formLayout->setContentsMargins(20, 20, 20, 20);
    
    // 工单编号（自动生成，只读）
    m_orderIdEdit = new QLineEdit(this);
    m_orderIdEdit->setReadOnly(true);
    m_orderIdEdit->setObjectName("orderIdEdit");
    formLayout->addRow("工单编号:", m_orderIdEdit);
    
    // 工单类型（下拉框）
    m_orderTypeCombo = new QComboBox(this);
    m_orderTypeCombo->addItem("常规维护");
    m_orderTypeCombo->addItem("紧急维修");
    m_orderTypeCombo->setObjectName("orderTypeCombo");
    formLayout->addRow("工单类型:", m_orderTypeCombo);
    
    // 标题（必填）
    m_titleEdit = new QLineEdit(this);
    m_titleEdit->setObjectName("titleEdit");
    m_titleEdit->setPlaceholderText("请输入工单标题");
    formLayout->addRow("标题*:", m_titleEdit);
    
    // 描述（多行文本）
    m_descriptionEdit = new QTextEdit(this);
    m_descriptionEdit->setObjectName("descriptionEdit");
    m_descriptionEdit->setPlaceholderText("请输入详细描述");
    m_descriptionEdit->setMaximumHeight(100);
    formLayout->addRow("描述:", m_descriptionEdit);
    
    // 设备ID
    m_equipIdEdit = new QLineEdit(this);
    m_equipIdEdit->setObjectName("equipIdEdit");
    m_equipIdEdit->setPlaceholderText("例如：EQP-1001");
    formLayout->addRow("设备ID:", m_equipIdEdit);
    
    // 船舶ID
    m_shipIdEdit = new QLineEdit(this);
    m_shipIdEdit->setObjectName("shipIdEdit");
    m_shipIdEdit->setPlaceholderText("例如：SHIP-01");
    formLayout->addRow("船舶ID:", m_shipIdEdit);
    
    // 关联计划ID（可选）
    m_planIdEdit = new QLineEdit(this);
    m_planIdEdit->setObjectName("planIdEdit");
    m_planIdEdit->setPlaceholderText("例如：PLAN-001（可选）");
    formLayout->addRow("关联计划ID:", m_planIdEdit);
    
    // 备件消耗表格
    m_spareTable = new QTableWidget(this);
    m_spareTable->setColumnCount(2);
    QStringList spareHeaders;
    spareHeaders << "备件ID" << "消耗数量";
    m_spareTable->setHorizontalHeaderLabels(spareHeaders);
    m_spareTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_spareTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_spareTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    // 设置所有列宽度相同
    m_spareTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_spareTable->verticalHeader()->setDefaultSectionSize(50);  // 设置行高
    m_spareTable->setMaximumHeight(200);
    
    // 备件操作按钮
    QHBoxLayout *spareButtonLayout = new QHBoxLayout();
    m_addSpareButton = new QPushButton("添加备件", this);
    m_removeSpareButton = new QPushButton("删除选中", this);
    m_removeSpareButton->setEnabled(false);
    // 设置按钮大小
    m_addSpareButton->setMinimumSize(120, 50);
    m_removeSpareButton->setMinimumSize(120, 50);
    connect(m_addSpareButton, &QPushButton::clicked, this, &NewWorkOrderDialog::onAddSpareClicked);
    connect(m_removeSpareButton, &QPushButton::clicked, this, &NewWorkOrderDialog::onRemoveSpareClicked);
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
    
    formLayout->addRow("备件消耗（可选）:", spareWidget);
    
    // 按钮
    m_okButton = new QPushButton("确定", this);
    m_cancelButton = new QPushButton("取消", this);
    m_okButton->setObjectName("okButton");
    m_cancelButton->setObjectName("cancelButton");
    
    connect(m_okButton, &QPushButton::clicked, this, &NewWorkOrderDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &NewWorkOrderDialog::onCancelClicked);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);
}

void NewWorkOrderDialog::applyStyles()
{
    // 设置输入框高度
    m_titleEdit->setMinimumHeight(45);
    m_equipIdEdit->setMinimumHeight(45);
    m_shipIdEdit->setMinimumHeight(45);
    m_planIdEdit->setMinimumHeight(45);
    m_okButton->setMinimumSize(120, 50);
    m_cancelButton->setMinimumSize(120, 50);
    
    // 设置按钮蓝色样式，与主系统一致
    this->setStyleSheet(
        "QPushButton#okButton {"
            "padding: 10px 20px;"
            "border-radius: 5px;"
            "border: none;"
            "background: #6CA6CD;"
            "color: #ffffff;"
            "font-size: 18px;"
        "}"
        "QPushButton#okButton:hover {"
            "background: #5B9BD5;"
        "}"
        "QPushButton#okButton:pressed {"
            "background: #4A8BC4;"
        "}"
        "QPushButton#cancelButton {"
            "padding: 10px 20px;"
            "border-radius: 5px;"
            "border: none;"
            "background: #CCCCCC;"
            "color: #333333;"
            "font-size: 18px;"
        "}"
        "QPushButton#cancelButton:hover {"
            "background: #BBBBBB;"
        "}"
        "QPushButton#cancelButton:pressed {"
            "background: #AAAAAA;"
        "}"
        "QPushButton {"
            "padding: 10px 20px;"
            "border-radius: 5px;"
            "border: none;"
            "background: #6CA6CD;"
            "color: #ffffff;"
            "font-size: 18px;"
        "}"
        "QPushButton:hover {"
            "background: #5B9BD5;"
        "}"
        "QPushButton:pressed {"
            "background: #4A8BC4;"
        "}"
        "QPushButton:disabled {"
            "background: #CCCCCC;"
            "color: #888888;"
        "}"
        "QLineEdit, QTextEdit, QComboBox {"
            "font-size: 18px;"
        "}"
        "QLabel {"
            "font-size: 18px;"
        "}"
        "QTableWidget {"
            "font-size: 18px;"
        "}"
        "QTableWidget::item {"
            "padding: 8px;"
        "}"
        "QHeaderView::section {"
            "font-size: 18px;"
            "padding: 10px;"
            "background-color: #6CA6CD;"
            "color: white;"
        "}"
    );
}

bool NewWorkOrderDialog::validateInput()
{
    if (m_titleEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "标题不能为空！");
        return false;
    }
    
    if (m_titleEdit->text().trimmed().length() > 200) {
        QMessageBox::warning(this, "输入错误", "标题长度不能超过200个字符！");
        return false;
    }
    
    return true;
}

void NewWorkOrderDialog::onOkClicked()
{
    if (!validateInput()) {
        return;
    }
    
    // 构建工单数据
    m_workOrderData.orderId = m_orderIdEdit->text().trimmed();
    m_workOrderData.orderType = m_orderTypeCombo->currentText();
    m_workOrderData.title = m_titleEdit->text().trimmed();
    m_workOrderData.description = m_descriptionEdit->toPlainText().trimmed();
    m_workOrderData.relatedEquipId = m_equipIdEdit->text().trimmed();
    m_workOrderData.shipId = m_shipIdEdit->text().trimmed();
    m_workOrderData.relatedPlanId = m_planIdEdit->text().trimmed();
    m_workOrderData.status = "待分配";
    m_workOrderData.createTime = QDateTime::currentDateTime();
    m_workOrderData.creatorId = m_creatorId;
    
    // 创建工单
    if (!m_workOrderManager || !m_workOrderManager->createWorkOrder(m_workOrderData)) {
        QString errorMsg = m_workOrderManager ? m_workOrderManager->getLastError() : "工单管理器未初始化";
        QMessageBox::warning(this, "创建失败", QString("创建工单失败：%1").arg(errorMsg));
        return;
    }
    
    // 创建备件消耗记录
    if (m_sparePartsManager) {
        QList<SparePartsConsumptionData> consumptions = getSpareConsumptions();
        if (!consumptions.isEmpty()) {
            // 为每条记录生成唯一的消耗ID
            // 先生成一个基础ID，然后为每个记录添加序号
            QString basePrefix = QString("CS-%1").arg(QDate::currentDate().toString("yyyyMMdd"));
            int startCount = 0;
            
            // 查询当天已有的消耗记录数量
            if (m_sparePartsManager && m_sparePartsManager->getDatabaseManager() && m_sparePartsManager->getDatabaseManager()->isConnected()) {
                QSqlDatabase db = m_sparePartsManager->getDatabaseManager()->getDatabase();
                QSqlQuery query(db);
                query.prepare("SELECT COUNT(*) FROM WORK_ORDER_SPARE WHERE CONSUME_ID LIKE ?");
                query.addBindValue(QString("%1-%%").arg(basePrefix));
                if (query.exec() && query.next()) {
                    startCount = query.value(0).toInt();
                }
                query.finish();
            }
            
            for (int i = 0; i < consumptions.size(); ++i) {
                int seqNum = startCount + i + 1;
                QString consumeId = QString("%1-%2").arg(basePrefix).arg(seqNum, 3, 10, QChar('0'));
                consumptions[i].consumeId = consumeId;
            }
            
            if (!m_sparePartsManager->createConsumptions(consumptions)) {
                QString errorMsg = m_sparePartsManager->getLastError();
                QMessageBox::warning(this, "警告", QString("工单创建成功，但保存备件消耗记录失败：%1").arg(errorMsg));
                // 不阻止对话框关闭，因为工单已经创建成功
            }
        }
    }
    
    QMessageBox::information(this, "成功", "工单创建成功！");
    accept();  // 关闭对话框
}

void NewWorkOrderDialog::onCancelClicked()
{
    reject();  // 关闭对话框
}

WorkOrderData NewWorkOrderDialog::getWorkOrderData() const
{
    return m_workOrderData;
}

void NewWorkOrderDialog::onAddSpareClicked()
{
    addSpareRow();
}

void NewWorkOrderDialog::onRemoveSpareClicked()
{
    int currentRow = m_spareTable->currentRow();
    if (currentRow >= 0) {
        removeSpareRow(currentRow);
    }
}

void NewWorkOrderDialog::addSpareRow()
{
    int row = m_spareTable->rowCount();
    m_spareTable->insertRow(row);
    
    // 备件ID
    QTableWidgetItem *spareIdItem = new QTableWidgetItem();
    spareIdItem->setText("");  // 初始为空
    m_spareTable->setItem(row, 0, spareIdItem);
    
    // 消耗数量（使用QDoubleSpinBox）
    QDoubleSpinBox *quantitySpinBox = new QDoubleSpinBox(this);
    quantitySpinBox->setMinimum(0.0);
    quantitySpinBox->setMaximum(999999.0);
    quantitySpinBox->setDecimals(4);
    quantitySpinBox->setValue(0.0);
    m_spareTable->setCellWidget(row, 1, quantitySpinBox);
}

void NewWorkOrderDialog::removeSpareRow(int row)
{
    if (row >= 0 && row < m_spareTable->rowCount()) {
        m_spareTable->removeRow(row);
    }
}

QList<SparePartsConsumptionData> NewWorkOrderDialog::getSpareConsumptions() const
{
    QList<SparePartsConsumptionData> consumptions;
    
    QString orderId = m_orderIdEdit->text().trimmed();
    if (orderId.isEmpty()) {
        return consumptions;
    }
    
    QDateTime currentTime = QDateTime::currentDateTime();
    
    // 从表格中读取备件消耗数据
    for (int row = 0; row < m_spareTable->rowCount(); ++row) {
        QTableWidgetItem *spareIdItem = m_spareTable->item(row, 0);
        if (!spareIdItem) continue;
        
        QString spareId = spareIdItem->text().trimmed();
        if (spareId.isEmpty()) continue;  // 跳过空行
        
        QDoubleSpinBox *quantitySpinBox = qobject_cast<QDoubleSpinBox*>(m_spareTable->cellWidget(row, 1));
        if (!quantitySpinBox) continue;
        
        double quantity = quantitySpinBox->value();
        if (quantity <= 0) continue;  // 跳过数量为0的行
        
        SparePartsConsumptionData consumption;
        consumption.orderId = orderId;
        consumption.spareId = spareId;
        consumption.consumeQuantity = quantity;
        consumption.consumeTime = currentTime;
        consumption.operatorId = m_creatorId;
        
        // 生成消耗ID（在创建时会重新生成，这里先留空）
        consumption.consumeId = "";
        
        consumptions.append(consumption);
    }
    
    return consumptions;
}

