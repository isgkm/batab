#include "shortcutlistrowwidget.h"

#include <QPushButton>

ShortcutListRowWidget::ShortcutListRowWidget(const QList<QKeySequence>& initial,
                                             QWidget* parent)
    : QWidget{parent}, m_layout{new QHBoxLayout(this)} {
    m_layout->setContentsMargins(0, 0, 0, 0);
    for (const auto& seq : initial) {
        addEditor(seq);
    }
    auto* addButton = new QPushButton(tr("+"), this);
    connect(addButton, &QPushButton::clicked, this,
            [this]() { addEditor(QKeySequence()); });
    m_layout->addWidget(addButton);
}

QList<QKeySequence> ShortcutListRowWidget::shortcuts() const {
    QList<QKeySequence> result;

    for (auto* editor : m_editors) {
        if (!editor->keySequence().isEmpty()) {
            result.append(editor->keySequence());
        }
    }

    return result;
}

void ShortcutListRowWidget::addEditor(const QKeySequence& sequence) {
    auto* editor = new QKeySequenceEdit(sequence, this);

    connect(editor, &QKeySequenceEdit::editingFinished, this,
            [this] { emit shortcutsChanged(shortcuts()); });

    auto* removeButton = new QPushButton("x", this);
    connect(removeButton, &QPushButton::clicked, this,
            [this, editor, removeButton] {
                m_editors.removeAll(editor);
                editor->deleteLater();
                removeButton->deleteLater();
                emit shortcutsChanged(shortcuts());
            });

    m_editors.append(editor);
    m_layout->insertWidget(m_layout->count() - 1, editor);
    m_layout->insertWidget(m_layout->count() - 1, removeButton);
}
