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

//
// get sesssion of from authentication
// sessioid is used in subsequent calls
// ----------------------------------------------------------------------------------------
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


//
// get BG
// ----------------------------------------------------------------------------------------
bool dexcom_share::dexcomShareData()
{
   if( (m_bLoggedIn == false) || (m_strSessionId.length() == 0) )
   {
      return false;
   }
   string strUrl = m_strShareUrlbase + m_strShareGetBG;
   string strMinues = "1440";
   string strMaxCount = "3";

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
   m_vReadings.clear();
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

//
// 
// ----------------------------------------------------------------------------------------
bool dexcom_share::getBG_Reading( dexcom_share::vector_BG &a_vbg )
{
   lock_guard<std::mutex> lg( m_muxBG );
   a_vbg = m_vReadings;
   return true;
}

//
// 
// ----------------------------------------------------------------------------------------
bool dexcom_share::start()
{
   auto thdLogin = async( &dexcom_share::login, this );
   future_status status = thdLogin.wait_for( chrono::seconds( m_nReqTimeout_sec ) );  // change to wait_for
   if( status == future_status::timeout )
   {
      error( "login request timed out" );
      return false;
   }

   bool bRes = thdLogin.get();
   if( bRes == true )
   {
      bRes = false;
      // get BG
      auto thdBG = async( &dexcom_share::dexcomShareData, this );
      status = thdBG.wait_for( chrono::seconds( m_nReqTimeout_sec ) );
      if( status == future_status::timeout )
      {
         error( "BG request timed out" );
         return false;
      }

      bRes = thdBG.get();
      if( bRes == false )
      {
         error( "BG request failed" );
         return false;
      }
   } else
   {
      // login failed
      error( "login failed" );
      return false;
   }

   return true;
}
