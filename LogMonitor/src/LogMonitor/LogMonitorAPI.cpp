//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
//

#include "pch.h"

using namespace std;
#pragma comment(lib, "wevtapi.lib")
#pragma comment(lib, "tdh.lib")
#pragma comment(lib, "ws2_32.lib")  // For ntohs function
#pragma comment(lib, "shlwapi.lib")


LogWriter logWriter;

std::unique_ptr<EtwMonitor> g_etwMon(nullptr);

void InitializeEtwMonitor(
    std::shared_ptr<SourceETW> sourceETW,
    std::vector<ETWProvider>& etwProviders)
{
    for (auto provider : sourceETW->Providers)
    {
        etwProviders.push_back(provider);
    }
}

void CreateEtwMonitor(
    std::vector<ETWProvider>& etwProviders,
    LogMonitorEventCallback2 onEventCallback)
{
    try
    {
        g_etwMon = make_unique<EtwMonitor>(etwProviders, onEventCallback);
    }
    catch (...)
    {
        logWriter.TraceError(L"Invalid providers. Check them using 'logman query providers'");
    }
}

void StopLogMonitor()
{
    // implement graceful shutdown
}

/// <summary>
/// Start the monitors by delegating to the helper functions based on log source type
/// </summary>
/// <param name="settings">The LoggerSettings object containing configuration</param>
void StartMonitors(_In_ LoggerSettings &settings, LogMonitorEventCallback2 onEventCallback)
{
    // Vectors to store ETW providers
    std::vector<ETWProvider> etwProviders;

    // Iterate through each log source defined in the settings, only ETW sources are processed here
    for (auto source : settings.Sources)
    {
        switch (source->Type)
        {
        case LogSourceType::ETW:
        {
            std::shared_ptr<SourceETW> sourceETW =
                std::reinterpret_pointer_cast<SourceETW>(source);
            InitializeEtwMonitor(
                sourceETW,
                etwProviders);
            break;
        }
        default:
            logWriter.TraceWarning(
                L"Log source type not supported for monitoring");
            break;
        }
    }

    // Create and start EtwMonitor if there are ETW providers
    if (!etwProviders.empty())
    {
        CreateEtwMonitor(
            etwProviders,
            onEventCallback
        );
    }
}

bool StartLogMonitor(const WCHAR* configPath, LogMonitorEventCallback2 onEventCallback)
{
    LoggerSettings settings;
    // read the config file
    bool configFileReadSuccess = OpenConfigFile(const_cast<PWCHAR>(configPath), settings);

    // start the monitors
    if (configFileReadSuccess)
    {
        StartMonitors(settings, onEventCallback);
        return true;
    }
    else
    {
        logWriter.TraceError(L"Invalid configuration file.");
        return false;
    }
}
