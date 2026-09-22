#ifndef SHORTCUTLISTROWWIDGET_H
#define SHORTCUTLISTROWWIDGET_H

#include <QHBoxLayout>
#include <QKeySequenceEdit>
#include <QWidget>

class ShortcutListRowWidget final : public QWidget {
    Q_OBJECT
  public:
    explicit ShortcutListRowWidget(const QList<QKeySequence>& initial,
                                   QWidget* parent = nullptr);

  signals:
    void shortcutsChanged(const QList<QKeySequence>& shortcuts);

  private:
    QList<QKeySequenceEdit*> m_editors;
    QHBoxLayout* m_layout;

    [[nodiscard]] QList<QKeySequence> shortcuts() const;
    void addEditor(const QKeySequence& sequence);
};

#endif  // SHORTCUTLISTROWWIDGET_H
