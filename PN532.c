#include "PN532.h"
#include "i2c.h"

#define PN532_PACKBUFFSIZ 64
unsigned int pn532_packetbuffer[PN532_PACKBUFFSIZ];

unsigned int block_1 = 48;
unsigned int block_2 = 49;

int pn532ack[] = {0x01, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};


void PN532_init()
{
    // send dummy command to wake board up
    getFirmwareVersion();
    setPassiveActivationRetries(0xFF);

    // configure board to read RFID tags
    SAMConfig();
}


void readACKFrame(int addr)
{
    unsigned char ack_readbytes[7];

    // debug_printf("Read ACK Frame");

    i2c_Start(addr);

    int i = 0;
    while (i < 6) {
        ack_readbytes[i] = i2c_Read();
        // debug_printf("%x ", ack_readbytes[i]);
        i++;
    }
    ack_readbytes[i] = i2c_ReadLast();
    // debug_printf("%x \n", ack_readbytes[i]);

    i2c_Stop();

    /*
    if (0 == strncmp((char *)ack_readbytes, (char *)pn532ack, 7)) {
        debug_printf("ACK Frame ok\n");
    } else {
        debug_printf("ACK Frame fail\n");
    }
    */
}


void sendCommand(unsigned int command[], int cmdlen, int addr)
{
    unsigned int checksum;
    int size = 8 + cmdlen;
    unsigned char i2c_messages[size];

    cmdlen++;
    checksum = PN532_PREAMBLE + PN532_STARTCODE1 + PN532_STARTCODE2;

    //Preampel and Startcode
    i2c_messages[0]=PN532_PREAMBLE;
    i2c_messages[1]=PN532_STARTCODE1;
    i2c_messages[2]=PN532_STARTCODE2;
    // Length = TFI +  PD0 to PDn
    i2c_messages[3]=cmdlen;
    //  Length Checksum LCS = Lower byte of [LEN + LCS] = 00
    unsigned int cmdlen_1=~cmdlen + 1;
    i2c_messages[4]=cmdlen_1;
    //TFI
    i2c_messages[5]=PN532_HOSTTOPN532;
    checksum += PN532_HOSTTOPN532;
    //Command and data packets

    int j = 6;
    for (int i = 0; i < cmdlen - 1; i++) {
        i2c_messages[j] = command[i];
        checksum += command[i];
        j++;
    }

    // Data Checksum DCS = Lower byte of [TFI + PD0 + PD1 + � + PDn + DCS] = 00
    unsigned int checksum_1=~checksum;
    i2c_messages[j] = checksum_1;
    //Postampel
    i2c_messages[j + 1] = PN532_POSTAMBLE;

    // debug_printf("Send Command: ");
    int x = 0;
    while(x < size)
    {
      // debug_printf("%x ", i2c_messages[x]);
      x++;
    }
    // debug_printf("\n");

    i2c_Start(addr);
    i2c_Write(i2c_messages, size);
    i2c_Stop();

    i2c_Start(PN532_DEVICE_ADDR+1);
    unsigned char statusbyte=i2c_ReadLast();
    //debug_printf("Read Status byte %x \n", statusbyte);
    i2c_Stop();

}


void NFC_delay(void)
{
    int i = 0, x = 0;
    for (i = 0; i < 59999; i++) {
        x++;
    }
}


void readResponseFrames(unsigned int addr, unsigned int * result)
{
    NFC_delay();
    i2c_Start(addr);

    int response_1[8];

    int i = 0;
    while (i < 8) {
    response_1[i] = i2c_Read();
        // debug_printf("%x ", response_1[i]);
        i++;
    }

    if (response_1[1] != 0x00 && response_1[2] != 0x00)
    {
        // debug_printf("\n");
        return;
    }

    int response_2[response_1[4]];
    int j = 0;

    i = 0;
    while(i < response_1[4] - 1) {
        response_2[i] = i2c_Read();
        if (i != response_1[4] - 2) {
            // debug_printf("[%x] ", response_2[i]);
            result[j++] = response_2[i];
        } else {
            // debug_printf("%x ", response_2[i]);
        }
        i++;
    }
    response_2[i] = i2c_ReadLast();
    i2c_Stop();
    // debug_printf("%x \n", response_2[i]);
}


void readPassiveTargetID(unsigned int cardbaudrate, unsigned int * uid)
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


void SAMConfig()
{
    pn532_packetbuffer[0] = PN532_SAMCONFIGURATION;
    pn532_packetbuffer[1] = 0x01; // normal mode;
    pn532_packetbuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
    pn532_packetbuffer[3] = 0x01; // use IRQ pin!

    sendCommand(pn532_packetbuffer, 4, PN532_DEVICE_ADDR);
    readACKFrame(PN532_DEVICE_ADDR + 1);
    readResponseFrame(PN532_DEVICE_ADDR + 1);
}


void readResponseFrame(unsigned int addr)
{
    unsigned int result[40];
    readResponseFrames(addr, result);
}


void writeDataBlock (unsigned int blockNumber, unsigned int *data[])
{
    /* Prepare the first command */
    pn532_packetbuffer[0] = PN532_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                      /* Card number */
    pn532_packetbuffer[2] = PN532_MIFARE_WRITE;       /* Mifare Write command = 0xA0 */
    pn532_packetbuffer[3] = blockNumber;            /* Block Number (0..63 for 1K, 0..255 for 4K) */
    pn532_packetbuffer[4] = data[0];
    pn532_packetbuffer[5] = data[1];
    pn532_packetbuffer[6] = data[2];
    pn532_packetbuffer[7] = data[3];
    pn532_packetbuffer[8] = data[4];
    pn532_packetbuffer[9] = data[5];
    pn532_packetbuffer[10] = data[6];
    pn532_packetbuffer[11] = data[7];
    pn532_packetbuffer[12] = data[8];
    pn532_packetbuffer[13] = data[9];
    pn532_packetbuffer[14] = data[10];
    pn532_packetbuffer[15] = data[11];
    pn532_packetbuffer[16] = data[12];
    pn532_packetbuffer[17] = data[13];
    pn532_packetbuffer[18] = data[14];
    pn532_packetbuffer[19] = data[15];

    sendCommand(pn532_packetbuffer, 20, PN532_DEVICE_ADDR);
    readACKFrame(PN532_DEVICE_ADDR+1);
    readResponseFrame(PN532_DEVICE_ADDR+1);
}


void readDataBlock (unsigned int blockNumber, unsigned int * data)
{
    /* Prepare the command */
    pn532_packetbuffer[0] = PN532_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                      /* Target number */
    pn532_packetbuffer[2] = PN532_MIFARE_READ;        /* Mifare Read command = 0x30 */
    pn532_packetbuffer[3] = blockNumber;            /* Block Number (0..63 for 1K, 0..255 for 4K) */

    sendCommand(pn532_packetbuffer, 4, PN532_DEVICE_ADDR);
    readACKFrame(PN532_DEVICE_ADDR+1);
    readResponseFrames(PN532_DEVICE_ADDR+1, data);
}


int authenticateBlock (unsigned int uid[], unsigned int uidLen, unsigned int blockNumber, unsigned int keyNumber, unsigned int *keyData[])
{
    unsigned int i;

    unsigned int keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    // Prepare the authentication command //
    pn532_packetbuffer[0] = PN532_INDATAEXCHANGE;   /* Data Exchange Header */
    pn532_packetbuffer[1] = 1;                              /* Max card numbers */
    pn532_packetbuffer[2] = (keyNumber) ? PN532_AUTH_WITH_KEYB : PN532_AUTH_WITH_KEYA;
    pn532_packetbuffer[3] = blockNumber;                    /* Block Number (1K = 0..63, 4K = 0..255 */
    pn532_packetbuffer[4] = keyData[0];
    pn532_packetbuffer[5] = keyData[1];
    pn532_packetbuffer[6] = keyData[2];
    pn532_packetbuffer[7] = keyData[3];
    pn532_packetbuffer[8] = keyData[4];
    pn532_packetbuffer[9] = keyData[5];

    //memcpy (pn532_packetbuffer+4, keya, 6);
    for (i = 0; i < uidLen; i++) {
        pn532_packetbuffer[10 + i] = uid[i]; // 4 byte card ID
    }

    sendCommand(pn532_packetbuffer, 10 + uidLen, PN532_DEVICE_ADDR);
    readACKFrame(PN532_DEVICE_ADDR + 1);

    unsigned int result[16];
    readResponseFrames(PN532_DEVICE_ADDR + 1, result);

    if(result[0] == 0x14 || result[0] == 0x27) {
        return 0;
    }


    return 1;
}


void getFirmwareVersion()
{
    pn532_packetbuffer[0] = PN532_FIRMWAREVERSION;
    sendCommand(pn532_packetbuffer, 1, PN532_DEVICE_ADDR);
    readACKFrame(PN532_DEVICE_ADDR + 1);
    readResponseFrame(PN532_DEVICE_ADDR + 1);
}


int writeStringToNFC(char * string)
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
            for (i = 16;i < 32; i++) {
                data[i - 16] = string[i];
            }
            writeDataBlock (block_2, data);
            return 1;
        }
    }
    return 0;
}


int readStringFromNFC(char * string)
{
    unsigned int data[17];

    if(findTargetAndAuth(block_1)) {
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


int findTargetAndAuth(unsigned int block)
{
    unsigned int uidLength = 4;
    int uid[] = {0x0, 0x0, 0x0, 0x0};

    readPassiveTargetID(PN532_MIFARE_ISO14443A, uid);
    unsigned int key[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    return authenticateBlock(uid, uidLength, block, 0, key);
}


void convertDataToString(unsigned int data[], char * string, int start)
{
    for(int i = start; i < 16 + start; i++) {
        string[i] = data[i + 1 - start];
    }
}