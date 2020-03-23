#pragma once
#include "loggingInterface.h"

#include <string>
#include <thread>
#include <condition_variable>
#include <memory>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
   #include <spdlog/spdlog.h>
   #include <spdlog/sinks/daily_file_sink.h>
#pragma GCC diagnostic pop


namespace logging
{

    enum class LOG_TYPE { ERROR, INFO, WARN, VERBOSE };
    using logLevel_t = LOG_TYPE;

    class log final : public logggingInterface
    {
        private:
            logLevel_t                          m_logLevel = LOG_TYPE::INFO;
            std::shared_ptr<spdlog::logger>     m_pLogger;

            bool _log( const std::string& a_strMessage, const logLevel_t a_Level );

        public:
            virtual ~log();
            void setLogger ( const std::string& a_strLogName, const std::string& a_strLogPath ) override;

            void logInfo  ( const std::string& a_strMessage ) override { _log( a_strMessage, LOG_TYPE::INFO );    };
            void logWarn  ( const std::string& a_strMessage ) override { _log( a_strMessage, LOG_TYPE::WARN );    };
            void logError ( const std::string& a_strMessage ) override { _log( a_strMessage, LOG_TYPE::ERROR );   };
            void logDebug ( const std::string& a_strMessage ) override { _log( a_strMessage, LOG_TYPE::VERBOSE ); };

            void levelInfo()    override { m_logLevel = LOG_TYPE::INFO;    };
            void levelWarning() override { m_logLevel = LOG_TYPE::WARN;    };
            void levelError()   override { m_logLevel = LOG_TYPE::ERROR;   };
            void levelDebug()   override { m_logLevel = LOG_TYPE::VERBOSE; };
    };

}


namespace sync_tools
{
    ///
    /// \brief The monitor class - syncronization monitor
    ///
    class monitor
    {
        private:
            bool                        m_bFlag     = false;
            std::condition_variable     m_cv;
            std::mutex                  m_mux;
        public:
            monitor() {}
            monitor( monitor& ) = delete;
            monitor( monitor&& ) = delete;

            void signal()
            {
                std::unique_lock<std::mutex> lock( m_mux );
                m_bFlag = true;
                m_cv.notify_one();
            }

            void wait()
            {
                std::unique_lock<std::mutex> lock( m_mux );
                m_cv.wait( lock, [&]() -> bool { return m_bFlag; } );
                m_bFlag = false;
            }
    };

}
