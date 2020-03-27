#include "../include/dex-mgr.h"

#include <boost/format.hpp>

using namespace std;


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
    m_log = a_log;

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
        m_log.logError( (boost::format( "error %s reading config file") % sstr.str()).str() );
        return false;
    }

    m_sp = make_shared<sync_tools::monitor>();   /// moditor for syncing glucose writer and reader
    m_log.logInfo( "starting reader" );
    thread thd( &dexshareManager::reader, this, a_logbg );
    while( m_bReaderReady == false )
    {
        std::this_thread::yield( ); // spin until reader is ready
    }
    m_thdReader = std::move( thd );
    m_ds.userName( strAccount );
    m_ds.password( strPassword );
    m_ds.accoundId( strApplicationId );

    m_log.logInfo( "starting dexshare" );
    m_ds.start( m_sp, m_log );
    return true;
}


///
/// \brief dexcom_share::wait
///
void dexshareManager::wait()
{
    m_ds.wait();   // wait for dexcom_share::_start to end
    m_log.logDebug( "dexshare closing" );
    m_sp->signal();      // issue a sig to dexshareManager::reader in case in wait
    m_thdReader.join();  // wait for dexshareManager::reader to stop
    m_log.logDebug( "reader ended" );
    return;
}


///
/// \brief dexshareManager::stop - signal start thd to stop
///
void dexshareManager::stop()
{
    m_ds.stop();  // stop dexcom_share::_start
    m_log.logDebug( "stop issued on dexcom share BG requester" );
    m_bReaderStop = true;   // stop dexshareManager::reader
    m_log.logDebug( "stop issued on dexcom BG reader" );
}



///
/// \brief dexshareManager::reader - read log and cache bg readings
/// \param a_sync
/// \param a_log_bg
/// \param a_log_level
///
void dexshareManager::reader( std::function< void( const std::string &) > a_log_bg  )
{
    string strBgFormat = R"(dt:%d,st:%d,wt:%d bg:%d,trend:%d)";
    m_log.logInfo( "entering BG reader" );

    dexcom_share::vector_BG vBg;
    m_bReaderReady = true;  // not perfect, but good enough sync, monitor start in wait condition
    while( m_bReaderStop == false )
    {
        m_sp->wait();   // signaled whenever data is ready
        if( m_bReaderStop == true ) continue;
        m_log.logDebug( "read bg" );
        m_ds.getBG_Reading( vBg );

        for( const auto &bg : vBg  )
        {
            a_log_bg( (boost::format( strBgFormat ) % bg.dt % bg.st % bg.wt % bg.bg % static_cast<int32_t>(bg.trend)).str() );
        }
        m_log.logDebug( "read complete" );
    }
    m_log.logDebug( "exit reader" );
}
