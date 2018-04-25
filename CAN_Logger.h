/***************************************************************************
  This is a library that logs specified CAN IDs

 ***************************************************************************/
#ifndef __CAN_LOGGER_H__
#define __CAN_LOGGER_H__

#include "Arduino.h"
#include "FlexCAN.h"
#include <vector>
#include <SD.h>

class LogClass : public CANListener 
{
public:
	void gotFrame(CAN_message_t &frame, int mailbox); 	//overrides the parent version so we can actually do something
	void storeFrame(CAN_message_t &frame);
	void writeFrame(int index);

	std::vector< uint16_t > _ids;
	std::vector< uint64_t > _data;
	bool _recordAll = false;
	char _logname[13];

};

class CAN_Logger
{
  public:
    CAN_Logger();
    CAN_Logger(int8_t speedPin, bool highSpeedMode);
    CAN_Logger(int8_t speedPin, bool highSpeedMode, int8_t bus);
    CAN_Logger(int8_t speedPin, bool highSpeedMode, int8_t bus, int8_t txAlt, int8_t rxAlt);

    bool begin(std::vector< uint16_t> ids, String lname, uint32_t baud = 500000);

    void getData(uint64_t data[])	
    {	std::copy(logClass._data.begin(), logClass._data.end(), data);	}
    void getIDs(uint16_t ids[])
    {	std::copy(logClass._ids.begin(), logClass._ids.end(), ids);		}
    int getNumIDs()
    {	return logClass._ids.size();	}

  private:
 	LogClass logClass;

	int8_t _speedPin;
	bool _highSpeedMode;
	int8_t _bus;
	uint8_t _txAlt;
	uint8_t _rxAlt;

};

#endif