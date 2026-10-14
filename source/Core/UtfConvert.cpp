//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include "Square/Core/UtfConvert.h"
#if defined(_WIN32)
    #include <windows.h>
#elif defined(__APPLE__)
    #include <CoreFoundation/CoreFoundation.h>
#else // UNIX
    #include <iconv.h>
    #include <cerrno>
#endif

namespace Square
{

///////////////////////////////////////////////////////////////////////////////
#if defined(_WIN32)
// Native Windows API — WideCharToMultiByte / MultiByteToWideChar
// wchar_t is UTF-16LE on Windows; used as the pivot for all conversions.
// UTF-32 has no native Win32 API, so it goes through UTF-16 manually.
namespace
{
    static WString win_u8_to_wstr(const std::u8string& s)
    {
        if (s.empty()) return {};
        int len = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
        WString result(len, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), result.data(), len);
        return result;
    }

    static std::u8string win_wstr_to_u8(const WString& ws)
    {
        if (ws.empty()) return {};
        int len = WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), nullptr, 0, nullptr, nullptr);
        std::u8string result(len, '\0');
        WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), result.data(), len, nullptr, nullptr);
        return result;
    }

    static WString win_u16_to_wstr(const U16String& s)
    {
        // char16_t and wchar_t are both 2 bytes on Windows — same layout
        return WString(reinterpret_cast<const wchar_t*>(s.data()), s.size());
    }

    static U16String win_wstr_to_u16(const WString& ws)
    {
        return U16String(reinterpret_cast<const char16_t*>(ws.data()), ws.size());
    }

    static WString win_u32_to_wstr(const U32String& s)
    {
        WString result;
        result.reserve(s.size());
        for (char32_t cp : s)
        {
            if (cp < 0x10000u)
            {
                result.push_back((wchar_t)cp);
            }
            else
            {
                cp -= 0x10000u;
                result.push_back((wchar_t)(0xD800u | (cp >> 10)));
                result.push_back((wchar_t)(0xDC00u | (cp & 0x3FFu)));
            }
        }
        return result;
    }

    static U32String win_wstr_to_u32(const WString& ws)
    {
        U32String result;
        const wchar_t* p   = ws.data();
        const wchar_t* end = p + ws.size();
        while (p < end)
        {
            uint32_t c = (uint16_t)*p++;
            if (c >= 0xD800u && c <= 0xDBFFu && p < end)
            {
                uint32_t low = (uint16_t)*p;
                if (low >= 0xDC00u && low <= 0xDFFFu)
                {
                    ++p;
                    c = 0x10000u + ((c - 0xD800u) << 10) + (low - 0xDC00u);
                }
            }
            result.push_back((char32_t)c);
        }
        return result;
    }
}

    SQUARE_API std::u8string to_utf8(const U16String &s) { return win_wstr_to_u8(win_u16_to_wstr(s)); }
    SQUARE_API std::u8string to_utf8(const U32String &s) { return win_wstr_to_u8(win_u32_to_wstr(s)); }
    SQUARE_API U16String to_utf16(const std::u8string &s) { return win_wstr_to_u16(win_u8_to_wstr(s)); }
    SQUARE_API U16String to_utf16(const U32String &s) { return win_wstr_to_u16(win_u32_to_wstr(s)); }
    SQUARE_API U32String to_utf32(const std::u8string &s) { return win_wstr_to_u32(win_u8_to_wstr(s)); }
    SQUARE_API U32String to_utf32(const U16String &s) { return win_wstr_to_u32(win_u16_to_wstr(s)); }
    SQUARE_API U16String to_ucs2(const std::u8string &s) { return win_wstr_to_u16(win_u8_to_wstr(s)); }
    SQUARE_API U32String to_ucs4(const std::u8string &s) { return to_utf32(s); }
    SQUARE_API U32String to_ucs4(const U16String &s) { return to_utf32(s); }

///////////////////////////////////////////////////////////////////////////////
#elif defined(__APPLE__)
// CoreFoundation — native Apple UTF conversion API
namespace
{
    static CFStringRef cf_from_utf8(const std::u8string& s)
    {
        return CFStringCreateWithBytes(
            kCFAllocatorDefault,
            reinterpret_cast<const UInt8*>(s.data()),
            CFIndex(s.size()),
            kCFStringEncodingUTF8,
            false
        );
    }

    static CFStringRef cf_from_utf16(const U16String& s)
    {
        return CFStringCreateWithCharacters(
            kCFAllocatorDefault,
            reinterpret_cast<const UniChar*>(s.data()),
            CFIndex(s.size())
        );
    }

    static CFStringRef cf_from_utf32(const U32String& s)
    {
        return CFStringCreateWithBytes(
            kCFAllocatorDefault,
            reinterpret_cast<const UInt8*>(s.data()),
            CFIndex(s.size() * sizeof(char32_t)),
            kCFStringEncodingUTF32LE,
            false
        );
    }

    static std::u8string cf_to_utf8(CFStringRef str)
    {
        if (!str) return {};
        CFIndex length = CFStringGetLength(str);
        CFIndex used = 0;
        CFStringGetBytes(str, CFRangeMake(0, length), kCFStringEncodingUTF8, 0, false, nullptr, 0, &used);
        std::u8string result(used, '\0');
        CFStringGetBytes(str, CFRangeMake(0, length), kCFStringEncodingUTF8, 0, false,
                         reinterpret_cast<UInt8*>(result.data()), used, nullptr);
        return result;
    }

    static U16String cf_to_utf16(CFStringRef str)
    {
        if (!str) return {};
        CFIndex length = CFStringGetLength(str);
        U16String result(length, u'\0');
        CFStringGetCharacters(str, CFRangeMake(0, length),
                              reinterpret_cast<UniChar*>(result.data()));
        return result;
    }

    static U32String cf_to_utf32(CFStringRef str)
    {
        if (!str) return {};
        CFIndex length = CFStringGetLength(str);
        CFIndex used = 0;
        CFStringGetBytes(str, CFRangeMake(0, length), kCFStringEncodingUTF32LE, 0, false, nullptr, 0, &used);
        U32String result(used / CFIndex(sizeof(char32_t)), U'\0');
        CFStringGetBytes(str, CFRangeMake(0, length), kCFStringEncodingUTF32LE, 0, false,
                         reinterpret_cast<UInt8*>(result.data()), used, nullptr);
        return result;
    }
}

    SQUARE_API std::u8string to_utf8(const U16String &s)
    {
        CFStringRef str = cf_from_utf16(s);
        auto result = cf_to_utf8(str);
        CFRelease(str);
        return result;
    }

    SQUARE_API std::u8string to_utf8(const U32String &s)
    {
        CFStringRef str = cf_from_utf32(s);
        auto result = cf_to_utf8(str);
        CFRelease(str);
        return result;
    }

    SQUARE_API U16String to_utf16(const std::u8string &s)
    {
        CFStringRef str = cf_from_utf8(s);
        auto result = cf_to_utf16(str);
        CFRelease(str);
        return result;
    }

    SQUARE_API U16String to_utf16(const U32String &s)
    {
        CFStringRef str = cf_from_utf32(s);
        auto result = cf_to_utf16(str);
        CFRelease(str);
        return result;
    }

    SQUARE_API U32String to_utf32(const std::u8string &s)
    {
        CFStringRef str = cf_from_utf8(s);
        auto result = cf_to_utf32(str);
        CFRelease(str);
        return result;
    }

    SQUARE_API U32String to_utf32(const U16String &s)
    {
        CFStringRef str = cf_from_utf16(s);
        auto result = cf_to_utf32(str);
        CFRelease(str);
        return result;
    }

    SQUARE_API U16String to_ucs2(const std::u8string &s)
    {
        CFStringRef str = cf_from_utf8(s);
        auto result = cf_to_utf16(str);
        CFRelease(str);
        return result;
    }

    SQUARE_API U32String to_ucs4(const std::u8string &s) { return to_utf32(s); }
    SQUARE_API U32String to_ucs4(const U16String &s) { return to_utf32(s); }

///////////////////////////////////////////////////////////////////////////////
#else
// UNIX — iconv (POSIX)
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#  define SQ_UTF16_NATIVE "UTF-16BE"
#  define SQ_UTF32_NATIVE "UTF-32BE"
#else
#  define SQ_UTF16_NATIVE "UTF-16LE"
#  define SQ_UTF32_NATIVE "UTF-32LE"
#endif

namespace
{
    static bool iconv_convert(
        const char* from_enc, const char* to_enc,
        const char* in_data, size_t in_bytes,
        String& out)
    {
        iconv_t cd = iconv_open(to_enc, from_enc);
        if (cd == (iconv_t)-1) return false;

        out.resize(in_bytes * 4 + 4);
        char*  in_ptr   = const_cast<char*>(in_data);
        size_t in_left  = in_bytes;
        char*  out_ptr  = out.data();
        size_t out_left = out.size();

        while (in_left > 0)
        {
            size_t ret = iconv(cd, &in_ptr, &in_left, &out_ptr, &out_left);
            if (ret == (size_t)-1)
            {
                if (errno == E2BIG)
                {
                    size_t done = out.size() - out_left;
                    out.resize(out.size() * 2);
                    out_ptr  = out.data() + done;
                    out_left = out.size() - done;
                }
                else { iconv_close(cd); return false; }
            }
        }

        out.resize(out.size() - out_left);
        iconv_close(cd);
        return true;
    }
}

    SQUARE_API std::u8string to_utf8(const U16String &s)
    {
        String out;
        iconv_convert(SQ_UTF16_NATIVE, "UTF-8",
                      reinterpret_cast<const char*>(s.data()), s.size() * sizeof(char16_t), out);
        return out;
    }

    SQUARE_API std::u8string to_utf8(const U32String &s)
    {
        String out;
        iconv_convert(SQ_UTF32_NATIVE, "UTF-8",
                      reinterpret_cast<const char*>(s.data()), s.size() * sizeof(char32_t), out);
        return out;
    }

    SQUARE_API U16String to_utf16(const std::u8string &s)
    {
        String out;
        iconv_convert("UTF-8", SQ_UTF16_NATIVE, s.data(), s.size(), out);
        return U16String(reinterpret_cast<const char16_t*>(out.data()), out.size() / sizeof(char16_t));
    }

    SQUARE_API U16String to_utf16(const U32String &s)
    {
        String out;
        iconv_convert(SQ_UTF32_NATIVE, SQ_UTF16_NATIVE,
                      reinterpret_cast<const char*>(s.data()), s.size() * sizeof(char32_t), out);
        return U16String(reinterpret_cast<const char16_t*>(out.data()), out.size() / sizeof(char16_t));
    }

    SQUARE_API U32String to_utf32(const std::u8string &s)
    {
        String out;
        iconv_convert("UTF-8", SQ_UTF32_NATIVE, s.data(), s.size(), out);
        return U32String(reinterpret_cast<const char32_t*>(out.data()), out.size() / sizeof(char32_t));
    }

    SQUARE_API U32String to_utf32(const U16String &s)
    {
        String out;
        iconv_convert(SQ_UTF16_NATIVE, SQ_UTF32_NATIVE,
                      reinterpret_cast<const char*>(s.data()), s.size() * sizeof(char16_t), out);
        return U32String(reinterpret_cast<const char32_t*>(out.data()), out.size() / sizeof(char32_t));
    }

    SQUARE_API U16String to_ucs2(const std::u8string &s)
    {
        String out;
        iconv_convert("UTF-8", "UCS-2LE", s.data(), s.size(), out);
        return U16String(reinterpret_cast<const char16_t*>(out.data()), out.size() / sizeof(char16_t));
    }

    SQUARE_API U32String to_ucs4(const std::u8string &s) { return to_utf32(s); }
    SQUARE_API U32String to_ucs4(const U16String &s) { return to_utf32(s); }

#undef SQ_UTF16_NATIVE
#undef SQ_UTF32_NATIVE
#endif

}
