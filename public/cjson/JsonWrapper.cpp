#include "JsonWrapper.h"
#include <stdexcept>
#include <iostream>

// 默认构造函数：创建空对象
JsonWrapper::JsonWrapper() 
    : m_pRoot( ::cJSON_CreateObject() ) 
{
    // std::cout << "JsonWrapper::JsonWrapper(1) " << std::endl; 
}

JsonWrapper::JsonWrapper( const std::string &stJson )
    : m_pRoot( nullptr )
{
    this->parse( stJson );
}

JsonWrapper::JsonWrapper(JsonWrapper&& other) noexcept : 
    m_pRoot (other.m_pRoot) {
    other.m_pRoot = nullptr;
}

JsonWrapper& JsonWrapper::operator=(JsonWrapper&& other) noexcept {
    if (this != &other) {
        this->clear();
        this->m_pRoot = other.m_pRoot;
        other.m_pRoot = nullptr;
    }
    return *this;
}



JsonWrapper::~JsonWrapper()
{
    this->clear();
}


// 解析 JSON 字符串
void JsonWrapper::parse(const std::string& stJson) {
    this->clear();
    this->m_pRoot = ::cJSON_Parse( stJson.c_str());
    if (nullptr == this->m_pRoot ) {
        throw std::runtime_error( ::cJSON_GetErrorPtr() );
    }
}



// 获取子节点（链式调用）
JsonWrapper JsonWrapper::getObject(const std::string& key) const {
    validateNode("get() on non-object");
    // cJSON* child = ::cJSON_GetObjectItemCaseSensitive( this->m_pRoot , key.c_str());
    cJSON* child = ::cJSON_GetObjectItem( this->m_pRoot , key.c_str() ) ;
    if ( nullptr == child) {
        throw std::runtime_error("Key not found: " + key);
    }
    return JsonWrapper(child);
}

// 获取数组元素（链式调用）
JsonWrapper JsonWrapper::getArrayItem(size_t index) const {
    this->validateNode("getArrayItem() on non-array");
    cJSON* item = ::cJSON_GetArrayItem( this->m_pRoot , static_cast<int>(index));
    if ( nullptr == item) {
        throw std::out_of_range("Array index out of range");
    }
    return JsonWrapper(item);
}



std::string JsonWrapper::getString(const std::string& key) const {
    this->validateNode("get() on non-object");
    cJSON* child = ::cJSON_GetObjectItem( this->m_pRoot , key.c_str() ) ;
    if ( nullptr == child) {
        throw std::runtime_error("Key not found: " + key);
    }
    return child->valuestring;
}

bool JsonWrapper::getBool(const std::string& key) const {
    this->validateNode("get() on non-object");
    cJSON* child = ::cJSON_GetObjectItem( this->m_pRoot , key.c_str() ) ;
    if ( nullptr == child) {
        throw std::runtime_error("Key not found: " + key);
    }
    return child->valueint ;
}


int JsonWrapper::getInt(const std::string& key) const {
    this->validateNode("get() on non-object");
    cJSON* child = ::cJSON_GetObjectItem( this->m_pRoot , key.c_str() ) ;
    if ( nullptr == child) {
        throw std::runtime_error("Key not found: " + key);
    }
    return child->valueint ;
}

double JsonWrapper::getDouble(const std::string& key) const {
    this->validateNode("get() on non-object");
    cJSON* child = ::cJSON_GetObjectItem( this->m_pRoot , key.c_str() ) ;
    if ( nullptr == child) {
        throw std::runtime_error("Key not found: " + key);
    }
    return child->valuedouble ;
}


// add JSON 数据
JsonWrapper& JsonWrapper::addItemToObject(const std::string& key, const std::string& value){
    this->validateNode("addItemToObject() on non-object");
    ::cJSON_AddItemToObject( this->m_pRoot , key.c_str() , ::cJSON_CreateString(value.c_str()) );
    return *this;
}

JsonWrapper& JsonWrapper::addItemToObject(const std::string& key, int value){
    this->validateNode("addItemToObject() on non-object");
    ::cJSON_AddItemToObject( this->m_pRoot , key.c_str() , ::cJSON_CreateNumber(value) );
    return *this;
}
JsonWrapper& JsonWrapper::addItemToObject(const std::string& key, double value){
    this->validateNode("addItemToObject() on non-object");
    ::cJSON_AddItemToObject( this->m_pRoot , key.c_str() , ::cJSON_CreateNumber(value) );
    return *this;
}
    

JsonWrapper& JsonWrapper::replaceItemToObject(const std::string& key, const std::string& value){
    this->validateNode("replaceItemToObject() on non-object");
    ::cJSON_ReplaceItemInObject( this->m_pRoot , key.c_str() , ::cJSON_CreateString(value.c_str()) );
    return *this;
}

JsonWrapper& JsonWrapper::replaceItemToObject(const std::string& key, int value){
    this->validateNode("replaceItemToObject() on non-object");
    ::cJSON_ReplaceItemInObject( this->m_pRoot , key.c_str() , ::cJSON_CreateNumber(value) );
    return *this;
}

JsonWrapper& JsonWrapper::replaceItemToObject(const std::string& key, double value){
    this->validateNode("replaceItemToObject() on non-object");
    ::cJSON_ReplaceItemInObject( this->m_pRoot , key.c_str() , ::cJSON_CreateNumber(value) );
    return *this;
}



// 修改 JSON 数据：添加数组元素
JsonWrapper& JsonWrapper::pushArrayItem(const std::string& arrayKey, const JsonWrapper& value) 
{
    this->validateNode("pushArrayItem() on non-object");
    cJSON* array = ::cJSON_GetObjectItem( this->m_pRoot , arrayKey.c_str());
    if (!array || ! ::cJSON_IsArray(array)) {
        throw std::runtime_error("Key is not an array: " + arrayKey);
    }
    ::cJSON_AddItemToArray(array, value.m_pRoot );
    return *this;
}

// 序列化为字符串
std::string JsonWrapper::toString() const {
    char* buffer = cJSON_Print( this->m_pRoot );
    std::string result(buffer);
    ::free(buffer);
    return result;
}

// 私有构造函数：从 cJSON 节点创建
JsonWrapper::JsonWrapper(cJSON* node) 
    :  m_pRoot(node) 
{

}

// 验证节点有效性
void JsonWrapper::validateNode(const std::string& context) const 
{
    if ( nullptr == this->m_pRoot ) {
        throw std::runtime_error(context + ": Invalid JSON node");
    }
}

// 清理资源
void JsonWrapper::clear() 
{
    if ( nullptr != this->m_pRoot ) {
        ::cJSON_Delete( this->m_pRoot );
        this->m_pRoot = nullptr;
    }
}

// 类型检查方法
bool JsonWrapper::isString( std::string key ) const 
{
    validateNode("isString() on non-object");
    cJSON* child = ::cJSON_GetObjectItem( this->m_pRoot , key.c_str() ) ;
    if ( nullptr == child) {
        throw std::runtime_error("Key not found: " + key);
    }
    return ::cJSON_IsString( child );
}

bool JsonWrapper::isInt( std::string key ) const 
{
    validateNode("isInt() on non-object");
    cJSON* child = ::cJSON_GetObjectItem( this->m_pRoot , key.c_str() ) ;
    if ( nullptr == child) {
        throw std::runtime_error("Key not found: " + key);
    }
    return ::cJSON_IsNumber( child ); 
}

bool JsonWrapper::isDouble( std::string key ) const 
{
    validateNode("isDouble() on non-object");
    cJSON* child = ::cJSON_GetObjectItem( this->m_pRoot , key.c_str() ) ;
    if ( nullptr == child) {
        throw std::runtime_error("Key not found: " + key);
    }
    return ::cJSON_IsNumber( child ); 
}

bool JsonWrapper::isBool( std::string key ) const 
{
    validateNode("isBool() on non-object");
    cJSON* child = ::cJSON_GetObjectItem( this->m_pRoot , key.c_str() ) ;
    if ( nullptr == child) {
        throw std::runtime_error("Key not found: " + key);
    }
    return ::cJSON_IsNumber( child ); 
}

bool JsonWrapper::isArray( std::string key ) const 
{
    validateNode("isArray() on non-object");
    cJSON* child = ::cJSON_GetObjectItem( this->m_pRoot , key.c_str() ) ;
    if ( nullptr == child) {
        throw std::runtime_error("Key not found: " + key);
    }
    return ::cJSON_IsArray( child ); 
}

bool JsonWrapper::isObject( std::string key ) const 
{
    validateNode("isObject() on non-object");
    cJSON* child = ::cJSON_GetObjectItem( this->m_pRoot , key.c_str() ) ;
    if ( nullptr == child) {
        throw std::runtime_error("Key not found: " + key);
    }
    return ::cJSON_IsObject( child ); 
}





