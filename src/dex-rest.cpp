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
namespace json = nlohmann;

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
      sstrErr << "login parse - error parsing to json:" << sstr.str() << ", login failed" << pe.what();
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
      sstrErr << "login post - error posting:" << sstr.str() << ", reading failed" << e.what();
      error( sstrErr.str() );
      return false;
   }

   m_nLoginStatusCode = response.status_code;
   if( (m_nLoginStatusCode != m_cnHttpOk) )
   {
      stringstream sstrErr;
      sstrErr << "login error code - error posting:" << response.status_code << ", " << response.text << ", reading failed";
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
auto dexcom_share::dexcomShareData()
{
   if( (m_bLoggedIn == false) || (m_strSessionId.length() == 0) )
   {
      return std::make_tuple( false, 0LU );
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
      sstrErr << "bg request - error posting:" << response.text << ", BG reading failed" << e.what();
      error( sstrErr.str() );
      return std::make_tuple( false, 0LU );
   }
   if( response.status_code != m_cnHttpOk )
   {
      stringstream sstrErr;
      sstrErr << "bg error code - error:" << response.status_code << ", " << response.text << ", reading failed";
      error( sstrErr.str() );
      return std::make_tuple( false, 0LU );
   }

   json::json js_results;
   // sample response
   // "[{\"DT\":\"\\/Date(1587664805000+0000)\\/\",\"ST\":\"\\/Date(1587679205000)\\/\",\"Trend\":5,\"Value\":74,\"WT\":\"\\/Date(1587679205000)\\/\"}]"
   try
   {
      js_results = json::json::parse( response.text );
   } catch( json::json::parse_error &pe )
   {
      stringstream sstrErr;
      sstrErr << "bg parse - error parsing to json:" << response.text << ", bg reading failed" << pe.what();
      error( sstrErr.str() );
      return std::make_tuple( false, 0LU );
   }

   // protect m_vReadings 
   lock_guard<std::mutex> lg( m_muxBG );

   data::bg_data bg_value;
   uint64_t ulLastDispDate = 0;
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
          sstr << "bg bad value" << value;
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

       bg_value.DT    = stoll( strDt );
       bg_value.ST    = stoll( strSt );
       bg_value.WT    = stoll( strWt );
       bg_value.value = nBG;
       bg_value.trend = nTrend;
       ulLastDispDate = bg_value.DT;

       m_vReadings.push_back( bg_value );
   }
   return std::make_tuple( true, ulLastDispDate );
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
bool dexcom_share::start( shared_ptr<sync_tools::monitor> a_pSync, logging::log& a_log )
{
   a_log.logInfo( "dex shared stub called" );
   thread thd( &dexcom_share::_start, this, a_pSync, a_log );
   m_thdDexcomShare = std::move( thd );
   return true;
}

///
/// \brief dexcom_share::_start - login and get bg values and
///
void dexcom_share::_start( shared_ptr<sync_tools::monitor> a_pSync, logging::log a_log )
{
   a_log.logInfo( "dex shared started" );
   bool bLoggedIn  = false;
   uint64_t ulLastDispDate = 0;

   while( m_bStop == false )
   {
      if( false == bLoggedIn ) 
      {
         a_log.logInfo( "logging in" );
         auto thdLogin = async( &dexcom_share::login, this );
         future_status status = thdLogin.wait_for( chrono::seconds( m_nReqTimeout_sec ) );  
         if( status == future_status::timeout )
         {
            a_log.logError( "login request timed out" );
            continue; 
         }
         bLoggedIn = thdLogin.get();
         if( true == bLoggedIn ) a_log.logInfo( "logged in" );
         if( isError() )
         {
            for( auto &item : errors() )
            {
                a_log.logError( (boost::format( "login %s" ) % item).str() );
            }
            clearErrors();
         }
      }

      uint64_t secondsToNextRead;
      if( bLoggedIn == true )
      {
         // get BG
         // check for missing data and ajust
         a_log.logInfo( "getting BG" );
         auto thdBG = async( &dexcom_share::dexcomShareData, this );
         future_status status = thdBG.wait_for( chrono::seconds( m_nReqTimeout_sec ) );
         if( status == future_status::timeout )
         {
            // thread is stuck, abandon and start again but log so a fix can be found
            a_log.logError( "BG request timed out" );
            bLoggedIn = false;  // assume for now that a failure means need to re-loggin
         }
         auto [bRes, ulDispDate] = thdBG.get();
         // if new disp date then calc next read time, else pause and read again
         if( (ulLastDispDate != ulDispDate) )
         {
             ulLastDispDate = ulDispDate;
             auto [secondsToNextRead_tmp, strCurrentTime, strLastReadTime] = common::secondsToNextRead( ulDispDate );
             secondsToNextRead = secondsToNextRead_tmp;
             a_log.logInfo( (boost::format( "new reading: current time:%s, displaytime:%s, seconds to next read:%d" ) % strCurrentTime % strLastReadTime % secondsToNextRead).str() );
         } else
         {
             secondsToNextRead = 15;
             a_log.logInfo( (boost::format( "no reading: seconds to next read:%d" ) % secondsToNextRead).str() );
         }

         if( bRes == false )
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
             a_log.logInfo( "received BG, signaling reader" );
             a_pSync->signal();
         }
      } else
      {
         a_log.logError( "login failed, will try again" );
         secondsToNextRead = 20;
      }

      int32_t nRepeat = (secondsToNextRead / 5) + (secondsToNextRead % 5);  // number of 5s sleeps
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
