#include "UserInterfaceTask.hpp"

#include "UIController.h"
#include "ViewModel.h"
#include "Backlight.h"


UserInterfaceTask::UserInterfaceTask(
    DisplayInterface &display,
    ButtonDriverInterface &button,
    BTDriverInterface &btDriver,
    const WifiDriverInterface &wifiDriver,
    const AzureDriverInterface &azureDriver,
    TimerDriverInterface &timer,
    SettingsInterface &settings,
    DeviceMetaInterface &meta,
    BacklightDriverInterface &backlight,
    const FirmwareUpgradeTask & fwTask,
    QueueInterface<bool> &isHeatElementOnQueue,
    QueueInterface<ConnectionStatusChangeRequest::Enum> &connStatusChangeQueue)
: Task(),
  m_display(display),
  m_buttonDriver(button),
  m_btDriver(btDriver),
  m_wifiDriver(wifiDriver),
  m_azureDriver(azureDriver),
  m_timerDriver(timer),
  m_settings(settings),
  m_meta(meta),
  m_backlightDriver(backlight),
  m_firmwareUpgrade(fwTask),
  m_isHeatElementOnQueue(isHeatElementOnQueue),
  m_btStatusQueue(connStatusChangeQueue)
{
}

void UserInterfaceTask::run(bool eternalLoop)
{
  UIController uiController(m_timerDriver, m_buttonDriver, m_btDriver, m_wifiDriver, m_azureDriver, m_firmwareUpgrade, Screen::factoryResetScreen);

  const BacklightOptions::Enum dimOption = BacklightOptions::Normal; // not supported to change atm
  Backlight backlight(m_timerDriver.getTimeSinceBootMS(), m_backlightDriver);
  bool isRUHeatElementActive = false;

  while (eternalLoop)
  {
    bool wasGUIActive = uiController.control(m_settings, m_btStatusQueue, backlight.isScreenFullyDimmed());
    bool isScreenFullyDimmed = backlight.dim(wasGUIActive, m_timerDriver.getTimeSinceBootMS(), dimOption);
    isRUHeatElementActive = getHeatElementStateFromQueue(isRUHeatElementActive);

    ViewModel viewModel(uiController.getUIState(), m_settings, m_meta, isRUHeatElementActive, isScreenFullyDimmed);
    m_display.drawScreen(viewModel);
    delayMS(50u);

    // perform backlight adjustment with 2x the frequency to ensure smooth fading
    backlight.dim(wasGUIActive, m_timerDriver.getTimeSinceBootMS(), dimOption);
    delayMS(50u);
  }
}

bool UserInterfaceTask::getHeatElementStateFromQueue(bool currentState)
{
  bool queueOutput = false;

  if (m_isHeatElementOnQueue.pop(queueOutput, 0U))
  {
    currentState = queueOutput;
  }

  return currentState;
}
