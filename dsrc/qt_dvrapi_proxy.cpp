#include "qt_dvrapi_proxy.h"
#include "qt_bll_proxy.h"
#include "mplayerApi.h"
// #include "DVR/include/dvrApi.h"
// #include "DVR/include/dvrFiles.h"
// #include "DVR/include/dvrTypes.h"
#include "dvrApi.h"
#include "dvrFiles.h"
#include "dvrTypes.h"

#include <iostream>
#include <QStringList>
#include <QRect>
#include <QGuiApplication>
// #include "carbitUtil/properties.h"
// #include "carbitUtil/log.h"s
#include "properties.h"
#include "log.h"

#include "bllLog.h"
#include <sys/stat.h>

#include <condition_variable>
#include <unistd.h>
#include <queue>
#include <thread>
#include "skylog.h"
static uint8_t seeking = 0;  //搜台标记


static std::mutex dvrmtx;  // 创建一个互斥锁

static std::condition_variable dvrcv;


struct funcClass;
typedef int (*P_ASYNC)(funcClass&);

struct funcClass
{
   P_ASYNC pFunc;
   int p1;    // 每个函数的参数
   int p2;    // 在这里用作延迟时间
    int p3;
    int p4;
    int p5;
};
static std::queue<funcClass>  pFuncArr ; //函数数组
static Qt_dvrapi_proxy *instance =nullptr;

int static asynchronous_get_file_number(funcClass& ft){

    LOGD("SKY asynchronous_get_file_number: %d,%d",ft.p1,ft.p2);
    int res = dvrapi_get_file_number((DVR_CID)ft.p1, (DVR_FILE_TYPE)ft.p2);
    instance->messageNotify(102,QString::number(res));
}

int static asyn_acc_onoff(funcClass& ft){

    LOGD("SKY asyn_acc_onoff: %d,%d",ft.p1,ft.p2);
    try {
         int res = dvrapi_acc_onoff(ft.p1 == 1);
        // 可能抛出异常的代码
    
    } catch (const std::system_error& e) {
       LOGD("Caught a system_error: %s" , e.what()) ;
        // 处理错误
    } catch (const std::exception& e) {
         LOGD("Caught an exception:  %s" , e.what()) ;
        
        // 处理其他标准库异常
    } catch (...) {
         LOGD("Caught an unknown exception: ") ;
        // 处理未知异常
    }

   
   
}



int  asynchronous_get_file_list(funcClass& ft)
{
    SKYLOGD("SKY  asynchronous_get_file_list %d,%d,%d,%d",(DVR_CID)ft.p1, (DVR_FILE_TYPE)ft.p2, ft.p3, ft.p4);

    int res = instance->get_sdcard_status();
    QString str;
    switch(res){
        case  DVR_SDCARD_NONE :
            str = "// sd卡状态未知";
            break;
        case DVR_SDCARD_NO:
            str = "// 没有sd卡";
            break;
        case DVR_SDCARD_FULL:
            str = "// sd卡不可覆盖区满";
            break;
        case DVR_SDCARD_WARNING:
          str = "// sd卡不可覆盖区不足1G";
          break;
        case DVR_SDCARD_ENOUGH:
          str = "// sd卡不可覆盖区空间充足";
          break;
        case DVR_SDCARD_ABNORMAL:
          str = "// 有SD卡，但是卡不可用(未格式化，卡坏了，无法读写，等错误)";
          break;
    }

    SKYLOGD("sdcard_status :%s",str.toStdString().c_str());
    if(res == DVR_SDCARD_NONE || res == DVR_SDCARD_NO || res == DVR_SDCARD_ABNORMAL)
    {
        instance->messageNotify(101,"");
        return -1;
    }
        
    QJsonArray outNodeArray;
    if(ft.p4 <=0 || ft.p4 > 10000)
    {
        instance->messageNotify(101,"");
        return -1;
    }

    DVR_FILE_NODE *nodeArray = new DVR_FILE_NODE[ft.p4];
    struct stat statbuf;
    int rtn = dvrapi_syncget_file_list((DVR_CID)ft.p1, (DVR_FILE_TYPE)ft.p2, ft.p3, ft.p4, nodeArray);
    for (int i = 0; i < rtn; i++)
    {
        QJsonObject item;
        QString name;
        name.append((char *)nodeArray[i].name);
        item["name"] = name;

        QString filePath = instance->get_file_path((DVR_CID)ft.p1, (DVR_FILE_TYPE)ft.p2, (int)nodeArray[i].seqno, name);
        QString thumbPath;
        if(nodeArray[i].type == 1){
                thumbPath = filePath;
                filePath =  filePath.right(filePath.size()-1);
                //SKYLOGD("thumbPath  %s",resfilePath,resthumbPath,filePath.toStdString().c_str());

                
        }
        else{
            thumbPath = instance->get_thumbnail_path((DVR_CID)ft.p1, (DVR_FILE_TYPE)ft.p2, (int)nodeArray[i].seqno, name);
        }

        int resfilePath = stat(filePath.toStdString().c_str(), &statbuf);
        int resthumbPath = stat(thumbPath.toStdString().c_str(), &statbuf);

        if(resfilePath == 0 && resthumbPath == 0){
            
            item["filePath"] = filePath;
            item["thumbPath"] = "file:/" + thumbPath;
        }
        else{
            SKYLOGD("ERROR %d,%d corruption DVR file  %s",resfilePath,resthumbPath,filePath.toStdString().c_str());

            item["filePath"] = "";
            item["thumbPath"] = "";
        }
         
    
        outNodeArray.append(item);
    }
    delete[] nodeArray;
    QJsonDocument jsonDoc(outNodeArray);
    SKYLOGD(" file_list %s",jsonDoc.toJson());
    instance->messageNotify(101,jsonDoc.toJson());
    
}
// 异步执行函数
void static AsynchronousExecution()
{   
    int tempcount = 0;
    while (true)
    {
        funcClass ft;
        {
            std::unique_lock<std::mutex> lock(dvrmtx);
            dvrcv.wait(lock, [] { return !pFuncArr.empty(); });
            ft = pFuncArr.front();
            pFuncArr.pop();
        }
    
        SKYLOGD("SKY  AsynchronousExecution runthread : %d",tempcount++);
        ft.pFunc(ft);
    }
    

}
// 实现单例模式，并实现qt_dvrapi_proxy.h中的函数
Qt_dvrapi_proxy *Qt_dvrapi_proxy::getInstance()
{
    
    if(!instance)
        instance = new Qt_dvrapi_proxy;

    return instance;
}
static void mp_status_notify(MPLAYER_STATUS istatus)
{
    emit Qt_dvrapi_proxy::getInstance()->mplayPlayStatus(0, istatus);
}

static void mp_position_notify(uint32_t iposition)
{
    emit Qt_dvrapi_proxy::getInstance()->mplayPlayStatus(1, iposition);
}
static void mp_duration_notify(uint32_t iduraiton)
{
    emit Qt_dvrapi_proxy::getInstance()->mplayPlayStatus(2, iduraiton);
}

static void dvrapi_callback_onNotify(DVR_NOTIFY notify, uint32_t len, const void *content)
{
    Q_UNUSED(len);
    QString str_status;
    if (DVR_NOTIFY_SNAPSHOT_STATUS == notify || DVR_NOTIFY_LOCKCAMERA_STATUS == notify)
    {
        int status = *((int *)content);
        str_status.append(QString::number(status));
    }

    if (DVR_NOTIFY_ACCOFF_DONE == notify)
    {
        SKYLOGD("dvrapi_callback_onNotify --> DVR_NOTIFY_ACCOFF_DONE");
        yps_bll_mcu_exec_command(YPS_COMMAND_MCU_DVR_STATE, 1, 0);
    }
    emit Qt_dvrapi_proxy::getInstance()->messageNotify(notify, str_status);
}
static void dvrapi_callback_onFileList(DVR_CID cid, DVR_FILE_TYPE ftype, const DVR_FILE_LIST_REQ_ACK *result)
{
    Q_UNUSED(cid);
    Q_UNUSED(ftype);
    Q_UNUSED(result);
}
static void dvrapi_callback_onFileNode(DVR_CID cid, DVR_FILE_TYPE ftype, const DVR_FILE_NODE *node)
{
    Q_UNUSED(cid);
    Q_UNUSED(ftype);
    Q_UNUSED(node);
}

void dvrapi_callback_onFileDelete(DVR_CID cid, DVR_FILE_TYPE ftype, const DVR_FILE_NODE *node)
{
    Q_UNUSED(cid);
    Q_UNUSED(ftype);
    Q_UNUSED(node);
}

void dvrapi_callback_onFileAdded(DVR_CID cid, DVR_FILE_TYPE ftype, const DVR_FILE_NODE *node)
{
    Q_UNUSED(cid);
    Q_UNUSED(ftype);
    Q_UNUSED(node);
}
static int init_mplayer()
{
    MPLAYER_NOTIFY_CB mpCB;
    mpCB.pStatusCB = mp_status_notify;
    mpCB.pPositionCB = mp_position_notify;
    mpCB.pDurationCB = mp_duration_notify;
    
    QVariantList qVarList;

    int ret = YPSMP_OK;
    //释放上一个
    ret = mplayer_destory();
    if (ret == YPSMP_ERR)
    {
        
        qVarList << "mplayer_destory failed!!!" << ret;
        Qt_bll_proxy::getInstance()->log(qVarList);
        qVarList.clear();
    }


    qVarList << "Qt_dvrapi_proxy::init_mplayer  sleep (begin) " ;
    Qt_bll_proxy::getInstance()->log(qVarList);
    qVarList.clear();

    sleep(1);

    qVarList << "Qt_dvrapi_proxy::init_mplayer sleep (end) " ;
    Qt_bll_proxy::getInstance()->log(qVarList);
    qVarList.clear();



    ret = mplayer_create(&mpCB);
    if (ret == YPSMP_ERR)
    {
        qVarList << "mplayer_create failed!!!" << ret;
        Qt_bll_proxy::getInstance()->log(qVarList);
        qVarList.clear();
    }

    return ret;
}
Qt_dvrapi_proxy::Qt_dvrapi_proxy()
{
    setLogLevel(LL_DEBUG);
    static std::thread t(AsynchronousExecution);

    t.detach();
}

int Qt_dvrapi_proxy::connect()
{
    static DVR_CALLBACK dvrapi_callback = {
        .onNotify = dvrapi_callback_onNotify,
        .onFileList = dvrapi_callback_onFileList,
        .onFileNode = dvrapi_callback_onFileNode,
        .onFileDelete = dvrapi_callback_onFileDelete,
        .onFileAdded = dvrapi_callback_onFileAdded,
    };
    return dvrapi_connect(&dvrapi_callback);
  
}
/**
 * @brief 显示摄像头
 **/
int Qt_dvrapi_proxy::show_camera(int cid1, int show1, QRect rect1, int cid2, int show2, QRect rect2)
{

    QVariantList qVarList;
    qVarList << "show_camera DVR_CID_FRONT " << show1 << " show_camera DVR_CID_BACK " << show2;
 //   Qt_bll_proxy::getInstance()->log(qVarList);

    Q_UNUSED(cid1);
    Q_UNUSED(cid2);
    DVR_SHOW_CONFIG show_config;
    memset(&show_config, 0, sizeof(show_config));
    show_config.config[DVR_CID_FRONT].enable = show1 == 1;
    show_config.config[DVR_CID_BACK].enable = show2 == 1;
    show_config.config[DVR_CID_LEFT].enable = false;
    show_config.config[DVR_CID_RIGHT].enable = false;

    show_config.config[DVR_CID_FRONT].pos.x = rect1.x();
    show_config.config[DVR_CID_FRONT].pos.y = rect1.y();
    show_config.config[DVR_CID_FRONT].pos.w = rect1.width();
    show_config.config[DVR_CID_FRONT].pos.h = rect1.height();
    
    
    show_config.config[DVR_CID_BACK].pos.x = rect2.x();
    show_config.config[DVR_CID_BACK].pos.y = rect2.y();
    show_config.config[DVR_CID_BACK].pos.w = rect2.width();
    show_config.config[DVR_CID_BACK].pos.h = rect2.height();
   // show_config.config[DVR_CID_BACK].zorder = 30;
    
    return dvrapi_show_camera(&show_config);
}
/**
 * @brief 隐藏摄像头画面的显示
 * */
int Qt_dvrapi_proxy::hide_camera()
{
    dvrapi_hide_camera((DVR_CID)0);
    dvrapi_hide_camera((DVR_CID)1);
    return 1;
}

/**
 * @brief 抓拍照片
 *
 * @param cid
 * @return int32_t
 */
int Qt_dvrapi_proxy::snapshot(int cid, int number)
{
    return dvrapi_snapshot((DVR_CID)cid, number);
}

/**
 * @brief 锁定某个摄像头
 * 调用本函数后，这个摄像头当前记录以及此前此后的视频文件将会被加锁，无法自动删除
 * @param cid
 * @return int32_t
 */
int Qt_dvrapi_proxy::lock_camera(int cid)
{
    return dvrapi_lock_camera((DVR_CID)cid);
}

/**
 * @brief 解锁定某个摄像头
 * 调用本函数后，这个摄像头会立即解除锁定状态
 * @param cid
 * @return int32_t
 */
int Qt_dvrapi_proxy::unlock_camera(int cid)
{
    return dvrapi_unlock_camera((DVR_CID)cid);
}

/**
 * @brief 格式化SD卡
 *
 * @return int32_t
 */
int Qt_dvrapi_proxy::format_sdcard()
{
    return dvrapi_format_sdcard();
}

/**
 * @brief 系统恢复出厂设置
 *
 * @return int32_t
 */
int Qt_dvrapi_proxy::reset_system()
{
    return dvrapi_reset_system();
}

/**
 * @brief 获取当前SD的状态
 *
 * @return DVR_SDCARD_STATUS
 */
int Qt_dvrapi_proxy::get_sdcard_status()
{
    return dvrapi_get_sdcard_status();
}

/**
 * @brief 获取当前dvr服务运行状态
 *
 * @return true 正在运行
 * @return false 未运行
 */
bool Qt_dvrapi_proxy::get_dvr_status()
{
    return dvrapi_get_dvr_status();
}

/**
 * @brief 摄像头在屏幕上是否显示
 *
 * @param cid
 * @return true 显示
 * @return false 隐藏
 */
bool Qt_dvrapi_proxy::get_show_status(int cid)
{
    return dvrapi_get_show_status((DVR_CID)cid);
}

/**
 * @brief 获取行车记录仪保存视频的状态
 *
 * @param cid
 * @return true 正在保存
 * @return false 停止保存
 */
bool Qt_dvrapi_proxy::get_record_status(int cid)
{
    return dvrapi_get_record_status((DVR_CID)cid);
}

/**
 * @brief 获取行车记录仪mic的状态
 *
 * @param cid
 * @return true 正在录音
 * @return false 停止录音
 */
bool Qt_dvrapi_proxy::get_mic_status(int cid)
{
    return dvrapi_get_mic_status((DVR_CID)cid);
}

/**
 * @brief 获取行车记录仪某个摄像头锁定状态
 *
 * @param cid
 * @return true 锁定
 * @return false 非锁定
 */
bool Qt_dvrapi_proxy::get_lock_status(int cid)
{
    return dvrapi_get_lock_status((DVR_CID)cid);
}

/**
 * @brief 获取文件数量
 *
 * @param cid 摄像头
 * @param file_type 文件类型
 * @return int32_t
 * 异步返回，Notify。
 */
int Qt_dvrapi_proxy::get_file_number(int cid, int file_type)
{
    return 0; //dvrapi_get_file_number((DVR_CID)cid, (DVR_FILE_TYPE)file_type);
}

// /**
//  * @brief 获取文件列表
//  *
//  * @param cid 摄像头
//  * @param file_type 文件类型
//  * @param from 从第几个开始
//  * @param count 获取多少个
//  * @return int32_t
//  */
// int Qt_dvrapi_proxy::get_file_list(int cid, int file_type, int from, int count)
// {
//     return dvrapi_get_file_list((DVR_CID)cid, (DVR_FILE_TYPE)file_type, from, count);
// }

/**
 * @brief 删除某个文件
 * 注意，删除动作由dvr服务执行，删除时，会同步删除缩略图，同步修改本地文件列表
 * @param pNode 文件节点
 * @return int32_t
 */
int Qt_dvrapi_proxy::delete_file(int cid, int type, int seqno, QString file_name)
{
    DVR_FILE_NODE node;
    node.cid = (DVR_CID)cid;
    node.type = (DVR_FILE_TYPE)type;
    node.seqno = seqno;
    strncpy((char *)node.name, file_name.toStdString().c_str(), 32);

    return dvrapi_delete_file(&node);
}

/**
 * @brief 获取某个文件的全路径
 *
 * @param pNode
 * @param oPath 输出的全路径
 * @param len   oPath 的长度
 * @return int32_t
 */
QString Qt_dvrapi_proxy::get_file_path(int cid, int type, int seqno, QString file_name)
{

    DVR_FILE_NODE node;
    node.cid = (DVR_CID)cid;
    node.type = (DVR_FILE_TYPE)type;
    node.seqno = seqno;
    strncpy((char *)node.name, file_name.toStdString().c_str(), 32);

    char path[256] = {0};
    dvrapi_get_file_path(&node, path, 256);
    QString str;
    str.append(path);
    return str;
}

/**
 * @brief 获取某个文件的缩略图全路径
 *
 * @param pNode
 * @param oPath 输出的全路径
 * @param len   oPath 的长度
 * @return const char*
 */
QString Qt_dvrapi_proxy::get_thumbnail_path(int cid, int type, int seqno, QString file_name)
{
    DVR_FILE_NODE node;
    node.cid = (DVR_CID)cid;
    node.type = (DVR_FILE_TYPE)type;
    node.seqno = seqno;
    strncpy((char *)node.name, file_name.toStdString().c_str(), 32);

    char path[256] = {0};
    dvrapi_get_thumbnail_path(&node, path, 256);
    QString str;
    str.append(path);
    return str;
}

/**
 * @brief 锁定某个文件
 * 在浏览时，锁定某个视频文件。注意图片不支持锁定和解锁，因为图片本身就是锁定状态
 *
 * @param pNode
 * @return int32_t
 */
int Qt_dvrapi_proxy::lock_file(int cid, int type, int seqno, QString file_name)
{
    DVR_FILE_NODE node;
    node.cid = (DVR_CID)cid;
    node.type = (DVR_FILE_TYPE)type;
    node.seqno = seqno;
    strncpy((char *)node.name, file_name.toStdString().c_str(), 32);

    return dvrapi_lock_file(&node);
}

/**
 * @brief 解锁定某个文件
 * 解锁定某个文件意义在于文件会被挪到循环存储区
 *
 * @param pNode
 * @return int32_t
 */
int Qt_dvrapi_proxy::unlock_file(int cid, int type, int seqno, QString file_name)
{
    DVR_FILE_NODE node;
    node.cid = (DVR_CID)cid;
    node.type = (DVR_FILE_TYPE)type;
    node.seqno = seqno;
    strncpy((char *)node.name, file_name.toStdString().c_str(), 32);

    return dvrapi_unlock_file(&node);
}

/**
 * @brief 获取文件数量
 *
 * @param cid 摄像头
 * @param file_type 文件类型
 * @return int32_t 返回文件数量
 */
int Qt_dvrapi_proxy::syncget_file_number(int cid, int file_type)
{
    
    std::unique_lock <std::mutex> lck(dvrmtx);
    pFuncArr.push(funcClass{asynchronous_get_file_number,cid,file_type});
    dvrcv.notify_all();

    return  0 ;//dvrapi_syncget_file_number((DVR_CID)cid, (DVR_FILE_TYPE)file_type);
}

/**
 * @brief 获取文件信息列表
 *
 * @param cid 摄像头
 * @param file_type 文件类型
 * @param from 从第几个开始
 * @param count 获取多少个
 * @param outNodeArray 输出的文件列表
 * @return int32_t 返回文件数量
 */
static int counter = 0;
QString Qt_dvrapi_proxy::syncget_file_list(int cid, int file_type, int from, int count)
{
    std::unique_lock <std::mutex> lck(dvrmtx);
    pFuncArr.push(funcClass{asynchronous_get_file_list,cid,file_type,from,count});
    dvrcv.notify_all();
    return "";
}

void Qt_dvrapi_proxy::cppSlotB(const QString &str, int value)
{
    std::cout << "Qt_dvrapi_proxy::cppSlotB" << str.toStdString() << value;
    // emit this->cppSignalB(str,value);
}

/**
 * 播放某个文件
 */
int Qt_dvrapi_proxy::mplay_play(QString file_path, int x, int y, int w, int h)
{
    QVariantList qVarList;

    qVarList << "Qt_dvrapi_proxy::mplay_play init_mplayer (begin) " ;
    Qt_bll_proxy::getInstance()->log(qVarList);
    qVarList.clear();

    init_mplayer();

    qVarList << "Qt_dvrapi_proxy::mplay_play init_mplayer (end) " ;
    Qt_bll_proxy::getInstance()->log(qVarList);
    qVarList.clear();


    int rtn = mplayer_play(file_path.toStdString().c_str(), x, y, w, h);
    
    qVarList << "Qt_dvrapi_proxy::mplay_play" << file_path << x << y << w << h << " res = " << rtn;
    Qt_bll_proxy::getInstance()->log(qVarList);
    qVarList.clear();


    return rtn;
}
/**
 * 暂停播放器
 */
int Qt_dvrapi_proxy::mplay_pause()
{
    return mplayer_pause();
}
/**
 * 继续播放
 */
int Qt_dvrapi_proxy::mplay_resume()
{
    return mplayer_resume();
}
/**
 * 停止播放
 */
int Qt_dvrapi_proxy::mplay_stop()
{
    int rtn = mplayer_stop();
    return rtn;
}
/**
 * @brief 跳转到指定时间
 *
 * @param timeInms 时间，单位ms
 * @return int -1 失败YPSMP_ERR，0 成功YPSMP_OK
 */
int Qt_dvrapi_proxy::mplay_seek(int timeInms)
{
    return mplayer_seek(timeInms);
}

/**
 * @brief 获取mplayer的版本信息
 *
 */
QString Qt_dvrapi_proxy::mplay_get_version()
{
    QString str;

    const char *rtn = mplayer_get_version();
    if (rtn)
        str.append(rtn);
    return str;
}

/**
 * @brief 获取mplayerApi的版本信息
 *
 */
QString Qt_dvrapi_proxy::mplayApi_get_version()
{
    QString str;

    const char *rtn = mplayerApi_get_version();
    if (rtn)
        str.append(rtn);
    return str;
}

/**
 * @brief 获取dvrapp的版本信息
 *
 */
QString Qt_dvrapi_proxy::get_dvrapp_version()
{
    QString str;
    const char *rtn = dvrapp_get_version();
    if (rtn)
        str.append(rtn);
    return str;
}

/**
 * @brief 配置某个摄像头
 *
 * @param cid 摄像头ID
 * @param config 摄像头相关配置。
 * @return int32_t see DVR_OK
 */
int Qt_dvrapi_proxy::set_config(int cid, QString jsonData)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData.toUtf8());
    QJsonObject jsonObj = jsonDoc.object();

    DVR_CAM_CONFIG conf;
    if (dvrapi_config_get((DVR_CID)cid, &conf) == DVR_OK){
        conf.enable = jsonObj.contains("enable") ? jsonObj["enable"].toBool() :  conf.enable;
        conf.width = jsonObj.contains("width") ? jsonObj["width"].toInt() : conf.width;
        conf.height = jsonObj.contains("height") ? jsonObj["height"].toInt() : conf.height;
        conf.saveseconds = jsonObj.contains("saveseconds") ? jsonObj["saveseconds"].toInt() : conf.saveseconds;
    }
   

    int res = dvrapi_config_set((DVR_CID)cid, &conf);
    dvrapi_config_validate((DVR_CID)cid);
    return res;
}

/**
 * @brief 获取某个摄像头的配置信息
 *
 * @param cid 摄像头ID
 * @param config 摄像头相关配置。
 * @return int32_t see DVR_OK
 */
QString Qt_dvrapi_proxy::get_config(int cid)
{

    QString rtn;
    DVR_CAM_CONFIG conf;
    if (dvrapi_config_get((DVR_CID)cid, &conf) == DVR_OK)
    {
        QJsonObject jsonObj;
        jsonObj["enable"] = conf.enable;
        jsonObj["width"] = conf.width;
        jsonObj["height"] = conf.height;
        jsonObj["fps"] = conf.fps;
       /// jsonObj["bitrate"] = conf.bitrate;
        jsonObj["saveseconds"] = conf.saveseconds;
      //  jsonObj["encoder"] = conf.encoder;
      //  jsonObj["picsize"] = conf.picsize;
        // jsonObj["osdpos.x"] = conf.osdpos.x;
        // jsonObj["osdpos.y"] = conf.osdpos.y;
        // jsonObj["osdpos.w"] = conf.osdpos.w;
        // jsonObj["osdpos.h"] = conf.osdpos.h;
        // jsonObj["osdcolor"] = (int)conf.osdcolor;
        // jsonObj["osdbcolor"] = (int)conf.osdbcolor;
        QJsonDocument jsonDoc(jsonObj);
        rtn = jsonDoc.toJson();
    }

   // qDebug() << "dvrapi_config_get" << rtn;
    return rtn;
}

/**
 * @brief 获取dvrApi的版本信息
 *
 */
QString Qt_dvrapi_proxy::get_dvrapi_version()
{
    QString str;
    const char *rtn = dvrapi_get_version();
    if (rtn)
        str.append(rtn);
    return str;
}

int Qt_dvrapi_proxy::start_record()
{
    return dvrapi_start_record();
}

int Qt_dvrapi_proxy::stop_record()
{
    return dvrapi_stop_record();
}
int Qt_dvrapi_proxy::start_dvr()
{
    return dvrapi_start_dvr();
}

int Qt_dvrapi_proxy::stop_dvr()
{
    return (int)dvrapi_stop_dvr();
}
int Qt_dvrapi_proxy::acc_onoff(bool on)
{
    std::unique_lock <std::mutex> lck(dvrmtx);
    pFuncArr.push(funcClass{asyn_acc_onoff,on?1:0});
    dvrcv.notify_all();
    return 0;
}


void Qt_dvrapi_proxy::start_mplay_service()
{
    // 获取当前应用程序所在的路径
    QString strPath = QCoreApplication::applicationDirPath();
    // 判断当前路径是不是以/apps开头
    if (strPath.startsWith("/apps"))
    {
        service_start("new_mplayer");
    }
    else if (strPath.startsWith("/backup"))
    {
        service_start("mplayer");
    }
    
}
bool Qt_dvrapi_proxy::config_get_autorecord()
{
    return dvrapi_config_get_autorecord();
}   
int Qt_dvrapi_proxy::config_set_autorecord(bool status)
{
    return dvrapi_config_set_autorecord(status);
}