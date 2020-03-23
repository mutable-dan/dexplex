#include "../include/common.h"


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
        spdlog::flush_every( std::chrono::seconds(3) );
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
        return true;
    }
    return false;
}
