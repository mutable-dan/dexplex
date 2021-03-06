#include "../include/common.h"
#include <time.h>
#include <filesystem>
#include <tuple>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace  std;
namespace fs = std::filesystem;
namespace dt = boost::gregorian;
namespace pt = boost::posix_time;

///
/// \brief logging::log::~log
///
logging::log::~log()
{
    if( true == (bool)m_pLogger )
    {
        m_pLogger->flush();
    }
}

///
/// \brief logging::log::setLogger
/// \param a_strLogName
/// \param a_strLogPath
///
void logging::log::setLogger( const std::string &a_strLogName, const std::string &a_strLogPath )
{
    if( false == (bool)m_pLogger )
    {
        m_pLogger = spdlog::daily_logger_mt( a_strLogName, a_strLogPath, 0, 0 );
    }
    if( true == (bool)m_pLogger )
    {
        //spdlog::flush_every( std::chrono::seconds(3) );
        m_pLogger->flush_on( spdlog::level::err );
    }
}

///
/// \brief logging::log::_log
/// \param a_strMessage
/// \param a_Level
/// \return
///
bool logging::log::_log( const std::string& a_strMessage, const logLevel_t a_Level )
{
    if( true == (bool)m_pLogger )
    {
        switch( a_Level )
        {
            case  logging::logLevel_t::INFO:
                m_pLogger->info( a_strMessage );
                break;
            case logging::logLevel_t::WARN:
                m_pLogger->warn( a_strMessage );
                break;
            case logging::logLevel_t::ERROR:
                m_pLogger->error( a_strMessage );
                break;
            case logging::logLevel_t::VERBOSE:
                m_pLogger->debug( a_strMessage );
                break;
            default:
                m_pLogger->error( a_strMessage );
                return false;
        }
        m_pLogger->flush();
        return true;
    }
    return false;
}

void logging::log::setLevel()
{
    switch( m_logLevel )
    {
        case logging::LOG_TYPE::INFO:
            m_pLogger->set_level(  spdlog::level::info );
            break;

        case logging::LOG_TYPE::WARN:
            m_pLogger->set_level(  spdlog::level::warn );
            break;

        case logging::LOG_TYPE::ERROR:
            m_pLogger->set_level(  spdlog::level::err );
            break;

        case logging::LOG_TYPE::VERBOSE:
            m_pLogger->set_level(  spdlog::level::debug );
            break;

        default:
            m_pLogger->set_level(  spdlog::level::info );
            break;
    }

}

///
/// \brief logging::find_log_newest
/// \note does not account for two files with same date because it should not be possible unless renamed
/// \param a_strPath
/// \return
///
std::string logging::find_log_newest( const std::string &a_strPath )
{
    dt::date dte_newest;
    fs::path path = a_strPath;
    fs::path path_newest;
    for( auto &entry : fs::directory_iterator( path ) )
    {
        if( entry.is_regular_file() )
        {
            string strFn = entry.path().filename().stem();
            if( strFn.substr( 0, 3 ) == "bg_" )
            {
                string strDatePart = strFn.substr( 3 );
                dt::date d( dt::from_simple_string( strDatePart ) );
                if( dte_newest.is_not_a_date() )
                {
                    dte_newest = d;
                    path_newest = entry.path();
                } else
                {
                    if( (d > dte_newest) && (entry.file_size() > 0) )
                    {
                        dte_newest = d;
                        path_newest = entry.path();
                    }
                }
            }
        }
    }
    //return to_iso_extended_string( path.s );
    return path_newest.string();
}


///
/// \brief common::timeTickToString - convert unix time is nanoseco to date time in ms
/// \param a_ulTimens
/// \param a_strDateTime
/// \return
///
std::string& common::timeTickToString( int64_t a_ulTimens, std::string &a_strDateTime, const char* a_pszFormat )
{
    // How do I construct an ISO 8601 datetime in C++
    // https://stackoverflow.com/questions/9527960/how-do-i-construct-an-iso-8601-datetime-in-c
    string strFormat = "%Y-%m-%dT%T%z";
    if( a_pszFormat != nullptr )
    {
        strFormat = a_pszFormat;
    }
    char buff[ sizeof("YYYY-MM-DDTHH:mm:ss.sssZ")*2 ];
    struct tm *ptm_v;
    a_ulTimens /= 1000;
    ptm_v = gmtime( static_cast<time_t*>( &a_ulTimens ) );
    strftime( buff, sizeof(buff), strFormat.c_str(), ptm_v );
    a_strDateTime = buff;
    return a_strDateTime;
}

///
/// \brief common::dexcomNextReadDelay
/// \note get last display time and add 5 minutes to get the next expected time
///       expectedtime - currenttime -> seconds to next read, then pad with a few seconds
/// \param a_ulSystemTime
/// \return
///
auto common::secondsToNextRead( uint64_t a_ulDispTime ) -> std::tuple<uint64_t, std::string, std::string>
{
    // dt:1586621356000 milli sec timestamp
    // dt:1586621356 sec timestamp
    //dt::date  dt( a_ulSystemTime/1000 );
    pt::ptime ptCurrentTime( pt::second_clock::local_time() );
    // pt::ptime ptCurrentTime( pt::second_clock::universal_time() );
    pt::ptime ptLastRead( pt::from_time_t( a_ulDispTime/1000 ) );

    pt::ptime nextExPectedReadTime = ptLastRead + pt::minutes( 5 );   // last disp read + 5min
    pt::time_duration secToNextRead = nextExPectedReadTime - ptCurrentTime;
    secToNextRead += pt::seconds( 10 );

    #ifndef NDEBUG
        string strValue;
        strValue = pt::to_iso_extended_string( ptCurrentTime );
        strValue = pt::to_iso_extended_string( ptLastRead );
        strValue = pt::to_iso_extended_string( nextExPectedReadTime );
        (void)strValue;
    #endif

    // handle error from svr where date becomes 1970
    if( (secToNextRead.total_seconds() > 600) || (secToNextRead.total_seconds() < 0 ) )
    {
        secToNextRead = pt::seconds( 20 );
    }

    return std::make_tuple( secToNextRead.total_seconds(), pt::to_iso_extended_string( ptCurrentTime ), pt::to_iso_extended_string( ptLastRead ) );
}


int32_t common::countOfMisssing( const int64_t a_dataLastRead )
{
    pt::ptime ptCurrentTime( pt::second_clock::universal_time() );
    pt::ptime ptLastRead( pt::from_time_t( a_dataLastRead/1000 ) );
    pt::time_duration secondsElapsed = ptCurrentTime - ptLastRead;
    #ifndef NDEBUG
        string strValue;
        strValue = pt::to_iso_extended_string( ptCurrentTime );
        strValue = pt::to_iso_extended_string( ptLastRead );
        (void)strValue;
    #endif
    return ( secondsElapsed.total_seconds() /  pt::minutes( 5 ).total_seconds() );
}

///
/// \brief common::dropPrivleges drop root and use new
/// \note or use authbind
/// \param a_newPriv
/// \return
///
bool common::dropPrivleges( const uint16_t a_newPriv )
{

    auto gid = getgid();
    auto uid = getuid();
    if( (gid ==0) && (uid == 0) )
    {
        if (setgid( a_newPriv ) == -1)
        {
            /* handle error condition */
            return false;
        }
        if (setuid( a_newPriv ) == -1)
        {
            /* handle error condition */
            return false;
        }
        return true;
    } else
    {
        return false;
    }

}
