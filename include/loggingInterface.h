#pragma once

#include <string>

namespace logging
{
    class logggingInterface
    {
        public:
            virtual void logInfo   ( const std::string& a_strMessage ) = 0;
            virtual void logWarn   ( const std::string& a_strMessage ) = 0;
            virtual void logError  ( const std::string& a_strMessage ) = 0;
            virtual void logDebug  ( const std::string& a_strMessage ) = 0;

            virtual void setLogger ( const std::string& a_strLogName, const std::string& a_strLogPath ) = 0;
            virtual void setLevelInfo()   = 0;
            virtual void setLevelWarning()= 0;
            virtual void setLevelError()  = 0;
            virtual void setLevelDebug()  = 0;

            virtual bool isReady()        = 0;
    };
}
