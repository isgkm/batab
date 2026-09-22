#ifndef GENERALSETTINGSWIDGET_H
#define GENERALSETTINGSWIDGET_H

#include <QWidget>

namespace Ui {
class GeneralSettingsWidget;
}  // namespace Ui

class GeneralSettingsWidget final : public QWidget {
    Q_OBJECT

  public:
    Q_DISABLE_COPY_MOVE(GeneralSettingsWidget)

    explicit GeneralSettingsWidget(QWidget* parent = nullptr);
    ~GeneralSettingsWidget() override;

  private:
    Ui::GeneralSettingsWidget* m_ui;

    void populateHotCornerComboboxOptions();
};

#endif  // GENERALSETTINGSWIDGET_H
