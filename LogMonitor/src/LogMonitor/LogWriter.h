//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
//

#pragma once

class LogWriter final
{
public:
    LogWriter()
    {
        InitializeSRWLock(&m_stdoutLock);
        
        DWORD dwMode;

        stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

        if (!GetConsoleMode(stdoutHandle, &dwMode))
        {
           m_isConsole = false;
        }
    };

    ~LogWriter() {}

private:
    HANDLE stdoutHandle;
    SRWLOCK m_stdoutLock;
    bool m_isConsole = true;


    void FlushStdOut()
    {
        if (m_isConsole)
        {
            fflush(stdout);
        }
    }

public:
    bool WriteLog(
        _In_ HANDLE       FileHandle,
        _In_ LPCVOID      Buffer,
        _In_ DWORD        NumberOfBytesToWrite,
        _In_ LPDWORD      NumberOfBytesWritten,
        _In_ LPOVERLAPPED Overlapped
    )
    {
        bool result;

        AcquireSRWLockExclusive(&m_stdoutLock);

        result = WriteFile(FileHandle, Buffer, NumberOfBytesToWrite, NumberOfBytesWritten, Overlapped);

        ReleaseSRWLockExclusive(&m_stdoutLock);

        return result;
    }

    void WriteConsoleLog(
        _In_ const std::wstring&& LogMessage
    )
    {
        AcquireSRWLockExclusive(&m_stdoutLock);

        std::wstring FormattedString = LogMessage;

        // only add new line characters to strings not ending in a new line character to avoid blank lines
        if(LogMessage.find_last_of(L"\n") != LogMessage.size() - 1) {
            FormattedString = LogMessage + L"\n";
        }
        
        int size_in_bytes;

        std::string encoded = Utility::WideStringToUtf8(FormattedString.c_str(), size_in_bytes);
        WriteFile(stdoutHandle, encoded.c_str(), (DWORD) size_in_bytes, NULL, NULL);
        
        ReleaseSRWLockExclusive(&m_stdoutLock);
    }

    void WriteConsoleLog(
        _In_ const std::wstring& LogMessage
    )
    {
        WriteConsoleLog(LogMessage.c_str());
    }

    void TraceError(
        _In_ LPCWSTR Message
    )
    {
        SYSTEMTIME st;
        GetSystemTime(&st);

        std::wstring formattedMessage = Utility::FormatString(L"[%s][LOGMONITOR] ERROR: %s",
            Utility::SystemTimeToString(st).c_str(),
            Message);

        WriteConsoleLog(formattedMessage);
    }

    void TraceWarning(
        _In_ LPCWSTR Message
    )
    {
        SYSTEMTIME st;
        GetSystemTime(&st);

        std::wstring formattedMessage = Utility::FormatString(L"[%s][LOGMONITOR] WARNING: %s",
            Utility::SystemTimeToString(st).c_str(),
            Message);

        WriteConsoleLog(formattedMessage);
    }

    void TraceInfo(
        _In_ LPCWSTR Message
    )
    {
        SYSTEMTIME st;
        GetSystemTime(&st);

        std::wstring formattedMessage = Utility::FormatString(L"[%s][LOGMONITOR] INFO: %s",
            Utility::SystemTimeToString(st).c_str(),
            Message);

        WriteConsoleLog(formattedMessage);
    }
};

extern LogWriter logWriter;
