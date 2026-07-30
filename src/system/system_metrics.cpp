// clang-format off
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#include <dxgi.h>
#else
#include <sys/resource.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>
#endif
// clang-format on

#include "system/system_metrics.h"
#include <algorithm>
#include <format>
#include <fstream>
#include <sstream>

namespace app::system {

#ifdef _WIN32
static std::string ReadRegistryString(HKEY hKeyParent, const wchar_t* subKey, const wchar_t* valueName) {
    HKEY hKey;
    if (RegOpenKeyExW(hKeyParent, subKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return "";
    }

    DWORD dwSize = 0;
    DWORD dwType = 0;
    LONG  result = RegQueryValueExW(hKey, valueName, nullptr, &dwType, nullptr, &dwSize);
    if (result != ERROR_SUCCESS || (dwType != REG_SZ && dwType != REG_EXPAND_SZ) || dwSize == 0) {
        RegCloseKey(hKey);
        return "";
    }

    std::vector<wchar_t> buffer(dwSize / sizeof(wchar_t) + 2, 0);
    result = RegQueryValueExW(hKey, valueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer.data()), &dwSize);
    RegCloseKey(hKey);

    if (result == ERROR_SUCCESS) {
        int mbLen = WideCharToMultiByte(CP_UTF8, 0, buffer.data(), -1, nullptr, 0, nullptr, nullptr);
        if (mbLen > 0) {
            std::string mbBuffer(mbLen - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, buffer.data(), -1, mbBuffer.data(), mbLen, nullptr, nullptr);
            return mbBuffer;
        }
    }
    return "";
}

static std::string GetWindowsOSName() {
    std::string productName =
        ReadRegistryString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"ProductName");
    std::string displayVersion =
        ReadRegistryString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"DisplayVersion");
    if (displayVersion.empty()) {
        displayVersion =
            ReadRegistryString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"ReleaseId");
    }
    std::string buildNumber = ReadRegistryString(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        L"CurrentBuildNumber"
    );
    if (buildNumber.empty()) {
        buildNumber =
            ReadRegistryString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"CurrentBuild");
    }

    // Windows 11 Build >= 22000 compatibility fix for registry ProductName
    if (!buildNumber.empty()) {
        try {
            int buildInt = std::stoi(buildNumber);
            if (buildInt >= 22000 && productName.find("Windows 10") != std::string::npos) {
                size_t pos = productName.find("Windows 10");
                productName.replace(pos, 10, "Windows 11");
            }
        } catch (...) {}
    }

    if (productName.empty()) {
        productName = "Windows OS";
    }

    std::string osDetails = productName;
    if (!displayVersion.empty() || !buildNumber.empty()) {
        osDetails += " (";
        if (!displayVersion.empty()) {
            osDetails += displayVersion;
            if (!buildNumber.empty()) osDetails += ", ";
        }
        if (!buildNumber.empty()) {
            osDetails += "Build " + buildNumber;
        }
        osDetails += ")";
    }

    return osDetails;
}

static std::string GetDirectXGpuName() {
    IDXGIFactory* pFactory = nullptr;
    if (SUCCEEDED(CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pFactory))) && pFactory) {
        IDXGIAdapter* pAdapter = nullptr;
        if (SUCCEEDED(pFactory->EnumAdapters(0, &pAdapter)) && pAdapter) {
            DXGI_ADAPTER_DESC desc;
            if (SUCCEEDED(pAdapter->GetDesc(&desc))) {
                pAdapter->Release();
                pFactory->Release();

                int mbLen = WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, nullptr, 0, nullptr, nullptr);
                if (mbLen > 0) {
                    std::string gpuName(mbLen - 1, '\0');
                    WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, gpuName.data(), mbLen, nullptr, nullptr);
                    return gpuName;
                }
            }
            pAdapter->Release();
        }
        pFactory->Release();
    }
    return "";
}

static uint64_t FileTimeToUint64(const FILETIME& ft) {
    return (static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
}
#else
static std::string ReadFileFirstLine(const std::string& path) {
    std::ifstream file(path);
    std::string   line;
    if (std::getline(file, line)) {
        size_t last = line.find_last_not_of("\r\n ");
        if (last != std::string::npos) line = line.substr(0, last + 1);
        return line;
    }
    return "";
}

static std::string GetLinuxOSName() {
    std::string   osName;
    std::ifstream file("/etc/os-release");
    std::string   line;
    while (std::getline(file, line)) {
        if (line.rfind("PRETTY_NAME=", 0) == 0) {
            std::string val = line.substr(12);
            if (!val.empty() && val.front() == '"') val.erase(0, 1);
            if (!val.empty() && val.back() == '"') val.pop_back();
            osName = val;
            break;
        }
    }
    struct utsname uts;
    if (uname(&uts) == 0) {
        if (osName.empty()) {
            osName = std::string(uts.sysname) + " " + uts.release;
        } else {
            osName += " (Kernel " + std::string(uts.release) + ")";
        }
    }
    return !osName.empty() ? osName : "Linux OS";
}
#endif

void SystemMetricsCollector::SetGpuModel(const std::string& gpuModel) {
    if (!gpuModel.empty()) {
        m_staticInfo.gpuModel = gpuModel;
    }
}

SystemMetricsCollector& SystemMetricsCollector::Instance() {
    static SystemMetricsCollector instance;
    return instance;
}

SystemMetricsCollector::SystemMetricsCollector() { FetchStaticInfo(); }

void SystemMetricsCollector::FetchStaticInfo() {
    if (m_initialized) return;

#ifdef _WIN32
    // OS Name
    m_staticInfo.osName = GetWindowsOSName();

    // Architecture
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);
    m_numProcessors = sysInfo.dwNumberOfProcessors;
    switch (sysInfo.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64:
        m_staticInfo.architecture = "x64 (64-bit)";
        break;
    case PROCESSOR_ARCHITECTURE_ARM64:
        m_staticInfo.architecture = "ARM64";
        break;
    case PROCESSOR_ARCHITECTURE_INTEL:
        m_staticInfo.architecture = "x86 (32-bit)";
        break;
    default:
        m_staticInfo.architecture = "Unknown";
        break;
    }

    // CPU Model
    std::string cpuName = ReadRegistryString(
        HKEY_LOCAL_MACHINE,
        L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        L"ProcessorNameString"
    );
    if (!cpuName.empty()) {
        size_t first          = cpuName.find_first_not_of(' ');
        m_staticInfo.cpuModel = (first != std::string::npos) ? cpuName.substr(first) : cpuName;
    } else {
        m_staticInfo.cpuModel = "x86_64 Processor";
    }

    // GPU Model (Native DXGI API)
    std::string gpuName   = GetDirectXGpuName();
    m_staticInfo.gpuModel = !gpuName.empty() ? gpuName : "DirectX 11 Graphics Adapter";

    // Disk Info
    ULARGE_INTEGER freeBytes, totalBytes, totalFree;
    if (GetDiskFreeSpaceExW(L"C:\\", &freeBytes, &totalBytes, &totalFree)) {
        double freeGB         = static_cast<double>(totalFree.QuadPart) / (1024.0 * 1024.0 * 1024.0);
        double totalGB        = static_cast<double>(totalBytes.QuadPart) / (1024.0 * 1024.0 * 1024.0);
        m_staticInfo.diskInfo = std::format("C: {:.0f} GB free / {:.0f} GB", freeGB, totalGB);
    } else {
        m_staticInfo.diskInfo = "C: Fixed Drive";
    }

    // Motherboard / System Board Info
    std::string sysVendor =
        ReadRegistryString(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", L"SystemManufacturer");
    std::string sysProduct =
        ReadRegistryString(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", L"SystemProductName");
    std::string baseVendor =
        ReadRegistryString(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", L"BaseBoardManufacturer");
    std::string baseProduct =
        ReadRegistryString(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", L"BaseBoardProduct");

    auto IsValid = [](const std::string& s) {
        if (s.empty() || s == "WL" || s.find("Raticate") != std::string::npos) return false;
        if (s.find("To be filled") != std::string::npos || s.find("Default") != std::string::npos) return false;
        return true;
    };

    if (IsValid(sysProduct)) {
        if (IsValid(sysVendor) && sysProduct.find(sysVendor) == std::string::npos) {
            m_staticInfo.motherboard = sysVendor + " " + sysProduct;
        } else {
            m_staticInfo.motherboard = sysProduct;
        }
    } else if (IsValid(baseProduct)) {
        if (IsValid(baseVendor) && baseProduct.find(baseVendor) == std::string::npos) {
            m_staticInfo.motherboard = baseVendor + " " + baseProduct;
        } else {
            m_staticInfo.motherboard = baseProduct;
        }
    } else {
        m_staticInfo.motherboard = "System Board";
    }

#else
    // OS Name
    m_staticInfo.osName = GetLinuxOSName();

    long nCpus      = sysconf(_SC_NPROCESSORS_ONLN);
    m_numProcessors = (nCpus > 0) ? static_cast<uint32_t>(nCpus) : 1;

    // Architecture
    struct utsname uts;
    if (uname(&uts) == 0) {
        m_staticInfo.architecture = uts.machine;
    } else {
        m_staticInfo.architecture = "x86_64";
    }

    // CPU Model
    std::ifstream cpuFile("/proc/cpuinfo");
    std::string   line;
    m_staticInfo.cpuModel = "";
    while (std::getline(cpuFile, line)) {
        if (line.find("model name") != std::string::npos || line.find("Processor") != std::string::npos) {
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                m_staticInfo.cpuModel = line.substr(colon + 2);
                break;
            }
        }
    }
    if (m_staticInfo.cpuModel.empty()) {
        m_staticInfo.cpuModel = "x86_64 Processor";
    }

    // GPU Model (Updated natively via OpenGL context)
    if (m_staticInfo.gpuModel.empty()) {
        m_staticInfo.gpuModel = "OpenGL Device";
    }

    // Disk Info
    struct statvfs diskStat;
    if (statvfs("/", &diskStat) == 0) {
        double freeGB         = static_cast<double>(diskStat.f_bavail * diskStat.f_frsize) / (1024.0 * 1024.0 * 1024.0);
        double totalGB        = static_cast<double>(diskStat.f_blocks * diskStat.f_frsize) / (1024.0 * 1024.0 * 1024.0);
        m_staticInfo.diskInfo = std::format("/: {:.0f} GB free / {:.0f} GB", freeGB, totalGB);
    } else {
        m_staticInfo.diskInfo = "/: Root Mount";
    }

    // Motherboard Info
    std::string boardVendor = ReadFileFirstLine("/sys/class/dmi/id/board_vendor");
    std::string boardName   = ReadFileFirstLine("/sys/class/dmi/id/board_name");
    if (!boardName.empty()) {
        m_staticInfo.motherboard = !boardVendor.empty() ? (boardVendor + " " + boardName) : boardName;
    } else {
        m_staticInfo.motherboard = "Linux Mainboard";
    }
#endif

    m_initialized = true;
}

const SystemInfo& SystemMetricsCollector::GetStaticInfo() {
    if (!m_initialized) {
        FetchStaticInfo();
    }
    return m_staticInfo;
}

PerformanceSnapshot SystemMetricsCollector::GetPerformanceSnapshot() {
    auto now = std::chrono::steady_clock::now();
    if (m_hasCachedSnapshot) {
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastFetchTime).count();
        if (elapsedMs < 1000) {
            return m_cachedSnapshot;
        }
    }

    PerformanceSnapshot snap{};

#ifdef _WIN32
    // RAM Metrics
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        snap.systemRamTotalMB = static_cast<size_t>(memInfo.ullTotalPhys / (1024 * 1024));
        snap.systemRamUsedMB  = static_cast<size_t>((memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024 * 1024));
    }

    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        snap.processRamUsedMB = static_cast<size_t>(pmc.WorkingSetSize / (1024 * 1024));
    }

    // CPU Metrics
    FILETIME idleTime, kernelTime, userTime;
    FILETIME procCreation, procExit, procKernel, procUser;

    if (GetSystemTimes(&idleTime, &kernelTime, &userTime)
        && GetProcessTimes(GetCurrentProcess(), &procCreation, &procExit, &procKernel, &procUser)) {

        uint64_t idle   = FileTimeToUint64(idleTime);
        uint64_t kernel = FileTimeToUint64(kernelTime);
        uint64_t user   = FileTimeToUint64(userTime);

        uint64_t procK = FileTimeToUint64(procKernel);
        uint64_t procU = FileTimeToUint64(procUser);

        if (m_lastSysKernel != 0) {
            uint64_t sysKernelDiff = kernel - m_lastSysKernel;
            uint64_t sysUserDiff   = user - m_lastSysUser;
            uint64_t idleDiff      = idle - m_lastSysIdle;

            uint64_t totalSysDiff = sysKernelDiff + sysUserDiff;
            if (totalSysDiff > 0) {
                uint64_t activeSysDiff = (totalSysDiff >= idleDiff) ? (totalSysDiff - idleDiff) : 0;
                float    rawSysCpu     = static_cast<float>(activeSysDiff * 100.0 / totalSysDiff);
                m_smoothedCpu = (m_smoothedCpu == 0.0f) ? rawSysCpu : (m_smoothedCpu * 0.85f + rawSysCpu * 0.15f);
            }

            uint64_t procKDiff     = procK - m_lastProcKernel;
            uint64_t procUDiff     = procU - m_lastProcUser;
            uint64_t procTotal     = procKDiff + procUDiff;
            uint32_t numProcessors = m_numProcessors;

            if (totalSysDiff > 0 && numProcessors > 0) {
                float rawProcCpu = static_cast<float>(procTotal * 100.0 / (totalSysDiff * numProcessors));
                m_smoothedProcCpu =
                    (m_smoothedProcCpu == 0.0f) ? rawProcCpu : (m_smoothedProcCpu * 0.85f + rawProcCpu * 0.15f);
            }
        }

        m_lastSysIdle    = idle;
        m_lastSysKernel  = kernel;
        m_lastSysUser    = user;
        m_lastProcKernel = procK;
        m_lastProcUser   = procU;
    }
#else
    // RAM Metrics using sysinfo system call
    struct sysinfo sysInfo;
    if (sysinfo(&sysInfo) == 0) {
        uint64_t memUnit  = sysInfo.mem_unit ? sysInfo.mem_unit : 1;
        uint64_t totalRam = static_cast<uint64_t>(sysInfo.totalram) * memUnit;
        uint64_t freeRam  = static_cast<uint64_t>(sysInfo.freeram + sysInfo.bufferram) * memUnit;

        snap.systemRamTotalMB = static_cast<size_t>(totalRam / (1024 * 1024));
        snap.systemRamUsedMB  = static_cast<size_t>((totalRam > freeRam ? totalRam - freeRam : 0) / (1024 * 1024));
    }

    // Process RAM using getrusage (ru_maxrss in KB)
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        snap.processRamUsedMB = static_cast<size_t>(usage.ru_maxrss / 1024);
    }

    // Process & System CPU Metrics via getrusage and sysinfo
    struct rusage procUsage;
    if (getrusage(RUSAGE_SELF, &procUsage) == 0) {
        uint64_t userUs      = static_cast<uint64_t>(procUsage.ru_utime.tv_sec) * 1000000 + procUsage.ru_utime.tv_usec;
        uint64_t sysUs       = static_cast<uint64_t>(procUsage.ru_stime.tv_sec) * 1000000 + procUsage.ru_stime.tv_usec;
        uint64_t totalProcUs = userUs + sysUs;

        if (m_lastProcMicroSec > 0) {
            auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastProcTime).count();
            if (elapsedUs > 0 && m_numProcessors > 0) {
                uint64_t procUsDiff = (totalProcUs >= m_lastProcMicroSec) ? (totalProcUs - m_lastProcMicroSec) : 0;
                float    rawProcCpu = static_cast<float>(procUsDiff * 100.0 / (elapsedUs * m_numProcessors));
                m_smoothedProcCpu   = (m_smoothedProcCpu == 0.0f) ? rawProcCpu : (m_smoothedProcCpu * 0.85f + rawProcCpu * 0.15f);
            }
        }
        m_lastProcMicroSec = totalProcUs;
        m_lastProcTime     = now;
    }

    struct sysinfo sysInfoCpu;
    if (sysinfo(&sysInfoCpu) == 0) {
        float loadAvg   = static_cast<float>(sysInfoCpu.loads[0]) / 65536.0f;
        float rawSysCpu = (m_numProcessors > 0) ? (loadAvg / m_numProcessors) * 100.0f : 0.0f;
        m_smoothedCpu   = (m_smoothedCpu == 0.0f) ? rawSysCpu : (m_smoothedCpu * 0.85f + rawSysCpu * 0.15f);
    }
#endif

    // Clamp percentage ranges cleanly
    snap.totalCpuUsage   = (std::max)(0.0f, (std::min)(100.0f, m_smoothedCpu));
    snap.processCpuUsage = (std::max)(0.0f, (std::min)(100.0f, m_smoothedProcCpu));

    m_cachedSnapshot    = snap;
    m_lastFetchTime     = now;
    m_hasCachedSnapshot = true;

    return snap;
}

} // namespace app::system
