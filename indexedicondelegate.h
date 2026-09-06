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

    void setNavigationActive(bool active)
    {
        m_navigationActive = active;
    }

private:
    bool m_navigationActive{};
};

#endif  // INDEXEDICONDELEGATE_H
