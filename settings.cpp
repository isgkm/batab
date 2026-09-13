#include "settings.h"

#include "constants.h"

Settings::Settings()
    : m_settings(QSettings::IniFormat, QSettings::UserScope, "Batab",
                 "settings")
{
}

Settings &Settings::getInstance()
{
    static Settings s_instance;

    return s_instance;
}

int Settings::selectionCommitTimeoutMs() const
{
    return m_settings
        .value(Constants::SETTINGS_KEY_SELECTION_COMMIT_TIMEOUT_MS, 1000)
        .toInt();
}

void Settings::setSelectionCommitTimeoutMs(int timeMs)
{
    m_settings.setValue(Constants::SETTINGS_KEY_SELECTION_COMMIT_TIMEOUT_MS,
                        timeMs);
}

int Settings::hotCornerSize() const
{
    return m_settings.value(Constants::SETTINGS_KEY_HOT_CORNER_SIZE, 32)
        .toInt();
};

void Settings::setHotCornerSize(int sizePx)
{
    m_settings.setValue(Constants::SETTINGS_KEY_HOT_CORNER_SIZE, sizePx);
}

int Settings::hotCornerElapsedTimeMs() const
{
    return m_settings
        .value(Constants::SETTINGS_KEY_HOT_CORNER_ELAPSED_TIME_MS, 1000)
        .toInt();
}

void Settings::setHotCornerElapsedTimeMs(int timeMs)
{
    m_settings.setValue(Constants::SETTINGS_KEY_HOT_CORNER_ELAPSED_TIME_MS,
                        timeMs);
}
