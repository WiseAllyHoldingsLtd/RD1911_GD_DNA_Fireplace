#include "SettingsStorageInterface.h"

/* Ensure unique keys! Max length including \0 is 16. */

const char SettingUID::RUN_MODE[] = "RUNMODE";
const char SettingUID::WIFI_SSID[] = "WIFISSID";
const char SettingUID::WIFI_PASS[] = "WIFIPASS";
const char SettingUID::STATIC_IP[] = "STATIC_IP";
const char SettingUID::STATIC_NETMASK[] = "STATIC_NETMASK";
const char SettingUID::STATIC_GATEWAY[] = "STATIC_GW";
const char SettingUID::STATIC_DNS1[] = "STATIC_DNS1";
const char SettingUID::STATIC_DNS2[] = "STATIC_DNS2";
const char SettingUID::PRIMARY_HEATING_MODE[] = "PRIMARY_HM";
const char SettingUID::COMFORT_TEMP[] = "COMFTEMP";
const char SettingUID::ECO_TEMP[] = "ECOTEMP";
const char SettingUID::OVERRIDE_TYPE_MODE[] = "OVERRIDE1";
const char SettingUID::OVERRIDE_DATE[] = "OVERRIDE2";
const char SettingUID::OVERRIDE_TIME[] = "OVERRIDE3";
const char SettingUID::IS_BUTTON_LOCK_ON[] = "BUTTONLOCK";
const char SettingUID::UTC_OFFSET[] = "UTCOFFSET";
const char SettingUID::IS_TIME_SET[] = "TIMESET";
const char SettingUID::WEEK_PROGRAM_MONDAY[] = "WEEKPROG_MON";
const char SettingUID::WEEK_PROGRAM_TUESDAY[] = "WEEKPROG_TUE";
const char SettingUID::WEEK_PROGRAM_WEDNESDAY[] = "WEEKPROG_WED";
const char SettingUID::WEEK_PROGRAM_THURSDAY[] = "WEEKPROG_THU";
const char SettingUID::WEEK_PROGRAM_FRIDAY[] = "WEEKPROG_FRI";
const char SettingUID::WEEK_PROGRAM_SATURDAY[] = "WEEKPROG_SAT";
const char SettingUID::WEEK_PROGRAM_SUNDAY[] = "WEEKPROG_SUN";
const char SettingUID::FW_ATTEMPTS[] = "FW_ATTEMPTS";
const char SettingUID::FW_ID[] = "FW_ID";
const char SettingUID::FW_VERSION_CODE[] = "FW_VER_CODE";
const char SettingUID::FORGET_ME_STATE[] = "FORGET_ME";
const char SettingUID::WEEK_PROGRAM_EXT_MONDAY[] = "WEEKPROGEXT_MON";
const char SettingUID::WEEK_PROGRAM_EXT_TUESDAY[] = "WEEKPROGEXT_TUE";
const char SettingUID::WEEK_PROGRAM_EXT_WEDNESDAY[] = "WEEKPROGEXT_WED";
const char SettingUID::WEEK_PROGRAM_EXT_THURSDAY[] = "WEEKPROGEXT_THU";
const char SettingUID::WEEK_PROGRAM_EXT_FRIDAY[] = "WEEKPROGEXT_FRI";
const char SettingUID::WEEK_PROGRAM_EXT_SATURDAY[] = "WEEKPROGEXT_SAT";
const char SettingUID::WEEK_PROGRAM_EXT_SUNDAY[] = "WEEKPROGEXT_SUN";
