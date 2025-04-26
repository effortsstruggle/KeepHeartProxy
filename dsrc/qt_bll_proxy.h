#ifndef QT_BLL_PROXY_H
#define QT_BLL_PROXY_H
#include <QObject>
#include <QVariantList>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/uinput.h>
#include "qt_bll_proxy_global.h"
#include "yps_mcu.h"
#include "yps_bll.h"

#define gettid() syscall(SYS_gettid)

class Q_DECL_EXPORT Qt_bll_proxy : public QObject
{
    Q_OBJECT
public:
    static Qt_bll_proxy *getInstance();
    
    Q_PROPERTY(int animation_play_status READ getAnimationStatus NOTIFY animation_status)
    Q_PROPERTY(int enable_key_event READ getEnableKeyEvent WRITE setEnableKeyEvent NOTIFY enable_key_event_changed)
public:
    Q_INVOKABLE int qt_dateset_get_int(int index);
    Q_INVOKABLE int qt_dateset_get_uint(int index);
    Q_INVOKABLE float qt_dateset_get_float(int index);
    Q_INVOKABLE QString qt_dateset_get_string(int index);
    Q_INVOKABLE QString qt_dateset_get_object(int index);

    Q_INVOKABLE QString getCurrentThreadId();
    Q_INVOKABLE int getAnimationStatus();

    Q_INVOKABLE int qt_set_property(QString name, QString value);
    Q_INVOKABLE QString qt_get_property(QString name, QString def_value);
    Q_INVOKABLE QString qt_system(QString cmd,QString paras);
    Q_INVOKABLE int ec_exec_command(int type, int code, int value);

    /**
     *@description: 在原有接口上进行修改，尽量保持原接口不动
     *@author: sky
     *@param type[in] 枚举值
     *@param code[in] 数据
     *@param value[in] 使用IEAI时，目前用来标记读写 0:是写 ; 1:是读
     *@return 
     *@date: 2024-07-03 17:32:21
    */
    Q_INVOKABLE int ieai_mcu_exec_command(int type, int code, int value);
    Q_INVOKABLE int mcu_exec_command(int type, int code, int value);
    Q_INVOKABLE int mcu_exec_json_command(int type,QString value);
    Q_INVOKABLE int bt_exec_command(int index, int type, int code, QString value);
    Q_INVOKABLE int tpms_exec_command(int type, int code, QString value);
    Q_INVOKABLE QString bll_get_version(void);

    Q_INVOKABLE int init(QString config);

    Q_INVOKABLE void send_touch_event(int x, int y, int type,int slot);
    Q_INVOKABLE void log(QVariantList msgs);
    Q_INVOKABLE void ec_start();

    Q_INVOKABLE int getEnableKeyEvent(); 
    Q_INVOKABLE void setEnableKeyEvent(int enable);

    void on_page_key_handler(int code, int status);
    /**
     *@description: 接受mcu的按键输入，当有按键按下就会接受到消息，当前按键位的值为1，可同时传输多个按键值                    
                    当按键抬起时 传入的值为0，不做处理，所以现在将一次按下处理为一个完整的按键消息。不处理组合键
     *@author: sky
     *@param data1[in] 原始按键消息值
     *@return 
     *@date: 2024-07-05 17:00:14
    */
    void ieai_sample_key_proxy(  int data1);

    void on_yps_bll_bt_notify(int index, int type, int code, QString value);
    void on_yps_mcu_notify_handler(yps_notify_mcu_type_t type, unsigned int code, unsigned int value);
    void on_yps_animation_status(int status);
    void on_yps_bll_notify_handler(yps_notify_bll_type_t type, unsigned int code, unsigned int value);
    void on_yps_ec_notify_handel(int type, int code, int value);
    /**
     *@description: 使用IEAI时 由mcu回调函数调用发送消息到qml中
     *@author: sky
     *@param code[in] 输入参数1
     *@param data1[in] 输入参数2
     *@param data2[in] 输出参数1
     *@return 
     *@date: 2024-07-03 17:51:50
    */
    void on_ieai_mcu_notify_handler(int code, int data1, int data2);
signals:
    // 信号：可以直接在QML中访问信号
    void data_update_notify();                      // 一个无参信号
    void cppSignalB(const QString &str, int value); // 一个带参数信号
    void mcu_notify(int type, unsigned int code, unsigned int value);
    void ieai_mcu_notify(int type, unsigned int code, unsigned int value);
    void page_key_event(int status, int code);
    void bt_notify(int index, int type, int code, QString value);
    void animation_status(int status);
    void bll_notify(int type, int code, int value);
    void ec_notify(int type, int code, int value);
    void enable_key_event_changed(int enable);

public slots:
    // 槽函数：可以直接在QML中访问public槽函数
    void cppSlotA();                              // 一个无参槽函数
    void cppSlotB(const QString &str, int value); // 一个带参数槽函数
    

private:
    static void *data_update_thread(void *p);
    QObject *m_focus_object;
    int m_animation_status;
    int m_uinput_fd;
    int m_enable_key_event;
    Qt_bll_proxy();
    // 禁止外部拷贝构造
    Qt_bll_proxy(const Qt_bll_proxy &signal);
    // 禁止外部赋值操作
    const Qt_bll_proxy &operator=(const Qt_bll_proxy &signal);
    ~Qt_bll_proxy()
    {
        if (m_uinput_fd > 0)
        {
            ioctl(m_uinput_fd, UI_DEV_DESTROY);

            close(m_uinput_fd);
        }
    }
};

#endif // QT_BLL_PROXY_H
