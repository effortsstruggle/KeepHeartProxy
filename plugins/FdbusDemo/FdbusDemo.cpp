#include "FdbusDemo.h"

// #include "logdef.h"
#include "MsgDef.h"

#include <../proto/MsgDef.pb.h>
static fdbusDemo * instance = new fdbusDemo ;
extern "C" PluginInterface* createPlugin() {

    instance->ConnectServer("SSSSSS");
    return instance;
}


fdbusDemo::fdbusDemo():PluginFdbusAdapter(FDBUSDEMO_NAME)
{
    SetAsyncSplitNum(ASYNC_TEST9);
}


// int main(int argc, const char** argv) {


//     instance->executeEx(1,"adasda ",3,4,5,7);

//   //  LOGD("CCCCCC GetPluginName ;%s",instance->GetPluginName().c_str());
//     return 0;
// }