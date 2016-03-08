/*
    The NFC driver.
*/

#ifndef _PN532_H
#define _PN532_H


#define PN532_PREAMBLE 0x00
#define PN532_STARTCODE1 0x00
#define PN532_STARTCODE2 0xFF
#define PN532_POSTAMBLE 0x00

#define PN532_HOSTTOPN532 0xD4

#define PN532_FIRMWAREVERSION 0x02
#define PN532_POWERDOWN 0x16
#define PN532_GETGENERALSTATUS 0x04
#define PN532_SAMCONFIGURATION  0x14
#define PN532_INLISTPASSIVETARGET 0x4A
#define PN532_INDATAEXCHANGE 0x40
#define PN532_MIFARE_READ 0x30
#define PN532_MIFARE_WRITE 0xA0

#define PN532_AUTH_WITH_KEYA 0x60
#define PN532_AUTH_WITH_KEYB 0x61


#define PN532_WAKEUP 0x55

#define PN532_SPI_STATREAD 0x02
#define PN532_SPI_DATAWRITE 0x01
#define PN532_SPI_DATAREAD 0x03
#define PN532_SPI_READY 0x01

#define PN532_DEVICE_ADDR 72


#define PN532_MIFARE_ISO14443A 0x0

#define PN532_COMMAND_RFCONFIGURATION 0x32



void PN532_init(void);

void readACKFrame(int addr);

void sendCommand(unsigned int command[], int cmdlen, int addr);

void NFC_delay(void);

void readResponseFrames(unsigned int addr, unsigned int *result);
    
void readPassiveTargetID(unsigned int cardbaudrate, unsigned int *uid);

void setPassiveActivationRetries(unsigned int maxRetries);

void SAMConfig(void);

void readResponseFrame(unsigned int addr);

void writeDataBlock (unsigned int blockNumber, unsigned int *data[]);

void readDataBlock(unsigned int blockNumber, unsigned int *data);

int authenticateBlock (unsigned int uid[], unsigned int uidLen,
    unsigned int blockNumber, unsigned int keyNumber, unsigned int *keyData[]);

void getFirmwareVersion(void);

int writeStringToNFC(char *string);

int readStringFromNFC(char *string);

int findTargetAndAuth(unsigned int block);

void convertDataToString(unsigned int data[], char *string, int start);







#endif