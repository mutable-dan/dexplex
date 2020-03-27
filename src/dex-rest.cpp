#include "../include/dex-rest.h"
#include <sstream>
#include <future>
#include <chrono>
#include <exception>

#include <boost/format.hpp>

#include "../../json/include/nlohmann/json.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Weffc++"
   // https://github.com/whoshuu/cpr/
   #include <cpr/cpr.h>
#pragma GCC diagnostic pop


using namespace std;


///
/// \brief dexcom_share::login
/// get sesssion of from authentication, sessioid is used in subsequent calls
///
/// \return
///
bool dexcom_share::login()
{
   m_strSessionId.clear();
   if( m_strUserName.empty() || m_strPassword.empty() || m_strAccoundId.empty() )
   {
      error( "incomplete credientials: user,pwd or accoundId" );
      return false;
   }
   string strAuth = R"({"accountName" : "%s", "password" : "%s", "applicationId" : "%s"})";
   stringstream sstr;
   sstr << boost::format( strAuth ) % m_strUserName % m_strPassword % m_strAccoundId;

   string strUrl = m_strShareUrlbase + m_strShareLogin;
   nlohmann::json js;
   try
   {
      js = nlohmann::json::parse( sstr.str().c_str() );
   } catch( nlohmann::json::parse_error &pe )
   {
      stringstream sstrErr;
      sstrErr << "error parsing to json:" << sstr.str() << ", login failed" << pe.what();
      error( sstrErr.str() );
      return false;
   }

   // make request
   
   cpr::Response response;
   try
   {
      response = cpr::Post( 
            cpr::Url{ strUrl }, 
            cpr::Header{ {"User-Agent", "Dexcom Share/3.0.2.11 CFNetwork/711.2.23 Darwin/14.0.0"} }, 
            cpr::Header{ {"Content-Type", "application/json"} },
            cpr::Body( js.dump() ) );
   } catch( std::exception &e )
   {
      stringstream sstrErr;
      sstrErr << "error posting:" << sstr.str() << ", reading failed" << e.what();
      error( sstrErr.str() );
      return false;
   }

   m_nLoginStatusCode = response.status_code;
   if( (m_nLoginStatusCode != m_cnHttpOk) )
   {
      stringstream sstrErr;
      sstrErr << "error posting:" << response.status_code << ", " << response.text << ", reading failed";
      error( sstrErr.str() );
      return false;
   }

   // remove quotes around sessionid
   m_strSessionId = response.text;
   m_strSessionId.erase( m_strSessionId.begin() );
   m_strSessionId.erase( m_strSessionId.end()-1 );
   m_bLoggedIn = true;
   return true;
}



///
/// \brief dexcom_share::dexcomShareData, get blood glucose (bg) value, use session id
/// \return
///
bool dexcom_share::dexcomShareData()
{
   if( (m_bLoggedIn == false) || (m_strSessionId.length() == 0) )
   {
      return false;
   }
   string strUrl      = m_strShareUrlbase + m_strShareGetBG;
   string strMinues   = std::to_string( m_nMinutes );
   string strMaxCount = std::to_string( m_nMaxCount );

   cpr::Response response;
   try
   {
      response = cpr::Post( 
            cpr::Url{ strUrl }, 
            cpr::Parameters{ { "sessionId", m_strSessionId  }, { "minutes", strMinues }, { "maxcount", strMaxCount } },
            cpr::Body( "" )  // without body, content-length is not sent
            );
   } catch( std::exception &e )
   {
      stringstream sstrErr;
      sstrErr << "error posting:" << response.text << ", BG reading failed" << e.what();
      error( sstrErr.str() );
      return false;
   }
   if( response.status_code != m_cnHttpOk )
   {
      stringstream sstrErr;
      sstrErr << "error posting:" << response.status_code << ", " << response.text << ", reading failed";
      error( sstrErr.str() );
      return false;
   }

   nlohmann::json js_results;
   try
   {
      js_results = nlohmann::json::parse( response.text );
   } catch( nlohmann::json::parse_error &pe )
   {
      stringstream sstrErr;
      sstrErr << "error parsing to json:" << response.text << ", bg reading failed" << pe.what();
      error( sstrErr.str() );
      return false;
   }

   // protect m_vReadings 
   lock_guard<std::mutex> lg( m_muxBG );

   bg_t bg_value;
   for( auto &[key, value] : js_results.items() )
   {
       string  strDt;    
       string  strSt;    
       string  strWt;    
       int32_t nBG;    
       int32_t nTrend;

       try
       {
          strDt    = value[ "DT" ];
          strSt    = value[ "ST" ];
          strWt    = value[ "WT" ];
          nBG      = value[ "Value" ];
          nTrend   = value[ "Trend" ];
       } catch( std::exception &e )
       {
          stringstream sstr;
          sstr << "bad avlue" << value;
          error( sstr.str() );
          continue;
       }
      

       const int32_t cnRemoveDate = 6;
       strDt.erase( strDt.begin(), strDt.begin()+cnRemoveDate );
       strSt.erase( strSt.begin(), strSt.begin()+cnRemoveDate );
       strWt.erase( strWt.begin(), strWt.begin()+cnRemoveDate );
       
       const int32_t cnRemoveMillisec = 13;
       strDt.erase( strDt.begin()+cnRemoveMillisec, strDt.end() );
       strSt.erase( strSt.begin()+cnRemoveMillisec, strSt.end() );
       strWt.erase( strWt.begin()+cnRemoveMillisec, strWt.end() );

       bg_value.dt    = stoll( strDt );
       bg_value.st    = stoll( strSt );
       bg_value.wt    = stoll( strWt );
       bg_value.bg    = nBG;
       bg_value.trend = nTrend;

       m_vReadings.push_back( bg_value );
   }

   return true;
}


///
/// \brief dexcom_share::getBG_Reading, block vReadings and return a copy
/// \param a_vbg
/// \return
///
bool dexcom_share::getBG_Reading( dexcom_share::vector_BG &a_vbg )
{
   lock_guard<std::mutex> lg( m_muxBG );
   a_vbg = m_vReadings;
   m_vReadings.clear();
   return true;
}


///
/// \brief dexcom_share::start - call private start method async
/// \return
///
bool dexcom_share::start( shared_ptr<sync_tools::monitor> a_pSync, logging::log &a_log )
{
   thread thd( &dexcom_share::_start, this, a_pSync, a_log );
   m_thd = std::move( thd );
   return true;
}


///
/// \brief dexcom_share::_start - login and get bg values and
///
void dexcom_share::_start( shared_ptr<sync_tools::monitor> a_pSync, logging::log a_log )
{
   bool bLoggedIn  = false;
   while( m_bStop == false )
   {
      if( false == bLoggedIn ) 
      {
         auto thdLogin = async( &dexcom_share::login, this );
         future_status status = thdLogin.wait_for( chrono::seconds( m_nReqTimeout_sec ) );  
         if( status == future_status::timeout )
         {
            a_log.logError( "login request timed out" );
            continue; 
         }
         bLoggedIn = thdLogin.get();
         if( isError() )
         {
            for( auto &item : errors() )
            {
                a_log.logError( (boost::format( "login %s" ) % item).str() );
            }
            clearErrors();
         }
      }

      if( bLoggedIn == true )
      {
         // get BG
         // check for missing data and ajust
         auto thdBG = async( &dexcom_share::dexcomShareData, this );
         future_status status = thdBG.wait_for( chrono::seconds( m_nReqTimeout_sec ) );
         if( status == future_status::timeout )
         {
            // thread is stuck, abandon and start again but log so a fix can be found
            a_log.logError( "BG request timed out" );
            bLoggedIn = false;  // assume for now that a failure means need to re-loggin
         }
         if( thdBG.get() == false )
         {
             if( isError() )
             {
                for( auto &item : errors() )
                {
                    // errors from BG request
                    a_log.logError( (boost::format( "BG request %s" ) % item).str() );
                }
                clearErrors();
             }
            bLoggedIn = false;
         } else
         {
             a_pSync->signal();
         }

      } else
      {
         a_log.logError( "login failed, will try again" );
      }

      int32_t nRepeat = m_nShareCheckInterval * 60 / 5;  // number of 5s sleeps
      while( --nRepeat > 0 )
      {
         this_thread::sleep_for( chrono::seconds( 5 ) );
         if( true ==  m_bStop )
         {
            break;
         }
      }
   }
}
