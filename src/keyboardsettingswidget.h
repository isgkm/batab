#ifndef KEYBOARDSETTINGSWIDGET_H
#define KEYBOARDSETTINGSWIDGET_H

#include <QWidget>

namespace Ui {
class KeyboardSettingsWidget;
}  // namespace Ui

class KeyboardSettingsWidget final : public QWidget {
    Q_OBJECT

  public:
    Q_DISABLE_COPY_MOVE(KeyboardSettingsWidget)

    explicit KeyboardSettingsWidget(QWidget* parent = nullptr);
    ~KeyboardSettingsWidget() override;

  private:
    Ui::KeyboardSettingsWidget* m_ui;
    QVector<QWidget*> m_queuedToDelete;

    void updateTableData();
};

#endif  // KEYBOARDSETTINGSWIDGET_H
