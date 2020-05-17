#include "../include/dex-mgr.h"
#include "../../json/include/nlohmann/json.hpp"

#include <boost/format.hpp>

using namespace std;
using json = nlohmann::json;


///
/// \brief dexshareManager::start - run rest client on an interval
/// \param a_cfg
/// \param a_logbg
/// \param a_logLevel
/// \return
///
bool dexshareManager::start( mutlib::config                              &a_cfg,
                             logging::log                                &a_log,
                             std::function< void( const std::string &) > &a_logbg )
{
    m_appLogger = a_log;

    string  strAccount;
    string  strPassword;
    string  strApplicationId;
    int32_t nHttpTimout = 30;
    bool bComplete = true;
    stringstream sstr;
    sstr << "errors processing config file: ";
    if( a_cfg.read( "dex.config" ) )
    {
        if( false == a_cfg.get( "account", strAccount ) )
        {
            bComplete = false;
            sstr << " missing account";
        }
        if( false == a_cfg.get( "password", strPassword ) )
        {
            bComplete = false;
            sstr << " missing passowrd";
        }
        if( false == a_cfg.get( "applicationid", strApplicationId ) )
        {
            bComplete = false;
            sstr << " missing applicationid";
        }
        if( false == a_cfg.get( "httptimeout", nHttpTimout ) )
        {
            bComplete = false;
            sstr << " missing applicationid";
        }
    }
    // config read

    if( bComplete == false )
    {
        m_appLogger.logError( (boost::format( "error %s reading config file") % sstr.str()).str() );
        return false;
    }

    data::cache_param_t fnProcessLine = processCachline;
    if( false == m_cache.cashLoad( "logs", fnProcessLine ) )
    {
        a_log.logWarn( "error loading cache" );
    } else
    {
        a_log.logInfo( "cache loaded" );
    }

    m_spMonitor = make_shared<sync_tools::monitor>();   /// moditor for syncing glucose writer and reader
    m_appLogger.logInfo( "starting reader" );
    thread thd( &dexshareManager::reader, this, a_logbg );
    while( m_bReaderReady == false )
    {
        std::this_thread::yield( ); // spin until reader is ready
    }
    m_thdReader = std::move( thd );
    m_ds.userName( strAccount );
    m_ds.password( strPassword );
    m_ds.accoundId( strApplicationId );

    m_appLogger.logInfo( "starting dexshare" );
    m_ds.start( m_spMonitor, m_appLogger, m_cache.lastReadDate() );

    int32_t nPort = 8080;
    a_cfg.get( "port", nPort );
    thread thdRest( &restServer::restHttpServer::startRestServer, &m_rest, nPort, &a_cfg, &m_cache, &m_appLogger );
    m_thdRestServer = std::move( thdRest );

    return true;
}


///
/// \brief dexcom_share::wait
///
void dexshareManager::wait()
{
    m_ds.wait();   // wait for dexcom_share::_start to end
    m_appLogger.logDebug( "dexshare closing" );
    m_spMonitor->signal();      // issue a sig to dexshareManager::reader in case in wait
    m_thdReader.join();  // wait for dexshareManager::reader to stop
    m_appLogger.logDebug( "reader ended" );
    m_thdRestServer.join();
    m_appLogger.logDebug( "rest server ended" );
    return;
}


///
/// \brief dexshareManager::stop - signal start thd to stop
///
void dexshareManager::stop()
{
    m_rest.stopRestServer();
    m_ds.stop();  // stop dexcom_share::_start
    m_appLogger.logDebug( "stop issued on dexcom share BG requester" );
    m_bReaderStop = true;   // stop dexshareManager::reader
    m_appLogger.logDebug( "stop issued on dexcom BG reader" );
}


///
/// \brief dexshareManager::reader - read log and cache bg readings
/// \param a_sync
/// \param a_log_bg
/// \param a_log_level
///
void dexshareManager::reader( std::function< void( const std::string &) > a_log_bg  )
{
    string strBgFormat = R"(bg reader: systime:%s,displaytime:%s,dt:%d,st:%d,wt:%d,bg:%d,trend:%d)";
    m_appLogger.logInfo( "bg reader: entering BG reader" );

    dexcom_share::vector_BG vBg;
    m_bReaderReady = true;  // not perfect, but good enough sync, monitor start in wait condition
    while( m_bReaderStop == false )
    {
        m_spMonitor->wait();   // signaled whenever data is ready
        if( m_bReaderStop == true ) continue;
        m_appLogger.logDebug( "bg reader: read bg" );
        m_ds.getBG_Reading( vBg );

        for( const auto &bg : vBg  )
        {
            auto [bRes, prevValue] = m_cache.front();
            if( bRes == true )
            {
                if( bg.DT == prevValue.DT )
                {
                    m_appLogger.logWarn( "bg reader: BG has not updated since last reading, duplicate" );
                    continue;
                }
            }
            string strSystemTime;
            string strDisplayTime;
            common::timeTickToString( bg.ST, strSystemTime );
            common::timeTickToString( bg.DT, strDisplayTime );
            a_log_bg( (boost::format( strBgFormat ) % strSystemTime % strDisplayTime % bg.DT % bg.ST % bg.WT % bg.value % static_cast<int32_t>(bg.trend)).str() );
            m_cache.push( bg );
        }
        m_appLogger.logDebug( "bg reader: read complete" );
    }
    m_appLogger.logDebug( "bg reader: exit" );
}


