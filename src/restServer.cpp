#include "../include/restServer.h"

#include <iostream>
#include <filesystem>
#include <map>
#include <memory>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <boost/format.hpp>

#include "./../json/include/nlohmann/json.hpp"

//using namespace restbed;
using json = nlohmann::json;

using namespace std;


namespace restServer
{
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
           auto headers    = request->get_headers( );
           auto path       = request->get_path();
           auto method     = request->get_method();
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
    /// \brief restServer::restHttpServer::cert
    /// \param session
    /// \note .well-known/acme-challenge/token
    /// \example curl  "http://127.0.0.1/.well-known/acme-challenge/token
    void restServer::restHttpServer::cert_challenge( const shared_ptr< rest::Session > session )
    {
        const auto request = session->get_request( );
        if( request->get_method() == "GET" )
        {
            string strFilename = request->get_path();
            strFilename.erase( 0, 1 );
            ifstream inf( "./" + strFilename, ifstream::in );
            if( inf.is_open() )
            {
                stringstream sstr;
                sstr << inf.rdbuf();

                const multimap< string, string > headers
                {
                    { "Content-Type", "text/html" },
                    { "Content-Length", ::to_string( sstr.str().length( ) ) }
                };
                session->close( rest::OK, sstr.str(), headers );
            } else
            {
                session->close( rest::NOT_FOUND );
            }
        }
    }

    ///
    /// \brief entry_handler
    /// \param session
    /// \note nighscout glucose api
    /// \test curl  "http://127.0.0.1/api/v1/entries.json?count=1&rr=1587961452708" -H  "accept: application/json" -H "User-Agent: xDrip+"| python3 -m json.tool
    void restServer::restHttpServer::entry_handler( const shared_ptr< rest::Session > session )
    {
        // api-secret
        const auto request = session->get_request( );
        auto headers = request->get_headers( );
        auto method  = request->get_method();  // ex http
        auto path    = request->get_path();
        int32_t nCount = 1;
        bool bAuth = false;

        m_pLog->logInfo( (boost::format( "rest server: entry_handler: request *** new %s %s %s %s %s:%s" ) %
                          request->get_host() % method % request->get_version() % request->get_protocol() %
                          request->get_path() % request->get_port()).str()  );

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

        auto fn = [&request, &nCount, &bAuth, this]( const shared_ptr< rest::Session > /*session*/, const rest::Bytes & /*body*/ ) -> void
        {
            auto headers = request->get_headers( );
            auto queryParams = request->get_query_parameters();  // multimap<string>
            auto queryPath   = request->get_path_parameters();  // multimap<string>

            for( auto header : headers )
            {
                m_pLog->logInfo( (boost::format( "rest server: entry_handler: headers %s : %s" ) % header.first % header.second).str() );
                if( header.first == "User-Agent" )
                {
                    // simple way to try to limit to xdrip clients until xdrip provides swome auth mechanism or token
                    if( header.second.find( "xDrip+" ) != std::string::npos )
                    {
                        bAuth = true;
                    }
                }
            }
            for( auto qp : queryParams )
            {
                if( qp.first == "count" )
                {
                    nCount = std::stoi( qp.second.c_str() );
                }
                m_pLog->logInfo( (boost::format( "rest server: entry_handler: query %s : %s" ) % qp.first % qp.second).str() );
            }
            for( auto qp : queryPath )
            {
                if( qp.first == "count" )
                {
                    nCount = std::stoi( qp.second.c_str() );
                }
                m_pLog->logInfo( (boost::format( "rest server: entry_handler: post %s : %s" ) % qp.first % qp.second).str() );
            }
            return;
        };
        session->fetch( nContent_length, fn );

        // expected output
        //    [
        //        {
        //            "date": 1588125626000,
        //            "dateString": "2020-04-29T02:00:26.000Z",
        //            "device": "share2",
        //            "direction": "Flat",
        //            "sgv": 123,
        //            "sysTime": "2020-04-27T03:10:18.000Z",
        //            "trend": 4,
        //            "type": "sgv",
        //            "utcOffset": 0
        //        },
        //        {
        //            "date": 1588125927000,
        //            "dateString": "2020-04-29T02:05:26.000Z",
        //            "device": "share2",
        //            "direction": "Flat",
        //            "sgv": 125,.
        //            "sysTime": "2020-04-27T03:05:18.000Z",
        //            "trend": 4,
        //            "type": "sgv",
        //            "utcOffset": 0
        //        }
        //    ]

        json js;
        if( (path == "/api/v1/entries.json") && (bAuth == true) )
        {
            if( false == m_pCache->verify_request( nCount ) )
            {
                nCount = m_pCache->size();
            }
            auto [bOk, vItem ] = m_pCache->front( nCount );
            if( bOk == true )
            {
                for( auto item : vItem )
                {
                    m_pLog->logInfo( (boost::format( "rest server: entry_handler: ST:%d BG:%d" ) % item.ST % item.value ).str() );
                    string strDT;
                    string strST;
                    common::timeTickToString( item.ST, strDT, "%FT%TZ" );
                    common::timeTickToString( item.ST, strST );
                    json jsItem =
                    {
                        { "sgv",        item.value          },
                        { "date",       item.ST             },
                        { "dateString", strDT.c_str()       },
                        { "Trend",      item.trend          },
                        { "direction",  restHttpServer::trend( item.trend ) },
                        { "device",     "share2"            },
                        { "type",       "sgv"               },
                        { "utcOffset",  -4                  },
                        { "sysTime",    strST.c_str()       }
                    };
                    js.push_back( jsItem );
                }
            }
        } else
        {
            session->close( rest::NOT_FOUND );
            return;
        }
        string strRes = js.dump();
        session->close( rest::OK, strRes, { { "Content-Length", std::to_string(strRes.length()) } } );

        m_pLog->logDebug( (boost::format( "rest server: entry_handler debug: %s" ) % strRes ).str() );
        m_pLog->logInfo( (boost::format( "rest server: (rest server is local, not using dexcom services) entry_handler: request *** complete %s %s %s %s %s:%s" ) %
                          request->get_host() % method % request->get_version() % request->get_protocol() %
                          request->get_path() % request->get_port()).str()  );
    }
}



///
/// \brief restServer::restHttpServer::startRestServer
/// \param a_unPort
/// \param a_pCache
/// \param a_pLog
///
void restServer::restHttpServer::startRestServer( const uint16_t a_unPort, mutlib::config *a_pConfig, data::bg_cache *a_pCache, logging::log *a_pLog )
{
    m_pCache = a_pCache;
    m_pLog   = a_pLog;
    m_pConfig= a_pConfig;
    if( m_pCache == nullptr )
    {
        a_pLog->logError( "invalid cache, rest service failed" );
        return;
    }
    if( m_pLog == nullptr )
    {
        fprintf( stdout, "invalid log, rest service failed\n" );
        return;
    }
    if( m_pConfig == nullptr )
    {
        fprintf( stdout, "invalid config, rest service failed\n" );
        return;
    }

    auto fnEntry = [=]( const std::shared_ptr<rest::Session> a_sess ) -> void
    {
        this->entry_handler( a_sess );
    };
    auto fnCert = [=]( const std::shared_ptr<rest::Session> a_sess ) -> void
    {
        this->cert_challenge( a_sess );
    };
    auto fnNotfound = [=]( const std::shared_ptr<rest::Session> a_sess ) -> void
    {
        const auto request = a_sess->get_request();
        string strPath = request->get_path();
        cout << strPath << endl;
    };


    auto entry = make_shared< rest::Resource >( );
    entry->set_paths( m_setNighScout );
    entry->set_method_handler( "GET", fnEntry );
    entry->set_method_handler( "POST", fnEntry );

    auto getCert = make_shared< rest::Resource >( );
    getCert->set_path( m_strChallenge );
    getCert->set_method_handler( "GET", fnCert );

    auto dexLogin = make_shared< rest::Resource >( );
    dexLogin->set_path( m_strDexShareLogin );
    dexLogin->set_method_handler( "POST", post_login );

    auto dexBg = make_shared< rest::Resource >( );
    dexBg->set_path( m_strDexShareBgValue );
    dexBg->set_method_handler( "POST", post_bg );


    auto settings = make_shared< rest::Settings >( );
    settings->set_port(  a_unPort );
    settings->set_default_header( "Connection", "close" );

    m_service.set_not_found_handler( fnNotfound );
    m_service.publish( entry );
    //m_service.publish( dexLogin );
    //m_service.publish( dexBg );
    m_service.start( settings );
    m_pLog->logInfo( "rest http server stopping" );
}


///
/// \brief restServer::restHttpServer::stopRestServer
///
void restServer::restHttpServer::stopRestServer()
{
    m_service.stop();
}


///
/// \brief trend - return string fro dexcom trend
/// \param a_uTrend
/// \return
///
const char* restServer::restHttpServer::trend( uint8_t a_uTrend )
{

    switch ( a_uTrend)
    {
        case 1:
            return "DoubleUp";
        case 2:
            return "SingleUp";
        case 3:
            return "FortyFiveUp";
        case 4:
            return "Flat";
        case 5:
            return "FortyFiveDown";
        case 6:
            return "SingleDown";
        case 7:
            return "DoubleDown";
        default:
            return "";
    }
}

