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
            std::condition_variable_any m_cv;
            std::mutex                  m_mux;
        public:
            monitor() {}
            monitor( monitor& ) = delete;
            monitor( monitor&& ) = delete;

            void signal()
            {
                m_mux.lock();
                m_bFlag = true;
                m_cv.notify_one();
                m_mux.unlock();
            }

            void wait()
            {
                m_mux.lock();
                m_cv.wait( m_mux, [&]() -> bool { return m_bFlag; } );
                m_mux.unlock();
            }
    };

}
