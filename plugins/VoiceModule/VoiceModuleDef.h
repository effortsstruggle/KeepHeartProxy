#pragma once
//定义枚举值
enum VOICE_COMMAND {

     //需要执行的消息
    CMD_CHECK_SUCCESS,      // 上位机校验成功，语音播报
    CMD_CHECK_FAILD,        // 上位机校验失败
    CMD_GET_VER,            // 上位机获取语音模组版本信息
    CMD_NO_SUPPORT,         // 暂不支持此功能
    CMD_WAKE_UP_SAY_HI,     // 主动唤醒 say hello
    CMD_QUIT_SEY_BYE,       // 主动退出 说再见
    CMD_SAY_HELLO,          // 唤醒回复你好
    CMD_NO_PLAYING,         // 暂未播放媒体
    CMD_OPEN_AC_FIRST,      // 请先打开空调后再喊我吧
    CMD_HAO_DE,             // 好的
    CMD_HAO_MUSIC_STOPED,   // 好的，已暂停播放
    CMD_HAO_AC_OPENED,      // 好的，空调已打开
    CMD_HAO_AC_CLOSED,      // 好的，空调已关闭
    CMD_HAO_TEMP_RISEN,     // 好的，温度已调高 temperature has risen
    CMD_HAO_TEMP_DROPED,    // 好的，温度已调低 Temperature has dropped
    CMD_HAO_OPEND_HOME,     // 好的，已打开首页
    CMD_HAO_OPEND_360,      // 好的，已打开360
    CMD_HAO_OPEND_SETTING,  // 好的，已打开设置
    CMD_HAO_AUTO_DRIVE,     // 好的，已进入自动驾驶
    CMD_HAO_OPEND_EC,       // 好的，已打开手机互联
    CMD_HAO_OPEND_APPCENTER,// 好的，已打开应用中心


    CMD_MAX,

    // 通知消息
    NT_VER,
    NT_PLAY_MUSIC,             // 播放音乐
    NT_PRE_MUSIC,              // 上一首
    NT_NEXT_MUSIC,             // 下一首
    NT_MUSIC_PAUSE,            // 暂停
    NT_OPEN_AC,                // 打开空调
    NT_CLOSE_AC,               // 关闭空调
    NT_TEMP_RISEN,             // 调高温度
    NT_TEMP_DROPED,            // 降低温度
    NT_OPEN_HOME,              // 打开首页
    NT_OPEN_360,               // 打开360
    NT_OPEN_SETTING,           // 打开设置
    NT_OPEN_AUTO_DRIVE,        // 进入自动驾驶
    NT_OPEN_OPEND_EC,          // 打开手机互联
    NT_OPEN_APPCENTER,         // 打开应用中心
    NT_VOICE_MODE_QUIT,        // 语音模块退下 这个消息不要响应

    NT_SOUND_PLAY_OVER,        // 语音播报结束(你说，在呢，来了，播放结束不需要切换通道，故不通知)
    NT_SAY_BYE_OVER,           // 退下语音播放完毕 
    NT_SOUND_WAKE_UP,          // 语音模块被唤醒
    NT_MAX,

};




