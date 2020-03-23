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

            //virtual template<typename T, typename TT

            virtual void setLogger ( const std::string& a_strLogName, const std::string& a_strLogPath ) = 0;
            virtual void levelInfo()   = 0;
            virtual void levelWarning()= 0;
            virtual void levelError()  = 0;
            virtual void levelDebug()  = 0;
    };
}
