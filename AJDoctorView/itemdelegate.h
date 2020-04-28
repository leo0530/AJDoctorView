#ifndef COMBODELEGATE_H
#define COMBODELEGATE_H

#include <QObject>
#include <QItemDelegate>
#include <QComboBox>
#include <QStyledItemDelegate>

class ComboDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit ComboDelegate(QObject *parent = nullptr);

   QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;//双击时，弹出的QComboBox中显示的内容
   void setEditorData(QWidget *editor, const QModelIndex &index) const;                          //设置QComboBox当前editor的数据
   void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;//设置QTableView的model的数据
signals:

public slots:
};

/*
 * ReadOnly
 */
class ReadOnlyDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    ReadOnlyDelegate(QObject *parent = 0): QItemDelegate(parent) {}

  //  void setItems(QStringList items);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex	&index) const
    {
        return NULL;
    }
};

/*
 * DoubleSpinBox
 */
class SpinBoxDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    SpinBoxDelegate(QObject *parent = 0);

    void setRange(double min, double max);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex	&index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const  QModelIndex &index) const;

private:
    double minValue;        //最小值
    double maxValue;        //最大值
};

/*
 * CheckBox
 */
class CheckBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    CheckBoxDelegate(QObject *parent = 0);

    void setColumn(int col);

protected:
    void paint(QPainter* painter,const QStyleOptionViewItem& option,const QModelIndex& index) const;
    bool editorEvent(QEvent *event,QAbstractItemModel *model,const QStyleOptionViewItem &option,const QModelIndex &index);

private:
    int column;         //设置复选框的列
};

/*
 * calendar
 */
class DateDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    DateDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex	&index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const  QModelIndex &index) const;
};

/*
 * TimeEdit
 */
class TimeDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    TimeDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex	&index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const  QModelIndex &index) const;
};

/*
 * DoubleLineEdit
 */
class DoubleLineEditDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    DoubleLineEditDelegate(QObject *parent = 0);

    void setRange(double bottom, double top, int decimals = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex	&index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const  QModelIndex &index) const;

private:
    int min;        //最小值
    int max;        //最大值
    int dec;        //小数位数
};

#endif // COMBODELEGATE_H
