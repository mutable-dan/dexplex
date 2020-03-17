#pragma once
#include <thread>
#include <condition_variable>

namespace logging
{
    enum class LOG_TYPE { ERROR, INFO, WARN, VERBOSE };
    using logLevel_t = LOG_TYPE;
}


namespace sync_tools
{
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
