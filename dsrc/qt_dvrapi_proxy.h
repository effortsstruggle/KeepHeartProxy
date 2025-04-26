
#ifndef QT_DVRAPI_PROXY_H
#define QT_DVRAPI_PROXY_H

#include <QObject>
#include <QRect>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
class Q_DECL_EXPORT Qt_dvrapi_proxy : public QObject
{
    Q_OBJECT
public:
    static Qt_dvrapi_proxy *getInstance();

public:
    /*
    @brief 连接dvr服务，成功后(返回值为0)，才能调用其他接口
    */
    Q_INVOKABLE int connect();
    /**
     * @brief 显示摄像头
     **/
    Q_INVOKABLE int show_camera(int cid1, int show1, QRect rect1, int cid2, int show2, QRect rect2);
    /**
     * @brief 隐藏摄像头画面的显示
     * */
    Q_INVOKABLE int hide_camera();

    /**
     * @brief 抓拍照片
     *
     * @param cid
     *  @param number 拍照张数
     * @return int32_t
     */
    Q_INVOKABLE int snapshot(int cid,int number);

    /**
     * @brief 锁定某个摄像头
     * 调用本函数后，这个摄像头当前记录以及此前此后的视频文件将会被加锁，无法自动删除
     * @param cid
     * @return int32_t
     */
    Q_INVOKABLE int lock_camera(int cid);

    /**
     * @brief 解锁定某个摄像头
     * 调用本函数后，这个摄像头会立即解除锁定状态
     * @param cid
     * @return int32_t
     */
    Q_INVOKABLE int unlock_camera(int cid);

    /**
     * @brief 格式化SD卡
     *
     * @return int32_t
     */
    Q_INVOKABLE int format_sdcard();

      /**
     * @brief 系统恢复出厂设置
     * 
     * @return int32_t 
     */
    Q_INVOKABLE int reset_system();

    /**
     * @brief 获取当前SD的状态
     *
     * @return DVR_SDCARD_STATUS
     */
    Q_INVOKABLE int get_sdcard_status();

    /**
     * @brief 获取当前dvr服务运行状态
     *
     * @return true 正在运行
     * @return false 未运行
     */
    Q_INVOKABLE bool get_dvr_status();

    /**
     * @brief 摄像头在屏幕上是否显示
     *
     * @param cid
     * @return true 显示
     * @return false 隐藏
     */
    Q_INVOKABLE bool get_show_status(int cid);

    /**
     * @brief 获取行车记录仪保存视频的状态
     *
     * @param cid
     * @return true 正在保存
     * @return false 停止保存
     */
    Q_INVOKABLE bool get_record_status(int cid);

    /**
     * @brief 获取行车记录仪mic的状态
     *
     * @param cid
     * @return true 正在录音
     * @return false 停止录音
     */
    Q_INVOKABLE bool get_mic_status(int cid);

    /**
     * @brief 获取行车记录仪某个摄像头锁定状态
     *
     * @param cid
     * @return true 锁定
     * @return false 非锁定
     */
    Q_INVOKABLE bool get_lock_status(int cid);

    /**
     * @brief 获取文件数量
     *
     * @param cid 摄像头
     * @param file_type 文件类型
     * @return int32_t
     * 异步返回，Notify。
     */
    Q_INVOKABLE int get_file_number(int cid, int file_type);

    // /**
    //  * @brief 获取文件列表
    //  *
    //  * @param cid 摄像头
    //  * @param file_type 文件类型
    //  * @param from 从第几个开始
    //  * @param count 获取多少个
    //  * @return int32_t
    //  */
    // Q_INVOKABLE int get_file_list(int cid, int file_type, int from, int count);

    /**
     * @brief 删除某个文件
     * 注意，删除动作由dvr服务执行，删除时，会同步删除缩略图，同步修改本地文件列表
     * @param pNode 文件节点
     * @return int32_t
     */
    Q_INVOKABLE int delete_file(int cid, int type, int seqno, QString file_name);

    /**
     * @brief 获取某个文件的全路径
     *
     * @param pNode
     * @param oPath 输出的全路径
     * @param len   oPath 的长度
     * @return int32_t
     */
    Q_INVOKABLE QString get_file_path(int cid, int type, int seqno, QString file_name);

    /**
     * @brief 获取某个文件的缩略图全路径
     *
     * @param pNode
     * @param oPath 输出的全路径
     * @param len   oPath 的长度
     * @return const char*
     */
    Q_INVOKABLE QString get_thumbnail_path(int cid, int type, int seqno, QString file_name);

    /**
     * @brief 锁定某个文件
     * 在浏览时，锁定某个视频文件。注意图片不支持锁定和解锁，因为图片本身就是锁定状态
     *
     * @param pNode
     * @return int32_t
     */
    Q_INVOKABLE int lock_file(int cid, int type, int seqno, QString file_name);

    /**
     * @brief 解锁定某个文件
     * 解锁定某个文件意义在于文件会被挪到循环存储区
     *
     * @param pNode
     * @return int32_t
     */
    Q_INVOKABLE int unlock_file(int cid, int type, int seqno, QString file_name);


    // 下面是关于文件的两个常用的同步调用
    /**
     * @brief 获取文件数量
     *
     * @param cid 摄像头
     * @param file_type 文件类型
     * @return int32_t 返回文件数量
     */
    Q_INVOKABLE int syncget_file_number(int cid, int file_type);

    /**
     * @brief 获取文件列表
     *
     * @param cid 摄像头
     * @param file_type 文件类型
     * @param from 从第几个开始
     * @param count 获取多少个
     * @param outNodeArray 输出的文件列表
     * @return int32_t 返回文件数量
     */
    Q_INVOKABLE QString syncget_file_list(int cid, int file_type, int from, int count);


    /**
     * 播放某个文件
     */
    Q_INVOKABLE int mplay_play(QString file_path, int x, int y, int w, int h);
    /**
     * 暂停播放器
     */
    Q_INVOKABLE int mplay_pause();
    /**
     * 继续播放
     */
    Q_INVOKABLE int mplay_resume();
    /**
     * 停止播放
     */
    Q_INVOKABLE int mplay_stop();

    /**
     * @brief 跳转到指定时间
     *
     * @param timeInms 时间，单位ms
     * @return int -1 失败YPSMP_ERR，0 成功YPSMP_OK
     */
    Q_INVOKABLE int mplay_seek(int timeInms);

    /**
     * @brief 获取mplayer的版本信息
     *
     */
    Q_INVOKABLE QString mplay_get_version();

    /**
     * @brief 获取mplayerApi的版本信息
     *
     */
    Q_INVOKABLE QString mplayApi_get_version();


     /**
     * @brief 获取dvrapp的版本信息
     * 
     */
    Q_INVOKABLE QString get_dvrapp_version();

    /**
     * @brief 获取dvrApi的版本信息
     * 
     */
    Q_INVOKABLE QString get_dvrapi_version();   


    /**
     * @brief 配置某个摄像头
     *
     * @param cid 摄像头ID
     * @param config 摄像头相关配置。
     * @return int32_t see DVR_OK
     */
    Q_INVOKABLE int set_config(int cid, QString config);

    /**
     * @brief 获取某个摄像头的配置信息
     *
     * @param cid 摄像头ID
     * @param config 摄像头相关配置。
     * @return int32_t see DVR_OK
     */
    Q_INVOKABLE QString get_config(int cid);

    /**
     * @brief 停止录像，preview不受影响
     * 注意，录像功能默认开启，只有调用过dvrapi_stop_record才需要重新调用dvrapi_start_record来恢复录像功能
     *
     * @return int32_t
     */
    Q_INVOKABLE int start_record();
    /**
     * @brief 停止录像功能
     * 只是停止录像，preview不受影响
     *
     * @return int32_t
     */
    Q_INVOKABLE int stop_record();

    /**
     * @brief 运行dvr服务程序
     *
     * @return int32_t
     */
    Q_INVOKABLE int start_dvr();

    /**
     * @brief 停止dvr服务程序，dvr服务程序会退出
     *
     * @return int32_t
     */
    Q_INVOKABLE int stop_dvr();

    /**
     * @brief 下电通知dvr
     *
     * @return int32_t
     */
    Q_INVOKABLE int acc_onoff(bool on);

    /**
     * @brief 启动对应的mplayer服务
     *     
     */
    Q_INVOKABLE void start_mplay_service();


     /**
     * @brief 获取当前配置项：保存录像
     *
     * @return bool true保存录像，false不保存录像
     */
    Q_INVOKABLE bool config_get_autorecord();

    /**
     * @brief 保存当前配置项：保存录像
     *
     * @param autorecord true保存录像，false不保存
     * @return int32_t see DVR_OK
     */
    Q_INVOKABLE int config_set_autorecord(bool autorecord);


signals:
    // 信号：可以直接在QML中访问信号

    void messageNotify(int notify, QString msg);
    void fileList(int cid, int ftype, int number);
    void FileNode(int cid, int ftype, int seqno, QString name);
    void mplayPlayStatus(int type, int value);
public slots:
    // 槽函数：可以直接在QML中访问public槽函数

    void cppSlotB(const QString &str, int value); // 一个带参数槽函数

private:
    Qt_dvrapi_proxy();
    // 禁止外部拷贝构造
    Qt_dvrapi_proxy(const Qt_dvrapi_proxy &signal);
    // 禁止外部赋值操作
    const Qt_dvrapi_proxy &operator=(const Qt_dvrapi_proxy &signal);
};

#endif
