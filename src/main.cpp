#include <iostream>
#include <queue>
#include <algorithm>
#include <functional>
#include <sstream>

#include <libdaemon/daemon.h>
#include <mutlib/config.h>
#include <mutlib/logger.h>

#include "../include/common.h"
#include "../include/dex-mgr.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
   // https://github.com/gabime/spdlog
   #include <spdlog/spdlog.h>
   #include <spdlog/sinks/daily_file_sink.h>
#pragma GCC diagnostic pop


using namespace std;

void display();

int main( int argc, char* argv[] )
{
   spdlog::set_level( spdlog::level::debug );
   auto dl   = spdlog::daily_logger_mt( "daily", "logs/dexmux.log", 0, 0 );
   auto bglog = spdlog::daily_logger_mt( "bg", "logs/bg.log", 0, 0 );   // blood glucose log

   spdlog::flush_every( std::chrono::seconds(3) );
   dl->flush_on( spdlog::level::err );

   spdlog::info( "{} starting", argv[0] );
   dl->info( "{} starting", argv[0] );

   // check shared ptr for logging
   if( false == (bool)dl )
   {
      std::cerr << "logging failed to start" << endl;
      return -1;
   }

   if( false == (bool)bglog )
   {
      std::cerr << "ibg logging failed to start" << endl;
      return -1;
   }
   // log check done

   /////////////////////////////////////////
   ///  setup logging callbacks
   // write bg to log
   auto loggingBG = [&] ( const std::string &a_strMessage ) -> void
   {
      bglog->info( a_strMessage );
   };
   std::function< void( const std::string& ) > fn_bgLog = loggingBG;

   // write app logging
   auto logging = [&] ( const std::string &a_strMessage, const logging::logLevel_t a_level ) -> void
   {
       switch( a_level )
       {
           case logging::logLevel_t::INFO:
               dl->info( a_strMessage );
               break;
           case logging::logLevel_t::WARN:
               dl->warn( a_strMessage );
               break;
           case logging::logLevel_t::ERROR:
               dl->error( a_strMessage );
               break;
           case logging::logLevel_t::VERBOSE:
               dl->debug( a_strMessage );
               break;
           default:
               dl->error( a_strMessage );
       }
   };
   std::function< void( const std::string &, const logging::logLevel_t) > fn_log = logging;
   /////////////////////////////////////////
   // callbacks done

   mutlib::config cfg;
   if( !cfg.read( "dex.config" ) )
   {
      dl->error( "error opening config file:" );
      return -1;
   }

   dl->info( "config file read ok" );


   tools::Daemon app;
   queue<string> qCommands;
   app.setName( string( argv[0] ) );
   
   if( 1 == argc )
   {
      display();
      return -1;
   }

   for( int32_t nIndex=1; nIndex<argc; ++nIndex )
   {
      string strCommand = argv[nIndex];
      transform( strCommand.begin(), strCommand.begin(), strCommand.end(), ::tolower );
      qCommands.push( strCommand );
   }    
   
   
   if( qCommands.front() == "start" )
   {
      qCommands.pop();
      bool bRunForeground( false ) ;
      bool bDebug( false );
      
      if( !qCommands.empty() ) 
      {
         if( qCommands.front() == "-f" )
         {
            bRunForeground = true;
            qCommands.pop();
         }
      }
      if( !qCommands.empty() ) 
      {
         if( qCommands.front() == "-d" )
         {
            bDebug = true;
            qCommands.pop();
         }
      }
      

      if( true == bRunForeground )
      {
         dl->info( "running in foreground" );
         // start prog body
         dexshareManager dsm;
         dsm.start( cfg, fn_bgLog, fn_log );
         dl->info( "waiting" );
         app.runForeground();
         dsm.stop();
         dsm.wait();
         dl->info( "done" );
      } else
      {
         dl->info( "running as daemon" );
         if( true == bDebug )
         {
            dl->info( "stdio and srderr not redirected" );
         }
         app.runDaemon( false, bDebug );

         // start prog body async
         dexshareManager dsm;
         if( true == dsm.start( cfg, fn_bgLog, fn_log ) )
         {
             dl->info( "started, main in wait state" );
             app.wait();
             dl->info( "shut down completing" );
         } else
         {
             dl->error( "failed to start" );
         }
         // issue a stop to any threads in prog body
      }
   } 
   else 
   if( qCommands.front() == "stop" )
   {
      dl->info( "stop issued" );
      qCommands.pop();
      app.stop();
   }
   else
   {
      display();
   }

   dl->info( "done" );
   dl->flush();
   bglog->flush();
   return 0;
}



void display()
{
   cerr << " dexcom share: start|stop [options] " << endl;
   cerr << " start runs as a daemon" << endl;
   cerr << " stop signals daemon to shutdown, can use ctrl-c or stop to stop a foreground app" << endl;
   cerr << " where options:" << endl;
   cerr << "    -f  run app in foreground and do not redirect stdout and stderr" << endl;
   cerr << "    -d  do not redirect stdout and stderr - usefull for running app as daemon with console output" << endl;
}


