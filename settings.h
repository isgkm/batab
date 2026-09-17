#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QtClassHelperMacros>

class Settings final {
  public:
    Q_DISABLE_COPY_MOVE(Settings)

    static Settings& getInstance();

    [[nodiscard]] int selectionCommitTimeoutMs() const;
    void setSelectionCommitTimeoutMs(int timeMs);

    [[nodiscard]] int hotCornerSize() const;
    void setHotCornerSize(int sizePx);

    [[nodiscard]] int hotCornerElapsedTimeMs() const;
    void setHotCornerElapsedTimeMs(int timeMs);

    [[nodiscard]] int quickSwitchHoldThresholdMs() const;
    void setQuickSwitchHoldThresholdMs(int timeMs);

  private:
    Settings();
    ~Settings() = default;

    QSettings m_settings;
};

#endif  // SETTINGS_H
