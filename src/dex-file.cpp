#include "../include/dex-file.h"

#include <string>
#include <future>
#include <fstream>
#include "cpr/async.h"

using namespace std;

bool dexcom_file::setFile( const std::string &a_strPath )
{
   m_path = a_strPath;
   m_bIsValidPath = filesystem::is_regular_file( m_path );
   return m_bIsValidPath;
}



bool dexcom_file::start( std::shared_ptr<sync_tools::monitor> a_pSync, logging::log &a_log, int64_t a_lastReadDate )
{
   m_lastReadDate = a_lastReadDate;
   a_log.logInfo( "dex file stub called" );
   thread thd( &dexcom_file::_start, this, a_pSync, a_log );
   m_thdDexcomFile = std::move( thd );
   return true;
}

void dexcom_file::_start( std::shared_ptr<sync_tools::monitor> a_pSync, logging::log a_log )
{
   //a_log.logInfo( "bg loop: dex file started" );
   while ( m_bStop == false )
   {
      auto [bRes, ulDispDate, strReturnInfo] = dexcomFileData();

   }
}


auto dexcom_file::dexcomFileData() -> std::tuple<bool, uint64_t, std::string>
{
   string strReturnMessage;
   uint64_t ulLastDispDate = 0;
   if( m_bIsValidPath )
   {
      if( filesystem::exists( m_path ) )
      {
         char szBuffer[1024];
         ifstream inf( m_path, ios::in );
         if( inf.is_open() )
         {
            inf.getline( szBuffer, sizeof(szBuffer) );
         }
      }

   } else
   {
      string strError( "invalid path" );
      return std::make_tuple( true, ulLastDispDate, std::move(strError) );
   }

   return std::make_tuple( true, ulLastDispDate, std::move(strReturnMessage) );
}

bool dexcom_file::getBG_Reading( vector_BG &a_vBg )
{
   return true;
}
