#include "../include/dex-file.h"

#include <csignal>
#include <string>
#include <future>
#include <string.h>
#include <iostream>

#include "cpr/async.h"

using namespace std;

sem_t dexcom_file::m_readerSem;

dexcom_file::~dexcom_file()
{
   if( m_if.is_open() )
   {
      m_if.close();
   }
}

bool dexcom_file::setFile( const std::string &a_strPath )
{
   m_path = a_strPath;
   m_bIsValidPath = filesystem::is_regular_file( m_path );
   return m_bIsValidPath;
}



bool dexcom_file::start( std::shared_ptr<sync_tools::monitor> a_pSync, logging::log &a_log, int64_t a_lastReadDate )
{
   if( m_bIsValidPath )
   {
      if( filesystem::exists( m_path ) )
      {
         m_if.open( m_path, ios::in );
         if( m_if.is_open() )
         {
            m_lastReadDate = a_lastReadDate;
            a_log.logInfo( "dex file stub called" );
            thread thd( &dexcom_file::_start, this, a_pSync, a_log );
            m_thdDexcomFile = std::move( thd );
            return true;
         }
      }
   }
   return false;
}

void dexcom_file::_start( std::shared_ptr<sync_tools::monitor> a_pSync, logging::log a_log )
{
   a_log.logInfo( "bg loop: dex file started" );
   sem_init( &dexcom_file::m_readerSem, 0, 0 );
   signal( SIGUSR1, dexcom_file::signal_handler );

   while ( m_bStop == false )
   {
      dexcomFileData();
      a_pSync->signal();  // signal reader
      sem_wait( &dexcom_file::m_readerSem );
      //cout << "file reader signal" << endl;
   }
   sem_destroy( &dexcom_file::m_readerSem );
}


//auto dexcom_file::dexcomFileData() -> std::tuple<bool, uint64_t, std::string>
void dexcom_file::dexcomFileData(  )
{
   data::bg_data bg_value;
   string strReturnMessage;
   uint64_t ulLastDispDate = 0;

   char szBuffer[8192];

   if( !m_if.eof() )
   {
      if( m_if.bad() || m_if.fail() )
      {
         strReturnMessage = "file descripter fail";
      }
      memset( szBuffer, 0, sizeof( szBuffer ) );
      m_if.getline( szBuffer, sizeof(szBuffer) );
      szBuffer[m_cnDT_End] = 0;
      string strDT = szBuffer+m_cnDT_Start;
      szBuffer[m_cnST_End] = 0;
      string strST = szBuffer+m_cnST_Start;
      szBuffer[m_cnWT_End] = 0;
      string strWT = szBuffer+m_cnWT_Start;

      // bg can be 2 or 3 chars
      // bg:66,trend:4
      // bg -> position 165 -> find ,
      char* pBG_start = strchr( szBuffer+161, ':' ) + 1;
      char* pBG_end   = strchr( szBuffer+161, ',' );
      *pBG_end = 0;
      string strBG = pBG_start;
      string strTrend = "0";  // ignore

      bg_value.strDT = strDT;
      bg_value.strST = strST;
      bg_value.strWT = strWT;
      bg_value.DT = stoll( strDT );
      bg_value.ST = stoll( strST );
      bg_value.WT = stoll( strWT );
      bg_value.value = stoi( strBG );
      bg_value.trend = 4;

      if( bg_value.DT > m_lastReadDate )
      {
         m_lastReadDate = bg_value.ST;
      }
      m_vReadings.push_back( bg_value );
   }

   //return std::make_tuple( true, bg_value.DT, std::move(strReturnMessage) );
}

bool dexcom_file::getBG_Reading( vector_BG &a_vBg )
{
   lock_guard<std::mutex> lg( m_muxBG );
   a_vBg = std::move( m_vReadings );
   return true;
}


void dexcom_file::signal_handler( const int32_t a_iSigNum )
{
   //cout << "dexcom_file::signal_handler()" << endl;
   sem_post( &dexcom_file::m_readerSem );
}
