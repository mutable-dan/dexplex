#include <iostream>
#include <queue>
#include <algorithm>
#include <functional>
#include <sstream>
#include <boost/format.hpp>

#include <daemon/daemon.h>
#include <mutlibs/config.h>
#include <sys/types.h>

#include "../include/common.h"
#include "../include/dex-mgr.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#pragma GCC diagnostic pop


using namespace std;


int main( int argc, char* argv[] )
{
    auto display = []()
    {
        cerr << " dexcom share: start|stop [options] " << endl;
        cerr << " start runs as a daemon" << endl;
        cerr << " stop signals daemon to shutdown, can use ctrl-c or stop to stop a foreground app" << endl;
        cerr << " where options:" << endl;
        cerr << "    -f  run app in foreground and do not redirect stdout and stderr" << endl;
        cerr << "    -d  do not redirect stdout and stderr - usefull for running app as daemon with console output" << endl;
    };
    const char pszConfig[] = "dex.config";

    // read config file
    mutlib::config cfg;
    if( !cfg.read( pszConfig ) )
    {
        cerr << "error opening config file:" << pszConfig << endl;
        return -1;
    }
    string strLogPath;
    if( false == cfg.get( "logpath", strLogPath ) )
    {
        strLogPath = "logs/";
    }
    string strLogLevel;
    if( false == cfg.get( "loglevel", strLogLevel ) )
    {
        strLogLevel = "info";
    }


    logging::log appLog;
    auto bglog = spdlog::daily_logger_mt( "bg", strLogPath + "/bg.log", 0, 0 );   // blood glucose log
    appLog.setLogger( "daily", strLogPath + "/dexplex.log" );                      // app logging

    appLog.setLevelInfo();  // NOTE: must be set from config
    appLog.logInfo( "********* starting service *********" );
    appLog.logInfo( (boost::format( "config log level requested %s" ) % strLogLevel).str() );
    appLog.logInfo( (boost::format( "config log path %s" ) % strLogPath).str() );

    string strPort;
    string strAccount;
    if( true == cfg.get( "port", strPort ) )
    {
        appLog.logInfo( (boost::format( "config port %s" ) % strPort).str() );
    }
    if( true == cfg.get( "account", strAccount ) )
    {
        appLog.logInfo( (boost::format( "config account %s" ) % strAccount).str() );
    }


    // set log level from config
    if( strLogLevel == "warn"  )
    {
        appLog.setLevelWarning();
        appLog.logInfo( "Log level set to warn" );
    }
    if( strLogLevel == "error" )
    {
        appLog.setLevelError();
        appLog.logInfo( "Log level set to error" );
    }
    if( strLogLevel == "debug" )
    {
        appLog.setLevelDebug();
        appLog.logInfo( "Log level set to debug" );
    }
    // *******************

    //check if logging is ready else stop
    if( false == appLog.isReady() )
    {
        std::cerr << "app logging failed to start" << endl;
        return -1;
    }
    if( false == (bool)bglog )
    {
        std::cerr << "blood glucose logging failed to start" << endl;
        return -1;
    }
    // log check done

    /////////////////////////////////////////
    ///  setup logging callback - can use the loginteface class, lets use std::function, they're fun and this is a fun app
    // write bg to log
    auto loggingBG = [&] ( const std::string &a_strMessage ) -> void
    {
        bglog->info( a_strMessage );
        bglog->flush();
    };
    std::function< void( const std::string& ) > fn_bgLog = loggingBG;
    /////////////////////////////////////////
    // callback done

    // app start
    appLog.logInfo( "config file read ok" );
    appLog.logInfo( (boost::format( "%s initializing" ) % argv[0]).str() );
    spdlog::info( "{} ...", argv[0] );     // conio

    tools::Daemon app;
    queue<string> qCommands;
    app.setName( string( argv[0] ) );

    if( 1 == argc )
    {
        display();
        return -1;
    }

    // collect app params
    for( int32_t nIndex=1; nIndex<argc; ++nIndex )
    {
        string strCommand = argv[nIndex];
        transform( strCommand.begin(), strCommand.begin(), strCommand.end(), ::tolower );
        qCommands.push( strCommand );
    }

    // can start or stop app
    // if starting, can start and run in foreground --> -f
    // if running as daemon, can also run in debug -d mode, does not disble console IO
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
            appLog.logInfo( "running in foreground" );
            // start prog body
            dexshareManager dsm;
            if( true == dsm.start( cfg, appLog, fn_bgLog ) )  // start app, send cfg, applogger and blood glucose callback
            {
                appLog.logInfo( "started, main in wait state" );
                app.runForeground();    // wait for signal, ctrl-c or sig15
                dsm.stop();
                dsm.wait();
                appLog.logInfo( "shut down completing" );
            } else
            {
                appLog.logError( "failed to start" );
            }
        } else
        {
            appLog.logInfo( "running as daemon" );
            if( true == bDebug )
            {
                appLog.logDebug( "stdio and srderr not redirected" );
            }

            app.runDaemon( false, bDebug );
            appLog.logInfo( (boost::format( "daemon pid:%s" ) % getpid()).str() );
            dexshareManager dsm;
            try
            {
                if( true == dsm.start( cfg, appLog, fn_bgLog ) )
                {
                    appLog.logInfo( "started, main in wait state" );
                    app.wait();  // wait for sig15 issue by stop
                    dsm.stop();
                    dsm.wait();
                    appLog.logInfo( "shut down completing" );
                } else
                {
                    appLog.logError( "failed to start" );
                }
            } catch( std::exception &e )
            {
                cerr << e.what() << endl;
                appLog.logInfo( e.what() );
            }
        }
    }
    else
        if( qCommands.front() == "stop" )
        {
            appLog.logInfo( "stop issued" );
            qCommands.pop();
            app.stop();  // send sig15
        }
        else
        {
            display();
        }

    appLog.logInfo( "shutdown complete" );
    bglog->flush();
    return 0;
}




