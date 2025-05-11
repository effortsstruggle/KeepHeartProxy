#pragma once 
/**
 *@brief: Json类
 *@version 
 *@since 
 *@date: 2024-10-16 10:17:34
*/
#include "cJSON.h"
#include <string>

class JsonWrapper
{
public:

    // 构造函数：创建空 JSON 对象
    explicit JsonWrapper();

    explicit JsonWrapper( const std::string &stJson );
    
    // 移动构造函数
    JsonWrapper(JsonWrapper&& other) noexcept;
    
    // 移动赋值运算符
    JsonWrapper& operator=(JsonWrapper&& other) noexcept;

    ~JsonWrapper();



     // 禁止拷贝（使用移动语义）
     JsonWrapper(const JsonWrapper&) = delete;
     JsonWrapper& operator=(const JsonWrapper&) = delete;
   

    // 解析 JSON 字符串
    void parse(const std::string& stJson);

    void reset() ;

    // 获取子对象（链式调用）
    JsonWrapper getObject(const std::string& key) const;
    // 获取数组元素（链式调用）
    JsonWrapper getArrayItem(size_t index) const;
    std::string getString(const std::string& key) const;
    bool getBool(const std::string& key) const;
    int getInt(const std::string& key) const;
    double getDouble(const std::string& key) const;

    // add JSON 数据
    JsonWrapper& addItemToObject(const std::string& key, const std::string& value);
    JsonWrapper& addItemToObject(const std::string& key, int value);
    JsonWrapper& addItemToObject(const std::string& key, double value);

    JsonWrapper& replaceItemToObject(const std::string& key, const std::string& value);
    JsonWrapper& replaceItemToObject(const std::string& key, int value);
    JsonWrapper& replaceItemToObject(const std::string& key, double value);


    JsonWrapper& pushArrayItem(const std::string& arrayKey, const JsonWrapper& value);


    // 检查当前节点类型
    bool isString( std::string key ) const;
    bool isInt( std::string key ) const;
    bool isDouble( std::string key ) const;
    bool isBool( std::string key ) const;
    bool isArray( std::string key ) const;
    bool isObject( std::string key ) const;
    
    // 序列化为字符串
    std::string toString() const;

    void clear() ;

private:
    // 私有构造函数：用于内部创建子节点
    explicit JsonWrapper(cJSON* node);
    // 辅助函数：检查节点有效性
    void validateNode(const std::string& context = "") const;

private:
    cJSON *m_pRoot ; 
};
