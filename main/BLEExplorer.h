/*
 * BLEExplorer.h
 *
 *  Created on: Sep 27, 2017
 *      Author: kolban
 */

#ifndef MAIN_BLEEXPLORER_H_
#define MAIN_BLEEXPLORER_H_
#include <freertos/FreeRTOS.h>
#include "JSON.h"
#include "BLEAdvertisedDevice.h"

class BLEExplorer {
public:
	BLEExplorer();
	virtual ~BLEExplorer();
	JsonArray scan();
	JsonArray connect(std::string addr);
	JsonObject enumerateDevices(BLEAdvertisedDevice device);
	JsonObject enumerateCharacteristics(BLERemoteService *p, std::map<std::string, BLERemoteCharacteristic*> *charact, std::string _addr);

	// --- SERVER --- //
	void createServer(std::string name);
	void deleteServer();
	JsonObject addService(std::string uuid);
	//void startServer();
	//void stopServer();
	void startAdvertising();
	void stopAdvertising();
	JsonObject addCharacteristic(std::string uuid, std::string service);

private:
};

#endif /* MAIN_BLEEXPLORER_H_ */
