/*
 * BLEExplorer.cpp
 *
 *  Created on: Sep 27, 2017
 *      Author: kolban
 */

#include "BLEExplorer.h"
#include "sdkconfig.h"
#include <esp_log.h>
#include <BLEDevice.h>
#include "Task.h"
#include "Memory.h"

#include "BLEClient.h"
static const char* LOG_TAG = "BLEExplorer";
static bool isRunning = false;
static BLEServer *pServer;
static BLEServiceMap *mServices;
static BLECharacteristicMap *mCharacteristics;

BLEExplorer::BLEExplorer() {
	// TODO Auto-generated constructor stub
}

BLEExplorer::~BLEExplorer() {
	// TODO Auto-generated destructor stub
}
/**
 * @brief Perform a BLE Scan and return the results.
 */
JsonArray BLEExplorer::scan() {
	ESP_LOGD(LOG_TAG, ">> scan");
	if(!isRunning){
		BLEDevice::init("");
		isRunning = true;
	}
	BLEScan* pBLEScan = BLEDevice::getScan();
	pBLEScan->setActiveScan(true);
	BLEScanResults scanResults = pBLEScan->start(5);
	JsonObject obj = JSON::createObject();
	JsonArray arr = JSON::createArray();
	arr.addString("BLE devices");
	for (int c = 0; c < scanResults.getCount(); ++c) {
		BLEAdvertisedDevice dev =  scanResults.getDevice(c);
		arr.addObject(enumerateDevices(dev));
	}
	obj.setInt("deviceCount", scanResults.getCount());
	ESP_LOGD(LOG_TAG, "<< scan");
	return arr;
} // scan
/*
 * @brief Connect to BLE server to get and enumerate services and characteristics
 */
JsonArray BLEExplorer::connect(std::string _addr){
	BLEAddress *pAddress = new BLEAddress(_addr);
	BLEClient*  pClient  = BLEDevice::createClient();

	// Connect to the remove BLE Server.
	pClient->connect(*pAddress);
	std::map<std::string, BLERemoteService*> *pRemoteServices = pClient->getServices();
	if (pRemoteServices == nullptr) {
		ESP_LOGD(LOG_TAG, "Failed to find services");
		return 0;
	}
//	Memory::startTraceAll();
	JsonArray arr = JSON::createArray();
	for (auto it=pRemoteServices->begin(); it!=pRemoteServices->end(); ++it) {
		std::map<std::string, BLERemoteCharacteristic*> *pRemoteCharacteristics = it->second->getCharacteristics();
		arr.addObject(enumerateCharacteristics(it->second, pRemoteCharacteristics, _addr));
	}


//	Memory::stopTrace();
//	Memory::dump();
	pClient->disconnect();
/*
	free(pClient);
	free(pAddress);   //FIXME using it here causing multi heap crash
	free(pRemoteServices);*/
	return arr;
}
/*
 * @brief Enumerate devices
 */
JsonObject BLEExplorer::enumerateDevices(BLEAdvertisedDevice device){ //TODO expandable complex info about device advertising
	JsonArray arr = JSON::createArray();
	JsonObject obj = JSON::createObject();
	obj.setString("id", device.getAddress().toString());
	obj.setString("parent", "#");
	obj.setString("text", device.getName());
	JsonObject state = JSON::createObject();
	state.setBoolean("opened", true);
	obj.setObject("state", state);
	JsonObject obj2 = JSON::createObject();
	//obj2.setString("id", device.getAddress().toString());
	obj2.setString("parent", device.getAddress().toString());
	obj2.setString("text", "Address - " + device.getAddress().toString());
	obj2.setBoolean("children", false);
	arr.addObject(obj2);
	//TODO add all this info
	/*
	bool		isAdvertisingService(BLEUUID uuid);
	bool        haveAppearance();
	bool        haveManufacturerData();
	bool        haveName();
	bool        haveRSSI();
	bool        haveServiceUUID();
	bool        haveTXPower();
	 */

	obj.setArray("children", arr);

	return obj;
}
/*
 * @brief Enumerate characteristics from given service
 */
JsonObject BLEExplorer::enumerateCharacteristics(BLERemoteService *p, std::map<std::string, BLERemoteCharacteristic*> *charact, std::string _addr){
	JsonObject obj = JSON::createObject();
	JsonArray arr = JSON::createArray();

	//obj.setString("id", p->getUUID().toString());  // todo add short uuid if service !UNKNOWN
	obj.setString("text", BLEUtils::gattServiceToString(p->getUUID().getNative()->uuid.uuid32) + " Service: " + p->getUUID().toString());
	obj.setString("icon", "service");
	JsonObject state = JSON::createObject();
	state.setBoolean("opened", true);
	obj.setObject("state", state);

	for (auto it=charact->begin(); it!=charact->end(); ++it) {  // TODO add descriptors enumerator, add short uuid if characteristic !UNKNOWN
		JsonObject ch = JSON::createObject();
		//ch.setString("id", it->second->getUUID().toString());
		ch.setString("text", BLEUtils::gattCharacteristicUUIDToString(p->getUUID().getNative()->uuid.uuid32) + " Characteristic: " + it->second->getUUID().toString());
		ch.setString("icon", "characteristic");
		arr.addObject(ch);
	}

	obj.setArray("children", arr);
	return obj;
}

void BLEExplorer::createServer(std::string name){
	BLEDevice::init(name);
	pServer = BLEDevice::createServer();
	mCharacteristics = new BLECharacteristicMap();
	mServices = new BLEServiceMap();
}

JsonObject BLEExplorer::addService(BLEUUID _uuid){

	BLEService *pservice = pServer->createService(_uuid.toString());
	mServices->setByUUID(pservice->getUUID().toString(), pservice);
	pservice->start();
	JsonObject obj = JSON::createObject();
	obj.setString("icon", "service");
	obj.setString("id", pservice->getUUID().toString());
	obj.setString("parent", "#");
	obj.setString("text", BLEUtils::gattServiceToString(pservice->getUUID().getNative()->uuid.uuid32) + " Service: " + pservice->getUUID().toString());

	return obj;
}

JsonObject BLEExplorer::addCharacteristic(BLEUUID uuid, BLEUUID service){
	BLEService *pservice = mServices->getByUUID(service);
	BLECharacteristic *charact = pservice->createCharacteristic(BLEUUID(uuid), {BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY});
	mCharacteristics->setByUUID(charact->getUUID().toString(), charact);
	pservice->start();
	charact->setValue("Private name");
	JsonObject obj = JSON::createObject();
	obj.setString("icon", "characteristic");
	obj.setString("id", charact->getUUID().toString());
	obj.setString("parent", pservice->getUUID().toString());
	obj.setString("text", BLEUtils::gattCharacteristicUUIDToString(uuid.getNative()->uuid.uuid32) + " Characteristic: " + charact->getUUID().toString());
	return obj;
}

JsonObject BLEExplorer::addDescriptor(BLEUUID uuid, BLEUUID charact){
	BLECharacteristic *pcharact = mCharacteristics->getByUUID(BLEUUID(charact));
	BLEDescriptor descr = BLEDescriptor(uuid);
	pcharact->addDescriptor(&descr);
	JsonObject obj = JSON::createObject();
	obj.setString("icon", "descriptor");
	obj.setString("id", descr.getUUID().toString());
	obj.setString("parent", pcharact->getUUID().toString());
	obj.setString("text", BLEUtils::gattDescriptorUUIDToString(uuid.getNative()->uuid.uuid32) + " Descriptor: " + uuid.toString());
	return obj;
}

void BLEExplorer::startAdvertising(){
	pServer->getAdvertising()->start();
}

void BLEExplorer::stopAdvertising(){
	pServer->getAdvertising()->stop();
}
