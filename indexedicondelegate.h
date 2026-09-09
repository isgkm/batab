#ifndef INDEXEDICONDELEGATE_H
#define INDEXEDICONDELEGATE_H

#include <QStyledItemDelegate>

class IndexedIconDelegate final : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const override;
};

#endif  // INDEXEDICONDELEGATE_H
