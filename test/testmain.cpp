


#include <stdio.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <QThread>

#include "../dsrc/SkyProxy.h"
#include "../dsrc/qt_bll_proxy.h"
#include "../public/utils/AsyncSerialCall.h"
#include "yps_bt.h"
#include <android/log.h>
#include "CmdDef.h"
using namespace std;
static  int theVmid = -1;
static int bd37033id = -1;
static int glb_errInfo = -1;
SkyProxy * skyproxy  = NULL;

class SkyNotify :public QObject{
       Q_OBJECT
public:
    SkyNotify(){}
    void  ConnectNotify(){

         
        bool ok = connect(skyproxy, &SkyProxy::skyPluginsNotify, this, &SkyNotify::onskyNotify,Qt::DirectConnection);
       // printf("connected %s \n",ok?" success ":" falied");
         ok = connect(this, &SkyNotify::testsignal, this, &SkyNotify::onskyNotify);

    }

    void emitTestSignal()
    {
        emit testsignal(1,1,"hahaahahahhaha");
    }
signals:
     void testsignal(int pid,int code,QString data);
public slots:
    void onskyNotify(int pid,int code,QString data){
  
         printf("onskyNotify pid = %d ,code = %d,data = %s \n",pid,code,data.toStdString().c_str());
         if(pid == theVmid){
            onVoiceModule(code);
         }
         else if( pid == bd37033id ){

         }
       
    }
     void onskyPluginsNotifyEx(int pid,int code,int p1,QString data){
         printf("onskyNotify pid = %d ,code = %d,data = %s",pid,code,data.toStdString().c_str());
     }

     void onBd37033(int code){ 

     }
    //语音模块的消息响应
    void onVoiceModule(int code){ 
        printf("---------------------------------\n");
        switch (code)
        {
        case NT_PLAY_MUSIC:
             printf("播放音乐 \n");
             skyproxy->pluginsExecuteEx(theVmid,CMD_HAO_DE,"8");
            break;
        case NT_PRE_MUSIC:
            printf("上一首 \n");
             skyproxy->pluginsExecuteEx(theVmid,CMD_HAO_DE,"8");
            break;
        case NT_NEXT_MUSIC:
            printf("下一首 \n");
             skyproxy->pluginsExecuteEx(theVmid,CMD_HAO_DE,"8");
            break;
        case NT_MUSIC_PAUSE:
            printf("暂停; \n");
             skyproxy->pluginsExecuteEx(theVmid,CMD_HAO_MUSIC_STOPED,"8");
            break;
        case NT_OPEN_AC:
            printf("打开空调; \n");
             skyproxy->pluginsExecuteEx(theVmid,CMD_HAO_AC_OPENED,"8");
            break;
        case NT_CLOSE_AC:
            printf("关闭空调; \n");
             skyproxy->pluginsExecuteEx(theVmid,CMD_HAO_AC_CLOSED,"8");
            break;
        case NT_TEMP_RISEN:
            printf("调高温度; \n");
             skyproxy->pluginsExecuteEx(theVmid,CMD_HAO_TEMP_RISEN,"8");
            break;
        case NT_TEMP_DROPED:
            printf("降低温度; \n");
             skyproxy->pluginsExecuteEx(theVmid,CMD_HAO_TEMP_DROPED,"8");
            break;
        case NT_OPEN_HOME:
            printf("打开主页; \n");
             skyproxy->pluginsExecuteEx(theVmid,CMD_HAO_OPEND_HOME,"8");
            break;
        case NT_OPEN_360:
            printf("打开360; \n");
             skyproxy->pluginsExecuteEx(theVmid,CMD_HAO_OPEND_360,"8");
            break;
        case NT_OPEN_SETTING:
            printf("打开设置\n");
             skyproxy->pluginsExecuteEx(theVmid,CMD_HAO_OPEND_SETTING,"8");
            break;
        case NT_OPEN_AUTO_DRIVE:
            printf("打开自动驾驶\n");
             skyproxy->pluginsExecuteEx(theVmid,CMD_HAO_AUTO_DRIVE,"8");
            break;
        case NT_OPEN_OPEND_EC:
            printf("打开互联\n");
             skyproxy->pluginsExecuteEx(theVmid,CMD_HAO_OPEND_EC,"8");
            break;
        case NT_OPEN_APPCENTER:
            printf("打开应用中心\n");
            skyproxy->pluginsExecuteEx(theVmid,CMD_HAO_OPEND_APPCENTER,"8");
            break;
        case NT_VOICE_MODE_QUIT:
             printf("播放再见 \n");
             sleep(5);
             //skyproxy->pluginsExecuteEx(theVmid,CMD_QUIT_SEY_BYE,"8");
             break;
        case NT_SOUND_WAKE_UP:
             printf("语音模块被唤醒\n");
             skyproxy->pluginsExecuteEx(bd37033id,SE_SET_SOURCE,"8");
             skyproxy->pluginsExecuteEx(theVmid,CMD_WAKE_UP_SAY_HI,"8");
            break;
        case NT_SOUND_PLAY_OVER:
             printf("语音播放完毕 \n");
            //  skyproxy->pluginsExecuteEx(bd37033id,SE_SET_SOURCE,"0");
            break;
        default:
            break;
        }

    }

};
 SkyNotify * skynotify = nullptr;
void bd37033_test(int argc, char *argv[])
{

    printf("Usage:\n");
    printf("\n\
        [-m  0/1]     0:set mute,1:set unmute  \n\
        [-v  0~95]    set volume  \n\
        [-c  0/1/2/3/]  select cahnnel \n");

    static int bd37033id = skyproxy->getPluginsID("bd37033");

    printf("bd37033 plugin id :%d \n",bd37033id);

    const char *optstring = "m:v:c:"; // 有三个选项-abc，其中c选项后有两个冒号，表示后面可选参数
 
    int ret = 0;
    sleep(1);
    int res  =  skyproxy->pluginsExecute(bd37033id,5,"");
    printf ("bd37033 is ready %d  \n",res );

    // while ((o = getopt(argc, argv, optstring)) != -1) {
    //     switch (o) {
    //         case 'm':
    //             printf("opt is m, oprarg is: %s\n", optarg);
    //             skyproxy->pluginsExecute(bd37033id,SE_SET_MUTE,optarg);
    //             break;
    //         case 'v':
    //             printf("opt is v, oprarg is: %s\n", optarg);
    //             skyproxy->pluginsExecute(bd37033id,SE_SET_VOLUME,optarg);
    //             break;
    //         case 'c':
    //             skyproxy->pluginsExecute(bd37033id,SE_SET_SOURCE,optarg);
    //             printf("opt is c, oprarg is: %s\n", optarg);
               
    //             break;
    //         case '?':
    //             printf("unknow oprarg \n");
    //             usage(); // 提示使用说明
    //             break;
    //         default:
    //              printf("opt is , oprarg is: %c\n", o);
    //     }
    // }
}
void pt2313_test(int argc, char *argv[]){

    int theid = skyproxy->getPluginsID("pt2313");

    printf("testPT2313 plugin id :%d \n",theid);

    const char *optstring = "m:v:c:"; // 有三个选项-abc，其中c选项后有两个冒号，表示后面可选参数
    
    sleep(1);
    int res  =  skyproxy->pluginsExecute(theid,5,"");
    printf ("PT2313 is ready %d \n",res );
    res  =  skyproxy->pluginsExecute(theid,4,"1");
    
    printf ("PT2313 set source  %d \n",res );
   // skyproxy->pluginsExecute(theid,PluginSe_pt2313::SE_SET_MUTE,optarg);

}
void fm_si475x_test(int argc, char *argv[]){

    int theid = skyproxy->getPluginsID("fm_4754c");

    printf("fm_4754c plugin id :%d \n",theid);

    skyproxy->pluginsExecute(theid,1,optarg);

}

int test_bll()
{
     Qt_bll_proxy * bll_proxy = Qt_bll_proxy::getInstance();

     if(!bll_proxy){
        printf("load bll_proxy failed \n");
        return -1;
    }
    printf("qt_dateset_get_object \n");
    bll_proxy->qt_dateset_get_object(DI_BT_BLUETOOTH_PHONE_BOOK_LIST);

    return 0;
 
}

int asc_fuc( FuncAndParam const & fp){

    printf("yes asc_fuc execute ,p1 = %f,p2 = %f,p3 = %d,p4 = %d \n",fp.p1,fp.p2,(int)fp.p3,(int)fp.p4);
}
int AsyncSerialCall_test()
{
    static AsyncSerialCall asc ;

    asc.SetLogTag( "qqqqq");
    asc.AddFunctionToQueue(FuncAndParam(asc_fuc,1,2,3,4));
}

int VoiceModule_test()
{
    int theSeid = skyproxy->getPluginsID("bd37033");

    printf("bd37033 plugin id :%d \n",theSeid);

    skyproxy->pluginsExecuteEx(theSeid,SE_SET_SOURCE,"8");
    skyproxy->pluginsExecuteEx(theSeid,SE_SET_MUTE,"0");
    skyproxy->pluginsExecuteEx(theSeid,SE_SET_VOLUME,"94");
   // skyproxy->pluginsExecuteEx(theSeid,SE_SET_VOLUME,"80");

    theVmid = skyproxy->getPluginsID("voiceModule");

    sleep(5);
    skyproxy->pluginsExecuteEx(theVmid,CMD_CHECK_SUCCESS,"");
    sleep(3);

    skyproxy->pluginsExecuteEx(theVmid,CMD_CHECK_FAILD,"");
    sleep(3);

    skyproxy->pluginsExecuteEx(theVmid,CMD_GET_VER,"8");
    sleep(3);

    skyproxy->pluginsExecuteEx(theVmid,CMD_NO_SUPPORT,"8");
    sleep(3);

    skyproxy->pluginsExecuteEx(theVmid,CMD_NO_PLAYING,"8");
    sleep(3);

    skyproxy->pluginsExecuteEx(theVmid,CMD_OPEN_AC_FIRST,"8");
    sleep(3);
    


}
void glb_errInfo_test()
{

    glb_errInfo = skyproxy->getPluginsID("errPraser_glb");
    printf("errPraser_glb id : %d \n",glb_errInfo);

    skyproxy->pluginsExecuteEx(glb_errInfo,0,"");
}
int main(int argc, char *argv[]) {
    skyproxy = SkyProxy::getInstance();
   
    if(!skyproxy){
        printf("load skyproxy failed \n");
        return -1;
    }

    skynotify = new SkyNotify();

    skynotify->ConnectNotify();
  //  skynotify->emitTestSignal();

//     bd37033_test(argc, argv);
//     pt2313_test(argc, argv);
//  //   AsyncSerialCall_test();
//     fm_si475x_test( argc, argv);
    VoiceModule_test();

    // glb_errInfo_test();
    sleep(200000000);
    printf("-------------------end---------------------------\n");
    return 0;
}
 #include "testmain.moc"