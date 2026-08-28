#ifndef INDEXEDICONDELEGATE_H
#define INDEXEDICONDELEGATE_H

#include <QStyledItemDelegate>

class IndexedIconDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

#endif  // INDEXEDICONDELEGATE_H
