#pragma once

#ifdef LOGMONITOR_EXPORTS
#define LOGMONITOR_API __declspec(dllexport)
#else
#define LOGMONITOR_API __declspec(dllimport)
#endif

// Callback signature: receives Etw Log Entry Data
// typedef void(*LogMonitorEventCallback)(EtwLogEntry logEntry);

typedef void (WINAPI *LogMonitorEventCallback2)(_In_ PEVENT_RECORD EventRecord);

extern "C" {

// Starts the log monitor. Returns true on success.
// configPath: path to JSON config file
// callback: function to call for each event
LOGMONITOR_API bool StartLogMonitor(
    _In_ const WCHAR * configPath, 
    _Out_ LogMonitorEventCallback2 onEventCallback);

// Stops the log monitor.
LOGMONITOR_API void StopLogMonitor();
}