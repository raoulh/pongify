#include "DialogNewSerie.h"
#include "ui_DialogNewSerie.h"
#include <QTime>
#include <QSpinBox>
#include <QTableWidgetItem>

DialogNewSerie::DialogNewSerie(bool isnew, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNewSerie)
{
    ui->setupUi(this);

    if (isnew)
        setWindowTitle("Nouvelle série");
    else
        setWindowTitle("Modifier");

    ui->comboBoxType->addItem("Elimination directe");
    ui->comboBoxType->addItem("A la ronde (Round-Robbin)");

    ui->comboBoxRanking->addItem("Open");
    ui->comboBoxRanking->addItem("NC");
    for (int i = 90; i > 0;i -= 5)
        ui->comboBoxRanking->addItem(QString::number(i));

    if (!isnew)
        ui->checkBoxDouble->setDisabled(true);

    ui->timeEditStart->setDisplayFormat("HH:mm");
    ui->timeEditStart->setEnabled(false);
    ui->timeEditStart->setTime(QTime(9, 0));
    connect(ui->checkStartTime, &QCheckBox::toggled, ui->timeEditStart, &QTimeEdit::setEnabled);

    //Handicap table config
    ui->tableWidgetHandicap->horizontalHeader()->setStretchLastSection(true);
    populateHandicapTable({0, 1, 2, 3, 4, 4, 5});
    updateHandicapVisibility();

    connect(ui->checkBoxHandicap, &QCheckBox::toggled, this, &DialogNewSerie::updateHandicapVisibility);
    connect(ui->checkBoxDouble, &QCheckBox::toggled, this, &DialogNewSerie::updateHandicapVisibility);

    connect(ui->btnAddHandicapRow, &QPushButton::clicked, this, [this]()
    {
        int row = ui->tableWidgetHandicap->rowCount();
        ui->tableWidgetHandicap->insertRow(row);

        auto *ecartItem = new QTableWidgetItem(QStringLiteral("%1+").arg(row));
        ecartItem->setFlags(ecartItem->flags() & ~Qt::ItemIsEditable);
        ui->tableWidgetHandicap->setItem(row, 0, ecartItem);

        auto *spin = new QSpinBox();
        spin->setMinimum(0);
        spin->setMaximum(99);
        //Default: same value as previous row
        if (row > 0)
        {
            auto *prevSpin = qobject_cast<QSpinBox *>(ui->tableWidgetHandicap->cellWidget(row - 1, 1));
            if (prevSpin)
                spin->setValue(prevSpin->value());
        }
        ui->tableWidgetHandicap->setCellWidget(row, 1, spin);

        //Update previous row label (remove '+' suffix)
        if (row > 0)
        {
            auto *prevItem = ui->tableWidgetHandicap->item(row - 1, 0);
            if (prevItem)
                prevItem->setText(QString::number(row - 1));
        }
    });

    connect(ui->btnRemoveHandicapRow, &QPushButton::clicked, this, [this]()
    {
        int row = ui->tableWidgetHandicap->rowCount();
        if (row <= 1) return; //keep at least one row
        ui->tableWidgetHandicap->removeRow(row - 1);

        //Update new last row label to add '+' suffix
        int lastRow = ui->tableWidgetHandicap->rowCount() - 1;
        auto *item = ui->tableWidgetHandicap->item(lastRow, 0);
        if (item)
            item->setText(QStringLiteral("%1+").arg(lastRow));
    });
}

DialogNewSerie::~DialogNewSerie()
{
    delete ui;
}

void DialogNewSerie::setName(QString name)
{
    ui->lineEditName->setText(name);
}

QString DialogNewSerie::getName()
{
    return ui->lineEditName->text();
}

QString DialogNewSerie::getRanking()
{
    return ui->comboBoxRanking->currentText();
}

void DialogNewSerie::setRanking(QString r)
{
    r = r.trimmed();
    for (int i = 0;i < ui->comboBoxRanking->count();i++)
    {
        if (ui->comboBoxRanking->itemText(i).toLower() == r.toLower())
        {
            ui->comboBoxRanking->setCurrentIndex(i);
            break;
        }
    }
}

QString DialogNewSerie::getType()
{
    if (ui->comboBoxType->currentIndex() == 0)
        return {"single"};

    return {"roundrobin"};
}

void DialogNewSerie::setType(QString t)
{
    t = t.trimmed();
    if (t.toLower() == "single")
        ui->comboBoxType->setCurrentIndex(0);
    else if (t.toLower() == "roundrobin")
        ui->comboBoxType->setCurrentIndex(1);
}

bool DialogNewSerie::getDouble()
{
    return ui->checkBoxDouble->isChecked();
}

void DialogNewSerie::setDouble(bool en)
{
    ui->checkBoxDouble->setChecked(en);
}

bool DialogNewSerie::getHandicap()
{
    if (getDouble())
        return false;
    return ui->checkBoxHandicap->isChecked();
}

void DialogNewSerie::setHandicap(bool en)
{
    if (getDouble())
        return;
    ui->checkBoxHandicap->setChecked(en);
}

QVariantList DialogNewSerie::getHandicapTable() const
{
    QVariantList table;
    for (int i = 0; i < ui->tableWidgetHandicap->rowCount(); i++)
    {
        auto *spin = qobject_cast<QSpinBox *>(ui->tableWidgetHandicap->cellWidget(i, 1));
        table.append(spin ? spin->value() : 0);
    }
    return table;
}

void DialogNewSerie::setHandicapTable(const QVariantList &table)
{
    populateHandicapTable(table);
}

void DialogNewSerie::populateHandicapTable(const QVariantList &table)
{
    ui->tableWidgetHandicap->setRowCount(0);
    for (int i = 0; i < table.size(); i++)
    {
        int row = ui->tableWidgetHandicap->rowCount();
        ui->tableWidgetHandicap->insertRow(row);

        QString label = (i == table.size() - 1) ? QStringLiteral("%1+").arg(i) : QString::number(i);
        auto *ecartItem = new QTableWidgetItem(label);
        ecartItem->setFlags(ecartItem->flags() & ~Qt::ItemIsEditable);
        ui->tableWidgetHandicap->setItem(row, 0, ecartItem);

        auto *spin = new QSpinBox();
        spin->setMinimum(0);
        spin->setMaximum(99);
        spin->setValue(table.at(i).toInt());
        ui->tableWidgetHandicap->setCellWidget(row, 1, spin);
    }
}

void DialogNewSerie::updateHandicapVisibility()
{
    bool visible = ui->checkBoxHandicap->isChecked() && !ui->checkBoxDouble->isChecked();
    ui->groupBoxHandicap->setVisible(visible);
}

QString DialogNewSerie::getStartTime() const
{
    if (!ui->checkStartTime->isChecked())
        return {};
    return ui->timeEditStart->time().toString("HH:mm");
}

void DialogNewSerie::setStartTime(const QString &time)
{
    if (time.isEmpty())
    {
        ui->checkStartTime->setChecked(false);
    }
    else
    {
        ui->checkStartTime->setChecked(true);
        ui->timeEditStart->setTime(QTime::fromString(time, "HH:mm"));
    }
}
