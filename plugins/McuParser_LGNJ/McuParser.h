#pragma once 

#include <cstdint>
#include <pthread.h>
#include <list>
#include <string>
#include <vector>
#include "ParserBase.h"
#include "McuData.h"
#include "McuDef.h"

struct NotifyItem {
    int type ;      //类型, 1 整形; 2 字符
    MCU_NOTIFY code;
    int data1;
    long data2;
    std::string str;
};

using namespace std;

class McuParser : public ParserBase
{
public:
    McuParser();
    virtual ~McuParser();

    int GetInt(MCU_INDEX index);
	uint32_t GetUInt(MCU_INDEX index);
    int SendCmd(MCU_COMMAND cmd, int data1, long data2);
    int SendCmd(MCU_COMMAND cmd, std::string const &str, double p1 ,double p2, double p3 ,double p4 );
    void SetNotify(OnMcuNotify notifier) {m_notifier = notifier;}
    void SetNotify(OnMcuNotifyStr notifier) {m_notifier_str = notifier;}
protected:
    virtual int ParseData( LoopBuffer& buf ) override;
    void AddToNotify(MCU_NOTIFY code, int data1, long data2 = 0);
    void AddToNotifyStr(MCU_NOTIFY code, int data1, char* data2);

    static void* ThreadProc(void* arg);
    void Proc();

    std::string convertToHexString(unsigned char* data, size_t length);
    std::vector<std::string> splitString(const std::string& str, char delimiter);


    bool m_running;
    list<NotifyItem*> m_notifyItems;

    OnMcuNotify m_notifier;
    OnMcuNotifyStr m_notifier_str;

    pthread_mutex_t	m_mutex;
    pthread_mutex_t	m_mutex2;



    reqMCUShakeHandCtrl m_objMcuShakeHandCtrl{{0xCA53 , sizeof(reqMCUShakeHandCtrl)-4 , 0x8000}};
    reqEngineCtrl  m_objEngineCtrl{{0xCA53 , sizeof(reqEngineCtrl)-4 , 0x8001}};
    reqACCtrl m_objACCtrl{{0xCA53 , sizeof(reqACCtrl)-4 , 0x8002}};
    reqElectricControlLiftingCtrl m_objElectricControlLiftingCtrl{{0xCA53 , sizeof(reqElectricControlLiftingCtrl)-4 , 0x8003}};
    reqHydraulicValveCtrl m_objHydraulicValveCtrl{{0xCA53 , sizeof(reqHydraulicValveCtrl)-4 , 0x8004}};
    reqGearboxCtrl m_objGearboxCtrl{{0xCA53 , sizeof(reqGearboxCtrl)-4 , 0x8005}};
    reqLedProgramCtrl m_objLedProgramCtrl{{0xCA53 , sizeof(reqLedProgramCtrl)-4 , 0x8006}};
    reqSkiyLightCtrl m_objSkiyLightCtrl{{0xCA53 , sizeof(reqSkiyLightCtrl)-4 , 0x8007}};
    reqRearviewMirrorCtrl  m_objRearviewMirrorCtrl{{0xCA53 , sizeof(reqRearviewMirrorCtrl)-4 , 0x8008}};
    reqAtmosphereLightsCtrl  m_objAtmosphereLightsCtrl{{0xCA53 , sizeof(reqAtmosphereLightsCtrl)-4 , 0x8009}};
    reqIVISettingCtrl m_objIVISettingCtrl{{0xCA53 , sizeof(reqIVISettingCtrl)-4 , 0x800A}};
    reqIVICtrlSOC m_objIVICtrlSOC{{0xCA53 , sizeof(reqIVICtrlSOC)-4 , 0x800B}};
    reqWorkCtrl m_objWorkCtrl{{0xCA53 , sizeof(reqWorkCtrl)-4 , 0x800C}};
    reqCalibrationRequest4HCtrl m_objCalibrationRequest4HCtrl{{0xCA53 , sizeof(reqCalibrationRequest4HCtrl)-4 , 0x800D}};
    reqSOCCtrlMCU m_objSocCtrlMcu{{0xCA53 , sizeof(reqSOCCtrlMCU)-4 , 0x800E}};
    reqTPMCtrl m_objTpmCtrl{{0xCA53 , sizeof(reqTPMCtrl)-4 , 0x8081}};


    tagMCUShakeHandPacket m_objMcuShakeHandStatus;
    tagEngineStatusPacket m_objEngineStatus;
    tagACStatusPacket m_objAcStatus;
    tagACStatusPacketEx m_objAcStatusEx; //空调状态扩展数�?
    tagElectricCtrlLiftingStatusPacket m_objElectricCtrlLiftingStatus ;
    tagHydraulicValveStatusPacket m_objHydraulicValveStatus ;
    tagGearboxStatusPacket m_objGearBoxStatus ;
    tagRearviewMirrorStatusPacket m_objRearviewMirrorStatus ;
    tagAtmosphereLightsStatusPacket m_objAtmosphereLightsStatus ;
    tagSOCSensorStatePacket m_objSocSensorStatus ;
    tagTBoxTimeStatePacket m_objTBoxTimeStatus ;
    tagTBoxTimeStatePacketEx m_objTBoxTimeStatusEx;
    tagWorkInfoPacket m_objWorkInfo ; 
    tagMcuPowerModePacket m_objMcuPowerModelStatus ;
    tagLightProgramSwitchPacket m_objLightProgramSwitchStatus ;
    tagHightSpeedSettingPacket m_objHightSpeedSettingStatus ;
    tagFuelConsumptionPacket m_objFuelConsumptionStatus ;
    tagCalibrationParameters4HPacket m_objCalibrationParameters4HStatus;
    tagCalibrationResult4HPacket m_objCalibrationResult4HStatus;
    tagDiagnosisTroubleCodePacket m_objDiagnosisTroubleCodeStatus;
    tagTPMPacket m_objTpmStatus ;
};
