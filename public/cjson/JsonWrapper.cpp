#include "JsonWrapper.h"
#include <stdexcept>

// 默认构造函数：创建空对象
JsonWrapper::JsonWrapper() 
    : m_pRoot( ::cJSON_CreateObject() ) 
{
    
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
JsonWrapper JsonWrapper::get(const std::string& key) const {
    validateNode("get() on non-object");
    cJSON* child = ::cJSON_GetObjectItemCaseSensitive( this->m_pRoot , key.c_str());
    if ( nullptr == child) {
        throw std::runtime_error("Key not found: " + key);
    }
    return JsonWrapper(child);
}

// 获取数组元素（链式调用）
JsonWrapper JsonWrapper::getArrayItem(size_t index) const {
    this->validateNode("getArrayItem() on non-array");
    cJSON* item = ::cJSON_GetArrayItem( this->m_pRoot , static_cast<int>(index));
    if (!item) {
        throw std::out_of_range("Array index out of range");
    }
    return JsonWrapper(item);
}

// 修改 JSON 数据：设置值
JsonWrapper& JsonWrapper::set(const std::string& key, const std::string& value) {
    this->validateNode("set() on non-object");
    ::cJSON_ReplaceItemInObjectCaseSensitive( this->m_pRoot , key.c_str(), ::cJSON_CreateString(value.c_str()));
    return *this;
}

JsonWrapper& JsonWrapper::set(const std::string& key, int value) {
    this->validateNode("set() on non-object");
    ::cJSON_ReplaceItemInObjectCaseSensitive( this->m_pRoot  , key.c_str(), ::cJSON_CreateNumber(value));
    return *this;
}

// 修改 JSON 数据：添加数组元素
JsonWrapper& JsonWrapper::pushArrayItem(const std::string& arrayKey, const JsonWrapper& value) {
    this->validateNode("pushArrayItem() on non-object");
    cJSON* array = ::cJSON_GetObjectItemCaseSensitive( this->m_pRoot , arrayKey.c_str());
    if (!array || ! ::cJSON_IsArray(array)) {
        throw std::runtime_error("Key is not an array: " + arrayKey);
    }
    ::cJSON_AddItemToArray(array, value.m_pRoot );
    return *this;
}



// 类型安全的数据获取
std::string JsonWrapper::getString() const {
    this->validateNode("getString() on non-string");
    return ::cJSON_GetObjectItemCaseSensitive( this->m_pRoot , "value")->valuestring;
}

int JsonWrapper::getInt() const {
    this->validateNode("getInt() on non-number");
    return ::cJSON_GetObjectItemCaseSensitive( this->m_pRoot, "value")->valueint;
}


double JsonWrapper::getDouble() const {
    this->validateNode("getDouble() on non-number");
    return ::cJSON_GetObjectItemCaseSensitive( this->m_pRoot, "value")->valuedouble;
}


bool JsonWrapper::getBool() const {
    this->validateNode("getBool() on non-number");
    return ::cJSON_GetObjectItemCaseSensitive( this->m_pRoot, "value")->valueint;
}

// 序列化为字符串
std::string JsonWrapper::toString() const {
    char* buffer = cJSON_Print( this->m_pRoot );
    std::string result(buffer);
    ::free(buffer);
    return result;
}

// 私有构造函数：从 cJSON 节点创建
JsonWrapper::JsonWrapper(cJSON* node) :  m_pRoot(node) {}

// 验证节点有效性
void JsonWrapper::validateNode(const std::string& context) const {
    if ( nullptr == this->m_pRoot ) {
        throw std::runtime_error(context + ": Invalid JSON node");
    }
}

// 清理资源
void JsonWrapper::clear() {
    if ( this->m_pRoot) {
        ::cJSON_Delete( this->m_pRoot );
        this->m_pRoot = nullptr;
    }
}

// 类型检查方法
bool JsonWrapper::isString() const 
{
     return ::cJSON_IsString( this->m_pRoot );
}

bool JsonWrapper::isInt() const 
{
     return ::cJSON_IsNumber( this->m_pRoot ); 
}

bool JsonWrapper::isDouble() const
{
    return ::cJSON_IsNumber( this->m_pRoot ); 
}

bool JsonWrapper::isBool() const
{
    return ::cJSON_IsNumber( this->m_pRoot ); 
}

bool JsonWrapper::isArray() const 
{ 
    return ::cJSON_IsArray( this->m_pRoot ); 
}

bool JsonWrapper::isObject() const 
{ 
    return ::cJSON_IsObject( this->m_pRoot ); 
}





