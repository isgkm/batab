#include "indexedicondelegate.h"

#include "constants.h"

#include <QPainter>

void IndexedIconDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    painter->save();

    const bool showHighlight =
        m_navigationActive && (opt.state & QStyle::State_Selected);
    painter->setPen(showHighlight ? opt.palette.highlightedText().color()
                                  : opt.palette.text().color());
    if (showHighlight)
    {
        painter->fillRect(opt.rect, opt.palette.highlight());
    }

    constexpr int margin{6};
    const QRect rect = opt.rect;
    const int slot = index.data(Constants::SLOT_INDEX_ROLE).toInt();
    const QString number = QString::number(slot + 1) + " -";
    const int numberWidth = opt.fontMetrics.horizontalAdvance(number);
    const QRect numberRect(rect.left() + margin, rect.top(), numberWidth,
                           rect.height());
    painter->drawText(numberRect, Qt::AlignVCenter | Qt::AlignLeft, number);

    const QRect iconRect(
        numberRect.right() + margin,
        rect.top() + ((rect.height() - opt.decorationSize.height()) / 2),
        opt.decorationSize.width(), opt.decorationSize.height());
    opt.icon.paint(painter, iconRect);

    const QRect textRect(iconRect.right() + margin, rect.top(),
                         rect.width() - (iconRect.right() + margin),
                         rect.height());

    const QString elidedText =
        opt.fontMetrics.elidedText(opt.text, Qt::ElideRight, textRect.width());

    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, elidedText);

    painter->restore();
}

QSize IndexedIconDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setWidth(option.rect.width());

    return size;
}
