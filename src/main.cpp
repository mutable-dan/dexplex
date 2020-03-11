#include <iostream>
#include <queue>
#include <algorithm>
#include <sstream>

#include <libdaemon/daemon.h>
#include <mutlib/config.h>
#include <mutlib/logger.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
   #include <spdlog/spdlog.h>
   #include <spdlog/sinks/daily_file_sink.h>
#pragma GCC diagnostic pop


using namespace std;

void display();


int main( int argc, char* argv[] )
{
   spdlog::set_level( spdlog::level::debug );
   auto dl =spdlog::daily_logger_mt( "daily", "logs/dexmux.log", 0, 0 );
   spdlog::info( "{} starting", argv[0] );
   dl->info( "{} starting", argv[0] );

   mutlib::config cfg;
   string strAccount;
   string strPassword;
   string strApplicationId;
   bool bComplete = true;
   stringstream sstr;
   sstr << "errors processing config file: ";
   if( cfg.read( "dex.config" ) )
   {
      if( false == cfg.get( "account", strAccount ) ) 
      {
         bComplete = false;
         sstr << " missing account";
      }
      if( false == cfg.get( "password", strPassword ) ) 
      {
         bComplete = false;
         sstr << " missing passowrd";
      }
      if( false == cfg.get( "applicationid", strApplicationId ) ) 
      {
         bComplete = false;
         sstr << " missing applicationid";
      }
   }

   if( bComplete == false )
   {
      dl->error( "config info missing: {}", sstr.str() );
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
         cout << "run in foreground" << endl << endl;
         // start prog body
         app.runForeground();
      } else
      {
         cout << "run as daemon" << endl << endl;
         if( true == bDebug )
         {
            cout << "stdio and srderr not redirected" << endl;
         }
         app.runDaemon( false, bDebug );
         cout << "start" << endl;

         // start prog body async
         app.wait();
         cout << "stop running" << endl;
         // issue a stop to any threads in prog body
      }
   } 
   else 
   if( qCommands.front() == "stop" )
   {
      cout << "stop" << endl;
      qCommands.pop();
      app.stop();
   }
   else
   {
      display();
   }
   
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


