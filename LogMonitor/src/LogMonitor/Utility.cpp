//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
//

#include "pch.h"

using namespace std;


///
/// Utility.cpp
///
/// Contains utility functions to format strings, convert time to string, etc.
///


///
/// Returns the string representation of a SYSTEMTIME structure that can be used
/// in an XML query for Windows Event collection.
///
/// \param SystemTime         SYSTEMTIME with the time to format.
///
/// \return The string with the human-readable time.
///
std::wstring
Utility::SystemTimeToString(
    SYSTEMTIME SystemTime
    )
{
    constexpr size_t STR_LEN = 64;
    wchar_t dateStr[STR_LEN] = { 0 };

#pragma warning(suppress : 38042)
    GetDateFormatEx(0, 0, &SystemTime, L"yyyy-MM-dd", dateStr, STR_LEN, 0);

    wchar_t timeStr[STR_LEN] = { 0 };
    GetTimeFormatEx(0, 0, &SystemTime, L"HH:mm:ss", timeStr, STR_LEN);

    return FormatString(L"%sT%s.000Z", dateStr, timeStr);
}


///
/// Returns a string representation of a FILETIME.
///
/// \param FileTime         FILETIME with the time to format.
///
/// \return The string with the human-readable time.
///
std::wstring
Utility::FileTimeToString(
    FILETIME FileTime
    )
{
    SYSTEMTIME systemTime;
    FileTimeToSystemTime(&FileTime, &systemTime);
    return SystemTimeToString(systemTime);
}

///
/// Returns a formatted wide string.
///
/// \param FormatString         Format string that follows the same specifications as format in printf.
/// \param ...                  Variable arguments list
///
/// \return The composed string.
///
std::wstring
Utility::FormatString(
    _In_ _Printf_format_string_ LPCWSTR FormatString,
    ...
    )
{
    std::wstring result = L"";
    va_list vaList;
    va_start(vaList, FormatString);

    size_t length = ::_vscwprintf(FormatString, vaList);

    try
    {
        result.resize(length);

        if (-1 == ::vswprintf_s(&result[0], length + 1, FormatString, vaList))
        {
            //
            // Ignore failure and continue
            //
        }
    }
    catch (...) {}

    va_end(vaList);

    return result;
}

///
/// Verify if the input stream is in UTF-8 format.
/// UTF-8 is the encoding of Unicode based on Internet Society RFC2279
/// ( See https://tools.ietf.org/html/rfc2279 )
///
/// \param lpstrInputStream     Pointer to a buffer with the wide text to evaluate.
/// \param iLen                 Size of the buffer.
///
/// \return If the text is UTF-8, return true. Otherwise, false.
///
bool
Utility::IsTextUTF8(
    LPCSTR InputStream,
    int Length
    )
{
    int nChars = MultiByteToWideChar(CP_UTF8,
                                      MB_ERR_INVALID_CHARS,
                                      InputStream,
                                      Length,
                                      NULL,
                                      0);

    return nChars > 0 || GetLastError() != ERROR_NO_UNICODE_TRANSLATION;
}

///
/// Verify if the input stream is in Unicode format.
///
/// \param lpstrInputStream     Pointer to a buffer with the wide text to evaluate.
/// \param iLen                 Size of the buffer.
///
/// \return If the text is unicode, return true. Otherwise, false.
///
bool
Utility::IsInputTextUnicode(
    LPCSTR InputStream,
    int Length
    )
{
    int  iResult = ~0; // turn on IS_TEXT_UNICODE_DBCS_LEADBYTE
    bool bUnicode;

    bUnicode = IsTextUnicode(InputStream, Length, &iResult);

    //
    // If the only hint is based on statistics, assume ANSI for short strings
    // This protects against short ansi strings like "this program can break" from being
    // detected as unicode
    //
    if (bUnicode && iResult == IS_TEXT_UNICODE_STATISTICS && Length < 100)
    {
        bUnicode = false;
    }

    return bUnicode;
}


///
/// Get the short path name of the file. If the function
/// failed to get the short path, then it returns original path.
///
/// \param Path Full path of a file or directory
///
/// \return Short path of a file or directory
///
std::wstring
Utility::GetShortPath(
    _In_ const std::wstring& Path
    )
{
    DWORD bufSz = 1024;
    std::vector<wchar_t> buf(bufSz);

    DWORD pathSz = GetShortPathNameW(Path.c_str(), &(buf[0]), bufSz);
    if (pathSz >= bufSz)
    {
        buf.resize(pathSz + 1);
        if (GetShortPathNameW(Path.c_str(), &(buf[0]), pathSz + 1))
        {
            wstring shortPath = &buf[0];
            return shortPath.c_str();
        }
    }
    else if (pathSz)
    {
        wstring shortPath = &buf[0];
        return shortPath.c_str();
    }

    return Path;
}


///
/// Get the long path name of the file. If the function
/// failed to get the long path, then it returns original path.
///
/// \param Path Full path of a file or directory
///
/// \return Long path of a file or directory
///
std::wstring
Utility::GetLongPath(
    _In_ const std::wstring& Path
    )
{
    DWORD bufSz = 1024;
    std::vector<wchar_t> buf(bufSz);

    DWORD pathSz = GetLongPathNameW(Path.c_str(), &(buf[0]), bufSz);
    if(pathSz >= bufSz)
    {
        buf.resize(pathSz + 1);
        if (GetLongPathNameW(Path.c_str(), &(buf[0]), pathSz + 1))
        {
            wstring longPath = &buf[0];
            return longPath.c_str();
        }
    }
    else if (pathSz)
    {
        wstring longPath = &buf[0];
        return longPath.c_str();
    }

    return Path;
}

///
/// Replaces all the occurrences in a wstring.
///
/// \param Str      The string to search substrings and replace them.
/// \param From     The substring to being replaced.
/// \param To       The substring to replace.
///
/// \return A wstring.
///
std::wstring Utility::ReplaceAll(_In_ std::wstring Str, _In_ const std::wstring& From, _In_ const std::wstring& To) {
    size_t start_pos = 0;

    while ((start_pos = Str.find(From, start_pos)) != std::string::npos) {
        Str.replace(start_pos, From.length(), To);
        start_pos += To.length(); // Handles case where 'To' is a substring of 'From'
    }
    return Str;
}

///
/// \brief Wrapper around WideCharToMultiByte function to convert wstring to utf8 
///
/// \param str The string to convert
/// \param byte_size optional int, returns string buffer size in bytes
/// 
/// \return a utf8 encoded string 
std::string Utility::WideStringToUtf8(_In_ const std::wstring &&str, _Out_ int& byte_size)
{
    std::string unicode_string;
    int string_length = (int) str.length();
    
    // get byte size required to resize and populate the unicode string
    byte_size = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), string_length, NULL, 0, NULL, NULL);

    if (byte_size > 0)
    {
        unicode_string.resize(byte_size);
        WideCharToMultiByte(CP_UTF8, 0, str.c_str(), string_length, &unicode_string[0], byte_size, NULL, NULL);
    }

    return unicode_string;
}

std::wstring
Utility::ConvertStringToUTF16(
    _In_reads_bytes_(StringSize) LPBYTE StringPtr,
    _In_ UINT StringSize,
    _In_ LM_FILETYPE EncodingType
    )
{
    std::wstring Result;
    if (StringSize == 0)
    {
        return std::wstring();
    }

    switch (EncodingType)
    {
    case LM_FILETYPE::UTF16LE:
    {
        Result = wstring((wchar_t*)StringPtr, (wchar_t*)(StringPtr + StringSize));
        break;
    }
    case LM_FILETYPE::UTF16BE:
    {
        Result = wstring((wchar_t*)StringPtr, (wchar_t*)(StringPtr + StringSize));

        //
        // Reverse each wide character, to make it little endian
        //
        for (unsigned int i = 0; i < Result.size(); i++)
        {
            Result[i] = (TCHAR)(((Result[i] << 8) & 0xFF00) + ((Result[i] >> 8) & 0xFF));
        }
        break;
    }
    case LM_FILETYPE::UTF8:
    {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)StringPtr, StringSize, NULL, 0);
        Result.resize(size_needed);

        MultiByteToWideChar(CP_UTF8, 0, (LPCCH)StringPtr, StringSize, (LPWSTR)(Result.data()), size_needed);
        break;
    }
    default:
    {
        std::string tempStr((char*)StringPtr, (char*)(StringPtr + StringSize));
        //
        // ANSI
        //
        Result = wstring(tempStr.begin(), tempStr.end());
    }
    }

    return Result;
}
