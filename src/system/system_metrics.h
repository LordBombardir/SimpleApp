#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace app::system {

struct SystemInfo {
    std::string osName;
    std::string architecture;
    std::string cpuModel;
    std::string gpuModel;
    std::string diskInfo;
    std::string motherboard;
};

struct PerformanceSnapshot {
    float  totalCpuUsage    = 0.0f; // in %
    float  processCpuUsage  = 0.0f; // in %
    size_t systemRamUsedMB  = 0;    // in MB
    size_t systemRamTotalMB = 0;    // in MB
    size_t processRamUsedMB = 0;    // in MB
};

class SystemMetricsCollector {
public:
    static SystemMetricsCollector& Instance();

    const SystemInfo&   GetStaticInfo();
    PerformanceSnapshot GetPerformanceSnapshot();

private:
    SystemMetricsCollector();

    void FetchStaticInfo();

    SystemInfo          m_staticInfo;
    bool                m_initialized       = false;
    PerformanceSnapshot m_cachedSnapshot    = {};
    bool                m_hasCachedSnapshot = false;
    uint32_t            m_numProcessors     = 1;

    std::chrono::steady_clock::time_point m_lastFetchTime;

    float m_smoothedCpu     = 0.0f;
    float m_smoothedProcCpu = 0.0f;

#ifdef _WIN32
    uint64_t m_lastSysIdle    = 0;
    uint64_t m_lastSysKernel  = 0;
    uint64_t m_lastSysUser    = 0;
    uint64_t m_lastProcKernel = 0;
    uint64_t m_lastProcUser   = 0;
    uint64_t m_lastTime       = 0;
#else
    uint64_t m_lastLinuxTotal  = 0;
    uint64_t m_lastLinuxActive = 0;
    uint64_t m_lastLinuxProc   = 0;
#endif
};

} // namespace app::system
