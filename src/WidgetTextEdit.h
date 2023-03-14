#ifndef WIDGETTEXTEDIT_H
#define WIDGETTEXTEDIT_H

#include <QtWidgets>

namespace Ui {
class WidgetTextEdit;
}

class WidgetTextEdit : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetTextEdit(QWidget *parent = 0);
    ~WidgetTextEdit();

    void setPlainText(QString t);
    void setHtml(QString t);

    QString getHtml();
    QString getPlainText();

private slots:
    void textBold();
    void textUnderline();
    void textItalic();
    void textStrike();
    void textSize(const int pidx);
    void textStyle(int styleIndex);
    void textColor();
    void textAlign(QAction *a);

    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();

private:
    Ui::WidgetTextEdit *ui;

    QAction *actionTextBold;
    QAction *actionTextUnderline;
    QAction *actionTextStrike;
    QAction *actionTextItalic;
    QAction *actionTextColor;
    QAction *actionAlignLeft;
    QAction *actionAlignCenter;
    QAction *actionAlignRight;
    QAction *actionAlignJustify;
    QComboBox *comboSize;

    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void fontChanged(const QFont &f);
    void alignmentChanged(Qt::Alignment a);
};

#endif // WIDGETTEXTEDIT_H
