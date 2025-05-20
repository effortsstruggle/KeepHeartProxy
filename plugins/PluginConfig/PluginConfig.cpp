#include "PluginConfig.h"
#include <thread>
#include <fstream>
#include <string.h>
#include "./PluginConfigLog.h"

extern "C" PluginInterface* createPlugin() {

    return Singleton_PluginConfig::getInstance();
}


 
PluginConfig::PluginConfig()
    : m_stCfgPath("/home/qin/workspace/KeepHeartProxyDependsLib/data/keepheart.cfg") 
    , m_pAsyncCall( nullptr )
{

     this->m_pAsyncCall = new AsyncCall();
     this->m_pAsyncCall->SetLogTag("plugin_config");
   
     //添加成员前，确保 Document 根节点是对象类型
     this->m_oJsonDoc.SetObject() ;
}


PluginConfig::~PluginConfig()
{
    std::cout << "PluginConfig::~PluginConfig(1)" << std::endl; 
    if( nullptr != this->m_pAsyncCall )
    {
        delete this->m_pAsyncCall ;
        this->m_pAsyncCall = nullptr ;
    }
    std::cout << "PluginConfig::~PluginConfig(2)" << std::endl; 

}

int PluginConfig::execute(int key,std::string const &data)
{

    NotifyParam oNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
    std::string stNotifyJson = "" ;
    std::string stInputData = "" ;
   
  if( ! data.empty() ) 
    {
        sonic_json::Document doc;
        doc.Parse(data);

        if (doc.HasParseError()) {
            std::cout << "Parse failed!" << std::endl;
            return -1;
        }

        // Find member by key
        if (!doc.IsObject()) {  // Check JSON value type.
            std::cout << "Incorrect doc type!" << std::endl;
            return -1;
        }

        auto m = doc.FindMember("data1");   
        if (m != doc.MemberEnd()) {
            const sonic_json::Node& key = m->name;
            sonic_json::Node& value = m->value;
            if (key.IsString()) {
                std::cout << "Key is: " << key.GetString() << std::endl;
            } else {
                std::cout << "Incoreect key type!" << std::endl;
            }

            if (value.IsString()) {
                // std::cout << "Value is " << value.GetString() << std::endl;
                stInputData = value.GetString();
            }else if( value.IsObject() ) {
                sonic_json::WriteBuffer wb;
                value.Serialize(wb);
                // std::cout << " sonic_json::Document : " << wb.ToString() << std::endl;  
                stInputData = wb.ToString();
            }else {
                std::cout << "Incoreect value type!" << std::endl;
            }
        } else {
            std::cout << "Find key doesn't exist!" << std::endl;
        }
        
        // sonic_json::WriteBuffer wb;
        // doc.Serialize(wb);
        // std::cout << " sonic_json::Document : " << wb.ToString() << std::endl;
    }

    switch (key)
    {
    case CFG_INIT:
        oNotifyParam = this->init();
        break;
    case CFG_CLOSE:
        oNotifyParam = this->close();
        break;     
    case CFG_READ:
        oNotifyParam = this->read( stInputData );
        break;
    case CFG_WRITE:
        oNotifyParam = this->write( stInputData );
        break;   
    case CFG_RESET:
        oNotifyParam = this->reset();
        break; 
    case CFG_READ_JSON:
        oNotifyParam = this->getJson( stInputData ) ;
        break;
    case CFG_READ_JSON_ALL:
        oNotifyParam = this->getAll();
        break;
    default:
        break;
    }


    {
        stNotifyJson = this->makeNotifyJson( oNotifyParam );
        this->Notify( key , stNotifyJson );
    }


    return oNotifyParam.m_s32Ret ; 
}

int PluginConfig::executeAsync( int key,std::string const &data )
{
    NotifyParam oNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError } ; 
    std::string stNotifyJson = "" ;
    std::string stInputData = "" ;

    
    //parse json
    if( ! data.empty() ) 
    {
        sonic_json::Document doc;
        doc.Parse(data);

        if (doc.HasParseError()) {
            std::cout << "Parse failed!" << std::endl;
            return -1;
        }

        // Find member by key
        if (!doc.IsObject()) {  // Check JSON value type.
            std::cout << "Incorrect doc type!" << std::endl;
            return -1;
        }

        auto m = doc.FindMember("data1");   
        if (m != doc.MemberEnd()) {
            const sonic_json::Node& key = m->name;
            sonic_json::Node& value = m->value;
            if (key.IsString()) {
                std::cout << "Key is: " << key.GetString() << std::endl;
            } else {
                std::cout << "Incoreect key type!" << std::endl;
            }

            if (value.IsString()) {
                stInputData = value.GetString();
            }else if( value.IsObject() ) {
                sonic_json::WriteBuffer wb;
                value.Serialize(wb);
                stInputData = wb.ToString();
            }else {
                std::cout << "Incoreect value type!" << std::endl;
            }
        } else {
            std::cout << "Find key doesn't exist!" << std::endl;
        }
    }

    std::cout << " PluginConfig::executeAsync key : " <<  key <<"  data: " << data << " input-data : " << stInputData <<std::endl;

    //exec
    switch (key)
    {
    case CFG_INIT:
        this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncInit , "" , 0 , 0 , 0 , 0 , "cfg_init")  );
        break;
    case CFG_CLOSE:
        this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncClose , "" , 0 , 0 , 0 , 0 , "cfg_close") );
        break;     
    case CFG_READ:
        this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncRead , stInputData , 0 , 0 , 0 , 0 , "cfg_read") );
        break;
    case CFG_WRITE:
        this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncWrite , stInputData , 0 , 0 , 0 , 0 , "cfg_write") );
        break;   
    case CFG_RESET:
        this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncReset , "" , 0 , 0 , 0 , 0 , "cfg_reset") );
        break; 
    case CFG_READ_JSON:
        this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncGetJson , stInputData , 0 , 0 , 0 , 0 , "cfg_getjson") );
        break;
    case CFG_READ_JSON_ALL:
        this->m_pAsyncCall->AddFunctionToQueue( FuncAndParam( PluginConfig::asyncGetAll , "" , 0 , 0 , 0 , 0 , "cfg_getall") );
        break;
    default:
        break;
    }

    return oNotifyParam.m_s32Ret ;
 }


int PluginConfig::asyncInit( FuncAndParam const &param )
{
    NotifyParam oNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
    std::string stNotifyJson = "" ;

    // init
    oNotifyParam = Singleton_PluginConfig::getInstance()->init();
    //make
    stNotifyJson = Singleton_PluginConfig::getInstance()->makeNotifyJson( oNotifyParam );
    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_INIT , stNotifyJson );


    return oNotifyParam.m_s32Ret ; 
}

NotifyParam PluginConfig::init()
{   
    NotifyParam oNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 

    // read file
    std::ifstream ifs( this->m_stCfgPath );
    if (! ifs.is_open() ) 
    {
        oNotifyParam = this->fillEmptyRoot();
        std::cout << "PluginConfig::init() file open fail , path : " << this->m_stCfgPath << std::endl;
        return oNotifyParam ;
    }

    std::string stLine , stContent ;
    while (std::getline(ifs,  stLine )) {  // 逐行读取
        stContent += stLine + "\n"; // 保留换行符（根据需求调整）
    }
    ifs.close();

    // parse json 
    std::cout << stContent << std::endl;
    this->m_oJsonDoc.Parse( stContent  );
    if ( this->m_oJsonDoc .HasParseError()) {
        oNotifyParam.m_s32Ret = -1 ;
        oNotifyParam.m_eErrorCode = NotifyErrorCode::eJsonParseError ;
        std::cout << "PluginConfig::init() json parse fail . " << std::endl;
        return oNotifyParam;
    }

    // Find member by key
    if (! this->m_oJsonDoc.IsObject() ) {  // Check JSON value type.
        oNotifyParam.m_s32Ret = -1 ;
        oNotifyParam.m_eErrorCode = NotifyErrorCode::eTypeError ;
        std::cout << "PluginConfig::init() Incorrect doc type!" << std::endl;
        return oNotifyParam;
    }

    return oNotifyParam;
}



int PluginConfig::asyncRead( FuncAndParam const &param )
{
    NotifyParam oNotifyParam = { 0 , "" ,  NotifyErrorCode::eInvialdError }; 
    std::string stNotifyJson = "" ;
    // read
    oNotifyParam = Singleton_PluginConfig::getInstance()->read( param.data );
    //make
    stNotifyJson = Singleton_PluginConfig::getInstance()->makeNotifyJson( oNotifyParam );
    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_READ , stNotifyJson );

    return oNotifyParam.m_s32Ret ; 

}

NotifyParam PluginConfig::read(std::string stData )
{
    std::cout << " PluginConfig::read -> data : " << stData <<std::endl ;
    NotifyParam oNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 


    auto m = this->m_oJsonDoc.FindMember( stData );   
    if (m != this->m_oJsonDoc.MemberEnd()) {
        const sonic_json::Node& key = m->name;
        sonic_json::Node& value = m->value;
        
        if( key.IsString() ){
            std::cout << "Key is: " << key.GetString() << std::endl;
        } else {
            std::cout << "Incoreect key type!" << std::endl;
        }


        sonic_json::Document oDoc ;
        oDoc.SetObject();

    
  
        if (value.IsString()) {

            sonic_json::Node typeNode( sonic_json::kSint );
            typeNode.SetInt64( 1 );

            sonic_json::Node valueNode( sonic_json::kString );
            valueNode.SetString( value.GetString(), oDoc.GetAllocator());
    
            oDoc.AddMember( sonic_json::StringView( "type" ) ,  std::move(typeNode) , oDoc.GetAllocator());
            oDoc.AddMember( sonic_json::StringView( "value" ) ,  std::move(valueNode) , oDoc.GetAllocator());

        }else if( value.IsInt64() ) {

            sonic_json::Node typeNode( sonic_json::kSint );
            typeNode.SetInt64( 2 );

            sonic_json::Node valueNode( sonic_json::kSint );
            valueNode.SetInt64( value.GetInt64() ) ;
    
            oDoc.AddMember( sonic_json::StringView( "type" ) ,  std::move(typeNode) , oDoc.GetAllocator());
            oDoc.AddMember( sonic_json::StringView( "value" ) ,  std::move(valueNode) , oDoc.GetAllocator());

        }else if( value.IsDouble() ) {

            sonic_json::Node typeNode( sonic_json::kSint );
            typeNode.SetInt64( 3 );

            sonic_json::Node valueNode( sonic_json::kReal );
            valueNode.SetDouble( value.GetDouble() ) ;
    
            oDoc.AddMember( sonic_json::StringView( "type" ) ,  std::move(typeNode) , oDoc.GetAllocator());
            oDoc.AddMember( sonic_json::StringView( "value" ) ,  std::move(valueNode) , oDoc.GetAllocator());

        }else if( value.IsObject() ){
            
            sonic_json::Node typeNode( sonic_json::kSint );
            typeNode.SetInt64( 4 );

            sonic_json::Node valueNode ;
            valueNode.CopyFrom( value , oDoc.GetAllocator());

            oDoc.AddMember( sonic_json::StringView( "type" ) ,  std::move(typeNode) , oDoc.GetAllocator());
            oDoc.AddMember( sonic_json::StringView( "value" ) ,  std::move(valueNode) , oDoc.GetAllocator());


        }else if( value.IsArray() ){

            sonic_json::Node typeNode( sonic_json::kSint );
            typeNode.SetInt64( 5 );

            sonic_json::Node valueNode ;
            valueNode.CopyFrom( value , oDoc.GetAllocator());

            oDoc.AddMember( sonic_json::StringView( "type" ) ,  std::move(typeNode) , oDoc.GetAllocator());
            oDoc.AddMember( sonic_json::StringView( "value" ) ,  std::move(valueNode) , oDoc.GetAllocator());



        }else {
            std::cout << "Incoreect value type!" << std::endl;
        }
        
        sonic_json::WriteBuffer oWb;
        oDoc.Serialize( oWb );
        oNotifyParam.m_stSuccessInfo = oWb.ToString();


    } else {
        std::cout << "Find key doesn't exist!" << std::endl;
    }

    return oNotifyParam;
}


int PluginConfig::asyncWrite( FuncAndParam const &param )
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
    std::string stNotifyJson = "" ;


    //WRITE
    objNotifyParam = Singleton_PluginConfig::getInstance()->write( param.data );
    //make
    stNotifyJson = Singleton_PluginConfig::getInstance()->makeNotifyJson( objNotifyParam );
    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_WRITE , stNotifyJson );

    return objNotifyParam.m_s32Ret ; 
}

NotifyParam PluginConfig::write(std::string stData)
{
    std::cout << " PluginConfig::write -> data : " << stData <<std::endl ;

    NotifyParam oNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
    std::string stKey = "" ;

    auto& oAlloc = this->m_oJsonDoc.GetAllocator();


    //parse json
    sonic_json::Document oDoc;
    oDoc.Parse( stData  );
    if ( oDoc.HasParseError()) {
        oNotifyParam.m_s32Ret = -1 ;
        oNotifyParam.m_eErrorCode = NotifyErrorCode::eJsonParseError ;
        std::cout << "PluginConfig::write() json parse fail . " << std::endl;
        return oNotifyParam;
    }
    // Find member by key
    if (! oDoc.IsObject() ) {  // Check JSON value type.
        oNotifyParam.m_s32Ret = -1 ;
        oNotifyParam.m_eErrorCode = NotifyErrorCode::eTypeError ;
        std::cout << "PluginConfig::write() Incorrect doc type!" << std::endl;
        return oNotifyParam;
    }

    auto m = oDoc.FindMember( sonic_json::StringView( "key" ) );   
    if (m != oDoc.MemberEnd()) {
        const sonic_json::Node& key = m->name;
        sonic_json::Node& value = m->value;
        
        if( key.IsString() ){
            std::cout << "Key is: " << key.GetString() << std::endl;
        } else {
            std::cout << "Incoreect key type!" << std::endl;
        }

        if (value.IsString()) {
            stKey = value.GetString();
            std::cout << "value is : " << stKey << std::endl;;
        } else {
            std::cout << "Incoreect value type!" << std::endl;
        }

    } else {
        std::cout << "Find key doesn't exist!" << std::endl;
    }


    m = oDoc.FindMember( sonic_json::StringView("value") );   
    if (m != oDoc.MemberEnd()) {
        const sonic_json::Node& key = m->name;
        sonic_json::Node& value = m->value;
        
        if( key.IsString() ){
            std::cout << "Key is: " << key.GetString() << std::endl;
        } else {
            std::cout << "Incoreect key type!" << std::endl;
        }

        // 检查并删除旧键（若需强制唯一性）
        if ( this->m_oJsonDoc.HasMember( sonic_json::StringView( stKey ) )) {
             this->m_oJsonDoc.RemoveMember( sonic_json::StringView( stKey ) );
        }

       

        if (value.IsString()) {
            sonic_json::Node oNode( sonic_json::kString );
            oNode.SetString( value.GetString(), oAlloc );
            this->m_oJsonDoc.AddMember( sonic_json::StringView( stKey ) ,  std::move(oNode) ,  oAlloc );
        }else if( value.IsInt64() ) {
            sonic_json::Node oNode( sonic_json::kSint );
            oNode.SetInt64( value.GetInt64() );
            this->m_oJsonDoc.AddMember( sonic_json::StringView( stKey ) , std::move(oNode) , oAlloc );
        }else if( value.IsDouble() ) {
            sonic_json::Node oNode( sonic_json::kReal );
            oNode.SetDouble( value.GetDouble() );
            this->m_oJsonDoc.AddMember( sonic_json::StringView( stKey ) , std::move(oNode) , oAlloc );
        }else if( value.IsObject() ) {
            sonic_json::Node oNode ;
            oNode.CopyFrom( value ,  oAlloc) ;
            this->m_oJsonDoc.AddMember( sonic_json::StringView( stKey ) , std::move( oNode ) , oAlloc );
        }else if( value.IsArray() ) {
            sonic_json::Node oNode ;
            oNode.CopyFrom( value ,  oAlloc) ;
            this->m_oJsonDoc.AddMember( sonic_json::StringView( stKey ) , std::move( oNode ) , oAlloc );
        }else {
            std::cout << "Incoreect value type!" << std::endl;
        }

    } else {
        std::cout << "Find key doesn't exist!" << std::endl;
    }

    oNotifyParam = this->sync();

    return oNotifyParam;
}


int PluginConfig::asyncReset( FuncAndParam const &param )
{
    NotifyParam objNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
    std::string stNotifyJson = "" ;

    //reset
    objNotifyParam = Singleton_PluginConfig::getInstance()->reset();
    //make
    stNotifyJson = Singleton_PluginConfig::getInstance()->makeNotifyJson( objNotifyParam );
    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_RESET , stNotifyJson );

    return objNotifyParam.m_s32Ret ; 
}

NotifyParam PluginConfig::reset()
{
    NotifyParam oNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError };     

    if ( -1 == ::remove( this->m_stCfgPath.c_str() ) )
    {
        oNotifyParam.m_s32Ret = -1 ;
        oNotifyParam.m_eErrorCode = NotifyErrorCode::eNullPointerError ;
        std::cout << "PluginConfig::reset() remove file fail. " << std::endl ;
        return oNotifyParam;
    } 

    //reset memory
    this->m_oJsonDoc = sonic_json::Document();  // 清空所有内容

    //reset
    oNotifyParam = this->init();

    return oNotifyParam;
}


int PluginConfig::asyncClose( FuncAndParam const &param )
{
    NotifyParam oNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
    std::string stNotifyJson = "" ;

    //close
    oNotifyParam =  Singleton_PluginConfig::getInstance()->close();
    //make
    stNotifyJson = Singleton_PluginConfig::getInstance()->makeNotifyJson( oNotifyParam );
    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_CLOSE , stNotifyJson );

    return oNotifyParam.m_s32Ret ; 
}

NotifyParam PluginConfig::close()
{
    NotifyParam oNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
    oNotifyParam = this->sync();
    return oNotifyParam;
}


int PluginConfig::asyncGetJson( FuncAndParam const &param )
{
    NotifyParam oNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
    std::string stNotifyJson = "" ;

    //getJson
    oNotifyParam =  Singleton_PluginConfig::getInstance()->getJson( param.data );
    //make
    stNotifyJson = Singleton_PluginConfig::getInstance()->makeNotifyJson( oNotifyParam );
    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_READ_JSON , stNotifyJson );

    return oNotifyParam.m_s32Ret ; 
}

NotifyParam PluginConfig::getJson( std::string stData )
{
    NotifyParam oNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 

    return oNotifyParam ;
}


int PluginConfig::asyncGetAll( FuncAndParam const &param )
{
    NotifyParam oNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 
    std::string stNotifyJson = "" ;

    // getAll
    oNotifyParam =  Singleton_PluginConfig::getInstance()->getAll();
    //make
    stNotifyJson = Singleton_PluginConfig::getInstance()->makeNotifyJson( oNotifyParam );
    // notify
    Singleton_PluginConfig::getInstance()->NotifyAsyn( PluginConfig::CFG_READ_JSON_ALL , stNotifyJson );

    return oNotifyParam.m_s32Ret ;
}

NotifyParam PluginConfig::getAll()
{
    NotifyParam oNotifyParam = { 0 , "" , NotifyErrorCode::eInvialdError }; 

    sonic_json::WriteBuffer oWb;
    this->m_oJsonDoc.Serialize( oWb );

    oNotifyParam.m_stSuccessInfo = oWb.ToString();

    return oNotifyParam ;
}   

NotifyParam PluginConfig::sync()
{
    NotifyParam objNotifyParam = {0 , "" , NotifyErrorCode::eInvialdError }; 

    //data
    sonic_json::WriteBuffer oWb;
    this->m_oJsonDoc.Serialize( oWb );
    std::string stContent = oWb.ToString();

    //覆盖写入（默认模式）​
    std::ofstream ofs( this->m_stCfgPath );
    if (!ofs.is_open()) {
        objNotifyParam.m_s32Ret = -1 ;
        objNotifyParam.m_eErrorCode = NotifyErrorCode::eOpenFileError ;
        std::cout << "Failed to open file for writing!" << std::endl;
        return objNotifyParam;
    }
    ofs << stContent ;
    // 显式关闭文件（可选）
    ofs.close();

    //success 
    objNotifyParam.m_stSuccessInfo = stContent ;

    return objNotifyParam ;
}

NotifyParam PluginConfig::fillEmptyRoot()
{
    NotifyParam objNotifyParam = {0 , "" , NotifyErrorCode::eInvialdError }; 

    this->m_oJsonDoc.AddMember( sonic_json::StringView( "pw" ) , sonic_json::Node( sonic_json::StringView( "111111" )) , this->m_oJsonDoc.GetAllocator());
    this->m_oJsonDoc.AddMember( sonic_json::StringView( "enablePW" ) , sonic_json::Node( sonic_json::StringView( "1" ) ) , this->m_oJsonDoc.GetAllocator());
    this->m_oJsonDoc.AddMember( sonic_json::StringView( "inputTimes" ) , sonic_json::Node( sonic_json::StringView( "6" ) ) , this->m_oJsonDoc.GetAllocator());
    this->m_oJsonDoc.AddMember( sonic_json::StringView( "diMcuUnitTemper" ) , sonic_json::Node( sonic_json::StringView( "0" ) ) , this->m_oJsonDoc.GetAllocator());
    this->m_oJsonDoc.AddMember( sonic_json::StringView( "diMcuLanguage" ) , sonic_json::Node( sonic_json::StringView( "0" ) ) , this->m_oJsonDoc.GetAllocator());
    this->m_oJsonDoc.AddMember( sonic_json::StringView( "Units" ) , sonic_json::Node( 0 ) , this->m_oJsonDoc.GetAllocator());
    this->m_oJsonDoc.AddMember( sonic_json::StringView( "diMcuUnitTime" ) , sonic_json::Node( 1 ) , this->m_oJsonDoc.GetAllocator());
    this->m_oJsonDoc.AddMember( sonic_json::StringView( "controlbrightMode" ) , sonic_json::Node( 0 ) , this->m_oJsonDoc.GetAllocator());
    this->m_oJsonDoc.AddMember( sonic_json::StringView( "dashBoardtbrightMode" ) , sonic_json::Node( 0 ) , this->m_oJsonDoc.GetAllocator());
    this->m_oJsonDoc.AddMember( sonic_json::StringView( "diRadioArea" ) , sonic_json::Node( 0 ) , this->m_oJsonDoc.GetAllocator());
    this->m_oJsonDoc.AddMember( sonic_json::StringView( "gyro_init_x" ) , sonic_json::Node( 0 ) , this->m_oJsonDoc.GetAllocator());
    this->m_oJsonDoc.AddMember( sonic_json::StringView( "gyro_init_y" ) , sonic_json::Node( 0 ) , this->m_oJsonDoc.GetAllocator());

    objNotifyParam = this->sync();

    return objNotifyParam ;
}

std::string PluginConfig::makeNotifyJson( NotifyParam &oNotifyParam )
{
    sonic_json::Document oDoc ;
    oDoc.SetObject();

    oDoc.AddMember( sonic_json::StringView( "result" ) , sonic_json::Node( oNotifyParam.m_s32Ret  ) , oDoc.GetAllocator());
    oDoc.AddMember( sonic_json::StringView( "sucess_notify" ) , sonic_json::Node( oNotifyParam.m_stSuccessInfo ) , oDoc.GetAllocator());
    oDoc.AddMember( sonic_json::StringView( "error_notify" ) , sonic_json::Node( oNotifyParam.m_eErrorCode ) , oDoc.GetAllocator());

    sonic_json::WriteBuffer oWb;
    oDoc.Serialize( oWb );

    return oWb.ToString();
}



