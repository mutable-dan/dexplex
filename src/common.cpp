#include "../include/common.h"
#include <time.h>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <filesystem>

using namespace  std;
namespace fs = std::filesystem;
namespace dt = boost::gregorian;

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


///
/// \brief common::timeTickToString - convert unix time is nanoseco to date time in ms
/// \param a_ulTimens
/// \param a_strDateTime
/// \return
///
std::string& common::timeTickToString( int64_t a_ulTimens, std::string &a_strDateTime, const char* a_pszFormat )
{
    string strFormat = "%Y-%m-%d %T%z";
    if( a_pszFormat != nullptr )
    {
        strFormat = a_pszFormat;
    }
    char buff[100];
    struct tm *ptm_v;
    a_ulTimens /= 1000;
    ptm_v = gmtime( static_cast<time_t*>( &a_ulTimens ) );
    strftime( buff, sizeof(buff), strFormat.c_str(), ptm_v );
    a_strDateTime = buff;
    return a_strDateTime;
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
