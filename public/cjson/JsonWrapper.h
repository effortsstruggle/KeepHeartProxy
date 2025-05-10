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

    // 获取子对象（链式调用）
    JsonWrapper get(const std::string& key) const;
    
    // 获取数组元素（链式调用）
    JsonWrapper getArrayItem(size_t index) const;


    // 修改 JSON 数据
    JsonWrapper& set(const std::string& key, const std::string& value);
    JsonWrapper& set(const std::string& key, int value);
    JsonWrapper& set(const std::string& key, double value);
    JsonWrapper& set(const std::string& key, bool value);
    JsonWrapper& pushArrayItem(const std::string& arrayKey, const JsonWrapper& value);

    // 类型安全的数据获取方法
    std::string getString( const std::string &key ) const;
    int getInt( const std::string &key ) const;
    double getDouble( const std::string &key ) const;
    bool getBool( const std::string &key ) const;
    // JsonWrapper getObject() const;
    // JsonWrapper getArray() const;

    // 检查当前节点类型
    bool isString() const;
    bool isInt() const;
    bool isDouble() const;
    bool isBool() const;
    bool isObject() const;
    bool isArray() const;

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
