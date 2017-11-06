#include "Memory.h"
#include "sdkconfig.h"
#include <esp_log.h>
#include <FATFS_VFS.h>
#include <FileSystem.h>
#include <FreeRTOS.h>
#include <Task.h>
#include <WebServer.h>
#include <WiFiEventHandler.h>
#include <WiFi.h>
#include <stdio.h>
#include <string>
#include "ESP32Explorer.h"
#include "BootWiFi.h"
#include "bt.h"

//static const char WIFI_SSID[]     = "sweetie";
//static const char WIFI_PASSWORD[] = "l16wint!";
//static const char *WIFI_SSID     = "Orange-8F54";
//static const char *WIFI_PASSWORD = "33413006";

extern "C" {
	int app_main(void);
}

//static const char* LOG_TAG = "ESP32_Explorer_MAIN";   // Logging tag


class ESP32_ExplorerTask: public Task {
	void run(void* data) override {
		ESP32_Explorer* pESP32_Explorer = new ESP32_Explorer();
		pESP32_Explorer->start();
	}
}; // ESP32_ExplorerTask


/**
 * @brief Wifi management task.
 */
class WiFiTask: public Task {
	void run(void *data) override {
		BootWiFi boot = BootWiFi();
		boot.boot();
		ESP32_ExplorerTask *pESP32_ExplorerTask = new ESP32_ExplorerTask();
		pESP32_ExplorerTask->setStackSize(6000);
		pESP32_ExplorerTask->start();
	} // End run
}; // WiFiTask


void task_webserver(void* ignore) {
	WiFiTask* pMyTask = new WiFiTask();
	pMyTask->setStackSize(6000);
	pMyTask->start();
	FreeRTOS::deleteTask();
} // task_webserver



/**
 * @brief Main entry point.
 */
int app_main(void) {
//	esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
  	ESP_LOGD("MAIN", "%d", xPortGetFreeHeapSize());
//Memory::init(100);
	task_webserver(nullptr);
	return 0;
} // app_main
