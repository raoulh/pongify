#include "WidgetTextEdit.h"
#include "ui_WidgetTextEdit.h"

WidgetTextEdit::WidgetTextEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetTextEdit)
{
    ui->setupUi(this);

    QToolBar *toolbar = new QToolBar();
    QAction *act;

    actionTextBold = toolbar->addAction(QIcon(":/img/text-editor/format_bold_black_24dp.svg"), tr("Bold"));
    actionTextBold->setShortcut(Qt::CTRL | Qt::Key_B);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    connect(actionTextBold, &QAction::triggered, this, &WidgetTextEdit::textBold);
    actionTextBold->setCheckable(true);

    actionTextItalic = toolbar->addAction(QIcon(":/img/text-editor/format_italic_black_24dp.svg"), tr("Italic"));
    actionTextItalic->setShortcut(Qt::CTRL | Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    connect(actionTextItalic, &QAction::triggered, this, &WidgetTextEdit::textItalic);
    actionTextItalic->setCheckable(true);

    actionTextUnderline = toolbar->addAction(QIcon(":/img/text-editor/format_underlined_black_24dp.svg"), tr("Underline"));
    actionTextUnderline->setShortcut(Qt::CTRL | Qt::Key_U);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    connect(actionTextUnderline, &QAction::triggered, this, &WidgetTextEdit::textUnderline);
    actionTextUnderline->setCheckable(true);

    actionTextStrike = toolbar->addAction(QIcon(":/img/text-editor/format_strikethrough_black_24dp.svg"), tr("Strikethrough"));
    actionTextStrike->setShortcut(Qt::CTRL | Qt::Key_U);
    QFont strike;
    strike.setStrikeOut(true);
    actionTextStrike->setFont(underline);
    connect(actionTextStrike, &QAction::triggered, this, &WidgetTextEdit::textStrike);
    actionTextStrike->setCheckable(true);

    QActionGroup *grp = new QActionGroup(this);
    connect(grp, &QActionGroup::triggered, this, &WidgetTextEdit::textAlign);

    actionAlignLeft = toolbar->addAction(QIcon(":/img/text-editor/format_align_left_black_24dp.svg"), tr("Align left"));
    actionAlignCenter = toolbar->addAction(QIcon(":/img/text-editor/format_align_center_black_24dp.svg"), tr("Align center"));
    actionAlignRight = toolbar->addAction(QIcon(":/img/text-editor/format_align_right_black_24dp.svg"), tr("Align right"));
    actionAlignJustify = toolbar->addAction(QIcon(":/img/text-editor/format_align_justify_black_24dp.svg"), tr("Align justify"));
    actionAlignLeft->setShortcut(Qt::CTRL | Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignLeft->setActionGroup(grp);
    actionAlignCenter->setShortcut(Qt::CTRL | Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignCenter->setActionGroup(grp);
    actionAlignRight->setShortcut(Qt::CTRL | Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignRight->setActionGroup(grp);
    actionAlignJustify->setShortcut(Qt::CTRL | Qt::Key_J);
    actionAlignJustify->setCheckable(true);
    actionAlignJustify->setActionGroup(grp);

    QMenu *styleMenu = new QMenu();
    act = styleMenu->addAction("Standard");
    act->setData(0);
    act = styleMenu->addAction(tr("Bullet List (Disc)"));
    act->setData(1);
    act = styleMenu->addAction(tr("Bullet List (Circle)"));
    act->setData(2);
    act = styleMenu->addAction(tr("Bullet List (Square)"));
    act->setData(3);
    act = styleMenu->addAction(tr("Ordered List (Decimal)"));
    act->setData(4);
    act = styleMenu->addAction(tr("Ordered List (Alpha lower)"));
    act->setData(5);
    act = styleMenu->addAction(tr("Ordered List (Alpha upper)"));
    act->setData(6);
    act = styleMenu->addAction(tr("Ordered List (Roman lower)"));
    act->setData(7);
    act = styleMenu->addAction(tr("Ordered List (Roman upper)"));
    act->setData(8);
    connect(styleMenu, &QMenu::triggered, this, [=](QAction *act)
    {
        textStyle(act->data().toInt());
    });
    QToolButton *actionListStyle = new QToolButton();
    actionListStyle->setMenu(styleMenu);
    actionListStyle->setPopupMode(QToolButton::InstantPopup);
    actionListStyle->setIcon(QIcon(":/img/text-editor/format_list_bulleted_black_24dp.svg"));
    actionListStyle->setText(tr("List"));
    toolbar->addWidget(actionListStyle);

    actionTextColor = toolbar->addAction(QIcon(":/img/text-editor/palette_black_24dp.svg"), tr("List"));
    connect(actionTextColor, &QAction::triggered, this, &WidgetTextEdit::textColor);

    comboSize = new QComboBox();
    comboSize->setEditable(true);
    toolbar->addWidget(comboSize);

    foreach(int size, QFontDatabase::standardSizes())
        comboSize->addItem(QString::number(size));

    ui->textEdit->setFontPointSize(26);

    connect(comboSize, &QComboBox::activated, this, &WidgetTextEdit::textSize);
    comboSize->setCurrentIndex(comboSize->findText(QString::number(QApplication::font()
                                                                   .pointSize())));

    ui->verticalLayout->insertWidget(0, toolbar);

    connect(ui->textEdit, &QTextEdit::currentCharFormatChanged,
            this, &WidgetTextEdit::currentCharFormatChanged);
    connect(ui->textEdit, &QTextEdit::cursorPositionChanged,
            this, &WidgetTextEdit::cursorPositionChanged);
    ui->textEdit->setFocus();

    fontChanged(ui->textEdit->font());
    alignmentChanged(ui->textEdit->alignment());
}

WidgetTextEdit::~WidgetTextEdit()
{
    delete ui;
}

void WidgetTextEdit::textBold()
{
    QTextCharFormat fmt;
    fmt.setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void WidgetTextEdit::textUnderline()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(actionTextUnderline->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void WidgetTextEdit::textStrike()
{
    QTextCharFormat fmt;
    fmt.setFontStrikeOut(actionTextStrike->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void WidgetTextEdit::textItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(actionTextItalic->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void WidgetTextEdit::textSize(const int pidx)
{
    auto p = comboSize->itemText(pidx);
    qreal pointSize = p.toFloat();
    if (p.toFloat() > 0)
    {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormatOnWordOrSelection(fmt);
    }
}

void WidgetTextEdit::textStyle(int styleIndex)
{
    QTextCursor cursor = ui->textEdit->textCursor();

    if (styleIndex != 0)
    {
        QTextListFormat::Style style = QTextListFormat::ListDisc;

        switch (styleIndex)
        {
        default:
        case 1:
            style = QTextListFormat::ListDisc;
            break;
        case 2:
            style = QTextListFormat::ListCircle;
            break;
        case 3:
            style = QTextListFormat::ListSquare;
            break;
        case 4:
            style = QTextListFormat::ListDecimal;
            break;
        case 5:
            style = QTextListFormat::ListLowerAlpha;
            break;
        case 6:
            style = QTextListFormat::ListUpperAlpha;
            break;
        case 7:
            style = QTextListFormat::ListLowerRoman;
            break;
        case 8:
            style = QTextListFormat::ListUpperRoman;
            break;
        }

        cursor.beginEditBlock();

        QTextBlockFormat blockFmt = cursor.blockFormat();

        QTextListFormat listFmt;

        if (cursor.currentList())
        {
            listFmt = cursor.currentList()->format();
        }
        else
        {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }

        listFmt.setStyle(style);

        cursor.createList(listFmt);

        cursor.endEditBlock();
    }
    else
    {
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.mergeBlockFormat(bfmt);
    }
}

void WidgetTextEdit::textColor()
{
    QColor col = QColorDialog::getColor(ui->textEdit->textColor(), this);
    if (!col.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
}

void WidgetTextEdit::textAlign(QAction *a)
{
    if (a == actionAlignLeft)
        ui->textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    else if (a == actionAlignCenter)
        ui->textEdit->setAlignment(Qt::AlignHCenter);
    else if (a == actionAlignRight)
        ui->textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    else if (a == actionAlignJustify)
        ui->textEdit->setAlignment(Qt::AlignJustify);
}

void WidgetTextEdit::currentCharFormatChanged(const QTextCharFormat &format)
{
    fontChanged(format.font());
}

void WidgetTextEdit::cursorPositionChanged()
{
    alignmentChanged(ui->textEdit->alignment());
}

void WidgetTextEdit::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = ui->textEdit->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    ui->textEdit->mergeCurrentCharFormat(format);
}

void WidgetTextEdit::fontChanged(const QFont &f)
{
    comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
    actionTextBold->setChecked(f.bold());
    actionTextItalic->setChecked(f.italic());
    actionTextUnderline->setChecked(f.underline());
    actionTextStrike->setChecked(f.strikeOut());
}

void WidgetTextEdit::alignmentChanged(Qt::Alignment a)
{
    if (a & Qt::AlignLeft)
        actionAlignLeft->setChecked(true);
    else if (a & Qt::AlignHCenter)
        actionAlignCenter->setChecked(true);
    else if (a & Qt::AlignRight)
        actionAlignRight->setChecked(true);
    else if (a & Qt::AlignJustify)
        actionAlignJustify->setChecked(true);
}

QString WidgetTextEdit::getHtml()
{
    return ui->textEdit->toHtml();
}

QString WidgetTextEdit::getPlainText()
{
    return ui->textEdit->toPlainText();
}

void WidgetTextEdit::setPlainText(QString t)
{
    ui->textEdit->setPlainText(t);
}

void WidgetTextEdit::setHtml(QString t)
{
    ui->textEdit->setHtml(t);
}
