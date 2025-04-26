模拟当前开发环境设置的结构
APP:目录下表示最上层应用调用
maindll: 表示qbll ，用来加载插件和插件的管理，提供从c++到qt的转换
plugindll:表示自定义的插件

每个插件重写PluginInterface.h类，并实现其中的ID，和名字获取的函数以及函数操作函数
每个ID不能重复，ID在0～100 之间，
并实现下边两个函数，供maindll加载插件库，
extern "C" PluginInterface* createPlugin() 
extern "C" void destroyPlugin(PluginInterface* plugin) 

上层app调用是传入插件ID，操作的枚举值，数据，插件依据枚举值和数据进行操作

如此之后maindll，基本不用修改，有新的功能添加实现插件即可，可以解放qbll维护人员出来




更简单的方式是直接使用函数，  int doSomething(int key,void * data) ，传入key和数据