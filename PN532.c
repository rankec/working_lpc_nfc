/*
    The NFC driver.
*/


#include "PN532.h"
#include "i2c.h"


#define PN532_PACKBUFFSIZ 64
unsigned int pn532_packetbuffer[PN532_PACKBUFFSIZ];

unsigned int block_1 = 48;
unsigned int block_2 = 49;

int pn532ack[] = {0x01, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};


/**
    Initializes the NFC module.
*/
void PN532_init(void)
{
    // send dummy command to wake board up
    getFirmwareVersion();
    setPassiveActivationRetries(0xFF);

    // configure board to read RFID tags
    SAMConfig();
}


/**
    Reads an ACK (acknowledge) frame.
    @param addr The address to be read from.
*/
void readACKFrame(int addr)
{
    unsigned char ack_readbytes[7];
    
    i2c_Start(addr);

    int i = 0;
    while (i++ < 6) {
        ack_readbytes[i] = i2c_Read();
    }
    ack_readbytes[i] = i2c_ReadLast();

    i2c_Stop();
}


/**
    Sends one or more commands to the NFC module.
    @param command The array of commands.
    @param cmdlen The number of commands.
    @param addr The address to be read from.
*/
void sendCommand(unsigned int command[], int cmdlen, int addr)
{
    unsigned int checksum;
    int size = 8 + cmdlen;
    unsigned char i2c_messages[size];

    cmdlen++;
    checksum = PN532_PREAMBLE + PN532_STARTCODE1 + PN532_STARTCODE2;

    // Preampel and Startcode
    i2c_messages[0] = PN532_PREAMBLE;
    i2c_messages[1] = PN532_STARTCODE1;
    i2c_messages[2] = PN532_STARTCODE2;
    
    // Length = TFI + PD0 to PDn
    i2c_messages[3] = cmdlen;

    // Length Checksum LCS = Lower byte of [LEN + LCS] = 00
    unsigned int cmdlen_1 = ~cmdlen + 1;
    i2c_messages[4] = cmdlen_1;
    
    // TFI
    i2c_messages[5] = PN532_HOSTTOPN532;
    checksum += PN532_HOSTTOPN532;

    int j = 6;
    for (int i = 0; i < cmdlen - 1; i++) {
        i2c_messages[j] = command[i];
        checksum += command[i];
        j++;
    }

    // Data Checksum DCS = Lower byte of [TFI + PD0 + PD1 + ? + PDn + DCS] = 00
    unsigned int checksum_1 = ~checksum;
    i2c_messages[j] = checksum_1;
    
    // Postampel
    i2c_messages[j + 1] = PN532_POSTAMBLE;

    i2c_Start(addr);
    i2c_Write(i2c_messages, size);
    i2c_Stop();

    i2c_Start(PN532_DEVICE_ADDR + 1);
    unsigned char statusbyte = i2c_ReadLast();
    i2c_Stop();
}


/**
    Pauses execution for a bit.
    This function is called to delay the program execution to give the NFC module time to... do things.
*/
void NFC_delay(void)
{
    int x = 0;
    for (int i = 0; i < 59999; i++) {
        x++;
    }
}


/**
    Reads response frames.
    @param addr The address to be read from.
    @param result The result. This is written to by the function.
*/
void readResponseFrames(unsigned int addr, unsigned int *result)
{
    NFC_delay();
    i2c_Start(addr);

    int response_1[8];

    int i = 0;
    while (i < 8) {
    response_1[i] = i2c_Read();
        i++;
    }

    if (response_1[1] != 0 && response_1[2] != 0) {
        return;
    }

    int response_2[response_1[4]];
    int j = 0;

    i = 0;
    while(i < response_1[4] - 1) {
        response_2[i] = i2c_Read();
        if (i != response_1[4] - 2) {
            result[j++] = response_2[i];
        }
        i++;
    }

    response_2[i] = i2c_ReadLast();
    i2c_Stop();
}


/**
    Finds and reads all available NFC tags.
    @param cardbaudrate The BAUD rate.
    @param uid The UID of the NFC tag.
*/    
void readPassiveTargetID(unsigned int cardbaudrate, unsigned int *uid)
{
    pn532_packetbuffer[0] = PN532_INLISTPASSIVETARGET;
    pn532_packetbuffer[1] = 1; // max 1 cards at once
    pn532_packetbuffer[2] = cardbaudrate;

    sendCommand(pn532_packetbuffer, 3, PN532_DEVICE_ADDR);
    readACKFrame(PN532_DEVICE_ADDR + 1);
    unsigned int data[20];
    readResponseFrames(PN532_DEVICE_ADDR + 1, data);
    uid[0] = data[6];
    uid[1] = data[7];
    uid[2] = data[8];
    uid[3] = data[9];
}


/**
    Sets passive activation entries.
    @param maxRetries The meximum number of retries.
*/
void setPassiveActivationRetries(unsigned int maxRetries)
{
    pn532_packetbuffer[0] = PN532_COMMAND_RFCONFIGURATION;
    pn532_packetbuffer[1] = 5;    // Config item 5 (MaxRetries)
    pn532_packetbuffer[2] = 0xFF; // MxRtyATR (default = 0xFF)
    pn532_packetbuffer[3] = 0x01; // MxRtyPSL (default = 0x01)
    pn532_packetbuffer[4] = maxRetries;

    sendCommand(pn532_packetbuffer, 5, PN532_DEVICE_ADDR);
    readACKFrame(PN532_DEVICE_ADDR + 1);
    readResponseFrame(PN532_DEVICE_ADDR + 1);
}


/**
    Configures the NFC module as an NFC reader.
*/
void SAMConfig(void)
{
    pn532_packetbuffer[0] = PN532_SAMCONFIGURATION;
    pn532_packetbuffer[1] = 0x01; // normal mode;
    pn532_packetbuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
    pn532_packetbuffer[3] = 0x01; // use IRQ pin!

    sendCommand(pn532_packetbuffer, 4, PN532_DEVICE_ADDR);
    readACKFrame(PN532_DEVICE_ADDR + 1);
    readResponseFrame(PN532_DEVICE_ADDR + 1);
}


/**
    Reads one reponse frame.
    @param addr The address to be read from.
*/
void readResponseFrame(unsigned int addr)
{
    unsigned int result[40];
    readResponseFrames(addr, result);
}


/**
    Writes a data block.
    @param blockNumber The block number.
    @param data The data to be written.
*/
void writeDataBlock (unsigned int blockNumber, unsigned int *data[])
{
    // Prepare the first command
    pn532_packetbuffer[0] = PN532_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                      // Card number
    pn532_packetbuffer[2] = PN532_MIFARE_WRITE;     // Mifare Write command = 0xA0
    pn532_packetbuffer[3] = blockNumber;            // Block Number (0..63 for 1K, 0..255 for 4K)
    
    for (int i = 0; i < 16; i++) {
        pn532_packetbuffer[i + 4] = data[i];
    }

    sendCommand(pn532_packetbuffer, 20, PN532_DEVICE_ADDR);
    readACKFrame(PN532_DEVICE_ADDR + 1);
    readResponseFrame(PN532_DEVICE_ADDR + 1);
}


/**
    Reads a data block.
    @param blockNumber The block number.
    @param data The data to be read. This is written to be the function.
*/
void readDataBlock(unsigned int blockNumber, unsigned int *data)
{
    // Prepare the command
    pn532_packetbuffer[0] = PN532_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                      // Target number
    pn532_packetbuffer[2] = PN532_MIFARE_READ;      // Mifare Read command = 0x30
    pn532_packetbuffer[3] = blockNumber;            // Block Number (0..63 for 1K, 0..255 for 4K)

    sendCommand(pn532_packetbuffer, 4, PN532_DEVICE_ADDR);
    readACKFrame(PN532_DEVICE_ADDR + 1);
    readResponseFrames(PN532_DEVICE_ADDR + 1, data);
}


/**
    Authenticates a block.
    @param uid The array of UIDs.
    @param uidLen The number of UIDs.
    @param blockNumber The block number.
    @param keyNumber The key number.
    @param keyData The key data.
*/
int authenticateBlock (unsigned int uid[], unsigned int uidLen, unsigned int blockNumber, unsigned int keyNumber, unsigned int *keyData[])
{
    unsigned int i;

    // Prepare the authentication command
    pn532_packetbuffer[0] = PN532_INDATAEXCHANGE;   // Data Exchange Header
    pn532_packetbuffer[1] = 1;                      // Max card numbers
    pn532_packetbuffer[2] = (keyNumber) ? PN532_AUTH_WITH_KEYB : PN532_AUTH_WITH_KEYA;
    pn532_packetbuffer[3] = blockNumber;            // Block Number (1K = 0..63, 4K = 0..255)
    pn532_packetbuffer[4] = keyData[0];
    pn532_packetbuffer[5] = keyData[1];
    pn532_packetbuffer[6] = keyData[2];
    pn532_packetbuffer[7] = keyData[3];
    pn532_packetbuffer[8] = keyData[4];
    pn532_packetbuffer[9] = keyData[5];

    for (i = 0; i < uidLen; i++) {
        pn532_packetbuffer[10 + i] = uid[i]; // 4 byte card ID
    }

    sendCommand(pn532_packetbuffer, 10 + uidLen, PN532_DEVICE_ADDR);
    readACKFrame(PN532_DEVICE_ADDR + 1);

    unsigned int result[16];
    readResponseFrames(PN532_DEVICE_ADDR + 1, result);

    if (result[0] == 0x14 || result[0] == 0x27) {
        return 0;
    }
    return 1;
}


/**
    Gets the firmware version.
*/
void getFirmwareVersion(void)
{
    pn532_packetbuffer[0] = PN532_FIRMWAREVERSION;
    sendCommand(pn532_packetbuffer, 1, PN532_DEVICE_ADDR);
    readACKFrame(PN532_DEVICE_ADDR + 1);
    readResponseFrame(PN532_DEVICE_ADDR + 1);
}


/**
    Writes a string to the NFC tag.
    @param The string to be written.
*/
int writeStringToNFC(char *string)
{
    if (findTargetAndAuth(block_1)) {
        unsigned int data[16];
        int i;
        for (i = 0; i < 16; i++) {
            data[i] = string[i];
        }
        writeDataBlock (block_1, data);
        if (findTargetAndAuth(block_2)) {
            unsigned int data[16];
            for (i = 16; i < 32; i++) {
                data[i - 16] = string[i];
            }
            writeDataBlock (block_2, data);
            return 1;
        }
    }
    return 0;
}


/**
    Reads a string from the NFC tag.
    @param string The string to be read. This is written to by the function.
*/
int readStringFromNFC(char *string)
{
    unsigned int data[17];

    if (findTargetAndAuth(block_1)) {
        readDataBlock(block_1, data);
        convertDataToString(data, string, 0);
        if (findTargetAndAuth(block_2)) {
            readDataBlock(block_2, data);
            convertDataToString(data, string, 16);
            return 1;
        }
    }
    return 0;
}


/**
    Finds and authenticates an NFC target.
    @param block The block to authenticate.
*/
int findTargetAndAuth(unsigned int block)
{
    unsigned int uidLength = 4;
    int uid[] = {0x0, 0x0, 0x0, 0x0};

    readPassiveTargetID(PN532_MIFARE_ISO14443A, uid);
    unsigned int key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    return authenticateBlock(uid, uidLength, block, 0, key);
}


/**
    Converts data to a string.
    @param data The data to be converted.
    @param string The string that contains the result. This is written to by the function.
    @param start Where to start converting.
*/
void convertDataToString(unsigned int data[], char *string, int start)
{
    for (int i = start; i < 16 + start; i++) {
        string[i] = data[i + 1 - start];
    }
}