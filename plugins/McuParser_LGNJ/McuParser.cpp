#include <unistd.h>
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip> // 用于 std::hex 和 std::setw
#include "McuParser.h"
#include "LoopBuffer.h"

#include <cJSON.h>

uint16_t mxc_crc16s_calc(const uint8_t *q, uint32_t len);

pthread_mutex_t *g_pThread = NULL;



McuParser::McuParser()
{
    m_notifier = NULL;
    pthread_mutex_init(&m_mutex, NULL);
    pthread_mutex_init(&m_mutex2, NULL);

    g_pThread = &m_mutex;

    m_running = true;
    pthread_t id;
    pthread_create(&id, NULL, ThreadProc, this);


}

McuParser::~McuParser()
{
    m_running = false;
    pthread_mutex_destroy(&m_mutex);
    pthread_mutex_destroy(&m_mutex2);
}

/**
 * 解析数据（须知协议）
 */
int McuParser::ParseData( LoopBuffer& buf )
{
    int len  = buf.GetSize();
    //是否够一个包头的大小
    if( len < sizeof(tagPacketHead)){
        #ifdef T5COMPILE
            // LOGD( "len < sizeof(tagPacketHead): %d" , len );  
         #else
           printf("len < sizeof(tagPacketHead): %d \n" , len);  
         #endif
         return -1;
    } 

    //获取包头
    tagPacketHead packetHead ;
    buf.GetData2((unsigned char*)&packetHead,sizeof(tagPacketHead));

    // 检查标记 ,如果不对 扔掉 一个字节
    if(packetHead.m_u16Flag != 0xCA53){
        #ifdef T5COMPILE
            LOGD("ERROR-data[0]: %#X",buf[0]);  
        #else
        #endif
        buf.Pop(1);
        return 1;
    }

    //是否读取到一个包的数据 ，包里边记的长度不包含flag 和 length，所以加上4 
    if(len < packetHead.m_u16Length + 4)
    {
        #ifdef T5COMPILE
            LOGD( " (len < rawdata->length + 4) "  );
        #else
            printf(" (len < rawdata->length + 4)");  
        #endif
        return -1;
    }

    // 检查是不是升级包
    if(packetHead.m_u16Command == 0xB001 )
    {
        tagMcuUpdatePacket updatePacket;
        buf.GetData2((unsigned char*)&updatePacket,sizeof(tagMcuUpdatePacket));
        if( mxc_crc16s_calc((unsigned char*)&updatePacket, sizeof(tagMcuUpdatePacket)-2 ) == updatePacket.m_u16Crc){
            AddToNotify(MCU_NOTIFY_OTA_RESULT, packetHead.m_u16Command , updatePacket.m_u8Result );
        }

        buf.Pop( len );
        return -1;
    } 


    /**
     * 普通数据包
     */
    pthread_mutex_lock(&m_mutex);

    if( packetHead.m_u16Command == 0x0000 ){ //MCU与SOC握手

        tagMCUShakeHandPacket tmpPack ;      //作为交换内存使用，避免数据的一次拷贝
        //获取数据
        buf.GetData2( (unsigned char*)&tmpPack,sizeof(tagMCUShakeHandPacket) );
        //计算 crc 校验
        unsigned char *tmp = (unsigned char *)&tmpPack;

        // #ifdef T5COMPILE
        //     for(int i = 0;i < sizeof(tagMCUShakeHandPacket);i++)
        //     {
        //         LOGD("MCUShake-data[ %d ]: %#X",i,buf[i]);  
        //     }
        // #else

        // #endif
        
        uint16_t retCrc = mxc_crc16s_calc(tmp, sizeof(tagMCUShakeHandPacket)-2 );
        if( retCrc != tmpPack.m_u16Crc )  
        { 
            #ifdef T5COMPILE
                LOGD( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #else
                printf( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #endif

            buf.Pop( sizeof(tagMCUShakeHandPacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        } 

        /**
         * 交换当前包
         */
        swap(tmpPack,this->m_objMcuShakeHandStatus );

        //弹出
        buf.Pop( sizeof(tagMCUShakeHandPacket) );

    }else if( packetHead.m_u16Command == 0x0001  ){ //发动机状态报文
        
        tagEngineStatusPacket tmpPack ;      //作为交换内存使用，避免数据的一次拷贝
        //获取数据
        buf.GetData2((unsigned char*)&tmpPack,sizeof(tagEngineStatusPacket));

        //计算 crc 校验
        unsigned char *tmp = (unsigned char *)&tmpPack;

        #ifdef T5COMPILE
            for(int i = 0;i < sizeof(tagEngineStatusPacket);i++)
            {
                LOGD("ENGINE-data[%d]: %#X",i,buf[i]);  
            }
        #else

        #endif

        uint16_t retCrc = mxc_crc16s_calc(tmp, sizeof(tagEngineStatusPacket)-2 );
        if( retCrc != tmpPack.m_u16Crc )  
        {
            #ifdef T5COMPILE
                LOGD( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #else
                printf( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #endif

            buf.Pop(  sizeof(tagEngineStatusPacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        } 

        /**
         * 交换当前包
         */
        swap(tmpPack,this->m_objEngineStatus );

        //弹出
        buf.Pop( sizeof(tagEngineStatusPacket));

    }else if( packetHead.m_u16Command == 0x0002  ){ //空调状态报文
         tagACStatusPacket tmpPack ;      //作为交换内存使用，避免数据的一次拷贝
        //获取数据
        buf.GetData2((unsigned char*)&tmpPack,sizeof(tagACStatusPacket));
        //计算 crc 校验
        unsigned char *tmp = (unsigned char *)&tmpPack;
        #ifdef T5COMPILE
            for(int i = 0;i < sizeof(tagACStatusPacket);i++)
            {
                LOGD("AC-data[%d]: %#X",i,buf[i]);  
            }
        #else

        #endif

        uint16_t retCrc = mxc_crc16s_calc(tmp, sizeof(tagACStatusPacket)-2 ); 
        if( retCrc != tmpPack.m_u16Crc )  
        {
            #ifdef T5COMPILE
                LOGD( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #else
                printf( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #endif

            buf.Pop(  sizeof(tagACStatusPacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        } 


        //空调状态（ 错误数据 ）处理
        if( this->m_objMcuShakeHandCtrl.m_u8CarModel == CarMode::CarMode_6M 
            || this->m_objMcuShakeHandCtrl.m_u8CarModel == CarMode::CarMode_6P1  
             || this->m_objMcuShakeHandCtrl.m_u8CarModel == CarMode::CarMode_6P2 
              || this->m_objMcuShakeHandCtrl.m_u8CarModel == CarMode::CarMode_7E 
              )
        {
           if( tmpPack.m_u8WindDirection != 1 && tmpPack.m_u8WindDirection != 4 )  
           {   
                tmpPack.m_u8WindDirection = 1;
           }   
        }
        else if( this->m_objMcuShakeHandCtrl.m_u8CarModel == CarMode::CarMode_8E  )
        {
            if( tmpPack.m_u8WindDirection != 1  && tmpPack.m_u8WindDirection != 2  && tmpPack.m_u8WindDirection != 5 )
            {
                tmpPack.m_u8WindDirection = 1;
            } 
        }



        /**
         * 交换当前包
         */
        swap(tmpPack,this->m_objAcStatus );

        //扩展变量
        if ( this->m_objAcStatus.m_u8TempUnit ) 
        {
            this->m_objAcStatusEx.m_u8AcGear = ( this->m_objAcStatus.m_u8Temp - 36) + 1;
        } 
        else //华氏温度
        {
            this->m_objAcStatusEx.m_u8AcGear = ( this->m_objAcStatus.m_u8Temp - 126) + 1; 
        }

        //弹出
        buf.Pop( sizeof(tagACStatusPacket));

    }else if( packetHead.m_u16Command == 0x0003  ){ //电控抬升状态报文

        tagElectricCtrlLiftingStatusPacket tmpPack ;      //作为交换内存使用，避免数据的一次拷贝
        //获取数据
        buf.GetData2((unsigned char*)&tmpPack,sizeof(tagElectricCtrlLiftingStatusPacket));

        //计算 crc 校验
        unsigned char *tmp = (unsigned char *)&tmpPack;

        #ifdef T5COMPILE
            for(int i = 0;i < sizeof(tagElectricCtrlLiftingStatusPacket);i++)
            {
                LOGD("ElectricCtrlLifting-data[%d]: %#X",i,buf[i]);  
            }
        #else

        #endif

        uint16_t retCrc = mxc_crc16s_calc(tmp, sizeof(tagElectricCtrlLiftingStatusPacket)-2 );
        if( retCrc != tmpPack.m_u16Crc )  
        {
            #ifdef T5COMPILE
                LOGD( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #else
                printf( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #endif
          
            buf.Pop(  sizeof(tagElectricCtrlLiftingStatusPacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        } 
        /**
         * 交换当前包
         */
        swap( tmpPack,this->m_objElectricCtrlLiftingStatus );
        //弹出
        buf.Pop( sizeof(tagElectricCtrlLiftingStatusPacket) ) ;

    }else if( packetHead.m_u16Command == 0x0004  ){ //液压阀状态报文

        tagHydraulicValveStatusPacket tmpPack ;      //作为交换内存使用，避免数据的一次拷贝
        //获取数据
        buf.GetData2((unsigned char*)&tmpPack,sizeof(tagHydraulicValveStatusPacket));

        //计算 crc 校验
        unsigned char *tmp = (unsigned char *)&tmpPack;

        #ifdef T5COMPILE
            for(int i = 0;i < sizeof(tagHydraulicValveStatusPacket);i++)
            {
                LOGD("HydraulicValve-data[%d]: %#X",i,buf[i]);  
            }
        #else

        #endif

        uint16_t retCrc = mxc_crc16s_calc(tmp, sizeof(tagHydraulicValveStatusPacket)-2 );
        if( retCrc != tmpPack.m_u16Crc )  
        {
            #ifdef T5COMPILE
                LOGD( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #else
                printf( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #endif

            buf.Pop(  sizeof(tagHydraulicValveStatusPacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        } 

        //处理液压阀无效值与错误数据

        //液压阀1
        if( tmpPack.m_objValve1.m_u16Flow == 0xffff)
            tmpPack.m_objValve1.m_u16Flow = 0x01f4;
        else if( tmpPack.m_objValve1.m_u16Flow > 1000 )
            tmpPack.m_objValve1.m_u16Flow = 1000;

        if( tmpPack.m_objValve1.m_u16TMax == 0xffff )
            tmpPack.m_objValve1.m_u16TMax = 0x0064;
        else if( tmpPack.m_objValve1.m_u16TMax > 6000)
            tmpPack.m_objValve1.m_u16TMax = 6000 ;

        if( tmpPack.m_objValve1.m_u8NumberOfProgressiveCurv == 0xff)
             tmpPack.m_objValve1.m_u8NumberOfProgressiveCurv = 0x01;
        else if( tmpPack.m_objValve1.m_u8NumberOfProgressiveCurv > 41)
            tmpPack.m_objValve1.m_u8NumberOfProgressiveCurv = 41 ;

        //液压阀2
        if( tmpPack.m_objValve2.m_u16Flow == 0xffff)
            tmpPack.m_objValve2.m_u16Flow = 0x01f4;
        else if( tmpPack.m_objValve2.m_u16Flow > 1000 )
            tmpPack.m_objValve2.m_u16Flow = 1000;

        if( tmpPack.m_objValve2.m_u16TMax == 0xffff )
            tmpPack.m_objValve2.m_u16TMax = 0x0064;
        else if( tmpPack.m_objValve2.m_u16TMax > 6000)
            tmpPack.m_objValve2.m_u16TMax = 6000 ;

        if( tmpPack.m_objValve2.m_u8NumberOfProgressiveCurv == 0xff)
             tmpPack.m_objValve2.m_u8NumberOfProgressiveCurv = 0x01;
        else if( tmpPack.m_objValve2.m_u8NumberOfProgressiveCurv > 41)
            tmpPack.m_objValve2.m_u8NumberOfProgressiveCurv = 41 ;


        //液压阀3
        if( tmpPack.m_objValve3.m_u16Flow == 0xffff)
            tmpPack.m_objValve3.m_u16Flow = 0x01f4;
        else if( tmpPack.m_objValve3.m_u16Flow > 1000 )
            tmpPack.m_objValve3.m_u16Flow = 1000;

        if( tmpPack.m_objValve3.m_u16TMax == 0xffff )
            tmpPack.m_objValve3.m_u16TMax = 0x0064;
        else if( tmpPack.m_objValve3.m_u16TMax > 6000)
            tmpPack.m_objValve3.m_u16TMax = 6000 ;

        if( tmpPack.m_objValve3.m_u8NumberOfProgressiveCurv == 0xff)
             tmpPack.m_objValve3.m_u8NumberOfProgressiveCurv = 0x01;
        else if( tmpPack.m_objValve3.m_u8NumberOfProgressiveCurv > 41)
            tmpPack.m_objValve3.m_u8NumberOfProgressiveCurv = 41 ;
 
        //液压阀4
        if( tmpPack.m_objValve4.m_u16Flow == 0xffff)
            tmpPack.m_objValve4.m_u16Flow = 0x01f4;
        else if( tmpPack.m_objValve4.m_u16Flow > 1000 )
            tmpPack.m_objValve4.m_u16Flow = 1000;

        if( tmpPack.m_objValve4.m_u16TMax == 0xffff )
            tmpPack.m_objValve4.m_u16TMax = 0x0064;
        else if( tmpPack.m_objValve4.m_u16TMax > 6000)
            tmpPack.m_objValve4.m_u16TMax = 6000 ;
            
        if( tmpPack.m_objValve4.m_u8NumberOfProgressiveCurv == 0xff)
             tmpPack.m_objValve4.m_u8NumberOfProgressiveCurv = 0x01;
        else if( tmpPack.m_objValve4.m_u8NumberOfProgressiveCurv > 41)
            tmpPack.m_objValve4.m_u8NumberOfProgressiveCurv = 41 ;

        //液压阀5
        if( tmpPack.m_objValve5.m_u16Flow == 0xffff)
            tmpPack.m_objValve5.m_u16Flow = 0x01f4;
        else if( tmpPack.m_objValve5.m_u16Flow > 1000 )
            tmpPack.m_objValve5.m_u16Flow = 1000;

        if( tmpPack.m_objValve5.m_u16TMax == 0xffff )
            tmpPack.m_objValve5.m_u16TMax = 0x0064;
        else if( tmpPack.m_objValve5.m_u16TMax > 6000)
            tmpPack.m_objValve5.m_u16TMax = 6000 ;

        if( tmpPack.m_objValve5.m_u8NumberOfProgressiveCurv == 0xff)
             tmpPack.m_objValve5.m_u8NumberOfProgressiveCurv = 0x01;
        else if( tmpPack.m_objValve5.m_u8NumberOfProgressiveCurv > 41)
            tmpPack.m_objValve5.m_u8NumberOfProgressiveCurv = 41 ;

        //液压阀6
        if( tmpPack.m_objValve6.m_u16Flow == 0xffff)
            tmpPack.m_objValve6.m_u16Flow = 0x01f4;
        else if( tmpPack.m_objValve6.m_u16Flow > 1000 )
            tmpPack.m_objValve6.m_u16Flow = 1000;

        if( tmpPack.m_objValve6.m_u16TMax == 0xffff )
            tmpPack.m_objValve6.m_u16TMax = 0x0064;
        else if( tmpPack.m_objValve6.m_u16TMax > 6000)
            tmpPack.m_objValve6.m_u16TMax = 6000 ;

        if( tmpPack.m_objValve6.m_u8NumberOfProgressiveCurv == 0xff)
             tmpPack.m_objValve6.m_u8NumberOfProgressiveCurv = 0x01;
        else if( tmpPack.m_objValve6.m_u8NumberOfProgressiveCurv > 41)
            tmpPack.m_objValve6.m_u8NumberOfProgressiveCurv = 41 ;

        /**
         * 交换当前包
         */
        swap( tmpPack,this->m_objHydraulicValveStatus );
        //弹出
        buf.Pop( sizeof(tagHydraulicValveStatusPacket) );

    }else if( packetHead.m_u16Command == 0x0005  ){ //变速箱状态报文

        tagGearboxStatusPacket tmpPack ;      //作为交换内存使用，避免数据的一次拷贝
        //获取数据
        buf.GetData2((unsigned char*)&tmpPack,sizeof(tagGearboxStatusPacket));

        //计算 crc 校验
        unsigned char *tmp = (unsigned char *)&tmpPack;

        #ifdef T5COMPILE
            for(int i = 0;i < sizeof(tagGearboxStatusPacket);i++)
            {
                LOGD("GearboxStatusPacket-data[%d]: %#X",i,buf[i]);  
            }
        #else

        #endif

        uint16_t retCrc = mxc_crc16s_calc(tmp, sizeof(tagGearboxStatusPacket)-2 );
        if( retCrc != tmpPack.m_u16Crc )  
        {
            #ifdef T5COMPILE
                LOGD( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #else
                printf( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #endif
            buf.Pop(  sizeof(tagGearboxStatusPacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        } 


        //处理异常值 //7E - 4H
    if( this->m_objMcuShakeHandCtrl.m_u8CarModel == CarMode::CarMode_4H )
    { 
        if( tmpPack.m_u8Gear > 2 || tmpPack.m_u8Gear < 0 )
            tmpPack.m_u8Gear = 2 ; 
    }
    else if( this->m_objMcuShakeHandCtrl.m_u8CarModel == CarMode::CarMode_7E )
    { 
        if( tmpPack.m_u8Gear > 3 || tmpPack.m_u8Gear < 0 )
            tmpPack.m_u8Gear = 3 ; 
    }
    else
    { 
        if( tmpPack.m_u8Gear > 9 || tmpPack.m_u8Gear < 0 )
            tmpPack.m_u8Gear = 9 ;
    }


    if( tmpPack.m_u8CarMode > 2 || tmpPack.m_u8CarMode < 0 )
        tmpPack.m_u8CarMode = 2;

      
        /**
         * 交换当前包
         */
        swap( tmpPack,this->m_objGearBoxStatus );
        //弹出
        buf.Pop( sizeof(tagGearboxStatusPacket) );

    }else if( packetHead.m_u16Command == 0x0006  ){ //后视镜状态报文

        tagRearviewMirrorStatusPacket tmpPack ;      //作为交换内存使用，避免数据的一次拷贝
        //获取数据
        buf.GetData2((unsigned char*)&tmpPack,sizeof(tagRearviewMirrorStatusPacket));

        //计算 crc 校验
        unsigned char *tmp = (unsigned char *)&tmpPack;


        #ifdef T5COMPILE
            for(int i = 0;i < sizeof(tagRearviewMirrorStatusPacket);i++)
            {
                LOGD("RearviewMirrorStatusPacket-data[%d]: %#X",i,buf[i]);  
            }
        #else

        #endif

        uint16_t retCrc = mxc_crc16s_calc(tmp, sizeof(tagRearviewMirrorStatusPacket)-2 );
        if( retCrc != tmpPack.m_u16Crc )  
        {
            #ifdef T5COMPILE
                LOGD( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #else
                printf( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #endif
            buf.Pop(  sizeof(tagRearviewMirrorStatusPacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        } 
        /**
         * 交换当前包
         */
        swap( tmpPack,this->m_objRearviewMirrorStatus );
        //弹出
        buf.Pop( sizeof(tagRearviewMirrorStatusPacket) );

    }else if( packetHead.m_u16Command == 0x0007  ){ //氛围灯状态报文

        tagAtmosphereLightsStatusPacket tmpPack ;      //作为交换内存使用，避免数据的一次拷贝
        //获取数据
        buf.GetData2((unsigned char*)&tmpPack,sizeof(tagAtmosphereLightsStatusPacket));
        //计算 crc 校验
        unsigned char *tmp = (unsigned char *)&tmpPack;

      #ifdef T5COMPILE
            for(int i = 0;i < sizeof(tagAtmosphereLightsStatusPacket);i++)
            {
                LOGD("AtmosphereLightsStatusPacket-data[%d]: %#X",i,buf[i]);  
            }
        #else

        #endif

        uint16_t retCrc = mxc_crc16s_calc(tmp, sizeof(tagAtmosphereLightsStatusPacket)-2 );
        if( retCrc != tmpPack.m_u16Crc )  
        {
            #ifdef T5COMPILE
                LOGD( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #else
                printf( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #endif
            buf.Pop(  sizeof(tagAtmosphereLightsStatusPacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        } 
        /**
         * 交换当前包
         */
        swap( tmpPack,this->m_objAtmosphereLightsStatus );
        //弹出
        buf.Pop( sizeof(tagAtmosphereLightsStatusPacket) );

    }else if( packetHead.m_u16Command == 0x0008  ){ //仪表采集数据

        tagSOCSensorStatePacket tmpPack ;      //作为交换内存使用，避免数据的一次拷贝
        //获取数据
        buf.GetData2((unsigned char*)&tmpPack,sizeof(tagSOCSensorStatePacket));
        //计算 crc 校验
        unsigned char *tmp = (unsigned char *)&tmpPack;

      #ifdef T5COMPILE
            for(int i = 0;i < sizeof(tagSOCSensorStatePacket);i++)
            {
                LOGD("SOCSensorStatePacket-data[%d]: %#X",i,buf[i]);  
            }
        #else

        #endif

        uint16_t retCrc = mxc_crc16s_calc(tmp, sizeof(tagSOCSensorStatePacket)-2 );
        if( retCrc != tmpPack.m_u16Crc )  
        {
            #ifdef T5COMPILE
                LOGD( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #else
                printf( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #endif
            buf.Pop(  sizeof(tagSOCSensorStatePacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        } 
        /**
         * 交换当前包
         */
        swap( tmpPack,this->m_objSocSensorStatus );
        //弹出
        buf.Pop( sizeof(tagSOCSensorStatePacket) );

    }else if( packetHead.m_u16Command == 0x0009  ){ //TBox时间

        tagTBoxTimeStatePacket tmpPack ;      //作为交换内存使用，避免数据的一次拷贝
        //获取数据
        buf.GetData2((unsigned char*)&tmpPack,sizeof(tagTBoxTimeStatePacket));

        //计算 crc 校验
        unsigned char *tmp = (unsigned char *)&tmpPack;
    
        #ifdef T5COMPILE
            for(int i = 0;i < sizeof(tagTBoxTimeStatePacket);i++)
            {
                LOGD("TBoxTimeStatePacket-data[%d]: %#X",i,buf[i]);  
            }
        #else

        #endif

        uint16_t retCrc = mxc_crc16s_calc(tmp, sizeof(tagTBoxTimeStatePacket)-2 );
        if( retCrc != tmpPack.m_u16Crc )  
        {
            #ifdef T5COMPILE
                LOGD( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #else
                printf( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #endif
            buf.Pop(  sizeof(tagTBoxTimeStatePacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        } 
        /**
         * 交换当前包
         */
        swap( tmpPack,this->m_objTBoxTimeStatus );



        this->m_objTBoxTimeStatusEx.m_s32Date = ( this->m_objTBoxTimeStatus.m_u8Year )  | ( this->m_objTBoxTimeStatus.m_u8Month << 8) | (this->m_objTBoxTimeStatus.m_u8Day << 16); 
        this->m_objTBoxTimeStatusEx.m_s32Time = ( this->m_objTBoxTimeStatus.m_u8Hour )  | ( this->m_objTBoxTimeStatus.m_u8Minute << 8) | ( this->m_objTBoxTimeStatus.m_u8Second << 16); 
        AddToNotify(MCU_NOTIFY_TIME,  this->m_objTBoxTimeStatusEx.m_s32Date ,this->m_objTBoxTimeStatusEx.m_s32Time ); 

        //弹出
        buf.Pop( sizeof(tagTBoxTimeStatePacket) );


    }else if( packetHead.m_u16Command == 0x000A  ){ //记亩作业信息

        tagWorkInfoPacket tmpPack ;      //作为交换内存使用，避免数据的一次拷贝
        //获取数据
        buf.GetData2((unsigned char*)&tmpPack,sizeof(tagWorkInfoPacket));

        //计算 crc 校验
        unsigned char *tmp = (unsigned char *)&tmpPack;

        #ifdef T5COMPILE
            for(int i = 0;i < sizeof(tagWorkInfoPacket);i++)
            {
                LOGD("WorkInfoPacket-data[%d]: %#X",i,buf[i]);  
            }
        #else

        #endif

        uint16_t retCrc = mxc_crc16s_calc(tmp, sizeof(tagWorkInfoPacket)-2 );
        if( retCrc != tmpPack.m_u16Crc )  
        {
            #ifdef T5COMPILE
                LOGD( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #else
                printf( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #endif
            buf.Pop(  sizeof(tagWorkInfoPacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        } 
        /**
         * 交换当前包
         */
        swap( tmpPack,this->m_objWorkInfo );
        //弹出
        buf.Pop( sizeof(tagWorkInfoPacket) );


    }else if( packetHead.m_u16Command == 0x000B ){ //mcu电源模式，通知SOC下电

         tagMcuPowerModePacket tmpPack ;      //作为交换内存使用，避免数据的一次拷贝
        //获取数据
        buf.GetData2((unsigned char*)&tmpPack,sizeof(tagMcuPowerModePacket));

        //计算 crc 校验
        unsigned char *tmp = (unsigned char *)&tmpPack;

        #ifdef T5COMPILE
            for(int i = 0;i < sizeof(tagMcuPowerModePacket);i++)
            {
                LOGD("McuPowerModePacket-data[%d]: %#X",i,buf[i]);  
            }
        #else

        #endif

        uint16_t retCrc = mxc_crc16s_calc(tmp, sizeof(tagMcuPowerModePacket)-2 );
        if( retCrc != tmpPack.m_u16Crc )  
        {
            #ifdef T5COMPILE
                LOGD( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #else
                printf( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #endif
            buf.Pop(  sizeof(tagMcuPowerModePacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        } 
        /**
         * 交换当前包
         */
        swap( tmpPack,this->m_objMcuPowerModelStatus );
        //弹出
        buf.Pop( sizeof(tagMcuPowerModePacket) );

    }else if( packetHead.m_u16Command == 0x000C ){ //灯管编程状态切换

        tagLightProgramSwitchPacket tmpPack ;      //作为交换内存使用，避免数据的一次拷贝
        //获取数据
        buf.GetData2((unsigned char*)&tmpPack,sizeof(tagLightProgramSwitchPacket));

        //计算 crc 校验
        unsigned char *tmp = (unsigned char *)&tmpPack;
    
        #ifdef T5COMPILE
            for(int i = 0;i < sizeof(tagLightProgramSwitchPacket);i++)
            {
                LOGD("tagLightProgramSwitchPacket-data[%d]: %#X",i,buf[i]);  
            }
        #else

        #endif

        uint16_t retCrc = mxc_crc16s_calc(tmp, sizeof(tagLightProgramSwitchPacket)-2 );
        if( retCrc != tmpPack.m_u16Crc )  
        {
            #ifdef T5COMPILE
                LOGD( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #else
                printf( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #endif
            buf.Pop(  sizeof(tagLightProgramSwitchPacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        } 
        /**
         * 交换当前包
         */
        swap( tmpPack,this->m_objLightProgramSwitchStatus );
        AddToNotify( MCU_NOTIFY_LIGHTPROGRAM_STATE, this->m_objLightProgramSwitchStatus.m_objPackHead.m_u16Command , this->m_objLightProgramSwitchStatus.m_u8Cmd ); 
         //弹出
        buf.Pop( sizeof(tagLightProgramSwitchPacket) );

    }else if( packetHead.m_u16Command == 0x000D ){ //最高车速设置

        tagHightSpeedSettingPacket tmpPack ;      //作为交换内存使用，避免数据的一次拷贝
        //获取数据
        buf.GetData2((unsigned char*)&tmpPack,sizeof(tagHightSpeedSettingPacket));
        //计算 crc 校验
        unsigned char *tmp = (unsigned char *)&tmpPack;
    
        #ifdef T5COMPILE
            for(int i = 0;i < sizeof(tagHightSpeedSettingPacket);i++)
            {
                LOGD("tagHightSpeedSettingPacket-data[%d]: %#X",i,buf[i]);  
            }
        #else

        #endif

        uint16_t retCrc = mxc_crc16s_calc(tmp, sizeof(tagHightSpeedSettingPacket)-2 );
        if( retCrc != tmpPack.m_u16Crc )  
        {
            #ifdef T5COMPILE
                LOGD( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #else
                printf( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #endif
            buf.Pop(  sizeof(tagHightSpeedSettingPacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        } 
        /**
         * 交换当前包
         */
        swap( tmpPack,this->m_objHightSpeedSettingStatus );

        AddToNotify( MCU_NOTIFY_HIGH_SPEED_SETTING_STATUS , this->m_objHightSpeedSettingStatus.m_objPackHead.m_u16Command , this->m_objHightSpeedSettingStatus.m_u8Cmd ); 
         //弹出
        buf.Pop( sizeof(tagHightSpeedSettingPacket) );
    }else if( packetHead.m_u16Command == 0x000E ){ //油耗信息

        tagFuelConsumptionPacket tmpPack ;      //作为交换内存使用，避免数据的一次拷贝
        //获取数据
        buf.GetData2((unsigned char*)&tmpPack,sizeof(tagFuelConsumptionPacket));
        //计算 crc 校验
        unsigned char *tmp = (unsigned char *)&tmpPack;
    
        #ifdef T5COMPILE
            for(int i = 0;i < sizeof(tagFuelConsumptionPacket);i++)
            {
                LOGD("tagFuelConsumptionPacket-data[%d]: %#X",i,buf[i]);  
            }
        #else

        #endif

        uint16_t retCrc = mxc_crc16s_calc(tmp, sizeof(tagFuelConsumptionPacket)-2 );
        if( retCrc != tmpPack.m_u16Crc )  
        {
            #ifdef T5COMPILE
                LOGD( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #else
                printf( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #endif
            buf.Pop(  sizeof(tagFuelConsumptionPacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        } 
        /**
         * 交换当前包
         */
        swap( tmpPack,this->m_objFuelConsumptionStatus );
         //弹出
        buf.Pop( sizeof(tagFuelConsumptionPacket) );
    }else if( packetHead.m_u16Command == 0x000F ){ //4h标定参数信息

        tagCalibrationParameters4HPacket tmpPack ;      //作为交换内存使用，避免数据的一次拷贝
        //获取数据
        buf.GetData2((unsigned char*)&tmpPack,sizeof(tagCalibrationParameters4HPacket));
        //计算 crc 校验
        unsigned char *tmp = (unsigned char *)&tmpPack;
    
        #ifdef T5COMPILE
            for(int i = 0;i < sizeof(tagCalibrationParameters4HPacket);i++)
            {
                LOGD("tagCalibrationParameters4HPacket-data[%d]: %#X",i,buf[i]);  
            }
        #else

        #endif

        uint16_t retCrc = mxc_crc16s_calc(tmp, sizeof(tagCalibrationParameters4HPacket)-2 );
        if( retCrc != tmpPack.m_u16Crc )  
        {
            #ifdef T5COMPILE
                LOGD( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #else
                printf( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #endif
            buf.Pop(  sizeof(tagCalibrationParameters4HPacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        } 
        /**
         * 交换当前包
         */
        swap( tmpPack,this->m_objCalibrationParameters4HStatus );
         //弹出
        buf.Pop( sizeof(tagCalibrationParameters4HPacket) );
    }else if( packetHead.m_u16Command == 0x0010 ){ //4H标定状态

        tagCalibrationResult4HPacket tmpPack ;      //作为交换内存使用，避免数据的一次拷贝
        //获取数据
        buf.GetData2((unsigned char*)&tmpPack,sizeof(tagCalibrationResult4HPacket));
        //计算 crc 校验
        unsigned char *tmp = (unsigned char *)&tmpPack;
    
        #ifdef T5COMPILE
            for(int i = 0;i < sizeof(tagCalibrationResult4HPacket);i++)
            {
                LOGD("tagCalibrationResult4HPacket-data[%d]: %#X",i,buf[i]);  
            }
        #else

        #endif

        uint16_t retCrc = mxc_crc16s_calc(tmp, sizeof(tagCalibrationResult4HPacket)-2 );
        if( retCrc != tmpPack.m_u16Crc )  
        {
            #ifdef T5COMPILE
                LOGD( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #else
                printf( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #endif
            buf.Pop(  sizeof(tagCalibrationResult4HPacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        } 
        /**
         * 交换当前包
         */
        swap( tmpPack,this->m_objCalibrationResult4HStatus );

        //添加通知
        AddToNotify( MCU_NOTIFY_4H_CALIBRATION_STATUS , this->m_objCalibrationResult4HStatus.m_u8CalibrationResult , this->m_objCalibrationResult4HStatus.m_u8ArgCalibrationResult );

         //弹出
        buf.Pop( sizeof(tagCalibrationResult4HPacket) );
    }else if( packetHead.m_u16Command == 0x0011 ){ //7E故障码

        tagDiagnosisTroubleCodePacket tmpPack ;      //作为交换内存使用，避免数据的一次拷贝
        //获取数据
        buf.GetData2((unsigned char*)&tmpPack,sizeof(tagDiagnosisTroubleCodePacket));
        //计算 crc 校验
        unsigned char *tmp = (unsigned char *)&tmpPack;
    
        #ifdef T5COMPILE
            for(int i = 0;i < sizeof(tagDiagnosisTroubleCodePacket);i++)
            {
                LOGD("tagDiagnosisTroubleCodePacket-data[%d]: %#X",i,buf[i]);  
            }
        #else

        #endif

        uint16_t retCrc = mxc_crc16s_calc(tmp, sizeof(tagDiagnosisTroubleCodePacket)-2 );
        if( retCrc != tmpPack.m_u16Crc )  
        {
            #ifdef T5COMPILE
                LOGD( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #else
                printf( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #endif
            buf.Pop(  sizeof(tagDiagnosisTroubleCodePacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        }
        /**
         * 交换当前包
         */
        swap( tmpPack,this->m_objDiagnosisTroubleCodeStatus );



        //添加通知
        cJSON* root = cJSON_CreateObject();
        if( NULL == root ){
            buf.Pop( sizeof(tagDiagnosisTroubleCodePacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        }
        cJSON_AddNumberToObject(root,"device_addr", this->m_objDiagnosisTroubleCodeStatus.m_u8SrouceAddr ); //设备地址
        cJSON_AddNumberToObject(root,"fault_code", this->m_objDiagnosisTroubleCodeStatus.m_u32DTC ); //故障码
        cJSON_AddNumberToObject(root,"fault_num", this->m_objDiagnosisTroubleCodeStatus.m_u8OC ); //故障次数


        //添加通知
        char* ptr = cJSON_PrintUnformatted(root);
        AddToNotifyStr( MCU_NOTIFY_7E_FAULT_CODE , 
            this->m_objDiagnosisTroubleCodeStatus.m_objPackHead.m_u16Command , 
            ptr
        );

        cJSON_free(ptr);
        cJSON_Delete(root);

         //弹出
        buf.Pop( sizeof(tagDiagnosisTroubleCodePacket) );

    }else if( packetHead.m_u16Command == 0x0081 ){ //测试工装响应

        tagTPMPacket tmpPack ;      //作为交换内存使用，避免数据的一次拷贝
        //获取数据
        buf.GetData2((unsigned char*)&tmpPack,sizeof(tagTPMPacket));
        //计算 crc 校验
        unsigned char *tmp = (unsigned char *)&tmpPack;
    
        #ifdef T5COMPILE
            for(int i = 0;i < sizeof(tagTPMPacket);i++)
            {
                LOGD("tagTPMPacket-data[%d]: %#X",i,buf[i]);  
            }
        #else

        #endif

        uint16_t retCrc = mxc_crc16s_calc(tmp, sizeof(tagTPMPacket)-2 );
        if( retCrc != tmpPack.m_u16Crc )  
        {
            #ifdef T5COMPILE
                LOGD( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #else
                printf( "cmd : %d , unknow crc error cur :%d,frompackage: %d ,skip...", packetHead.m_u16Command , retCrc, tmpPack.m_u16Crc ); 
            #endif
            buf.Pop(  sizeof(tagTPMPacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        } 
        /**
         * 交换当前包
         */
        swap( tmpPack,this->m_objTpmStatus );


        //转换字节为字符串
        std::vector<uint8_t> uTotalByte ;
        int uByteIndex = 0 ;
        while( this->m_objTpmStatus.m_u8Arg[ uByteIndex ] != 0x00 ){
            uTotalByte.push_back( this->m_objTpmStatus.m_u8Arg[ uByteIndex ] );
            uByteIndex ++ ;
        }
        std::string uTotalArg( uTotalByte.begin() , uTotalByte.end());
        LOGD( " -----------LOGD--------- %s : " , uTotalArg.c_str() );

        // cJSON_AddStringToObject(root,"vName",(char*)packet_btkey.vName);  //蓝牙名称
        //  cJSON_AddNumberToObject(root,"r_value", u8Arg1 );
        //转换数据交换格式
        char uDelimiter = ' ';
        cJSON* root = cJSON_CreateObject();
        if( root == NULL )  
        {
            buf.Pop(  sizeof(tagTPMPacket) );
            pthread_mutex_unlock(&m_mutex);
            return -1;
        } 


        if( this->m_objTpmStatus.m_u16Cmd == TPM_TEST_BT ){ //蓝牙检测模式

            cJSON_AddStringToObject(root,"bt_number", uTotalArg.c_str() ); // 蓝牙编号

        }else if( this->m_objTpmStatus.m_u16Cmd == TPM_TEST_WIFI ){ //WIFI检测模式

            cJSON_AddStringToObject(root,"wifi_number", uTotalArg.c_str() ); // WIFI编号

        }else if( this->m_objTpmStatus.m_u16Cmd == TPM_TEST_RGB ){ //RGB检测模式

            std::vector<std::string> tokens = this->splitString( uTotalArg, uDelimiter );

            cJSON_AddStringToObject(root,"r_value",tokens[0].c_str()); //R值
            cJSON_AddStringToObject(root,"g_value",tokens[1].c_str() ); //G值
            cJSON_AddStringToObject(root,"b_value",tokens[2].c_str() ); //B值

        }else if( this->m_objTpmStatus.m_u16Cmd == TPM_TEST_SOFTVERSION ){ //软件版本

        }else if( this->m_objTpmStatus.m_u16Cmd == TPM_TEST_BRIGHTNESS ){ //背光亮度

            cJSON_AddStringToObject(root,"pwm_value", uTotalArg.c_str() );   

        }else if( this->m_objTpmStatus.m_u16Cmd == TPM_TEST_EC_VERSION ){ //互联版本

        }else if( this->m_objTpmStatus.m_u16Cmd == TPM_TEST_LANGUAGE ){ //语言配置

            cJSON_AddStringToObject(root,"language_value", uTotalArg.c_str() );

        }else if( this->m_objTpmStatus.m_u16Cmd == TPM_TEST_VOICE_CHIPS ){ //语音芯片检测

        }else if( this->m_objTpmStatus.m_u16Cmd == TPM_TEST_NORMAL_SHOW ){ //恢复正常显示

        }else if( this->m_objTpmStatus.m_u16Cmd == TPM_TEST_CURRENT_CARTYPE ){ //读取当前车型

        }else if( this->m_objTpmStatus.m_u16Cmd == TPM_TEST_MODEIFY_CARTYPE ){ //配置车型
            
            cJSON_AddStringToObject(root,"car_type", uTotalArg.c_str() );

        }else if( this->m_objTpmStatus.m_u16Cmd == TPM_TEST_GET_SN ){ //读取SN
            
        }
    
        //添加通知
        char* ptr = cJSON_PrintUnformatted(root);
        AddToNotifyStr( MCU_NOTIFY_TPM_STATUS , 
                        this->m_objTpmStatus.m_u16Cmd ,
                        ptr
                    );

        cJSON_free(ptr);
        cJSON_Delete(root);

        //弹出
        buf.Pop( sizeof(tagTPMPacket) );
    }
    pthread_mutex_unlock(&m_mutex);
   
    return  1 ;
}

void McuParser::AddToNotify(MCU_NOTIFY code, int data1, long data2)
{
    NotifyItem *item = new NotifyItem;
    item->type = 1;
    item->code = code;
    item->data1 = data1;
    item->data2 = data2;

    pthread_mutex_lock(&m_mutex2);
    m_notifyItems.push_back(item);
    pthread_mutex_unlock(&m_mutex2);
}

 void McuParser::AddToNotifyStr(MCU_NOTIFY code, int data1, char* data2)
 {
    NotifyItem* item = new NotifyItem;
    item->type = 2;
    item->code = code;
    item->data1 = data1;
    item->str = data2;

    pthread_mutex_lock(&m_mutex2);
    m_notifyItems.push_back(item);
    pthread_mutex_unlock(&m_mutex2);

 }


int McuParser::GetInt(MCU_INDEX index)
{
    int ret = -1;

    pthread_mutex_lock(&m_mutex);

    switch (index)
    {
    case MI_MCU_VERSION: //MCU软件版本
        ret = this->m_objMcuShakeHandStatus.m_u32McuVersion ;
        break; 

    case MI_HARD_VERSION: //硬件版本
        ret = this->m_objMcuShakeHandStatus.m_u8HardVersion ;
        break;   

    case MI_MAX_RPM: //当前发动机最大转速
        ret = this->m_objEngineStatus.m_u16EngineSpeed ;
        break;

    case MI_REAL_TIME_RPM: //实时动力传输轴转速
        ret = this->m_objEngineStatus.m_u16OutputRmp ;
        break;

    case MI_REAL_WORK_HOURS: //发动机累计工作小时数显示
        ret = this->m_objEngineStatus.m_u32WorkTime ;
        break;

    case MI_DPF_DISABLE: //DPF再生禁止开关状态
        ret = this->m_objEngineStatus.m_u8DPFProhibitSwitchStatus ;
        break;

    case MI_DPF_ENABLE: //DPF再生开关状态
        ret = this->m_objEngineStatus.m_u8DPFSwitchStatus ;
        break;

    case MI_POLY_STATUS: //多态开关状态
        ret = this->m_objEngineStatus.m_u8MultiType ;
        break;
        
    case MI_AC_TEMP: //空调温度*2
    {
        if( this->m_objAcStatus.m_u8Temp > 0x40 )  //这里只处理了摄氏温度
            ret = 0x40 ;
        else if( this->m_objAcStatus.m_u8Temp < 0x24)
            ret = 0x24 ;
        else 
            ret = this->m_objAcStatus.m_u8Temp ;
    }
        break;
    case MI_AC_TEMP_UINT: //空调温度单位
        ret = this->m_objAcStatus.m_u8TempUnit ;
        break;

    case MI_AC_DIR: //空调风向
        ret = this->m_objAcStatus.m_u8WindDirection ;
        break;

    case MI_AC_STATUS: //空调AC状态
        ret = this->m_objAcStatus.m_u8ACStatus ;
        break;

    case MI_AC_AUTO_STATUS: //空调AUTO状态
        ret = this->m_objAcStatus.m_u8AUTOStatus ;
        break;

    case MI_AC_OFF_STATUS: //空调OFF状态
        ret = this->m_objAcStatus.m_u8OFFStatus ;
        break;

    case MI_AC_MODE: //空调内外循环状态
        ret = this->m_objAcStatus.m_u8LoopStatus ;
        break;

    case MI_AC_SPEED: //空调风速 ,特殊处理：若空调未关闭状态，风速直接为0，不需要接收实际空调值
    {
        if( 1 == this->m_objAcStatus.m_u8OFFStatus ) //关机状态
            ret = 0 ; 
        else {
            if( this->m_objAcStatus.m_u8WindSpeed > 6 )
                ret = 6;
            else 
                ret = this->m_objAcStatus.m_u8WindSpeed ;  
        }
            
    }  
        break;
    case MI_AC_GEAR: //空调档位
        ret = this->m_objAcStatusEx.m_u8AcGear ;
        break;

    case MI_POWER_LIFT_PLOW_DEPTH: //耕深设置
        ret = this->m_objElectricCtrlLiftingStatus.m_u16TillingDepth;
        break;

    case MI_POWER_LIFT_IND_LIGHT_UP: //指示灯 ，上升
    {
        ret = this->m_objElectricCtrlLiftingStatus.m_u8UpLed;
        // #ifdef T5COMPILE
        //     LOGD("ret m_u8UpLed : %d" , ret);
        // #else
        // #endif
    }
        break; 

    case MI_POWER_LIFT_IND_LIGHT_DOWN: //指示灯 ，下降
    {
        ret = this->m_objElectricCtrlLiftingStatus.m_u8DownLed;
        // #ifdef T5COMPILE
        //     LOGD("ret m_u8DownLed : %d" , ret);
        // #else
        // #endif
    }
        break; 

    case MI_POWER_LIFT_IND_LIGHT_SHOCK: //指示灯 ，减震
    {
        ret = this->m_objElectricCtrlLiftingStatus.m_u8DampingLed;
        // #ifdef T5COMPILE
        //     LOGD("ret m_u8DampingLed : %d" , ret);
        // #else
        // #endif
    }
        break; 

    case MI_POWER_LIFT_IND_LIGHT_ERROR: //指示灯 ，错误
        ret = this->m_objElectricCtrlLiftingStatus.m_u8ErrorLed;
        break; 

    case MI_POWER_LIFT_FAULT_CODE: //故障码
        ret = this->m_objElectricCtrlLiftingStatus.m_u32ErrorCode;
        break;

    case MI_POWER_LIFT_HEIGHT_LIMIT: //高度上限
        ret = this->m_objElectricCtrlLiftingStatus.m_u8UpLimit;
        break;   

    case MI_POWER_LIFT_DESCEND_SPEED: //下降速度
        ret = this->m_objElectricCtrlLiftingStatus.m_u8DownSpeed;
        break; 

    case MI_POWER_LIFT_FORCE_BLEND: //力位混合
        ret = this->m_objElectricCtrlLiftingStatus.m_u8MixturePot;
        break;  

    case MI_POWER_LIFT_MODE_SWITCH: //模式开关
        ret = this->m_objElectricCtrlLiftingStatus.m_u8RockerSwitch;
        break; 

    case MI_POWER_LIFT_SHOCK_SWITCH: //减震开关
        ret = this->m_objElectricCtrlLiftingStatus.m_u8DampingKey;
        break; 

    case MI_CONTROL_VALUE_1_MAX_FLOW: //控制阀1最大流量
        ret = this->m_objHydraulicValveStatus.m_objValve1.m_u16Flow;
        // #ifdef T5COMPILE
        //     LOGD("MI_CONTROL_VALUE_1_MAX_FLOW-ret : %d ",ret);  
        // #else
        // #endif
        break;  

    case MI_CONTROL_VALUE_1_PROGREAAION: //控制阀1渐进曲线
        ret = this->m_objHydraulicValveStatus.m_objValve1.m_u8NumberOfProgressiveCurv;
        // #ifdef T5COMPILE
        //     LOGD("MI_CONTROL_VALUE_1_PROGREAAION-ret : %d ",ret);  
        // #else
        // #endif
        break; 

    case MI_CONTROL_VALUE_1_MAX_TIME: // 控制阀1最大时间
        ret = this->m_objHydraulicValveStatus.m_objValve1.m_u16TMax;
        // #ifdef T5COMPILE
        //     LOGD("MI_CONTROL_VALUE_1_MAX_TIME-ret : %d ",ret);  
        // #else
        // #endif
        break;   

    case MI_CONTROL_VALUE_2_MAX_FLOW: //控制阀2最大流量
        ret = this->m_objHydraulicValveStatus.m_objValve2.m_u16Flow;
        // #ifdef T5COMPILE
        //     LOGD("MI_CONTROL_VALUE_2_MAX_FLOW-ret : %d ",ret);  
        // #else
        // #endif
        break;  

    case MI_CONTROL_VALUE_2_PROGREAAION: //控制阀2渐进曲线
        ret = this->m_objHydraulicValveStatus.m_objValve2.m_u8NumberOfProgressiveCurv;
        // #ifdef T5COMPILE
        //     LOGD("MI_CONTROL_VALUE_2_PROGREAAION-ret : %d ",ret);  
        // #else
        // #endif
        break; 
        
    case MI_CONTROL_VALUE_2_MAX_TIME: // 控制阀2最大时间
        ret = this->m_objHydraulicValveStatus.m_objValve2.m_u16TMax;
        // #ifdef T5COMPILE
        //     LOGD("MI_CONTROL_VALUE_2_MAX_TIME-ret : %d ",ret);  
        // #else
        // #endif
        break;  

    case MI_CONTROL_VALUE_3_MAX_FLOW: //控制阀3最大流量
        ret = this->m_objHydraulicValveStatus.m_objValve3.m_u16Flow;
        // #ifdef T5COMPILE
        //     LOGD("MI_CONTROL_VALUE_3_MAX_FLOW-ret : %d ",ret);  
        // #else
        // #endif
        break;  

    case MI_CONTROL_VALUE_3_PROGREAAION: //控制阀3渐进曲线
        ret = this->m_objHydraulicValveStatus.m_objValve3.m_u8NumberOfProgressiveCurv;
        // #ifdef T5COMPILE
        //     LOGD("MI_CONTROL_VALUE_3_PROGREAAION-ret : %d ",ret);  
        // #else
        // #endif
        break; 
        
    case MI_CONTROL_VALUE_3_MAX_TIME: // 控制阀3最大时间
        ret = this->m_objHydraulicValveStatus.m_objValve3.m_u16TMax;
        // #ifdef T5COMPILE
        //     LOGD("MI_CONTROL_VALUE_3_MAX_TIME-ret : %d ",ret);  
        // #else
        // #endif
        break; 

    case MI_CONTROL_VALUE_4_MAX_FLOW: //控制阀4最大流量
        ret = this->m_objHydraulicValveStatus.m_objValve4.m_u16Flow;
        // #ifdef T5COMPILE
        //     LOGD("MI_CONTROL_VALUE_4_MAX_FLOW-ret : %d ",ret);  
        // #else
        // #endif
        break;  

    case MI_CONTROL_VALUE_4_PROGREAAION: //控制阀4渐进曲线
        ret = this->m_objHydraulicValveStatus.m_objValve4.m_u8NumberOfProgressiveCurv;
        // #ifdef T5COMPILE
        //     LOGD("MI_CONTROL_VALUE_4_PROGREAAION-ret : %d ",ret);  
        // #else
        // #endif
        break; 
        
    case MI_CONTROL_VALUE_4_MAX_TIME: // 控制阀4最大时间
        ret =  this->m_objHydraulicValveStatus.m_objValve4.m_u16TMax;
        // #ifdef T5COMPILE
        //     LOGD("MI_CONTROL_VALUE_4_MAX_TIME-ret : %d ",ret);  
        // #else
        // #endif
        break;  

    case MI_CONTROL_VALUE_5_MAX_FLOW: //控制阀5最大流量
        ret = this->m_objHydraulicValveStatus.m_objValve5.m_u16Flow;
        // #ifdef T5COMPILE
        //     LOGD("MI_CONTROL_VALUE_5_MAX_FLOW-ret : %d ",ret);  
        // #else
        // #endif
        break;  

    case MI_CONTROL_VALUE_5_PROGREAAION: //控制阀5渐进曲线
        ret = this->m_objHydraulicValveStatus.m_objValve5.m_u8NumberOfProgressiveCurv;
        // #ifdef T5COMPILE
        //     LOGD("MI_CONTROL_VALUE_5_PROGREAAION-ret : %d ",ret);  
        // #else
        // #endif
        break; 
        
    case MI_CONTROL_VALUE_5_MAX_TIME: // 控制阀5最大时间
        ret =  this->m_objHydraulicValveStatus.m_objValve5.m_u16TMax;
        // #ifdef T5COMPILE
        //     LOGD("MI_CONTROL_VALUE_5_MAX_TIME-ret : %d ",ret);  
        // #else
        // #endif
        break;  

    case MI_CONTROL_VALUE_6_MAX_FLOW: //控制阀6最大流量
        ret = this->m_objHydraulicValveStatus.m_objValve6.m_u16Flow;  
        // #ifdef T5COMPILE
        //     LOGD("MI_CONTROL_VALUE_6_MAX_FLOW-ret : %d ",ret);  
        // #else
        // #endif
        break;  

    case MI_CONTROL_VALUE_6_PROGREAAION: //控制阀6渐进曲线
        ret = this->m_objHydraulicValveStatus.m_objValve6.m_u8NumberOfProgressiveCurv;
        // #ifdef T5COMPILE
        //     LOGD("MI_CONTROL_VALUE_6_PROGREAAION-ret : %d ",ret);  
        // #else
        // #endif
        break; 
        
    case MI_CONTROL_VALUE_6_MAX_TIME: // 控制阀6最大时间
        ret =  this->m_objHydraulicValveStatus.m_objValve6.m_u16TMax;
        // #ifdef T5COMPILE
        //     LOGD("MI_CONTROL_VALUE_6_MAX_TIME-ret : %d ",ret);  
        // #else
        // #endif
        break;  

    case MI_GEAR_BOX_GEAR: //当前档位
        ret = this->m_objGearBoxStatus.m_u8Gear;
        break; 
        
    case MI_GEAR_BOX_CAR_MODE: //整车模式
        ret = this->m_objGearBoxStatus.m_u8CarMode;
        break; 

    case MI_GEAR_BOX_AUTO_SHIFT:  //自动换档设置状态
        ret = this->m_objGearBoxStatus.m_u8AutoGearStatus;
        break; 

    case MI_GEAR_BOX_CURRENT_SPEED: //当前车速
        ret = this->m_objGearBoxStatus.m_u16CurVehicleSpeed;
        break; 
        
    case MI_GEAR_BOX_START_GEAR: //起步档位
        ret = this->m_objGearBoxStatus.m_u8StartGear;
        break; 
        
    case MI_GEAR_BOX_DIFF_LOCK: //手自动差速锁显示状态
        ret = this->m_objGearBoxStatus.m_u8DiffLock;
        break;  

    case MI_GEAR_BOX_FOUR_WHEEL_DRIVE: //手自动四驱显示
        ret = this->m_objGearBoxStatus.m_u8FourDriver;
        break; 
        
    case MI_PTO_CONTROL: //PTO启停控制状态
        ret = this->m_objGearBoxStatus.m_u8PTOStatus;
        break;

    case MI_PTO_RPM: //PTO转速控制(PTO档位)
        ret = this->m_objGearBoxStatus.m_u8PTOGear;
        break; 

    // case MI_GEAR_BOX_MAX_SPEED: //最高车速 
    //     ret = g_gear_box_max_speed;
    //     break; 

    case MI_REAR_MIRROR_STATUS:  //后视镜加热状态
        ret = this->m_objRearviewMirrorStatus.m_u8Heating;
        break;  
        
    case MI_AMBIENT_LIGHT: //氛围灯开关
        ret = this->m_objAtmosphereLightsStatus.m_u8Switch;
        break;  

    case MI_AMBIENT_LIGHT_MODE: //氛围灯模式
        ret = this->m_objAtmosphereLightsStatus.m_u8Mode;
        break; 

    case MI_AMBIENT_LIGHT_COLOR: //氛围灯颜色
        ret = this->m_objAtmosphereLightsStatus.m_u8Color;
        break; 

    case MI_METER_SENSITIVITY: //仪表采集光感值
        ret = this->m_objSocSensorStatus.m_u16Lux;
        break;   

    case MI_METER_BRIGHT_LEVEL: //仪表屏幕亮度
        ret = this->m_objSocSensorStatus.m_u8ScreenBrightness;
        break;  

    case MI_TIME_YEAR: //TBOX时间（年）
        ret = this->m_objTBoxTimeStatus.m_u8Year;
        break;

    case MI_TIME_MONTH: //TBOX时间（月）
        ret = this->m_objTBoxTimeStatus.m_u8Month;
        break;

    case MI_TIME_DAY: //TBOX时间（日）
        ret = this->m_objTBoxTimeStatus.m_u8Day;
        break;

    case MI_TIME_HOUR:  //TBOX时间（时）
        ret = this->m_objTBoxTimeStatus.m_u8Hour;
        break;

    case MI_TIME_MINUTE: //TBOX时间（分）
        ret = this->m_objTBoxTimeStatus.m_u8Minute;
        break; 
          
    case MI_TIME_SECOND: //TBOX时间（秒）
        ret = this->m_objTBoxTimeStatus.m_u8Second;
        break;

    case MI_WORK_ACCUMULATED_ACREAGE : //累计亩数
        ret = this->m_objWorkInfo.m_u32CumulativeVal ;
        break;

    case MI_WORK_CURRENT_ACREAGE : //当前亩数
        ret = this->m_objWorkInfo.m_u32CurVal ;
        break;

    case MI_WORK_LEFT_WIDTH_PARAM : //左幅宽参数
        ret = this->m_objWorkInfo.m_u16LeftWidth ;
        break;

    case MI_WORK_RIGHT_WIDTH_PARAM : //右幅宽参数
         ret = this->m_objWorkInfo.m_u16RightWidth ;
        break;

    case MI_WORK_ERROR_CODE : //错误码
        ret = this->m_objWorkInfo.m_u8ErrorCode ;
        break;

   case MI_MCU_POWER_OFF_NOTIY : //MCU->SOC下电
        ret = this->m_objMcuPowerModelStatus.m_u8Mode ;
        break;
   case MI_FUEL_CONSUMPTION_CURRENT : //本次油耗
        ret = this->m_objFuelConsumptionStatus.m_u32TripFuelConsumption ;
        break;
   case MI_FUEL_CONSUMPTION_TOTAL : //总油耗
        ret = this->m_objFuelConsumptionStatus.m_u32TotalFuelConsumption ;
        break;
   case MI_4H_CALIBRATION_ARG_1 : //当前标定设置页面参数1
        ret = this->m_objCalibrationParameters4HStatus.m_u16Arg1 ;
        break;
    case MI_4H_CALIBRATION_ARG_2 : //当前标定设置页面参数2
        ret = this->m_objCalibrationParameters4HStatus.m_u16Arg2 ;
        break;
    case MI_4H_CALIBRATION_ARG_3 : //当前标定设置页面参数3
        ret = this->m_objCalibrationParameters4HStatus.m_u16Arg3 ;
        break;
    case MI_4H_CALIBRATION_ARG_4 : //当前标定设置页面参数4
        ret = this->m_objCalibrationParameters4HStatus.m_u16Arg4 ;
        break;
    case MI_4H_CALIBRATION_ARG_5 : //当前标定设置页面参数5
        ret = this->m_objCalibrationParameters4HStatus.m_u16Arg5 ;
        break;
    case MI_4H_CALIBRATION_ARG_6 : //当前标定设置页面参数6
        ret = this->m_objCalibrationParameters4HStatus.m_u16Arg6 ;
        break;
    case MI_4H_CALIBRATION_ARG_7 : //当前标定设置页面参数7
        ret = this->m_objCalibrationParameters4HStatus.m_u16Arg7 ;
        break;
    case MI_4H_CALIBRATION_ARG_8 : //当前标定设置页面参数8
        ret = this->m_objCalibrationParameters4HStatus.m_u16Arg8 ;
        break;
    case MI_4H_CALIBRATION_FRONT_AXLE_SENSOR : //前桥转角传感器实际值
        ret = this->m_objCalibrationParameters4HStatus.m_u16FrontAxleSensorValue ;
        break;
    case MI_4H_CALIBRATION_PTO_GREA_SENSOR : //PTO档位传感器实际值
        ret = this->m_objCalibrationParameters4HStatus.m_u16PTOGreaSensorValue ;
        break;
    case MI_4H_CALIBRATION_AUXILIARY_GEARBOX_SENSOR : //副变速箱档位角度传感器实际值
        ret = this->m_objCalibrationParameters4HStatus.m_u16AuxiliaryGearboxSensorValue ;
        break;
    case MI_4H_CALIBRATION_MAIN_CLUTCH_SENSOR : //主离合器传感器实际值
        ret = this->m_objCalibrationParameters4HStatus.m_u16MainClutchSensorValue ;
        break;
    case MI_TCU_FAULT_DEVICE_ADDR : //TCU 故障设备地址
        ret = this->m_objDiagnosisTroubleCodeStatus.m_u8SrouceAddr ;
    break;
    case MI_TCU_FAULT_CODE : //TCU 故障码
        ret = this->m_objDiagnosisTroubleCodeStatus.m_u32DTC ;
    break;
    case MI_TCU_FAULT_NUMBER : //TCU故障次数
        ret = this->m_objDiagnosisTroubleCodeStatus.m_u8OC ;
    break;

    default:  
        break;
    }

    pthread_mutex_unlock(&m_mutex);

    return ret;
}


uint32_t McuParser::GetUInt(MCU_INDEX index)
{
	uint32_t ret  = 0;

	pthread_mutex_lock(&m_mutex);
    
	switch (index)
	{
	case MI_POWER_LIFT_FAULT_CODE:
		ret = this->m_objElectricCtrlLiftingStatus.m_u32ErrorCode;
		break;  
	default:  
		break;
	}
	pthread_mutex_unlock(&m_mutex);
	return ret;
}

/**
 * 发送请求命令 （SOC->MCU）
 */
int McuParser::SendCmd(MCU_COMMAND cmd, int data1, long data2)
{   
    int ret = 0;
    unsigned short u16Size = 0 ; 
    unsigned char *pCmd = nullptr ;
    switch (cmd)
    {
    case MC_SOC_NOTIFY_MCU_ON: //程序已启动
    {
        u16Size = sizeof( reqMCUShakeHandCtrl );
        this->m_objMcuShakeHandCtrl.m_u8Reserve1 = 0xff ;
        this->m_objMcuShakeHandCtrl.m_u8Reserve2 = 0xff ;
        this->m_objMcuShakeHandCtrl.m_u8CarModel = 0xff ;
        this->m_objMcuShakeHandCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objMcuShakeHandCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objMcuShakeHandCtrl;
    }
    break;
    case MC_SOC_NOTIFY_MCU_DIFFSPEEDRATIO: // 后桥差速比
    {
        u16Size = sizeof( reqMCUShakeHandCtrl );
        this->m_objMcuShakeHandCtrl.m_u8Reserve1 = data1 ;
        this->m_objMcuShakeHandCtrl.m_u8Reserve2 = 0xff ;
        this->m_objMcuShakeHandCtrl.m_u8CarModel = 0xff ;
        this->m_objMcuShakeHandCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objMcuShakeHandCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objMcuShakeHandCtrl;
    }
    break;
    case MC_SOC_NOTIFY_MCU_REARWHEELRADIUS: // 后轮半径
    {
        u16Size = sizeof( reqMCUShakeHandCtrl );
        this->m_objMcuShakeHandCtrl.m_u8Reserve1 = 0xff ;
        this->m_objMcuShakeHandCtrl.m_u8Reserve2 = data1 ;
        this->m_objMcuShakeHandCtrl.m_u8CarModel = 0xff ;
        this->m_objMcuShakeHandCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objMcuShakeHandCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objMcuShakeHandCtrl;
    }
    break;
    case MC_SOC_NOTIFY_MCU_CARMODEL: // 车型配置
    {
        u16Size = sizeof( reqMCUShakeHandCtrl );
        this->m_objMcuShakeHandCtrl.m_u8Reserve1 = 0xff ;
        this->m_objMcuShakeHandCtrl.m_u8Reserve2 = 0xff ;
        this->m_objMcuShakeHandCtrl.m_u8CarModel = data1 ;
        this->m_objMcuShakeHandCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objMcuShakeHandCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objMcuShakeHandCtrl;   
    }
    break;
    case MC_SET_MAX_RPM: // 发动机最大转速
    {
        u16Size = sizeof( reqEngineCtrl );
        this->m_objEngineCtrl.m_u16EngineSpeed = data1 ;
        this->m_objEngineCtrl.m_u8DPFProhibitSwitchStatus = 0xff ;
        this->m_objEngineCtrl.m_u8DPFSwitchStatus = 0xff ;
        this->m_objEngineCtrl.m_u8MultiType = 0xff ;
        this->m_objEngineCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objEngineCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objEngineCtrl;  
    }
    break;
    case MC_SET_DPF_DISABLE: //DPF再生禁止开关设置
    {
        u16Size = sizeof( reqEngineCtrl );
        this->m_objEngineCtrl.m_u16EngineSpeed = 0xffff ;
        this->m_objEngineCtrl.m_u8DPFProhibitSwitchStatus = data1 ;
        this->m_objEngineCtrl.m_u8DPFSwitchStatus = 0xff ;
        this->m_objEngineCtrl.m_u8MultiType = 0xff ;
        this->m_objEngineCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objEngineCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objEngineCtrl;  
    }
    break;
    case MC_SET_DPF_ENABLE: //DPF再生开关设置
    {
        u16Size = sizeof( reqEngineCtrl );
        this->m_objEngineCtrl.m_u16EngineSpeed = 0xffff ;
        this->m_objEngineCtrl.m_u8DPFProhibitSwitchStatus = 0xff ;
        this->m_objEngineCtrl.m_u8DPFSwitchStatus = data1 ;
        this->m_objEngineCtrl.m_u8MultiType = 0xff ;
        this->m_objEngineCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objEngineCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objEngineCtrl;  
    }
    break;
    case MC_SET_POLY_STATUS: //多态开关状态
    {
        u16Size = sizeof( reqEngineCtrl );
        this->m_objEngineCtrl.m_u16EngineSpeed = 0xffff ;
        this->m_objEngineCtrl.m_u8DPFProhibitSwitchStatus = 0xff ;
        this->m_objEngineCtrl.m_u8DPFSwitchStatus = 0xff ;
        this->m_objEngineCtrl.m_u8MultiType = data1 ;
        this->m_objEngineCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objEngineCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objEngineCtrl;  
    }
    break;
    case MC_SET_AC_TEMP: //空调温度
    {
        // uint8_t m_u8Temp;//温度
        // uint8_t m_u8WindDirection;//风向
        // uint8_t m_u8OffSwitch;//OFF开关
        // uint8_t m_u8AUTOStatus;//AUTO状态
        // uint8_t m_u8TempUnit;//温度单位
        // uint8_t m_u8LoopStatus;//内外循环状态
        // uint8_t m_u8WindSpeed;//风速
        // uint8_t m_u8AcCtrl;//压缩机控制
        // uint16_t m_u16Crc;

        u16Size = sizeof( reqACCtrl );
        this->m_objACCtrl.m_u8Temp = data1 ;
        this->m_objACCtrl.m_u8WindDirection = 0xff ;
        this->m_objACCtrl.m_u8OffSwitch = 0xff ;
        this->m_objACCtrl.m_u8AUTOStatus = 0xff ;
        this->m_objACCtrl.m_u8TempUnit = 0xff ;
        this->m_objACCtrl.m_u8LoopStatus = 0xff ;
        this->m_objACCtrl.m_u8WindSpeed = 0xff ;
        this->m_objACCtrl.m_u8AcCtrl = 0xff ;
        this->m_objACCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objACCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objACCtrl; 
    }
    break;
    case MC_SET_AC_DIR: //空调风向
    {
        u16Size = sizeof( reqACCtrl );
        this->m_objACCtrl.m_u8Temp = 0xff ;
        this->m_objACCtrl.m_u8WindDirection = data1 ;
        this->m_objACCtrl.m_u8OffSwitch = 0xff ;
        this->m_objACCtrl.m_u8AUTOStatus = 0xff ;
        this->m_objACCtrl.m_u8TempUnit = 0xff ;
        this->m_objACCtrl.m_u8LoopStatus = 0xff ;
        this->m_objACCtrl.m_u8WindSpeed = 0xff ;
        this->m_objACCtrl.m_u8AcCtrl = 0xff ;
        this->m_objACCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objACCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objACCtrl; 
    }
    break;
    case MC_SET_AC_SWITCH: //空调off开关
    {
        u16Size = sizeof( reqACCtrl );
        this->m_objACCtrl.m_u8Temp = 0xff ;
        this->m_objACCtrl.m_u8WindDirection = 0xff ;
        this->m_objACCtrl.m_u8OffSwitch = data1 ;
        this->m_objACCtrl.m_u8AUTOStatus = 0xff ;
        this->m_objACCtrl.m_u8TempUnit = 0xff ;
        this->m_objACCtrl.m_u8LoopStatus = 0xff ;
        this->m_objACCtrl.m_u8WindSpeed = 0xff ;
        this->m_objACCtrl.m_u8AcCtrl = 0xff ;
        this->m_objACCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objACCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objACCtrl;   
    }
    break;
    case MC_SET_AC_AUTO: //Auto设定
    {
        u16Size = sizeof( reqACCtrl );
        this->m_objACCtrl.m_u8Temp = 0xff ;
        this->m_objACCtrl.m_u8WindDirection = 0xff ;
        this->m_objACCtrl.m_u8OffSwitch = 0xff ;
        this->m_objACCtrl.m_u8AUTOStatus = data1 ;
        this->m_objACCtrl.m_u8TempUnit = 0xff ;
        this->m_objACCtrl.m_u8LoopStatus = 0xff ;
        this->m_objACCtrl.m_u8WindSpeed = 0xff ;
        this->m_objACCtrl.m_u8AcCtrl = 0xff ;
        this->m_objACCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objACCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objACCtrl;   
    }
    break;
    case MC_SET_AC_TEMP_UINT: //摄氏华氏切换
    {
        u16Size = sizeof( reqACCtrl );
        this->m_objACCtrl.m_u8Temp = 0xff ;
        this->m_objACCtrl.m_u8WindDirection = 0xff ;
        this->m_objACCtrl.m_u8OffSwitch = 0xff ;
        this->m_objACCtrl.m_u8AUTOStatus = 0xff ;
        this->m_objACCtrl.m_u8TempUnit = data1 ;
        this->m_objACCtrl.m_u8LoopStatus = 0xff ;
        this->m_objACCtrl.m_u8WindSpeed = 0xff ;
        this->m_objACCtrl.m_u8AcCtrl = 0xff ;
        this->m_objACCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objACCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objACCtrl;   
    }
    break;
    case MC_SET_AC_MODE: //内外循环切换
    {
        u16Size = sizeof( reqACCtrl );
        this->m_objACCtrl.m_u8Temp = 0xff ;
        this->m_objACCtrl.m_u8WindDirection = 0xff ;
        this->m_objACCtrl.m_u8OffSwitch = 0xff ;
        this->m_objACCtrl.m_u8AUTOStatus = 0xff ;
        this->m_objACCtrl.m_u8TempUnit = 0xff ;
        this->m_objACCtrl.m_u8LoopStatus = data1 ;
        this->m_objACCtrl.m_u8WindSpeed = 0xff ;
        this->m_objACCtrl.m_u8AcCtrl = 0xff ;
        this->m_objACCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objACCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objACCtrl;   
    }
    break;
    case MC_AC_SPEED: //空调风量 
    {   
        u16Size = sizeof( reqACCtrl );
        this->m_objACCtrl.m_u8Temp = 0xff ;
        this->m_objACCtrl.m_u8WindDirection = 0xff ;
        this->m_objACCtrl.m_u8OffSwitch = 0xff ;
        this->m_objACCtrl.m_u8AUTOStatus = 0xff ;
        this->m_objACCtrl.m_u8TempUnit = 0xff ;
        this->m_objACCtrl.m_u8LoopStatus = 0xff ;
        this->m_objACCtrl.m_u8WindSpeed = data1 ;
        this->m_objACCtrl.m_u8AcCtrl = 0xff ;
        this->m_objACCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objACCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objACCtrl;        
    }
    break;
    case MC_SET_AC_STATUS: //压缩机开关机
    {
        u16Size = sizeof( reqACCtrl );
        this->m_objACCtrl.m_u8Temp = 0xff ;
        this->m_objACCtrl.m_u8WindDirection = 0xff ;
        this->m_objACCtrl.m_u8OffSwitch = 0xff ;
        this->m_objACCtrl.m_u8AUTOStatus = 0xff ;
        this->m_objACCtrl.m_u8TempUnit = 0xff ;
        this->m_objACCtrl.m_u8LoopStatus = 0xff ;
        this->m_objACCtrl.m_u8WindSpeed = 0xff ;
        this->m_objACCtrl.m_u8AcCtrl = data1 ;
        this->m_objACCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objACCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objACCtrl;     
    }
    break;
    case MC_SET_POWER_LIFT_PLOW_DEPTH: //耕深设置
    {
        // uint16_t m_u16TillingDepth;//耕深设置
        // uint8_t  m_u8UpLimit;//高度上限
        // uint8_t  m_u8DownSpeed;//下降速度
        // uint8_t  m_u8MixturePot;//力液混合
        // uint8_t  m_u8RockerSwitch;//模式开关
        // uint8_t  m_u8DampingKey;//减震开关
        // uint16_t m_u16Crc;

        u16Size = sizeof( reqElectricControlLiftingCtrl );
        this->m_objElectricControlLiftingCtrl.m_u16TillingDepth = data1 ;
        this->m_objElectricControlLiftingCtrl.m_u8UpLimit = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8DownSpeed = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8MixturePot = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8RockerSwitch = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8DampingKey = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objElectricControlLiftingCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objElectricControlLiftingCtrl;  
    }
    break;
    case MC_SET_POWER_LIFT_HEIGHT_LIMIT: //高度上限
    {
        u16Size = sizeof( reqElectricControlLiftingCtrl );
        this->m_objElectricControlLiftingCtrl.m_u16TillingDepth = 0xffff ;
        this->m_objElectricControlLiftingCtrl.m_u8UpLimit = data1 ;
        this->m_objElectricControlLiftingCtrl.m_u8DownSpeed = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8MixturePot = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8RockerSwitch = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8DampingKey = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objElectricControlLiftingCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objElectricControlLiftingCtrl;  
    }
    break;
    case MC_SET_POWER_LIFT_DESCEND_SPEED: //下降速度
    {
        u16Size = sizeof( reqElectricControlLiftingCtrl );
        this->m_objElectricControlLiftingCtrl.m_u16TillingDepth = 0xffff ;
        this->m_objElectricControlLiftingCtrl.m_u8UpLimit = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8DownSpeed = data1 ;
        this->m_objElectricControlLiftingCtrl.m_u8MixturePot = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8RockerSwitch = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8DampingKey = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objElectricControlLiftingCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objElectricControlLiftingCtrl;  
    }
     break;
    case MC_SET_POWER_LIFT_FORCE_BLEND: //力位混合
    {
        u16Size = sizeof( reqElectricControlLiftingCtrl );
        this->m_objElectricControlLiftingCtrl.m_u16TillingDepth = 0xffff ;
        this->m_objElectricControlLiftingCtrl.m_u8UpLimit = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8DownSpeed = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8MixturePot = data1 ;
        this->m_objElectricControlLiftingCtrl.m_u8RockerSwitch = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8DampingKey = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objElectricControlLiftingCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objElectricControlLiftingCtrl;  
    }
    break;
    case MC_SET_POWER_LIFT_MODE_SWITCH: //模式开关
    {
        u16Size = sizeof( reqElectricControlLiftingCtrl );
        this->m_objElectricControlLiftingCtrl.m_u16TillingDepth = 0xffff ;
        this->m_objElectricControlLiftingCtrl.m_u8UpLimit = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8DownSpeed = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8MixturePot = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8RockerSwitch = data1 ;
        this->m_objElectricControlLiftingCtrl.m_u8DampingKey = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objElectricControlLiftingCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objElectricControlLiftingCtrl;  
    }
    break;
    case MC_SET_POWER_LIFT_SHOCK_SWITCH: //减震开关
    {
        u16Size = sizeof( reqElectricControlLiftingCtrl );
        this->m_objElectricControlLiftingCtrl.m_u16TillingDepth = 0xffff ;
        this->m_objElectricControlLiftingCtrl.m_u8UpLimit = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8DownSpeed = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8MixturePot = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8RockerSwitch = 0xff ;
        this->m_objElectricControlLiftingCtrl.m_u8DampingKey = data1 ;
        this->m_objElectricControlLiftingCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objElectricControlLiftingCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objElectricControlLiftingCtrl;  
    }
    break;
    case MC_SET_CONTROL_VALUE_1_MAX_FLOW: //控制阀1最大流量
    {
        // ValveProperty m_objValve1;
        // ValveProperty m_objValve2;
        // ValveProperty m_objValve3;
        // ValveProperty m_objValve4;
        // ValveProperty m_objValve5;
        // ValveProperty m_objValve6;
        // uint16_t m_u16Crc;

        u16Size = sizeof( reqHydraulicValveCtrl );
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16Flow = data1 ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve2.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve3.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve4.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve5.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve6.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objHydraulicValveCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objHydraulicValveCtrl;  
    }
    break;
    case MC_SET_CONTROL_VALUE_1_PROGREAAION: //控制阀1渐进曲线
    {
        u16Size = sizeof( reqHydraulicValveCtrl );
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u8NumberOfProgressiveCurv = data1 ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve2.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve3.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve4.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve5.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve6.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objHydraulicValveCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objHydraulicValveCtrl;  
    }
    break;
    case MC_SET_CONTROL_VALUE_1_MAX_TIME: //控制阀1最大时间
    {
        u16Size = sizeof( reqHydraulicValveCtrl );
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16TMax = data1 ;

        this->m_objHydraulicValveCtrl.m_objValve2.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve3.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve4.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve5.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve6.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objHydraulicValveCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objHydraulicValveCtrl; 
    }
    break;
    case MC_SET_CONTROL_VALUE_2_MAX_FLOW: //控制阀2最大流量
    {
        u16Size = sizeof( reqHydraulicValveCtrl );
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve2.m_u16Flow = data1 ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve3.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve4.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve5.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve6.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objHydraulicValveCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objHydraulicValveCtrl; 
    }
    break;
    case MC_SET_CONTROL_VALUE_2_PROGREAAION: //控制阀2渐进曲线
    {
        u16Size = sizeof( reqHydraulicValveCtrl );
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve2.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u8NumberOfProgressiveCurv = data1 ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve3.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve4.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve5.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve6.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objHydraulicValveCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objHydraulicValveCtrl; 
    }
    break;
    case MC_SET_CONTROL_VALUE_2_MAX_TIME: //控制阀2最大时间
    {
        
        u16Size = sizeof( reqHydraulicValveCtrl );
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve2.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u16TMax = data1 ;

        this->m_objHydraulicValveCtrl.m_objValve3.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve4.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve5.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve6.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objHydraulicValveCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objHydraulicValveCtrl; 
    }
    break;
    case MC_SET_CONTROL_VALUE_3_MAX_FLOW: //控制阀3最大流量
    {
        u16Size = sizeof( reqHydraulicValveCtrl );
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve2.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve3.m_u16Flow = data1 ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve4.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve5.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve6.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objHydraulicValveCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objHydraulicValveCtrl; 
    }
    break;
    case MC_SET_CONTROL_VALUE_3_PROGREAAION: //控制阀3渐进曲线
    {
        u16Size = sizeof( reqHydraulicValveCtrl );
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve2.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve3.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u8NumberOfProgressiveCurv = data1 ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve4.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve5.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve6.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objHydraulicValveCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objHydraulicValveCtrl; 
    }
    break;
    case MC_SET_CONTROL_VALUE_3_MAX_TIME: //控制阀3最大时间
    {
        u16Size = sizeof( reqHydraulicValveCtrl );
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve2.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve3.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u16TMax = data1 ;

        this->m_objHydraulicValveCtrl.m_objValve4.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve5.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve6.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objHydraulicValveCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objHydraulicValveCtrl; 
    }
     break;
    case MC_SET_CONTROL_VALUE_4_MAX_FLOW: //控制阀4最大流量
    {
        u16Size = sizeof( reqHydraulicValveCtrl );
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve2.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve3.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve4.m_u16Flow = data1 ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve5.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve6.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objHydraulicValveCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objHydraulicValveCtrl; 
    }
    break;
    case MC_SET_CONTROL_VALUE_4_PROGREAAION: //控制阀4渐进曲线 
    {
        u16Size = sizeof( reqHydraulicValveCtrl );
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve2.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve3.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve4.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u8NumberOfProgressiveCurv = data1 ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve5.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve6.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objHydraulicValveCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objHydraulicValveCtrl; 
    }
    break;
    case MC_SET_CONTROL_VALUE_4_MAX_TIME: //控制阀4最大时间
    {

        u16Size = sizeof( reqHydraulicValveCtrl );
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve2.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve3.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve4.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u16TMax = data1 ;

        this->m_objHydraulicValveCtrl.m_objValve5.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve6.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objHydraulicValveCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objHydraulicValveCtrl; 
    }
    break;
    case MC_SET_CONTROL_VALUE_5_MAX_FLOW : //控制阀5最大流量 
    {
        u16Size = sizeof( reqHydraulicValveCtrl );
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve2.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve3.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve4.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve5.m_u16Flow = data1 ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve6.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objHydraulicValveCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objHydraulicValveCtrl; 
    }
    break;
    case MC_SET_CONTROL_VALUE_5_PROGREAAION: //控制阀5渐进曲线 
    {
        u16Size = sizeof( reqHydraulicValveCtrl );
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve2.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve3.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve4.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve5.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u8NumberOfProgressiveCurv = data1 ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve6.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objHydraulicValveCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objHydraulicValveCtrl; 
    }
    break;
    case MC_SET_CONTROL_VALUE_5_MAX_TIME:  //控制阀5最大时间  
    {
        u16Size = sizeof( reqHydraulicValveCtrl );
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve2.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve3.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve4.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve5.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u16TMax = data1 ;

        this->m_objHydraulicValveCtrl.m_objValve6.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objHydraulicValveCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objHydraulicValveCtrl; 
    }
    break;
    case MC_SET_CONTROL_VALUE_6_MAX_FLOW: //控制阀6最大流量 
    {
        u16Size = sizeof( reqHydraulicValveCtrl );
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve2.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve3.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve4.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve5.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve6.m_u16Flow = data1 ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objHydraulicValveCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objHydraulicValveCtrl; 
    }
    break;
    case MC_SET_CONTROL_VALUE_6_PROGREAAION: //控制阀6渐进曲线  
    {
        u16Size = sizeof( reqHydraulicValveCtrl );
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve2.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve3.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve4.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve5.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve6.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u8NumberOfProgressiveCurv = data1 ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objHydraulicValveCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objHydraulicValveCtrl; 
    }
    break;
    case MC_SET_CONTROL_VALUE_6_MAX_TIME: //控制阀6最大时间
    {
        u16Size = sizeof( reqHydraulicValveCtrl );
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve1.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve2.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve2.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve3.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve3.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve4.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve4.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve5.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve5.m_u16TMax = 0xffff ;

        this->m_objHydraulicValveCtrl.m_objValve6.m_u16Flow = 0xffff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u8NumberOfProgressiveCurv = 0xff ;
        this->m_objHydraulicValveCtrl.m_objValve6.m_u16TMax = data1 ;

        this->m_objHydraulicValveCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objHydraulicValveCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objHydraulicValveCtrl; 
    }
    break;
    case MC_SET_GEAR_BOX_AUTO_SHIFT: //自动换档设置状态
    {
    // uint8_t  m_u8AutoGear;//自动换挡设置状态
    // uint16_t m_u16MaxVehicleSpeed;//最高车速
    // uint8_t  m_u8StartGear;//起步档位
    // uint8_t  m_u8PTOStatus;//PTO启停状态
    // uint8_t  m_u8PTOGear;//PTO档位
    // uint8_t  m_u8RearLiftHight;//后提升指定高度
    // uint16_t m_u16Crc;

        u16Size = sizeof( reqGearboxCtrl );
        this->m_objGearboxCtrl.m_u8AutoGear= data1 ;
        this->m_objGearboxCtrl.m_u16MaxVehicleSpeed = 0xffff ;
        this->m_objGearboxCtrl.m_u8StartGear = 0xff ;
        this->m_objGearboxCtrl.m_u8PTOStatus = 0xff ;
        this->m_objGearboxCtrl.m_u8PTOGear = 0xff ;
        this->m_objGearboxCtrl.m_u8RearLiftHight = 0xff;
        this->m_objGearboxCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objGearboxCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objGearboxCtrl;      
    }
    break;
    case MC_SET_GEAR_BOX_MAX_SPEED: //最高车速设置
    {
        u16Size = sizeof( reqGearboxCtrl );
        this->m_objGearboxCtrl.m_u8AutoGear= 0xff ;
        this->m_objGearboxCtrl.m_u16MaxVehicleSpeed = data1 ;
        this->m_objGearboxCtrl.m_u8StartGear = 0xff ;
        this->m_objGearboxCtrl.m_u8PTOStatus = 0xff ;
        this->m_objGearboxCtrl.m_u8PTOGear = 0xff ;
        this->m_objGearboxCtrl.m_u8RearLiftHight = 0xff;
        this->m_objGearboxCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objGearboxCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objGearboxCtrl;   
    }
    break;
    case MC_SET_GEAR_BOX_START_GEAR: //起步档位
    {
        u16Size = sizeof( reqGearboxCtrl );
        this->m_objGearboxCtrl.m_u8AutoGear= 0xff ;
        this->m_objGearboxCtrl.m_u16MaxVehicleSpeed = 0xffff ;
        this->m_objGearboxCtrl.m_u8StartGear = data1 ;
        this->m_objGearboxCtrl.m_u8PTOStatus = 0xff ;
        this->m_objGearboxCtrl.m_u8PTOGear = 0xff ;
        this->m_objGearboxCtrl.m_u8RearLiftHight = 0xff;
        this->m_objGearboxCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objGearboxCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objGearboxCtrl; 
    }
    break;
    case MC_SET_PTO_CONTROL: //PTO启停控制状态
    {
        u16Size = sizeof( reqGearboxCtrl );
        this->m_objGearboxCtrl.m_u8AutoGear= 0xff ;
        this->m_objGearboxCtrl.m_u16MaxVehicleSpeed = 0xffff ;
        this->m_objGearboxCtrl.m_u8StartGear = 0xff ;
        this->m_objGearboxCtrl.m_u8PTOStatus = data1 ;
        this->m_objGearboxCtrl.m_u8PTOGear = 0xff ;
        this->m_objGearboxCtrl.m_u8RearLiftHight = 0xff;
        this->m_objGearboxCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objGearboxCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objGearboxCtrl; 
    }
    break;
    case MC_SET_PTO_RPM: //PTO转速控制(档位)
    {
        u16Size = sizeof( reqGearboxCtrl );
        this->m_objGearboxCtrl.m_u8AutoGear= 0xff ;
        this->m_objGearboxCtrl.m_u16MaxVehicleSpeed = 0xffff ;
        this->m_objGearboxCtrl.m_u8StartGear = 0xff ;
        this->m_objGearboxCtrl.m_u8PTOStatus = 0xff ;
        this->m_objGearboxCtrl.m_u8PTOGear = data1 ;
        this->m_objGearboxCtrl.m_u8RearLiftHight = 0xff;
        this->m_objGearboxCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objGearboxCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objGearboxCtrl; 
    }
    break;
    case MC_SET_REAR_LIFT_HIGHT: // 后提升指定高度
    {
        u16Size = sizeof( reqGearboxCtrl );
        this->m_objGearboxCtrl.m_u8AutoGear= 0xff ;
        this->m_objGearboxCtrl.m_u16MaxVehicleSpeed = 0xffff ;
        this->m_objGearboxCtrl.m_u8StartGear = 0xff ;
        this->m_objGearboxCtrl.m_u8PTOStatus = 0xff ;
        this->m_objGearboxCtrl.m_u8PTOGear = 0xff ;
        this->m_objGearboxCtrl.m_u8RearLiftHight = data1;
        this->m_objGearboxCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objGearboxCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objGearboxCtrl; 
    }
    break;
    case MC_SET_LED_PROGRAM_MODE:  //灯光编程模式
    {
        // uint8_t  m_u8LedProgramMode;//当前灯光编程模式
        // uint8_t  m_u8FrontCeilingLed;//前顶棚
        // uint8_t  m_u8BackCeilingLed;//后顶棚
        // uint8_t  m_u8WorkLedLed;//前工作灯
        // uint8_t  m_u8FrontWaistLine;//腰线前工作灯
        // uint8_t  m_u8BackWaistLine;//腰线后工作灯
        // uint8_t  m_u8PositionLed;//位置灯
        // uint8_t  m_u8PeriodTime;//延时断电时间
        // uint16_t m_u16Crc;

        u16Size = sizeof( reqLedProgramCtrl );
        this->m_objLedProgramCtrl.m_u8LedProgramMode= data1 ;
        this->m_objLedProgramCtrl.m_u8FrontCeilingLed= 0xff ;
        this->m_objLedProgramCtrl.m_u8BackCeilingLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8WorkLedLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8FrontWaistLine = 0xff ;
        this->m_objLedProgramCtrl.m_u8BackWaistLine = 0xff ;
        this->m_objLedProgramCtrl.m_u8PositionLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8PeriodTime = 0xff ;
        this->m_objLedProgramCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objLedProgramCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objLedProgramCtrl; 
    }
    break;
    case MC_SET_FRONT_WORK_LIGHT:  //前顶棚工作灯信号
    {
        u16Size = sizeof( reqLedProgramCtrl );
        this->m_objLedProgramCtrl.m_u8LedProgramMode= 0xff ;
        this->m_objLedProgramCtrl.m_u8FrontCeilingLed= data1 ;
        this->m_objLedProgramCtrl.m_u8BackCeilingLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8WorkLedLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8FrontWaistLine = 0xff ;
        this->m_objLedProgramCtrl.m_u8BackWaistLine = 0xff ;
        this->m_objLedProgramCtrl.m_u8PositionLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8PeriodTime = 0xff ;
        this->m_objLedProgramCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objLedProgramCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objLedProgramCtrl; 
    }
    break;
    case MC_SET_REAR_WORK_LIGHT: //后顶棚工作灯信号
    {
        u16Size = sizeof( reqLedProgramCtrl );
        this->m_objLedProgramCtrl.m_u8LedProgramMode= 0xff ;
        this->m_objLedProgramCtrl.m_u8FrontCeilingLed= 0xff;
        this->m_objLedProgramCtrl.m_u8BackCeilingLed = data1 ;
        this->m_objLedProgramCtrl.m_u8WorkLedLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8FrontWaistLine = 0xff ;
        this->m_objLedProgramCtrl.m_u8BackWaistLine = 0xff ;
        this->m_objLedProgramCtrl.m_u8PositionLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8PeriodTime = 0xff ;
        this->m_objLedProgramCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objLedProgramCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objLedProgramCtrl; 
    }
    break;
    case MC_SET_FRONT_LIGHT: //前工作灯信号
    {
        u16Size = sizeof( reqLedProgramCtrl );
        this->m_objLedProgramCtrl.m_u8LedProgramMode= 0xff ;
        this->m_objLedProgramCtrl.m_u8FrontCeilingLed= 0xff;
        this->m_objLedProgramCtrl.m_u8BackCeilingLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8WorkLedLed = data1 ;
        this->m_objLedProgramCtrl.m_u8FrontWaistLine = 0xff ;
        this->m_objLedProgramCtrl.m_u8BackWaistLine = 0xff ;
        this->m_objLedProgramCtrl.m_u8PositionLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8PeriodTime = 0xff ;
        this->m_objLedProgramCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objLedProgramCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objLedProgramCtrl; 
    }
    break;
    case MC_SET_LINE_FRONT_WORK_LIGHT: //腰线前工作灯信号
    {
        u16Size = sizeof( reqLedProgramCtrl );
        this->m_objLedProgramCtrl.m_u8LedProgramMode= 0xff ;
        this->m_objLedProgramCtrl.m_u8FrontCeilingLed= 0xff;
        this->m_objLedProgramCtrl.m_u8BackCeilingLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8WorkLedLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8FrontWaistLine = data1 ;
        this->m_objLedProgramCtrl.m_u8BackWaistLine = 0xff ;
        this->m_objLedProgramCtrl.m_u8PositionLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8PeriodTime = 0xff ;
        this->m_objLedProgramCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objLedProgramCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objLedProgramCtrl; 
    }
    break;
    case MC_SET_LINE_REAR_WORK_LIGHT: //腰线后工作灯信号
    {
        u16Size = sizeof( reqLedProgramCtrl );
        this->m_objLedProgramCtrl.m_u8LedProgramMode= 0xff ;
        this->m_objLedProgramCtrl.m_u8FrontCeilingLed= 0xff;
        this->m_objLedProgramCtrl.m_u8BackCeilingLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8WorkLedLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8FrontWaistLine = 0xff ;
        this->m_objLedProgramCtrl.m_u8BackWaistLine = data1 ;
        this->m_objLedProgramCtrl.m_u8PositionLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8PeriodTime = 0xff ;
        this->m_objLedProgramCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objLedProgramCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objLedProgramCtrl; 
    }
    break;
    case MC_SET_POSITION_LIGHT: //位置灯信号
    {
        u16Size = sizeof( reqLedProgramCtrl );
        this->m_objLedProgramCtrl.m_u8LedProgramMode= 0xff ;
        this->m_objLedProgramCtrl.m_u8FrontCeilingLed= 0xff;
        this->m_objLedProgramCtrl.m_u8BackCeilingLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8WorkLedLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8FrontWaistLine = 0xff ;
        this->m_objLedProgramCtrl.m_u8BackWaistLine = 0xff ;
        this->m_objLedProgramCtrl.m_u8PositionLed = data1 ;
        this->m_objLedProgramCtrl.m_u8PeriodTime = 0xff ;
        this->m_objLedProgramCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objLedProgramCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objLedProgramCtrl; 
    }    
    break;
    case MC_SET_LIGHT_DELAY_POWER_OFF_TIME: //灯光延时关闭时间设置
    {     
        u16Size = sizeof( reqLedProgramCtrl );
        this->m_objLedProgramCtrl.m_u8LedProgramMode= 0xff ;
        this->m_objLedProgramCtrl.m_u8FrontCeilingLed= 0xff;
        this->m_objLedProgramCtrl.m_u8BackCeilingLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8WorkLedLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8FrontWaistLine = 0xff ;
        this->m_objLedProgramCtrl.m_u8BackWaistLine = 0xff ;
        this->m_objLedProgramCtrl.m_u8PositionLed = 0xff ;
        this->m_objLedProgramCtrl.m_u8PeriodTime = data1 ;
        this->m_objLedProgramCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objLedProgramCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objLedProgramCtrl;    
    }
    break;
    case MC_SET_LIGHT_STATE: //执行指定灯光
    {
        uint8_t byte0 = data1 & 0xFF;           // 用来取最低位字节
        uint8_t byte1 = (data1 >> 8) & 0xFF;    // 取次低位字节
        u16Size = sizeof( reqLedProgramCtrl );
        this->m_objLedProgramCtrl.m_u8LedProgramMode= 0xff ;
        this->m_objLedProgramCtrl.m_u8FrontCeilingLed= byte0 & 0x3;   
        this->m_objLedProgramCtrl.m_u8BackCeilingLed = (byte0 >> 2) & 0x3;
        this->m_objLedProgramCtrl.m_u8WorkLedLed = (byte0 >> 4) & 0x3;
        this->m_objLedProgramCtrl.m_u8FrontWaistLine = (byte0 >> 6) & 0x3; 
        this->m_objLedProgramCtrl.m_u8BackWaistLine = byte1 & 0x3;     
        this->m_objLedProgramCtrl.m_u8PositionLed = (byte1 >> 2) & 0x3; 
        this->m_objLedProgramCtrl.m_u8PeriodTime = (unsigned char)(data2);
        this->m_objLedProgramCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objLedProgramCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objLedProgramCtrl;    
    }
    break;
    case MC_SET_SKY_LIGHT_ADJUST: //天窗控制
    {
        u16Size = sizeof( reqSkiyLightCtrl );
        this->m_objSkiyLightCtrl.m_u8Cmd = data1;
        this->m_objSkiyLightCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objSkiyLightCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objSkiyLightCtrl; 
    }
    break;
    case MC_SET_LEFT_REAR_MIRROR_VERTICAL: //左后镜垂直
    {
        // uint8_t  m_u8RMSwitch; //后视镜调节开关
        // uint8_t  m_u8RMvertical; //后视镜方向调节垂直
        // uint8_t  m_u8RMhorizon; //后视镜方向调节水平
        // uint8_t  m_u8Heating;//后视镜加热状态
        // uint16_t m_u16Crc;

        u16Size = sizeof( reqRearviewMirrorCtrl );
        this->m_objRearviewMirrorCtrl.m_u8RMSwitch = 0x00;
        this->m_objRearviewMirrorCtrl.m_u8RMvertical = data1;
        this->m_objRearviewMirrorCtrl.m_u8RMhorizon = 0xff;
        this->m_objRearviewMirrorCtrl.m_u8Heating = 0xff ;
        this->m_objRearviewMirrorCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objRearviewMirrorCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objRearviewMirrorCtrl; 
    }
    break;
    case MC_SET_RIGHT_REAR_MIRROR_VERTICAL: //右后镜垂直
    {
        u16Size = sizeof( reqRearviewMirrorCtrl );
        this->m_objRearviewMirrorCtrl.m_u8RMSwitch = 0x01;
        this->m_objRearviewMirrorCtrl.m_u8RMvertical = data1;
        this->m_objRearviewMirrorCtrl.m_u8RMhorizon = 0xff;
        this->m_objRearviewMirrorCtrl.m_u8Heating = 0xff ;
        this->m_objRearviewMirrorCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objRearviewMirrorCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objRearviewMirrorCtrl; 
       
    }
    break;
    case MC_SET_LEFT_REAR_MIRROR_HORIZONTAL: //左后镜水平
    {
        u16Size = sizeof( reqRearviewMirrorCtrl );
        this->m_objRearviewMirrorCtrl.m_u8RMSwitch = 0x00;
        this->m_objRearviewMirrorCtrl.m_u8RMvertical = 0xff;
        this->m_objRearviewMirrorCtrl.m_u8RMhorizon = data1;
        this->m_objRearviewMirrorCtrl.m_u8Heating = 0xff ;
        this->m_objRearviewMirrorCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objRearviewMirrorCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objRearviewMirrorCtrl;  
    }
    break;
    case MC_SET_RIGHT_REAR_MIRROR_HORIZONTAL: //右后镜水平
    {
        u16Size = sizeof( reqRearviewMirrorCtrl );
        this->m_objRearviewMirrorCtrl.m_u8RMSwitch = 0x01;
        this->m_objRearviewMirrorCtrl.m_u8RMvertical = 0xff;
        this->m_objRearviewMirrorCtrl.m_u8RMhorizon = data1;
        this->m_objRearviewMirrorCtrl.m_u8Heating = 0xff ;
        this->m_objRearviewMirrorCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objRearviewMirrorCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objRearviewMirrorCtrl; 
    }
    break;
    case MC_SET_REAR_MIRROR_HEAT: //后镜加热开关 --- 不分左右后视镜
    {
        u16Size = sizeof( reqRearviewMirrorCtrl );
        this->m_objRearviewMirrorCtrl.m_u8RMSwitch = 0x01; 
        this->m_objRearviewMirrorCtrl.m_u8RMvertical = 0xff;
        this->m_objRearviewMirrorCtrl.m_u8RMhorizon = 0xff;
        this->m_objRearviewMirrorCtrl.m_u8Heating = data1 ;
        this->m_objRearviewMirrorCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objRearviewMirrorCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objRearviewMirrorCtrl; 
    }
    break;
    case MC_SET_AMBIENT_LIGHT: //氛围灯开关
    {
        // uint8_t m_u8Switch;//氛围灯开关
        // uint8_t m_u8Mode;//氛围灯模式
        // uint8_t m_u8Color;//氛围灯颜色
        // uint16_t m_u16Crc;

        u16Size = sizeof( reqAtmosphereLightsCtrl );
        this->m_objAtmosphereLightsCtrl.m_u8Switch = data1; 
        this->m_objAtmosphereLightsCtrl.m_u8Mode = 0x00;
        this->m_objAtmosphereLightsCtrl.m_u8Color = 0x00;
        this->m_objAtmosphereLightsCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objAtmosphereLightsCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objAtmosphereLightsCtrl; 
    }
    break;
    case MC_SET_AMBIENT_LIGHT_MODE: //氛围灯模式
    {
        u16Size = sizeof( reqAtmosphereLightsCtrl );
        this->m_objAtmosphereLightsCtrl.m_u8Switch = 0x00; 
        this->m_objAtmosphereLightsCtrl.m_u8Mode = data1;
        this->m_objAtmosphereLightsCtrl.m_u8Color = 0x00;
        this->m_objAtmosphereLightsCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objAtmosphereLightsCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objAtmosphereLightsCtrl; 
    }
    break;
    case MC_SET_AMBIENT_LIGHT_COLOR: //氛围灯颜色
    {
        u16Size = sizeof( reqAtmosphereLightsCtrl );
        this->m_objAtmosphereLightsCtrl.m_u8Switch = 0x00; 
        this->m_objAtmosphereLightsCtrl.m_u8Mode = 0x00;
        this->m_objAtmosphereLightsCtrl.m_u8Color = data1;
        this->m_objAtmosphereLightsCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objAtmosphereLightsCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objAtmosphereLightsCtrl; 
    }
    break;
    case MC_SET_METER_UNIT_LENGTH: //里程单位
    {
        // uint8_t  m_u8OddUnit : 2;
        // uint8_t  m_u8TempUnit : 2;
        // uint8_t  m_u8TimeUnit : 2;
        // uint8_t  m_u8TireUnit : 2;
        // uint8_t  m_u8ScreenBrightness : 4;//屏幕亮度
        // uint8_t  m_u8Language : 4;
        // uint32_t m_u32Time;
        // uint8_t  m_u8DarkDay : 2;//白天黑夜模式
        // uint8_t  m_u8AutoScreenOff : 4;//主题
        // uint16_t m_u16Crc;

        u16Size = sizeof( reqIVISettingCtrl );
        this->m_objIVISettingCtrl.m_u8OddUnit = data1; 
        this->m_objIVISettingCtrl.m_u8TempUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8TimeUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8TireUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8ScreenBrightness = 0x0F;
        this->m_objIVISettingCtrl.m_u8Language = 0x0F ;
        this->m_objIVISettingCtrl.m_u32Time = 0xffffffff;
        this->m_objIVISettingCtrl.m_u8DarkDay = 0x03;
        this->m_objIVISettingCtrl.m_u8AutoScreenOff = 0x0F;

        this->m_objIVISettingCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objIVISettingCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objIVISettingCtrl; 

    }
    break;
    case MC_SET_METER_UNIT_TEMPER: //温度格式
    {
        u16Size = sizeof( reqIVISettingCtrl );
        this->m_objIVISettingCtrl.m_u8OddUnit = 0x03; 
        this->m_objIVISettingCtrl.m_u8TempUnit = data1;
        this->m_objIVISettingCtrl.m_u8TimeUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8TireUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8ScreenBrightness = 0x0F;
        this->m_objIVISettingCtrl.m_u8Language = 0x0F ;
        this->m_objIVISettingCtrl.m_u32Time = 0xffffffff;
        this->m_objIVISettingCtrl.m_u8DarkDay = 0x03;
        this->m_objIVISettingCtrl.m_u8AutoScreenOff = 0x0F;

        this->m_objIVISettingCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objIVISettingCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objIVISettingCtrl; 
    }
    break;
    case MC_SET_METER_TIME_FORMAT: //时间格式
    {
        u16Size = sizeof( reqIVISettingCtrl );
        this->m_objIVISettingCtrl.m_u8OddUnit = 0x03; 
        this->m_objIVISettingCtrl.m_u8TempUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8TimeUnit = data1;
        this->m_objIVISettingCtrl.m_u8TireUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8ScreenBrightness = 0x0F;
        this->m_objIVISettingCtrl.m_u8Language = 0x0F ;
        this->m_objIVISettingCtrl.m_u32Time = 0xffffffff;
        this->m_objIVISettingCtrl.m_u8DarkDay = 0x03;
        this->m_objIVISettingCtrl.m_u8AutoScreenOff = 0x0F;

        this->m_objIVISettingCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objIVISettingCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objIVISettingCtrl; 
    }
    break;
    case MC_SET_TPMS_UNIT_PRESSURE: //设置胎压单位
    {
        u16Size = sizeof( reqIVISettingCtrl );
        this->m_objIVISettingCtrl.m_u8OddUnit = 0x03; 
        this->m_objIVISettingCtrl.m_u8TempUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8TimeUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8TireUnit = data1;
        this->m_objIVISettingCtrl.m_u8ScreenBrightness = 0x0F;
        this->m_objIVISettingCtrl.m_u8Language = 0x0F ;
        this->m_objIVISettingCtrl.m_u32Time = 0xffffffff;
        this->m_objIVISettingCtrl.m_u8DarkDay = 0x03;
        this->m_objIVISettingCtrl.m_u8AutoScreenOff = 0x0F;

        this->m_objIVISettingCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objIVISettingCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objIVISettingCtrl; 
    }
    break;
    case MC_SET_METER_BRIGHT_LEVEL: //仪表屏幕亮度调节模式
    {
        u16Size = sizeof( reqIVISettingCtrl );
        this->m_objIVISettingCtrl.m_u8OddUnit = 0x03; 
        this->m_objIVISettingCtrl.m_u8TempUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8TimeUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8TireUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8ScreenBrightness = data1;
        this->m_objIVISettingCtrl.m_u8Language = 0x0F ;
        this->m_objIVISettingCtrl.m_u32Time = 0xffffffff;
        this->m_objIVISettingCtrl.m_u8DarkDay = 0x03;
        this->m_objIVISettingCtrl.m_u8AutoScreenOff = 0x0F;
        this->m_objIVISettingCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objIVISettingCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objIVISettingCtrl; 
    }
    break;
    case MC_SET_METER_LANGUAGE:  // 语言
    {
        u16Size = sizeof( reqIVISettingCtrl );
        this->m_objIVISettingCtrl.m_u8OddUnit = 0x03; 
        this->m_objIVISettingCtrl.m_u8TempUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8TimeUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8TireUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8ScreenBrightness = 0x0F;
        this->m_objIVISettingCtrl.m_u8Language = data1 ;
        this->m_objIVISettingCtrl.m_u32Time = 0xffffffff;
        this->m_objIVISettingCtrl.m_u8DarkDay = 0x03;
        this->m_objIVISettingCtrl.m_u8AutoScreenOff = 0x0F;

        this->m_objIVISettingCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objIVISettingCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objIVISettingCtrl; 
    }
    break;
    case MC_SET_METER_TIME: //从1970到当前时间的时间戳，单位s
    {
        u16Size = sizeof( reqIVISettingCtrl );
        this->m_objIVISettingCtrl.m_u8OddUnit = 0x03; 
        this->m_objIVISettingCtrl.m_u8TempUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8TimeUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8TireUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8ScreenBrightness = 0x0F;
        this->m_objIVISettingCtrl.m_u8Language = 0x0F ;
        this->m_objIVISettingCtrl.m_u32Time = data1;
        this->m_objIVISettingCtrl.m_u8DarkDay = 0x03;
        this->m_objIVISettingCtrl.m_u8AutoScreenOff = 0x0F;

        this->m_objIVISettingCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objIVISettingCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objIVISettingCtrl; 
    }
    break;
    case MC_SET_DAY_MODEL: //白天黑夜模式
    {
        u16Size = sizeof( reqIVISettingCtrl );
        this->m_objIVISettingCtrl.m_u8OddUnit = 0x03; 
        this->m_objIVISettingCtrl.m_u8TempUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8TimeUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8TireUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8ScreenBrightness = 0x0F;
        this->m_objIVISettingCtrl.m_u8Language = 0x0F ;
        this->m_objIVISettingCtrl.m_u32Time = 0xffffffff;
        this->m_objIVISettingCtrl.m_u8DarkDay = data1;
        this->m_objIVISettingCtrl.m_u8AutoScreenOff = 0x0F;
        this->m_objIVISettingCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objIVISettingCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objIVISettingCtrl; 
    }
    break;
    case MC_SET_METER_THEME: //主题模式
    {
        u16Size = sizeof( reqIVISettingCtrl );
        this->m_objIVISettingCtrl.m_u8OddUnit = 0x03; 
        this->m_objIVISettingCtrl.m_u8TempUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8TimeUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8TireUnit = 0x03;
        this->m_objIVISettingCtrl.m_u8ScreenBrightness = 0x0F;
        this->m_objIVISettingCtrl.m_u8Language = 0x0F ;
        this->m_objIVISettingCtrl.m_u32Time = 0xffffffff;
        this->m_objIVISettingCtrl.m_u8DarkDay = 0x03;
        this->m_objIVISettingCtrl.m_u8AutoScreenOff = data1;
        this->m_objIVISettingCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objIVISettingCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objIVISettingCtrl; 
    }
    break;
    case MC_SET_METER_OTA: // 仪表打开升级页面
    {
        // uint8_t m_u8Cmd;
        // uint16_t m_u16Crc;
        u16Size = sizeof( reqIVICtrlSOC );
        this->m_objIVICtrlSOC.m_u8Cmd = data1; 
        this->m_objIVICtrlSOC.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objIVICtrlSOC , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objIVICtrlSOC; 
    }
    break;
    case MC_SET_WORK_CTRL: //作业控制
    {
        
        // uint8_t   m_u8Cmd;//作业控制，00结束，01开始，0xff无效值
        // uint16_t  m_u16LeftWidth;//左幅宽参数
        // uint16_t  m_u16RightWidth;//右幅宽参数
        // uint8_t   m_u8WorkType;//作业控制，作业类型
        // uint16_t m_u16Crc;

        u16Size = sizeof( reqWorkCtrl );
        this->m_objWorkCtrl.m_u8Cmd = data1; 
        this->m_objWorkCtrl.m_u16LeftWidth = 0xffff; 
        this->m_objWorkCtrl.m_u16RightWidth = 0xffff; 
        this->m_objWorkCtrl.m_u8WorkType = 0xff; 
        this->m_objWorkCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objWorkCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objWorkCtrl; 
    }
    break;
    case MC_SET_Left_WIDTH_PARAMETER: // 左幅宽参数
    {
        u16Size = sizeof( reqWorkCtrl );
        this->m_objWorkCtrl.m_u8Cmd = 0xff; 
        this->m_objWorkCtrl.m_u16LeftWidth = data1; 
        this->m_objWorkCtrl.m_u16RightWidth = 0xffff; 
        this->m_objWorkCtrl.m_u8WorkType = 0xff; 
        this->m_objWorkCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objWorkCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objWorkCtrl; 
    }
    break;
    case MC_SET_RIGHT_WIDTH_PARAMETER: // 右幅宽参数
    {
        u16Size = sizeof( reqWorkCtrl );
        this->m_objWorkCtrl.m_u8Cmd = 0xff; 
        this->m_objWorkCtrl.m_u16LeftWidth = 0xffff; 
        this->m_objWorkCtrl.m_u16RightWidth = data1; 
        this->m_objWorkCtrl.m_u8WorkType = 0xff; 
        this->m_objWorkCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objWorkCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objWorkCtrl; 
    }
    break;
    case MC_SET_WORK_TYPE: // 作业类型
    {
        u16Size = sizeof( reqWorkCtrl );
        this->m_objWorkCtrl.m_u8Cmd = 0xff; 
        this->m_objWorkCtrl.m_u16LeftWidth = 0xffff; 
        this->m_objWorkCtrl.m_u16RightWidth = 0xffff; 
        this->m_objWorkCtrl.m_u8WorkType = data1; 
        this->m_objWorkCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objWorkCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objWorkCtrl; 
    }
    break;
    case MC_4H_CALIBRATION_FRONT_AXLE_SENSOR_MIDDLE : //4H参数标定 - 前桥转角传感器中位标定
    {
        // uint8_t  m_u8FrontAxleSensorMiddle : 1;//前桥转角传感器中位标定
        // uint8_t  m_u8FrontAxleSensorLeft : 1;//前桥转角传感器左极限标定
        // uint8_t  m_u8FrontAxleSensorRight : 1;//前桥转角传感器右极限标定
        // uint8_t  m_u8PTOHGearAngle : 1;//PTO-H档角度
        // uint8_t  m_u8PTOMGearAngle : 1;//PTO-M档角度
        // uint8_t  m_u8PTOLGearAngle : 1;//PTO-L档角度
        // uint8_t  m_u8PTONGearAngle : 1;//PTO-N档角度

        // uint8_t  m_u8AuxiliaryGearBoxHGearAngle : 1;//副变速箱H档角度
        // uint8_t  m_u8AuxiliaryGearBoxLGearAngle : 1;//副变速箱L档角度
        // uint8_t  m_u8AuxiliaryGearBoxNGearAngle : 1;//副变速箱N档角度

        // uint8_t  m_u8ClutchAngleSensorReleas : 1;//离合器角度传感器松开
        // uint8_t  m_u8ClutchAngleSensorFloor : 1;//离合器角度传感器踩到底
        // uint8_t  : 4;

        // uint8_t  m_u8PageNum;//参数标定页码编号
        // uint8_t  m_u8ParamNum;//参数标定编号
        // uint8_t  m_u8Param;//标定参数值

        u16Size = sizeof( reqCalibrationRequest4HCtrl );

        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorMiddle = data1; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorLeft = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorRight = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOHGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOMGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOLGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTONGearAngle = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxHGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxLGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxNGearAngle = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorReleas = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorFloor = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8PageNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8ParamNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8Param = 0xffff; 

        this->m_objCalibrationRequest4HCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objCalibrationRequest4HCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objCalibrationRequest4HCtrl; 
    }
    break;
    case MC_4H_CALIBRATION_FRONT_AXLE_SENSOR_LEFT : //4H参数标定 -前桥转角传感器左极限标定
    {
        u16Size = sizeof( reqCalibrationRequest4HCtrl );

        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorMiddle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorLeft = data1; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorRight = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOHGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOMGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOLGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTONGearAngle = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxHGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxLGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxNGearAngle = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorReleas = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorFloor = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8PageNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8ParamNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8Param = 0xffff;

        this->m_objCalibrationRequest4HCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objCalibrationRequest4HCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objCalibrationRequest4HCtrl; 
    }
    break;
    case MC_4H_CALIBRATION_FRONT_AXLE_SENSOR_RIGHT : //4H参数标定 -前桥转角传感器右极限标定
    {
        u16Size = sizeof( reqCalibrationRequest4HCtrl );

        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorMiddle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorLeft = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorRight = data1; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOHGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOMGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOLGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTONGearAngle = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxHGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxLGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxNGearAngle = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorReleas = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorFloor = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8PageNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8ParamNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8Param = 0xffff; 

        this->m_objCalibrationRequest4HCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objCalibrationRequest4HCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objCalibrationRequest4HCtrl; 
    }
    break;
    case MC_4H_CALIBRATION_PTO_H_GEAR_ANGLE : //4H参数标定 -PTO-H档角度
    {
        u16Size = sizeof( reqCalibrationRequest4HCtrl );

        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorMiddle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorLeft = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorRight = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOHGearAngle = data1 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOMGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOLGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTONGearAngle = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxHGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxLGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxNGearAngle = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorReleas = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorFloor = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8PageNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8ParamNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8Param = 0xffff;  

        this->m_objCalibrationRequest4HCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objCalibrationRequest4HCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objCalibrationRequest4HCtrl; 
    }
    break;
    case MC_4H_CALIBRATION_PTO_M_GEAR_ANGLE : //4H参数标定 -PTO-M档角度
    {
        u16Size = sizeof( reqCalibrationRequest4HCtrl );

        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorMiddle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorLeft = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorRight = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOHGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOMGearAngle = data1 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOLGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTONGearAngle = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxHGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxLGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxNGearAngle = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorReleas = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorFloor = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8PageNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8ParamNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8Param = 0xffff;  

        this->m_objCalibrationRequest4HCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objCalibrationRequest4HCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objCalibrationRequest4HCtrl; 
    }
    break;
    case MC_4H_CALIBRATION_PTO_L_GEAR_ANGLE : //4H参数标定 -PTO-L档角度
    {
        u16Size = sizeof( reqCalibrationRequest4HCtrl );

        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorMiddle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorLeft = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorRight = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOHGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOMGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOLGearAngle = data1 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTONGearAngle = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxHGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxLGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxNGearAngle = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorReleas = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorFloor = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8PageNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8ParamNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8Param = 0xffff;  

        this->m_objCalibrationRequest4HCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objCalibrationRequest4HCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objCalibrationRequest4HCtrl; 
    }
    break;
    case MC_4H_CALIBRATION_PTO_N_GEAR_ANGLE : //4H参数标定 -PTO-N档角度
    {
        u16Size = sizeof( reqCalibrationRequest4HCtrl );

        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorMiddle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorLeft = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorRight = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOHGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOMGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOLGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTONGearAngle = data1 ; 

        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxHGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxLGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxNGearAngle = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorReleas = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorFloor = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8PageNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8ParamNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8Param = 0xffff; 

        this->m_objCalibrationRequest4HCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objCalibrationRequest4HCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objCalibrationRequest4HCtrl; 
    }
    break;
    case MC_4H_CALIBRATION_AUXILIARY_GEAR_BOX_H_GEAR_ANGLE : //4H参数标定 -副变速箱H档角度
    {
        u16Size = sizeof( reqCalibrationRequest4HCtrl );

        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorMiddle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorLeft = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorRight = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOHGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOMGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOLGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTONGearAngle = 0x00 ; 

        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxHGearAngle = data1; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxLGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxNGearAngle = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorReleas = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorFloor = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8PageNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8ParamNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8Param = 0xffff; 

        this->m_objCalibrationRequest4HCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objCalibrationRequest4HCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objCalibrationRequest4HCtrl; 
    }
    break;
    case MC_4H_CALIBRATION_AUXILIARY_GEAR_BOX_L_GEAR_ANGLE : //4H参数标定 -副变速箱L档角度
    {
        u16Size = sizeof( reqCalibrationRequest4HCtrl );

        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorMiddle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorLeft = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorRight = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOHGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOMGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOLGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTONGearAngle = 0x00 ; 

        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxHGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxLGearAngle = data1; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxNGearAngle = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorReleas = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorFloor = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8PageNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8ParamNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8Param = 0xffff; 

        this->m_objCalibrationRequest4HCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objCalibrationRequest4HCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objCalibrationRequest4HCtrl; 
    }
    break;
    case MC_4H_CALIBRATION_AUXILIARY_GEAR_BOX_N_GEAR_ANGLE : //4H参数标定 -副变速箱N档角度
    {
        u16Size = sizeof( reqCalibrationRequest4HCtrl );

        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorMiddle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorLeft = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorRight = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOHGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOMGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOLGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTONGearAngle = 0x00 ; 

        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxHGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxLGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxNGearAngle = data1; 

        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorReleas = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorFloor = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8PageNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8ParamNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8Param = 0xffff; 

        this->m_objCalibrationRequest4HCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objCalibrationRequest4HCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objCalibrationRequest4HCtrl; 
    }
    break;
    case MC_4H_CALIBRATION_CLUTCH_ANGLE_SENSOR_RELEAS : //4H参数标定 -离合器角度传感器松开
    {
        u16Size = sizeof( reqCalibrationRequest4HCtrl );

        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorMiddle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorLeft = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorRight = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOHGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOMGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOLGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTONGearAngle = 0x00 ; 

        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxHGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxLGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxNGearAngle = 0x00 ; 

        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorReleas = data1 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorFloor = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8PageNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8ParamNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8Param = 0xffff; 

        this->m_objCalibrationRequest4HCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objCalibrationRequest4HCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objCalibrationRequest4HCtrl; 
    }
    break;
    case MC_4H_CALIBRATION_CLUTCH_ANGLE_SENSOR_FLOOR : //4H参数标定 -离合器角度传感器踩到底 
    {
        u16Size = sizeof( reqCalibrationRequest4HCtrl );

        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorMiddle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorLeft = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorRight = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOHGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOMGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOLGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTONGearAngle = 0x00 ; 

        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxHGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxLGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxNGearAngle = 0x00 ; 

        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorReleas = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorFloor = data1; 

        this->m_objCalibrationRequest4HCtrl.m_u8PageNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8ParamNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8Param = 0xffff; 

        this->m_objCalibrationRequest4HCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objCalibrationRequest4HCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objCalibrationRequest4HCtrl; 
    }
    break;
    case MC_4H_CALIBRATION_PAGE_NUM : //4H参数标定 -参数标定页码编号
    {
        u16Size = sizeof( reqCalibrationRequest4HCtrl );

        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorMiddle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorLeft = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorRight = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOHGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOMGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOLGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTONGearAngle = 0x00 ; 

        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxHGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxLGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxNGearAngle = 0x00 ; 

        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorReleas = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorFloor = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8PageNum = data1; 
        this->m_objCalibrationRequest4HCtrl.m_u8ParamNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8Param = 0xffff; 

        this->m_objCalibrationRequest4HCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objCalibrationRequest4HCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objCalibrationRequest4HCtrl; 
    }
    break;
    case MC_4H_CALIBRATION_PARAM_NUM_AND_VALUE : //4H参数标定 -参数标定编号/参数值
    {
        u16Size = sizeof( reqCalibrationRequest4HCtrl );

        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorMiddle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorLeft = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8FrontAxleSensorRight = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOHGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOMGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTOLGearAngle = 0x00 ; 
        this->m_objCalibrationRequest4HCtrl.m_u8PTONGearAngle = 0x00 ; 

        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxHGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxLGearAngle = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8AuxiliaryGearBoxNGearAngle = 0x00 ; 

        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorReleas = 0x00; 
        this->m_objCalibrationRequest4HCtrl.m_u8ClutchAngleSensorFloor = 0x00; 

        this->m_objCalibrationRequest4HCtrl.m_u8PageNum = 0xff; 
        this->m_objCalibrationRequest4HCtrl.m_u8ParamNum = data1; 
        this->m_objCalibrationRequest4HCtrl.m_u8Param = data2; 

        this->m_objCalibrationRequest4HCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objCalibrationRequest4HCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objCalibrationRequest4HCtrl; 
    }
    break;
    case MC_SET_MCU_REBOOT: // soc通知mcu重启
    {
        u16Size = sizeof( reqSOCCtrlMCU );
        this->m_objSocCtrlMcu.m_u8Reserve = data1; 
        this->m_objSocCtrlMcu.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objSocCtrlMcu , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objSocCtrlMcu; 
    }
    break;
    default:
        ret = -1;
    break;
    }

    if(pCmd){
        //发送指令
         McuParserBase::SendData(pCmd,  u16Size);

        std::string stHex = this->convertToHexString( pCmd , u16Size); 
        #ifdef T5COMPILE
            LOGD("SendData : %s ", stHex.c_str());
            // for(int i = 0;i < u16Size;i++)
            // {
            //     LOGD("%02X ",pCmd[i]);
            // }
        #else
        std::cout << "SendData : " << stHex << std::endl;
            // for(int i = 0;i < u16Size;i++)
            // {
            //     printf("%02X ",pCmd[i]);
            // }
            // printf("\n");
        #endif
    }

    return ret;
}

/**
 * Overload 
 */
int McuParser::SendCmd(MCU_COMMAND cmd, std::string const &str, double p1 ,double p2, double p3 ,double p4 )
{
    int ret = 0;
    int sendLen = 0;
    unsigned short u16Size = 0 ; 
    unsigned char *pCmd = nullptr ;
    char *u8Separator = ",";
    cJSON *root = NULL ;

    switch (cmd)
    {
    case MC_SET_TPM_BT: //蓝牙检测  
    {// {"bt_version": "V2.1.7.36" , "bt_name" : "LGNJ11-1111"}  
        
        //清空 reqTPMCtrl -> m_u8Response
        std::memset(this->m_objTpmCtrl.m_u8Response, 0, sizeof(this->m_objTpmCtrl.m_u8Response));
        
        u16Size = sizeof( reqTPMCtrl );

        //解析Json串
        root = cJSON_Parse(str.c_str()); // 解析 JSON 字符串
        if (root == NULL) {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_BT : JSON parse error! ");
            #else
            #endif
            return -1;
        }

        cJSON *stBtVersionJson = cJSON_GetObjectItem(root, "bt_version");
        cJSON *stBtNameJson = cJSON_GetObjectItem(root, "bt_name");
        if( stBtVersionJson == NULL || stBtNameJson == NULL )
        {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_BT : stBtVersionJson || stBtNameJson is NULL ");
            #else
            #endif
            ret = -1 ;
            break;
        }

        char *stBtVersion = NULL;
        char *stBtName = NULL;
        if (cJSON_IsString(stBtVersionJson)) {
            stBtVersion = stBtVersionJson->valuestring;
           
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_BT : bt_version: %s , length : %d\n",  stBtVersion , strlen(stBtVersion));
            #else
            #endif

        }else{
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_BT : bt_version is not string type...");
            #else
            #endif
            ret = -1 ;
            break;
        }

        if (cJSON_IsString(stBtNameJson)) {
            stBtName = stBtNameJson->valuestring ;
            
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_BT : bt_name: %s, length : %d\n", stBtName , strlen(stBtName) );
            #else
            #endif
        }else{
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_BT : bt_name is not string type...");
            #else
            #endif
            ret = -1 ;
            break;
        }

        //赋值
        this->m_objTpmCtrl.m_u16Cmd = TPM_TEST_BT ; 
        std::memcpy( this->m_objTpmCtrl.m_u8Response  , stBtVersion ,strlen(stBtVersion));
        sendLen += strlen(stBtVersion);
      
        std::memcpy( this->m_objTpmCtrl.m_u8Response + sendLen , u8Separator ,strlen(u8Separator));
        sendLen += strlen(u8Separator);

        std::memcpy( this->m_objTpmCtrl.m_u8Response + sendLen , stBtName ,strlen(stBtName));
        sendLen += strlen(stBtName);

        this->m_objTpmCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objTpmCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objTpmCtrl;


    }
    break;
    case MC_SET_TPM_WIFI: //WIFI检测  
    {// {"wifi_name": "test_wifi"}  
        std::memset(this->m_objTpmCtrl.m_u8Response, 0, sizeof(this->m_objTpmCtrl.m_u8Response));
        
        u16Size = sizeof( reqTPMCtrl );

        //解析Json串
        root = cJSON_Parse(str.c_str()); // 解析 JSON 字符串
        if (root == NULL) {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_WIFI : JSON parse error! ");
            #else
            #endif
            return -1;
        }

        cJSON *stWifiNameJson = cJSON_GetObjectItem(root, "wifi_name");
        if( stWifiNameJson == NULL )
        {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_WIFI : stWifiNameJson is NULL ");
            #else
            #endif
            ret = -1 ;
            break;
        }


        char *stWifiName = NULL;
        if (cJSON_IsString(stWifiNameJson)) {
            stWifiName = stWifiNameJson->valuestring;
           
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_WIFI : wifi_name: %s , length : %d\n",  stWifiName , strlen(stWifiName));
            #else
            #endif
        }else{
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_WIFI : wifi_name is not string type...");
            #else
            #endif
            ret = -1 ;
            break;
        }

        //赋值
        this->m_objTpmCtrl.m_u16Cmd = TPM_TEST_WIFI ; 
        std::memcpy( this->m_objTpmCtrl.m_u8Response , stWifiName ,strlen(stWifiName));
        sendLen += strlen(stWifiName);

        this->m_objTpmCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objTpmCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objTpmCtrl;

    }
    break;
    case MC_SET_TPM_RGB: //RGB检测  
    {// {"rgb_result": "ok"}  
       std::memset(this->m_objTpmCtrl.m_u8Response, 0, sizeof(this->m_objTpmCtrl.m_u8Response));
        
        u16Size = sizeof( reqTPMCtrl );

        //解析Json串
        root = cJSON_Parse(str.c_str()); // 解析 JSON 字符串
        if (root == NULL) {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_RGB : JSON parse error! ");
            #else
            #endif
            return -1;
        }

        cJSON *stResultJson = cJSON_GetObjectItem(root, "rgb_result");
        if( stResultJson == NULL )
        {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_RGB : stResultJson is NULL ");
            #else
            #endif
            ret = -1 ;
            break;

        }

        char *stResult = NULL;

        if (cJSON_IsString(stResultJson)) {
            stResult = stResultJson->valuestring;
           
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_RGB : rgb_result: %s , length : %d\n",  stResult , strlen(stResult));
            #else
            #endif
        }else{
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_RGB : rgb_result is not string type...");
            #else
            #endif
            ret = -1 ;
            break;
        }

        //赋值
        this->m_objTpmCtrl.m_u16Cmd = TPM_TEST_RGB ; 
        std::memcpy( this->m_objTpmCtrl.m_u8Response , stResult ,strlen(stResult));
        sendLen += strlen(stResult);

        this->m_objTpmCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objTpmCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objTpmCtrl;

    }
    break;
    case MC_SET_TPM_SOFTVERSION: //软件版本 
    {// {"gauge_version": "V1.0.017" , "bll_version" : "V1.0.075" , "core_hardware_version" : "V1.1"}  
        std::memset(this->m_objTpmCtrl.m_u8Response, 0, sizeof(this->m_objTpmCtrl.m_u8Response));
        
        u16Size = sizeof( reqTPMCtrl );

        //解析Json串
        root = cJSON_Parse(str.c_str()); // 解析 JSON 字符串
        if (root == NULL) {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_SOFTVERSION : JSON parse error! ");
            #else
            #endif
            return -1;
        }

        cJSON *stGaugeVersionJson = cJSON_GetObjectItem(root, "gauge_version");
        cJSON *stBllVersionJson = cJSON_GetObjectItem(root, "bll_version");
        cJSON *stCoreHardwareVersionJson = cJSON_GetObjectItem(root, "core_hardware_version");

        if( stGaugeVersionJson == NULL ||  stBllVersionJson == NULL ||  stCoreHardwareVersionJson == NULL )
        {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_SOFTVERSION : stGaugeVersionJson || stBllVersionJson || stCoreHardwareVersionJson is NULL ");
            #else
            #endif
            ret = -1 ;
            break;
        }


        char *stGaugeVersion = NULL;
        char *stBllVersion = NULL;
        char *stCoreHardwareVersion = NULL;
        if (cJSON_IsString(stGaugeVersionJson)) {
            stGaugeVersion = stGaugeVersionJson->valuestring;
           
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_SOFTVERSION : gauge_version: %s , length : %d\n",  stGaugeVersion , strlen(stGaugeVersion));
            #else
            #endif
        }else{
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_SOFTVERSION : gauge_version is not string type...");
            #else
            #endif
            ret = -1 ;
            break;
        }

        if (cJSON_IsString(stBllVersionJson)) {
            stBllVersion = stBllVersionJson->valuestring;
           
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_SOFTVERSION : bll_version: %s , length : %d\n"
                ,  stBllVersion 
                , strlen(stBllVersion));
            #else
            #endif
        }else{
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_SOFTVERSION : bll_version is not string type...");
            #else
            #endif
            ret = -1 ;
            break;
        }

        if (cJSON_IsString(stCoreHardwareVersionJson)) {
            stCoreHardwareVersion = stCoreHardwareVersionJson->valuestring;
           
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_SOFTVERSION : core_hardware_version: %s , length : %d\n"
                ,  stCoreHardwareVersion 
                , strlen(stCoreHardwareVersion));
            #else
            #endif
        }else{
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_SOFTVERSION : core_hardware_version is not string type...");
            #else
            #endif
            ret = -1 ;
            break;
        }

        //赋值
        this->m_objTpmCtrl.m_u16Cmd = TPM_TEST_SOFTVERSION ; 
        std::memcpy( this->m_objTpmCtrl.m_u8Response , stGaugeVersion ,strlen(stGaugeVersion));
        sendLen += strlen(stGaugeVersion);

        std::memcpy( this->m_objTpmCtrl.m_u8Response + sendLen  , u8Separator ,strlen(u8Separator));
        sendLen += strlen(u8Separator);

        std::memcpy( this->m_objTpmCtrl.m_u8Response + sendLen , stBllVersion ,strlen(stBllVersion));
        sendLen += strlen(stBllVersion);

        std::memcpy( this->m_objTpmCtrl.m_u8Response + sendLen , u8Separator ,strlen(u8Separator));
        sendLen += strlen(u8Separator);

        std::memcpy( this->m_objTpmCtrl.m_u8Response + sendLen , stCoreHardwareVersion ,strlen(stCoreHardwareVersion));
        sendLen += strlen(stCoreHardwareVersion);

        this->m_objTpmCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objTpmCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objTpmCtrl;



    }
    break;
    case MC_SET_TPM_BRIGHTNESS: //背光亮度 
    {// {"brightness_result : ok "}  
        std::memset(this->m_objTpmCtrl.m_u8Response, 0, sizeof(this->m_objTpmCtrl.m_u8Response));
        
        u16Size = sizeof( reqTPMCtrl );

        //解析Json串
        root = cJSON_Parse(str.c_str()); // 解析 JSON 字符串
        if (root == NULL) {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_BRIGHTNESS : JSON parse error! ");
            #else
            #endif
            return -1;
        }

        cJSON *stResultJson = cJSON_GetObjectItem(root, "brightness_result");
        if( stResultJson == NULL )
        {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_BRIGHTNESS : stResultJson is NULL ");
            #else
            #endif
            ret = -1 ;
            break;
        }


        char *stResult = NULL;
        if (cJSON_IsString(stResultJson)) {
            stResult = stResultJson->valuestring;
           
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_BRIGHTNESS : rgb_result: %s , length : %d\n",  stResult , strlen(stResult));
            #else
            #endif
        }else{
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_BRIGHTNESS : rgb_result is not string type...");
            #else
            #endif
            ret = -1 ;
            break;
        }

        //赋值
        this->m_objTpmCtrl.m_u16Cmd = TPM_TEST_BRIGHTNESS ; 
        std::memcpy( this->m_objTpmCtrl.m_u8Response , stResult ,strlen(stResult));
        sendLen += strlen(stResult);

        this->m_objTpmCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objTpmCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objTpmCtrl;


    }
    break;
    case MC_SET_TPM_EC_VERSION: //互联版本 
    {// {"ec_sdk_version": "V1.0.017" , "platform_version" : "V1.0.075" , "easy_connect" : "V1.1"}  
        std::memset(this->m_objTpmCtrl.m_u8Response, 0, sizeof(this->m_objTpmCtrl.m_u8Response));

        u16Size = sizeof( reqTPMCtrl );

        //解析Json串
        root = cJSON_Parse(str.c_str()); // 解析 JSON 字符串
        if (root == NULL) {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_EC_VERSION : JSON parse error! ");
            #else
            #endif
            return -1;
        }

        cJSON *stEcSdkVersionJson = cJSON_GetObjectItem(root, "ec_sdk_version");
        cJSON *stPlatformVersionJson = cJSON_GetObjectItem(root, "platform_version");
        cJSON *stEasyConnectVersionJson = cJSON_GetObjectItem(root, "easy_connect");

        if( stEcSdkVersionJson == NULL || stPlatformVersionJson == NULL || stEasyConnectVersionJson == NULL )
        {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_EC_VERSION : stEcSdkVersionJson || stPlatformVersionJson || stEasyConnectVersionJson  is NULL ");
            #else
            #endif
            ret = -1 ;
            break;
        }


        char *stEcSdkVersion = NULL;
        char *stPlatformVersion = NULL;
        char *stEasyConnectVersion = NULL;

        if (cJSON_IsString(stEcSdkVersionJson)) {
            stEcSdkVersion = stEcSdkVersionJson->valuestring;
           
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_EC_VERSION : ec_sdk_version: %s , length : %d\n",  stEcSdkVersion , strlen(stEcSdkVersion));
            #else
            #endif
        }else{
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_EC_VERSION : ec_sdk_version is not string type...");
            #else
            #endif
            ret = -1 ;
            break;
        }

        if (cJSON_IsString(stPlatformVersionJson)) {
            stPlatformVersion = stPlatformVersionJson->valuestring;
           
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_EC_VERSION : platform_version: %s , length : %d\n"
                ,  stPlatformVersion 
                , strlen(stPlatformVersion));
            #else
            #endif
        }else{
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_EC_VERSION : platform_version is not string type...");
            #else
            #endif
                        ret = -1 ;
            break;
        }

        if (cJSON_IsString(stEasyConnectVersionJson)) {
            stEasyConnectVersion = stEasyConnectVersionJson->valuestring;
           
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_EC_VERSION : easy_connect: %s , length : %d\n"
                ,  stEasyConnectVersion 
                , strlen(stEasyConnectVersion));
            #else
            #endif
        }else{
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_EC_VERSION : core_hardware_version is not string type...");
            #else
            #endif
            ret = -1 ;
            break;
        }

        //赋值
        this->m_objTpmCtrl.m_u16Cmd = TPM_TEST_EC_VERSION ; 
        std::memcpy( this->m_objTpmCtrl.m_u8Response , stEcSdkVersion ,strlen(stEcSdkVersion));
        sendLen += strlen(stEcSdkVersion);

        std::memcpy( this->m_objTpmCtrl.m_u8Response + sendLen , u8Separator ,strlen(u8Separator));
        sendLen += strlen(u8Separator);

        std::memcpy( this->m_objTpmCtrl.m_u8Response + sendLen , stPlatformVersion ,strlen(stPlatformVersion));
        sendLen += strlen(stPlatformVersion);

        std::memcpy( this->m_objTpmCtrl.m_u8Response + sendLen , u8Separator ,strlen(u8Separator));
        sendLen += strlen(u8Separator);

        std::memcpy( this->m_objTpmCtrl.m_u8Response + sendLen , stEasyConnectVersion ,strlen(stEasyConnectVersion));
        sendLen += strlen(stEasyConnectVersion);

        this->m_objTpmCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objTpmCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objTpmCtrl;

    }
    break;
    case MC_SET_TPM_LANGUAGE: //语言设置
    {// {"language_set_result : ok "}  
       std::memset(this->m_objTpmCtrl.m_u8Response, 0, sizeof(this->m_objTpmCtrl.m_u8Response));
       
        u16Size = sizeof( reqTPMCtrl );

        //解析Json串
        root = cJSON_Parse(str.c_str()); // 解析 JSON 字符串
        if (root == NULL) {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_LANGUAGE : JSON parse error! ");
            #else
            #endif
            return -1;
        }

        cJSON *stResultJson = cJSON_GetObjectItem(root, "language_set_result");
        if( stResultJson == NULL )
        {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_LANGUAGE : stResultJson  is NULL ");
            #else
            #endif
                        ret = -1 ;
            break;
        }


        char *stResult = NULL;

        if (cJSON_IsString(stResultJson)) 
        {
            stResult = stResultJson->valuestring;
           
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_LANGUAGE : language_set_result: %s , length : %d\n",  stResult , strlen(stResult));
            #else
            #endif
        }
        else
        {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_LANGUAGE : language_set_result is not string type...");
            #else
            #endif
                        ret = -1 ;
            break;
        }

        //赋值
        this->m_objTpmCtrl.m_u16Cmd = TPM_TEST_LANGUAGE ; 
        std::memcpy( this->m_objTpmCtrl.m_u8Response , stResult ,strlen(stResult));
        sendLen += strlen(stResult);

        this->m_objTpmCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objTpmCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objTpmCtrl;


    }
    break;
    case MC_SET_TPM_VOICE_CHIPS: //语音芯片
    {// {"voice_version : V1.0.012 "}  
        std::memset(this->m_objTpmCtrl.m_u8Response, 0, sizeof(this->m_objTpmCtrl.m_u8Response));
       
        u16Size = sizeof( reqTPMCtrl );

        //解析Json串
        root = cJSON_Parse(str.c_str()); // 解析 JSON 字符串
        if (root == NULL) {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_VOICE_CHIPS : JSON parse error! ");
            #else
            #endif
            return -1;
        }

        cJSON *stVoiceVersionJson = cJSON_GetObjectItem(root, "voice_version");
        if( stVoiceVersionJson == NULL )
        {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_VOICE_CHIPS : stVoiceVersionJson  is NULL ");
            #else
            #endif
                        ret = -1 ;
            break;
        }


        char *stVoiceVersion = NULL;
        if (cJSON_IsString(stVoiceVersionJson)) 
        {
            stVoiceVersion = stVoiceVersionJson->valuestring;
           
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_VOICE_CHIPS : voice_version: %s , length : %d\n",  stVoiceVersion , strlen(stVoiceVersion));
            #else
            #endif
        }
        else
        {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_VOICE_CHIPS : voice_version is not string type...");
            #else
            #endif
                        ret = -1 ;
            break;
        }

        //赋值
        this->m_objTpmCtrl.m_u16Cmd = TPM_TEST_VOICE_CHIPS ; 
        std::memcpy( this->m_objTpmCtrl.m_u8Response , stVoiceVersion ,strlen(stVoiceVersion));
        sendLen += strlen(stVoiceVersion);

        this->m_objTpmCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objTpmCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objTpmCtrl;
    }
    break;
    case MC_SET_TPM_CURRENT_CARTYPE: //当前车型
    {// {"current_type : ok "}  
       std::memset(this->m_objTpmCtrl.m_u8Response, 0, sizeof(this->m_objTpmCtrl.m_u8Response));

       
        u16Size = sizeof( reqTPMCtrl );

        //解析Json串
        root = cJSON_Parse(str.c_str()); // 解析 JSON 字符串
        if (root == NULL) {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_CURRENT_CARTYPE : JSON parse error! ");
            #else
            #endif
            return -1;
        }

        cJSON *stCurrentCarTypeJson = cJSON_GetObjectItem(root, "current_type");
        if( stCurrentCarTypeJson == NULL )
        {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_CURRENT_CARTYPE : stCurrentCarTypeJson  is NULL ");
            #else
            #endif
                        ret = -1 ;
            break;
        }


        char *stCurrentCarType = NULL;

        if (cJSON_IsString( stCurrentCarTypeJson )) 
        {
            stCurrentCarType = stCurrentCarTypeJson->valuestring;
           
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_CURRENT_CARTYPE : current_type: %s , length : %d\n",  stCurrentCarType , strlen(stCurrentCarType));
            #else
            #endif
        }
        else
        {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_CURRENT_CARTYPE : current_type is not string type...");
            #else
            #endif
                        ret = -1 ;
            break;
        }

        //赋值
        this->m_objTpmCtrl.m_u16Cmd = TPM_TEST_CURRENT_CARTYPE ; 
        std::memcpy( this->m_objTpmCtrl.m_u8Response , stCurrentCarType ,strlen(stCurrentCarType));
        sendLen += strlen(stCurrentCarType);

        this->m_objTpmCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objTpmCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objTpmCtrl;

    }
    break;
    case MC_SET_TPM_MODEIFY_CARTYPE: //修改车型
    {// {"set_car_result : 当前车型 / FAIL "}  
       std::memset(this->m_objTpmCtrl.m_u8Response, 0, sizeof(this->m_objTpmCtrl.m_u8Response));

        u16Size = sizeof( reqTPMCtrl );

        //解析Json串
        root = cJSON_Parse(str.c_str()); // 解析 JSON 字符串
        if (root == NULL) {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_MODEIFY_CARTYPE : JSON parse error! ");
            #else
            #endif
            return -1;
        }

        cJSON *stSetCarTypeJson = cJSON_GetObjectItem(root, "set_car_result");
                if( stSetCarTypeJson == NULL )
        {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_MODEIFY_CARTYPE : stSetCarTypeJson  is NULL ");
            #else
            #endif
                        ret = -1 ;
            break;
        }


        char *stSetCarType = NULL;
        if (cJSON_IsString( stSetCarTypeJson )) 
        {
            stSetCarType = stSetCarTypeJson->valuestring;
           
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_MODEIFY_CARTYPE : set_car_result: %s , length : %d\n",  stSetCarType , strlen(stSetCarType));
            #else
            #endif
        }
        else
        {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_MODEIFY_CARTYPE : set_car_result is not string type...");
            #else
            #endif
                        ret = -1 ;
            break;
        }

        //赋值
        this->m_objTpmCtrl.m_u16Cmd = TPM_TEST_MODEIFY_CARTYPE ; 
        std::memcpy( this->m_objTpmCtrl.m_u8Response , stSetCarType ,strlen(stSetCarType));
        sendLen += strlen(stSetCarType);

        this->m_objTpmCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objTpmCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objTpmCtrl;

    }
    break;
    case MC_SET_TPM_NORMAL_SHOW: //正常显示
    {// {"show_set_result : ok "}  
       std::memset(this->m_objTpmCtrl.m_u8Response, 0, sizeof(this->m_objTpmCtrl.m_u8Response));
       
        u16Size = sizeof( reqTPMCtrl );

        //解析Json串
        root = cJSON_Parse(str.c_str()); // 解析 JSON 字符串
        if (root == NULL) {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_NORMAL_SHOW : JSON parse error! ");
            #else
            #endif
            return -1;
        }

        cJSON *stResultJson = cJSON_GetObjectItem(root, "show_set_result");
        if( stResultJson == NULL )
        {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_NORMAL_SHOW : stResultJson  is NULL ");
            #else
            #endif
            ret = -1 ;
            break;
        }

        char *stResult = NULL;
        if (cJSON_IsString(stResultJson)) 
        {
            stResult = stResultJson->valuestring;
           
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_NORMAL_SHOW : show_set_result: %s , length : %d\n",  stResult , strlen(stResult));
            #else
            #endif
        }
        else
        {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_NORMAL_SHOW : show_set_result is not string type...");
            #else
            #endif

            ret = -1 ;
            break;
        }

        //赋值
        this->m_objTpmCtrl.m_u16Cmd = TPM_TEST_NORMAL_SHOW ; 
        std::memcpy( this->m_objTpmCtrl.m_u8Response , stResult ,strlen(stResult));
        sendLen += strlen(stResult);

        this->m_objTpmCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objTpmCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objTpmCtrl;
    }
    break;
    case MC_SET_TPM_GET_SN: //获取SN
    {// {"ec_sn : 互联sn "}  
       std::memset(this->m_objTpmCtrl.m_u8Response, 0, sizeof(this->m_objTpmCtrl.m_u8Response));
       
        u16Size = sizeof( reqTPMCtrl );

        //解析Json串
        root = cJSON_Parse(str.c_str()); // 解析 JSON 字符串
        if (root == NULL) {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_GET_SN : JSON parse error! ");
            #else
            #endif
            return -1;
        }

        cJSON *stEcSnJson = cJSON_GetObjectItem(root, "ec_sn");
        if( stEcSnJson == NULL )
        {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_GET_SN : stEcSnJson  is NULL ");
            #else
            #endif
            ret = -1 ;
            break;
        }

        char *stEcSn = NULL;
        if (cJSON_IsString(stEcSnJson)) 
        {
            stEcSn = stEcSnJson->valuestring;
           
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_GET_SN : ec_sn: %s , length : %d\n",  stEcSn , strlen(stEcSn));
            #else
            #endif
        }
        else
        {
            #ifdef T5COMPILE
                LOGD("MC_SET_TPM_GET_SN : ec_sn is not string type...");
            #else
            #endif

            ret = -1 ;
            break;
        }

        //赋值
        this->m_objTpmCtrl.m_u16Cmd = TPM_TEST_GET_SN ; 
        std::memcpy( this->m_objTpmCtrl.m_u8Response , stEcSn ,strlen(stEcSn));
        sendLen += strlen(stEcSn);

        this->m_objTpmCtrl.m_u16Crc = mxc_crc16s_calc((unsigned char*)&this->m_objTpmCtrl , u16Size - 2 );
        pCmd = (unsigned char*)&this->m_objTpmCtrl;
    }
    break;

    default:
        ret = -1;
    break;
    }

    if( NULL != root ){
        cJSON_Delete(root);
    }

    if(pCmd){
        //发送指令
         McuParserBase::SendData(pCmd,  u16Size);

        std::string stHex = this->convertToHexString( pCmd , u16Size); 
        #ifdef T5COMPILE
            LOGD("SendData : %s ", stHex.c_str());
        #else
            std::cout << "SendData : " << stHex << std::endl;
        #endif
    }

    return ret;
}

void McuParser::Proc()
{
    while (m_running)
    {
        if (m_notifyItems.size() > 0)
        {
            NotifyItem *item;

            pthread_mutex_lock(&m_mutex2);
            item = *m_notifyItems.begin();
            m_notifyItems.pop_front();
            pthread_mutex_unlock(&m_mutex2);

     
            if(m_notifier != NULL && item->type == 1)
            {
                m_notifier(item->code, item->data1, item->data2);
                delete item;
            }else if(m_notifier_str != NULL && item->type == 2){
                m_notifier_str(item->code,item->data1,item->str);
                delete item;
            }

        }
        else
            usleep(1000 * 50);
    }
}

/**
 * 线程执行函数（Parser）
 */
void *McuParser::ThreadProc(void *arg)
{
    McuParser *parser = (McuParser *)arg;
    parser->Proc();

    return (void *)NULL;
}



std::string McuParser::convertToHexString(unsigned char* data, size_t length) {
    std::stringstream ss;
    for (size_t i = 0; i < length; ++i) {
        // 将每个字节转换为两位的十六进制表示
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
        if (i < length - 1) 
            ss << " ";
    }
    std::cout << ss.str() << std::endl;
    return ss.str();
}

std::vector<std::string> McuParser::splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream stream(str);
    std::string token;

    while (std::getline(stream, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}


#define KEY 0x5a

/* MXC PROTOCOL CRC16 CALCULATE */
static const uint16_t ccitt_table [256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};
uint16_t mxc_crc16s_calc(const uint8_t *q, uint32_t len)
{
    uint16_t crcValue = 0;

    while (len > 0)
    {
        crcValue =  ((crcValue & 0xff) << 8) ^ ccitt_table [(((crcValue & 0xFF00) >> 8) ^ *q) & 0xff];
        len--;
        q++;
    }
    // printf("%04X , value=%d \n",crcValue , crcValue);

    return crcValue;
}