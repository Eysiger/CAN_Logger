/***************************************************************************
  This is a library that logs specified CAN IDs

 ***************************************************************************/
#include "Arduino.h"
#include "FlexCAN.h"
#include "CAN_Logger.h"


/***************************************************************************
 Public FUNCTIONS
 ***************************************************************************/
CAN_Logger::CAN_Logger()
  : _speedPin(-1), _highSpeedMode(0), _bus(0), _txAlt(0), _rxAlt(0)
{ }

CAN_Logger::CAN_Logger(int8_t speedPin, bool highSpeedMode)
  : _speedPin(speedPin), _highSpeedMode(highSpeedMode), _bus(0), _txAlt(0), _rxAlt(0)
{ }

CAN_Logger::CAN_Logger(int8_t speedPin, bool highSpeedMode, int8_t bus)
  : _speedPin(speedPin), _highSpeedMode(highSpeedMode), _bus(bus),_txAlt(0), _rxAlt(0)
{ }

CAN_Logger::CAN_Logger(int8_t speedPin, bool highSpeedMode, int8_t bus, int8_t txAlt, int8_t rxAlt)
  : _speedPin(speedPin), _highSpeedMode(highSpeedMode), _bus(bus), _txAlt(txAlt), _rxAlt(rxAlt)
{ }

bool CAN_Logger::begin(std::vector< uint16_t > ids, String lname, uint32_t baud) {
  static struct CAN_filter_t defaultMask;

  if (_bus == 0)
  {
    Can0.begin(baud, defaultMask, _txAlt, _rxAlt);
  }
  else if (_bus==1)
  {
    Can1.begin(baud, defaultMask, _txAlt, _rxAlt);
  }
  else { return false; }

  pinMode(_speedPin, OUTPUT);
  if (_highSpeedMode) { 
    digitalWrite(_speedPin, LOW);		// High speed mode
  }
  else {
  	digitalWrite(_speedPin, HIGH);		// Low power mode
  }

  if (_bus == 0)
  {
    Can0.attachObj(&logClass);
    logClass.attachGeneralHandler();
  }
  else if (_bus==1)
  {
    Can1.attachObj(&logClass);
    logClass.attachGeneralHandler();
  }

  if (ids[0] == 0x999 ) 
  {
  	logClass._recordAll = true;
  	logClass._ids.clear();
  	logClass._data.clear();
  }
  else
  {
  	logClass._ids.assign(ids.begin(),ids.end());
  	logClass._data.resize(ids.size());
  }
  lname.toCharArray(logClass._logname, 13);

  return true;
}


void LogClass::storeFrame(CAN_message_t &frame)
{
	bool stored = false;
	if (!_ids.empty())
	{
		for (int i = 0; i < _ids.size(); i++)
		{
			//Serial.println(_ids[i]);
			if (frame.id == _ids[i]) 
			{
				uint32_t temp1 = (frame.buf[0] << 24) | (frame.buf[1] << 16) | (frame.buf[2] << 8) | frame.buf[3];
				uint32_t temp2 = (frame.buf[4] << 24) | (frame.buf[5] << 16) | (frame.buf[6] << 8) | frame.buf[7];
				_data[i] = (uint64_t(temp1) << 32) | temp2;
				stored = true;
				writeFrame(i);
				break;
			}
		}
	}
	if ((!stored) && (_recordAll))
	{
		Serial.println("New ID detected!");
		_ids.push_back(frame.id);
		uint32_t temp1 = (frame.buf[0] << 24) | (frame.buf[1] << 16) | (frame.buf[2] << 8) | frame.buf[3];
		uint32_t temp2 = (frame.buf[4] << 24) | (frame.buf[5] << 16) | (frame.buf[6] << 8) | frame.buf[7];
		_data.push_back((uint64_t(temp1) << 32) | temp2);
		writeFrame(_ids.size()-1);
	}
}

void LogClass::writeFrame(int index) 
{
	noInterrupts();
	File logFile = SD.open(_logname, FILE_WRITE);

	// if the file is available, write to it:
	if (logFile) 
	{
		String hex = String(uint8_t(_data[index] >> 56)) + " " + String(uint8_t(_data[index] >> 48)) + " " + String(uint8_t(_data[index] >> 40)) + " " + String(uint8_t(_data[index] >> 32)) + " " + String(uint8_t(_data[index] >> 24)) + " " + String(uint8_t(_data[index] >> 16)) + " " + String(uint8_t(_data[index] >> 8)) + " " + String(uint8_t(_data[index]));
		String logData = String(millis()) + "," + String(_ids[index], HEX) + "," + hex;
	  	logFile.println(logData);
		logFile.close();

		// print to the serial port too:
		// Serial.println(logData);
	}  
	// if the file isn't open, pop up an error:
	else 
	{
		Serial.print("error opening ");
		Serial.println(_logname);
	}
	interrupts();
}

void LogClass::gotFrame(CAN_message_t &frame, int mailbox)
{
    storeFrame(frame);
}

