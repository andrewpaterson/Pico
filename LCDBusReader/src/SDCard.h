#ifndef __SD_CARD_H__
#define __SD_CARD_H__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"


struct SSDCardPins
{
    int iSDClkPin;
    int iSDCmdPin;
    int iSDDat0Pin;
    int iSDDat1Pin;
    int iSDDat2Pin;
    int iSDDat3Pin;

    void Init(int iSDClkPin, int iSDCmdPin, int iSDDat0Pin, int iSDDat1Pin, int iSDDat2Pin, int iSDDat3Pin);
};


void sd_card_init(SSDCardPins *pPins);
void sd_initial_tick(SSDCardPins* pPins);
void sd_cmd0_go_idle(SSDCardPins* pPins, bool bSDMode);
bool sd_cmd8_interface_condition(SSDCardPins* pPins);


enum ESDState
{
    SDS_Idle,
    SDS_Ready,
    SDS_Identify,
    SDS_StandBy,
    SDS_Transmit,
    SDS_Data,
    SDS_Receive,
    SDs_Program,
    SDS_Disabled,

    SDS_Reserved
};


struct SSDCardStatus
{
    bool        bOutOfRange;
    bool        bAddressError;
    bool        bBlockLengthError;
    bool        bSequenceEraseError;
    bool        bWriteBlockEraseError;
    bool        bWriteProtectedViolation;
    bool        bCardIsLocked;
    bool        bLockOrUnlockFailed;
    bool        bPreviousCRCFailed;
    bool        bIllegalCommand;
    bool        bInternalECCFailed;
    bool        bInternalError;
    bool        bGeneralError;

    bool        bCSDOverwriteError;
    bool        bErasePartiallyFailed;
    bool        bInternalECCDisabled;
    bool        bEraseNotExecuted;

    ESDState    eState;

    bool        bReadyForData;
    bool        bExtensionFunctionEvent;
    bool        bApplicationCommandExpected;
    bool        bAuthenticationError;
};


struct SSDOCR
{
    bool bDonePoweringOn;
    bool b27_28Volts;
    bool b28_29Volts;
    bool b29_30Volts;
    bool b30_31Volts;
    bool b31_32Volts;
    bool b32_33Volts;
    bool b33_34Volts;
    bool b34_35Volts;
    bool b35_36Volts;
    bool bSwitchTo18Volts;     //S18A
    bool bOver2TBytes;         //CO2T
    bool bUHSIIStatus;
    bool bCardCapacityStatus;  //CCS
};


bool sd_acmd41_application_operating_condition(SSDCardPins* pPins, u_int16_t uiRCA, SSDOCR* pOCR);
bool repeat_sd_acmd41_application_operating_condition(SSDCardPins* pPins, u_int16_t uiRCA, SSDOCR* pOCR);


struct SSDCID
{
    uint8_t     uiManufacturerID;
    uint16_t    uiApplicationID;
    char        szProductName[6];
    uint8_t     uiProductRevision;
    int32_t     iProductSerialNumber;

    uint16_t    uiManufactureYear;
    uint8_t     uiManufactureMonth;
};


bool sd_cmd2_send_cid(SSDCardPins* pPins, SSDCID* pCID);


struct SSDR6Status
{
    bool        bPreviousCRCFailed;
    bool        bIllegalCommand;
    bool        bGeneralError;

    ESDState    eState;

    bool        bReadyForData;
    bool        bExtensionFunctionEvent;
    bool        bApplicationCommandExpected;
    bool        bAuthenticationError;
};


bool sd_cmd3_publish_relative_address(SSDCardPins* pPins, uint16_t* puiAddress, SSDR6Status* pStatus);


enum ESDRegisterStructure
{
    SDRS_StandardCapacity,
    SDRS_HighCapacity,
    SDRS_UltraCapacity,

    SDRS_Reserved
};


enum ESDFileFormat
{
    SDFF_HardDiskLikeWithPartitionTable,
    SDFF_FloppyDiskLikeBootSectorOnly,
    SDFF_UniversalFileFormat,
    SDFF_Unknown,

    SDFF_Reserved
};

struct SSDCSD
{
    ESDRegisterStructure    eCSDStructure;
    float                   fAsynchronousDataAccessTime;  //Nanoseconds.
    float                   fWorstCaseDataAccessTime;  //Clock cycles.
    float                   fSingleDataMaxTransmissionSpeed;  //Bits per second.
    bool                    bCommandClassSwitch;
    bool                    bCommandClassWriteProtect;
    bool                    bCommandClassLockCard;
    bool                    bCommandClassExtensions;
    int                     iMaxReadBlockLength;
    bool                    bReadPartialBlock;
    bool                    bWriteMisalignedBlock;
    bool                    bReadMisalignedBlock;
    bool                    bDSRImplemented;
    int                     iDeviceSize;
    uint8_t                 uiWriteSpeedFactor;
    int                     iMaxWriteBlocklength;
    bool                    bWritePartialBlock;
    bool                    bCopy;
    bool                    bPermanentWriteProtection;
    bool                    bTemporaryWriteProtection;
    ESDFileFormat           eFileFormat;
};


bool sd_cmd9_send_csd(SSDCardPins* pPins, uint16_t uiAddress, SSDCSD* pCSD);
bool sd_cmd7_select_or_deselect_card(SSDCardPins* pPins, uint16_t uiAddress, SSDCardStatus* pStatus);
bool sd_cmd17_read_single_block_narrow(SSDCardPins* pPins, int iBlock, int iExpectedBytes, uint8_t* pvData, SSDCardStatus* pStatus);


struct SSDFunctionSwitchStatus
{
    uint16_t    uiMaximumCurrentInMilliAmps;
    uint16_t    uiFunctionGroup6SupportBits;
    uint16_t    uiFunctionGroup5SupportBits;
    uint16_t    uiFunctionGroup4SupportBits;
    uint16_t    uiFunctionGroup3SupportBits;
    uint16_t    uiFunctionGroup2SupportBits;
    uint16_t    uiFunctionGroup1SupportBits;

    uint8_t     uiFunctionGroup6Selection;
    uint8_t     uiFunctionGroup5Selection;
    uint8_t     uiFunctionGroup4Selection;
    uint8_t     uiFunctionGroup3Selection;
    uint8_t     uiFunctionGroup2Selection;
    uint8_t     uiFunctionGroup1Selection;

    uint8_t     uiDataStructureVersion;

    uint16_t    uiFunctionGroup6BusyStatus;
    uint16_t    uiFunctionGroup5BusyStatus;
    uint16_t    uiFunctionGroup4BusyStatus;
    uint16_t    uiFunctionGroup3BusyStatus;
    uint16_t    uiFunctionGroup2BusyStatus;
    uint16_t    uiFunctionGroup1BusyStatus;
};


bool sd_cmd6_switch(SSDCardPins* pPins, bool bSwitch, uint8_t uiPowerLimit, uint8_t uiDriveStrength, uint8_t uiCommandSystem, uint8_t uiAccessMode, SSDCardStatus* pStatus, SSDFunctionSwitchStatus* pFunctionSwitchStatus);
bool check_sd_cmd6_switch(SSDCardPins* pPins, uint8_t uiPowerLimit, uint8_t uiDriveStrength, uint8_t uiCommandSystem, uint8_t uiAccessMode, SSDCardStatus* pStatus, SSDFunctionSwitchStatus* pFunctionSwitchStatus);
bool sd_acmd6_set_bus_width(SSDCardPins* pPins, u_int16_t uiAddress, u_int8_t uiBusWidth, SSDCardStatus* pStatus);
bool sd_cmd17_read_single_block_wide(SSDCardPins* pPins, int iBlock, int iExpectedBytes, uint8_t* pvData, SSDCardStatus* pStatus);
bool sd_cmd_23_set_block_count(SSDCardPins* pPins, int iCount, SSDCardStatus* pStatus);

#endif // #ifndef __SD_CARD_H__

