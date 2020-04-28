#include "../include/restServer.h"

#include <memory>

#include "./../json/include/nlohmann/json.hpp"

//using namespace restbed;
using json = nlohmann::json;

using namespace std;


namespace restServer
{
    void entry_handler( const std::shared_ptr< rest::Session > session );
    void post_login   ( const shared_ptr< rest::Session > session );
    void post_bg      ( const shared_ptr< rest::Session > session );

    ///
    /// \brief post_login
    /// \param session
    /// \note api for connecting to dexcom share
    ///
    void post_login( const shared_ptr< rest::Session > session )
    {
       const auto request = session->get_request( );

       int nContent_length = 0;
       auto headers = request->get_headers( );
       string strHeader = request->get_header( "Content-Length" );
       nContent_length = std::stoi( strHeader );


       auto fn = [ ]( const shared_ptr< rest::Session > /*session*/, const rest::Bytes & /*body*/ ) -> void
       {
           //fprintf( stdout, "%.*s\n", ( int ) body.size( ), body.data( ) );
           return;
       };
       session->fetch( nContent_length, fn );

       auto path = request->get_path();
       auto method = request->get_method();
       auto qp = request->get_query_parameters();
       auto pa = request->get_path_parameters();
       string strBody;
       request->get_body( strBody );
       json js = json::parse( strBody );

       string strAccntName = js.value( "accountName"  , "" );
       string strPasswd    = js.value( "password"     , "" );
       string strAppId     = js.value( "applicationId", "" );



       string strRes = "\"37644ecc-19f2-4fa6-8baf-39f4bab4b08b\"";
       session->close( rest::OK, strRes, { { "Content-Length", std::to_string(strRes.length()) } } );
    }

    ///
    /// \brief post_bg
    /// \param session
    /// \note api for connecting to dexcom share
    ///
    void post_bg( const shared_ptr< rest::Session > session )
    {
       const auto request = session->get_request( );

       int nContent_length = 0;
       auto fn = [&request]( const shared_ptr< rest::Session > session, const rest::Bytes & /*body*/ ) -> void
       {
           auto headers = request->get_headers( );
           auto path = request->get_path();
           auto method = request->get_method();
           auto mmapParams = request->get_query_parameters();
           //auto mmapParams = request->get_path_parameters();

           string strMaxcount;
           string strMinutes;
           string strSessionId;
           auto iter = mmapParams.find( "maxcount" );
           if( iter != mmapParams.end() )
           {
               strMaxcount = iter->second;
           }

           iter = mmapParams.find( "minutes" );
           if( iter != mmapParams.end() )
           {
               strMinutes = iter->second;
           }

           iter = mmapParams.find( "sessionId" );
           if( iter != mmapParams.end() )
           {
               strSessionId = iter->second;
           }

           //"[\n  {\n    \"DT\": \"\\\"Date(1587665105000+0000)\\\"\", \n    \"ST\": \"\\\"Date(1587665105000+0000)\\\"\", \n    \"Trend\": 5, \n    \"Value\": 74, \n    \"WT\": \"\\\"Date(1587665105000+0000)\\\"\"\n  }\n]\n"
           string strRet = "{ \"DT\" : \"\"Date(1587665105000+0000)\"\", \"ST\" : \"\"Date(1587665105000+0000)\"\", \"Trend\" : 5, \"Value\" : 74, \"WT\" : \"\"Date(1587665105000+0000)\"\" }";
           json js = { {{ "DT", "Date(1587665105000+0000)" }, { "ST", "Date(1587665105000+0000)" }, {"Trend", 5}, {"Value", 74}, {"WT", "Date(1587665105000+0000)"}} };
           string strJ = js.dump();
           session->close( rest::OK, strJ, { { "Content-Length", std::to_string( strJ.length() ) } } );
           return;
       };
       session->fetch( nContent_length, fn );
    }

    ///
    /// \brief entry_handler
    /// \param session
    /// \note nighscout glucose api
    ///
    void entry_handler( const shared_ptr< rest::Session > session )
    {
        // api-secret
        const auto request = session->get_request( );
        auto headers = request->get_headers( );
        auto method = request->get_method();
        auto path = request->get_path();

        int nContent_length = 0;
        string strHeader;
        if( request->has_header( "Content-Length" ) )
        {
            strHeader = request->get_header( "Content-Length" );
            if( strHeader.length() > 0 )
            {
                nContent_length = std::stoi( strHeader );
            }
        }
        int32_t nCount = 1;

        auto fn = [&method, &request, &nCount]( const shared_ptr< rest::Session > /*session*/, const rest::Bytes & /*body*/ ) -> void
        {
            auto headers = request->get_headers( );
            auto queryParams = request->get_query_parameters();  // multimap<string>

            fprintf( stdout, "%s entry %s\n", method.c_str(), request->get_path().c_str() );
            for( auto header : headers )
            {
                fprintf( stdout, "header: %s : %s \n", header.first.c_str(), header.second.c_str() );
            }
            for( auto qp : queryParams )
            {
                if( qp.first == "count" )
                {
                    nCount = std::stoi( qp.second.c_str() );
                }
                fprintf( stdout, "param: %s : %s \n", qp.first.c_str(), qp.second.c_str() );
            }
            fprintf( stdout, "---------------- \n" );
            return;
        };
        session->fetch( nContent_length, fn );


        json js;
        if( path == "/api/v1/entries.json")
        {
            js =
            {
                {
                    {"_id", "5ea64d3504b8fc0ad8e8cf56"},
                    {"sgv", 123},
                    {"date", 1587957018000},
                    {"dateString", "2020-04-27T03:10:18.000Z"},
                    {"trend", 4},
                    {"direction", "Flat"},
                    {"device", "share2"},
                    {"type", "sgv"},
                    {"utcOffset", 0},
                    {"sysTime", "2020-04-27T03:10:18.000Z"}
                },
                {
                    {"_id", "5ea64c0904b8fc0ad8e8c860"},
                    {"sgv", 125},
                    {"date", 1587956718000},
                    {"dateString", "2020-04-27T03:05:18.000Z"},
                    {"trend", 4},
                    {"direction", "Flat"},
                    {"device", "share2"},
                    {"type", "sgv"},
                    {"utcOffset", 0},
                    {"sysTime", "2020-04-27T03:05:18.000Z"}
                }
            };
        } else
        {
            js = { {{ "DT", "Date(1587665105000+0000)" }, { "ST", "Date(1587665105000+0000)" }, {"Trend", 5}, {"Value", 74}, {"WT", "Date(1587665105000+0000)"}} };
        }
        string strRes = js.dump();
        session->close( rest::OK, strRes, { { "Content-Length", std::to_string(strRes.length()) } } );
    }
}



void restServer::restHttpServer::startRestServer( const uint16_t a_unPort, logging::log& a_log )
{
    (void)a_log;
    auto entry = make_shared< rest::Resource >( );
    entry->set_paths( m_setNighScout );
    entry->set_method_handler( "GET", entry_handler );
    entry->set_method_handler( "POST", entry_handler );

    auto dexLogin = make_shared< rest::Resource >( );
    dexLogin->set_path( m_strDexShareLogin );
    dexLogin->set_method_handler( "POST", post_login );

    auto dexBg = make_shared< rest::Resource >( );
    dexBg->set_path( m_strDexShareBgValue );
    dexBg->set_method_handler( "POST", post_bg );


   auto settings = make_shared< rest::Settings >( );
   settings->set_port(  a_unPort );
   settings->set_default_header( "Connection", "close" );

   m_service.publish( entry );
   m_service.publish( dexLogin );
   m_service.publish( dexBg );
   m_service.start( settings );

}


void restServer::restHttpServer::stopRestServer()
{
    m_service.stop();
}
