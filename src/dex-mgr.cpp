#include "../include/dex-mgr.h"
#include <sstream>
#include <boost/format.hpp>

using namespace std;


///
/// \brief dexshareManager::start - run rest client on an interval
/// \param a_cfg
/// \param a_logbg
/// \param a_logLevel
/// \return
///
bool dexshareManager::start( mutlib::config &a_cfg, std::function< void( const std::string &) > &a_logbg,
                                                    std::function< void( const std::string &, const logging::logLevel_t ) > &a_logLevel )
{
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

   if( bComplete == false )
   {
     a_logLevel( sstr.str(), logging::logLevel_t::ERROR );
      return -1;
   }

   m_ds.userName( strAccount );
   m_ds.password( strPassword );
   m_ds.accoundId( strApplicationId );
   m_ds.start();
   m_ds.wait();  


   dexcom_share::vector_BG vBg;
   m_ds.getBG_Reading( vBg );

   stringstream sstr1;
   string strBgFormat = R"(dt:%d,st:%d,wt:%d bg:%d,trend:%d)";
   for( const auto &bg : vBg  )
   {
      sstr1 << boost::format( strBgFormat ) % bg.dt % bg.st % bg.wt % bg.bg % static_cast<int32_t>(bg.trend);
      a_logbg( sstr1.str() );
      sstr1.str( std::string() );
   }
   return true;
}

///
/// \brief dexshareManager::stop - signal start thd to stop
///
void dexshareManager::stop()
{
   m_ds.stop();
}
