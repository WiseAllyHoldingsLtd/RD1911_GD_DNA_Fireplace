#pragma once

#ifndef UNITTESTS
#include "esp_log.h"
#include "sdkconfig.h"
#else
#define ESP_LOGE( tag, format, ... ) do {} while(0)
#define ESP_LOGW( tag, format, ... ) do {} while(0)
#define ESP_LOGI( tag, format, ... ) do {} while(0)
#define ESP_LOGD( tag, format, ... ) do {} while(0)
#define ESP_LOGV( tag, format, ... ) do {} while(0)
#endif
