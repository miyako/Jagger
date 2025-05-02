// Jagger -- C++ implementation of Pattern-based Japanese Morphological Analyzer
//  $Id: train_jagger.cc 2070 2024-03-14 07:54:57Z ynaga $
// Copyright (c) 2022 Naoki Yoshinaga <ynaga@iis.u-tokyo.ac.jp>

//for CLI
#include <iostream>
#include <string>
#include <csignal>
#include <atomic>
#include <thread>

//for stdIn,stdOut
#include <sstream>
#include <iostream>

//for path
#include <filesystem>

#ifdef _WIN32
#include <Shlobj.h>
#include <io.h>
#include <Shlwapi.h>//PathFindExtension
#define SEP "\\"
#define SEPCHAR '\\'
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#define SEP "/"
#define SEPCHAR '/'
#endif
#ifndef _WIN32
//because jagger source is posix
#define _isatty ::isatty
#define _mmap ::mmap
#define _munmap ::munmap
#define __open ::open
#define __lseek ::lseek
#define _close ::close
#define _fopen std::fopen
#else
#include "getopt.h"
#define PROT_READ    0x1  // Pages can be read
#define PROT_WRITE   0x2  // Pages can be written to
#define PROT_EXEC    0x4  // Pages can be executed
#define PROT_NONE    0x0  // Pages cannot be accessed
#define PAGE_READONLY    0x02
#define PAGE_READWRITE   0x04
#define PAGE_EXECUTE     0x10
#define PAGE_NOACCESS    0x01
#define MAP_SHARED (FILE_MAP_READ | FILE_MAP_WRITE)
FILE* _fopen(const char* filename, const char* mode)
{
    wchar_t    buf[_MAX_PATH];
    wchar_t    _wfmode[99];    //should be enough
    if (MultiByteToWideChar(CP_UTF8, 0, mode, -1, (LPWSTR)_wfmode, 99))
    {
        if (MultiByteToWideChar(CP_UTF8, 0, filename, -1, (LPWSTR)buf, _MAX_PATH))
        {
            return _wfopen((const wchar_t*)buf, (const wchar_t*)_wfmode);
        }
    }
    return  fopen(filename, mode);
}
void utf8_to_wide(const char* utf8_str, std::wstring& utf16_str)
{
    if (utf8_str == NULL) {
        return;
    }

    int len = MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, NULL, 0);
    if (len == 0) {
        return;
    }

    std::vector<unsigned char>buf((len + 1) * sizeof(wchar_t));
    if (MultiByteToWideChar(CP_UTF8,
        0, utf8_str,
        -1,
        (LPWSTR)&buf[0],
        len)) {
        utf16_str = std::wstring((const wchar_t*)&buf[0]);
    }

    return;
}
void wide_to_utf8(const wchar_t* utf16_str, std::string& utf8_str)
{
    if (utf16_str == NULL) {
        return;
    }

    int len = WideCharToMultiByte(CP_UTF8, 0, utf16_str, -1, NULL, 0, NULL, NULL);
    if (len == 0) {
        return;
    }

    std::vector<unsigned char>buf((len + 1) * sizeof(char));
    if (WideCharToMultiByte(CP_UTF8,
        0, utf16_str,
        -1,
        (LPSTR)&buf[0],
        len, NULL, NULL)) {
        utf8_str = std::string((const char*)&buf[0]);
    }

    return;
}

void* _mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset, const std::string& fn)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    if (fd != -1) {
        hFile = (HANDLE)_get_osfhandle(fd);  // Convert file descriptor to HANDLE
        if (hFile == INVALID_HANDLE_VALUE) return NULL;  // Invalid file handle
    }

    std::wstring name;
    utf8_to_wide(fn.c_str(), name);
    const wchar_t* fileName = PathFindFileName(name.c_str());

    // try to open file mapping
    HANDLE hMap = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        name.c_str());

    if (hMap == NULL) {
        //create file mapping
        BY_HANDLE_FILE_INFORMATION fileInfo;
        if (GetFileInformationByHandle(hFile, &fileInfo)) {
            DWORD dwSizeHigh = fileInfo.nFileSizeHigh;
            DWORD dwSizeLow = fileInfo.nFileSizeLow;
            hMap = CreateFileMapping(hFile,
                NULL,
                PAGE_READWRITE,
                dwSizeHigh,
                dwSizeLow,
                fileName);
        }
        if (hMap == NULL) {
            hMap = CreateFileMapping(hFile,
                NULL,
                PAGE_READWRITE,
                0,
                0,
                NULL);
        }
    }

    if (hMap == NULL) {
        DWORD lastError = GetLastError();
        return NULL;
    }

    // Create a view of the file (map memory)
    void* mappedAddr = MapViewOfFile(hMap,
        FILE_MAP_WRITE,
        0,
        0,
        length);

    if (mappedAddr == NULL) {
        DWORD lastError = GetLastError();
        CloseHandle(hMap);
        return NULL;
    }

    // Return mapped memory address
    return mappedAddr;
}
void _munmap(void* addr, size_t length)
{
    UnmapViewOfFile(addr);
}
int __open(const char* utf8_path, int oflag, ...)
{
    std::wstring wide_path;

    utf8_to_wide(utf8_path, wide_path);
    if (wide_path.length() == 0) {
        return -1;  // Conversion failed
    }

    // Use CreateFileW to open the file with the appropriate flags
    HANDLE hFile = CreateFileW(
        wide_path.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD lastError = GetLastError();
        return -1;  // Return error if CreateFileW fails
    }

    // Convert the file handle to a file descriptor (for compatibility with _open)
    return _open_osfhandle((intptr_t)hFile, 0);  // The second argument 0 indicates no extra flags
}
off_t __lseek(int fd, off_t offset, int whence)
{
    // Convert the file descriptor to a Windows HANDLE
    HANDLE hFile = (HANDLE)_get_osfhandle(fd);

    if (hFile == INVALID_HANDLE_VALUE) {
        return -1;  // Invalid file descriptor
    }

    DWORD dwMoveMethod = 0;
    switch (whence) {
    case SEEK_SET:
        dwMoveMethod = FILE_BEGIN;
        break;
    case SEEK_CUR:
        dwMoveMethod = FILE_CURRENT;
        break;
    case SEEK_END:
        dwMoveMethod = FILE_END;
        break;
    default:
        return -1;  // Invalid whence value
    }

    // Move the file pointer to the specified offset
    LARGE_INTEGER liOffset;
    liOffset.QuadPart = offset;

    // Use SetFilePointerEx to set the file pointer
    LARGE_INTEGER liNewPointer;
    if (SetFilePointerEx(hFile, liOffset, &liNewPointer, dwMoveMethod)) {
        return (off_t)liNewPointer.QuadPart;
    }

    // If SetFilePointerEx fails, return -1
    return -1;
}
#endif

#define DEFAULT_DICNAME "jumandic-7.0-20130310"
#define DEFAULT_MODELNAME "kyoto+kwdlc"
#define NUM_POS_FIELD 4
#include "jagger.h"

namespace jagger {
    class tagger {
    private:
        ccedar::da_  _da;  // there may be cache friendly alignment
        uint16_t* _c2i; // UTF8 char and BOS -> id
        feat_info_t* _p2f; // pattern id -> feature (info)
        char* _fs;  // feature strings
        std::vector <std::pair <void*, size_t> > _mmaped;
        void* _read_array(const std::string& fn) {
            int fd = __open(fn.c_str(), O_RDONLY);
            if (fd == -1) {
                std::stringstream ss;
                ss << "no such file: " << fn.c_str();
                throw std::runtime_error(ss.str());
            }
            const size_t size = __lseek(fd, 0, SEEK_END); // get size
            __lseek(fd, 0, SEEK_SET);
            void* data = _mmap(0, size, PROT_READ, MAP_SHARED, fd, 0, fn);
            _close(fd);
            _mmaped.push_back(std::make_pair(data, size));
            return data;
        }
    public:
        tagger() : _da(), _c2i(0), _p2f(0), _fs(0), _mmaped() {}
        ~tagger() {
            for (size_t i = 0; i < _mmaped.size(); ++i)
                _munmap(_mmaped[i].first, _mmaped[i].second);
        }
        void read_model(const std::string& m) { // read patterns
            _da.set_array(_read_array(m + ".da"));
            _c2i = static_cast <uint16_t*> (_read_array(m + ".c2i"));
            _p2f = static_cast <feat_info_t*> (_read_array(m + ".p2f"));
            _fs = static_cast <char*> (_read_array(m + ".fs"));
        }
        void write_feature(std::ostringstream& output, const bool concat, const feat_info_t finfo) const {
            IF_COMPACT(output << std::string(&_fs[finfo.core_feat_offset], finfo.core_feat_len));
            if (concat) { // as unknown words
                IF_NOT_COMPACT(output << std::string(&_fs[finfo.feat_offset], finfo.core_feat_len));
                output << ",*,*,*\n" << std::endl;
            }
            else
                output << std::string(&_fs[finfo.feat_offset], finfo.feat_len);
        }
        void write_feature(simple_writer& writer, const bool concat, const feat_info_t finfo) const {
            IF_COMPACT(writer.write(&_fs[finfo.core_feat_offset], finfo.core_feat_len));
            if (concat) { // as unknown words
                IF_NOT_COMPACT(writer.write(&_fs[finfo.feat_offset], finfo.core_feat_len));
                writer.write(",*,*,*\n", 7);
            }
            else
                writer.write(&_fs[finfo.feat_offset], finfo.feat_len);
        }
        template <const bool TAGGING, const bool TTY>
        void run(std::ostringstream& output) const {
            union { struct { uint32_t shift : MAX_PATTERN_BITS, ctype : 4, id : 20; bool concat : 1; }; int r; } s_prev = {}, s = {};
            feat_info_t finfo = { _c2i[CP_MAX + 1] }; // BOS
            simple_reader reader;
            //      simple_writer writer;
            for (; !reader.eob();) {
                if ((*reader.ptr() == '\n')) { // EOS
                    if (s_prev.r)
                        if (TAGGING) {
                            //                  write_feature (writer, s_prev.concat, finfo);
                            write_feature(output, s_prev.concat, finfo);
                        }
                    //writer.write (TAGGING ? "EOS\n" : "\n", TAGGING ? 4 : 1);
                    if (TAGGING) {
                        output << "EOS";
                    }
                    output << std::endl;

                    s.shift = 1;
                    s_prev.r = 0; // *
                    finfo.ti = _c2i[CP_MAX + 1]; // BOS
                    if (TTY) {
                        //                writer.flush (); // line buffering
                        output << std::flush;
                    }
                    if (reader.eob()) {
                        break;
                    }
                }
                else {
                    s.r = _da.longestPatternSearch(reader.ptr(), reader.end(), finfo.ti, _c2i);
                    if (!s.shift) s.shift = u8_len(reader.ptr());
                    if (s_prev.r &&  // word that may concat with the future context
                        !(s.concat = (s_prev.ctype == s.ctype && // char type mismatch
                            s_prev.ctype != OTHER &&   // kanji, symbol
                            (s_prev.ctype != KANA || s_prev.shift + s.shift < 18)))) {
                        if (TAGGING) {
                            //                  write_feature (writer, s_prev.concat, finfo);
                            write_feature(output, s_prev.concat, finfo);
                        }
                        else {
                            //                  writer.write (" ", 1);
                            output << "\n";
                        }
                    }
                    finfo = _p2f[s.id];
                    s_prev = s; // *
                    //          writer.write (reader.ptr (), s.shift);
                    output << std::string(reader.ptr(), s.shift).c_str();
                }
                reader.advance(s.shift);
                if (!TTY /*&& ! writer.writable (1 << MAX_FEATURE_BITS)*/) output << std::flush;/* writer.flush ();*/
                if (TTY && reader.eob()) reader.read();
                if (!TTY && !reader.readable(1 << MAX_PATTERN_BITS)) reader.read();
            }
            if (s_prev.r) {
                if (TAGGING) {
                    //              write_feature (writer, s_prev.concat, finfo);
                    write_feature(output, s_prev.concat, finfo);
                }
                //        writer.write (TAGGING ? "EOS\n" : "\n", TAGGING ? 4 : 1);
                if (TAGGING) {
                    output << "EOS";
                }
                output << std::endl;
            }

            output << std::flush;
        }
    };
    static const char* FEAT_UNK = "\x09\xE5\x90\x8D\xE8\xA9\x9E\x2C\xE6\x99\xAE\xE9\x80\x9A\xE5\x90\x8D\xE8\xA9\x9E\x2C\x2A\x2C\x2A";
    static const char* FEAT_NUM = "\x09\xE5\x90\x8D\xE8\xA9\x9E\x2C\xE6\x95\xB0\xE8\xA9\x9E\x2C\x2A\x2C\x2A";
    static const char* FEAT_SYMBOL = "\x09\xE7\x89\xB9\xE6\xAE\x8A\x2C\xE8\xA8\x98\xE5\x8F\xB7\x2C\x2A\x2C\x2A";
    static int UC_SYMBOL_RANGE[][2] = { {0x0021, 0x002F}, {0x003A, 0x0040}, {0x005B, 0x0060}, {0x007B, 0x007E}, {0x00A1, 0x00BF}, {0x00D7, 0x00D7}, {0x00F7, 0x00F7}, {0x2000, 0x206F}, {0x20A0, 0x214F}, {0x2190, 0x2BFF}, {0x3000, 0x3004}, {0x3008, 0x303F}, {0x3200, 0x33FF}, {0xFE30, 0xFE4F}, {0xFE50, 0xFE6B}, {0xFF01, 0xFF0F}, {0xFF1A, 0xFF20}, {0xFF3B, 0xFF40}, {0xFF5B, 0xFF65}, {0xFFE0, 0xFFEF}, {0x10190, 0x1019C}, {0x1F000, 0x1FBFF}, {} }; // Symbol-like Unicode Blocks
    static const char* chars_[] = { "0123456789０１２３４５６７８９〇一二三四五六七八九十百千万億兆数・", "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺ", "ァアィイゥウェエォオカガキギクグケゲコゴサザシジスズセゼソゾタダチヂッツヅテデトドナニヌネノハバパヒビピフブプヘベペホボポマミムメモャヤュユョヨラリルレロヮワヰヱヲンヴヵヶヷヸヹヺーヽヾヿ", 0 }; // characters for concatenation
    class pattern_builder { // build patterns from training data and dictinary
    private:
        bag_t <std::string> _tbag, _fbag;
        std::vector <pat_info_t> _pi2sf; // pi -> <surf, prev_pos, shift, fi, count>
        std::vector <std::pair <size_t, int> > _ccnt;
        template <typename T>
        static inline void _write_array(const T* const data, const size_t size, const std::string& fn) {
            FILE* fp = _fopen(fn.c_str(), "wb");
            if (!fp) {
                std::stringstream ss;
                ss << "cannot write to: " << fn.c_str();
                throw std::runtime_error(ss.str());
            }
            std::fwrite(data, sizeof(T), size, fp);
            std::fclose(fp);
        }
        static const char* _strchr_n(const char* p, int c, int n) // find nth c
        {
            do if (n-- && (p = std::strchr(p, c))) ++p; else return --p; while (1);
        }
        // examine UTF8 sequence p consist of only numeric / alpha / kana characters
        static int check_ctype(const char* p, const int len, std::vector<char>& char_t, int n = ANY) {
            for (int b(0), offset(0); n && offset < len; offset += b) {
                size_t u = unicode(p + offset, b);
                if (u < char_t.size()) {
                    n &= char_t[u];
                }
                else {
                    std::fprintf(stderr, "(%ld) char_t index out of bounds:%ld\n", __LINE__, u);
                }
            }  
            return n;
        }
    public:
        pattern_builder() : _tbag(), _fbag(), _pi2sf(), _ccnt() {}
        ~pattern_builder() {}
        void extract_patterns(const std::string& train, const std::vector <std::string>& dict) {
            auto startTime = std::chrono::high_resolution_clock::now();
            bag_t <std::pair <std::string, int> >  pbag; // pattern -> pi
            std::vector <std::map <std::pair <int, int>, int> > pi2sfic; // pi -> <shift, feature> -> count
            std::vector <std::map <int, int> > si2ti2fi; // unseen seed -> features
            std::vector <int> ti2c(1, -1); // counter to set features (core) for unk

            std::vector<char>char_t(CP_MAX + 1);

            const long max_plen = 1 << MAX_PATTERN_BITS;
            _tbag.to_i("\tBOS");
            _tbag.to_i(FEAT_UNK);    // t1
            _tbag.to_i(FEAT_NUM);    // t2
            _tbag.to_i(FEAT_SYMBOL); // t3
            if (!dict.empty()) { // read seeds from dictionary
                std::fprintf(stderr, "reading seed patterns from dictionary...");
                for (std::vector <std::string>::const_iterator it = dict.begin(); it != dict.end(); ++it) {
                    FILE* fp = _fopen(it->c_str(), "r");
                    if (!fp) {
                        std::stringstream ss;
                        ss << "cannot read from file: " << it->c_str();
                        throw std::runtime_error(ss.str());
                    }
                    for (char line[BUF_SIZE]; std::fgets(line, BUF_SIZE, fp); ) {

                        const size_t len = std::strlen(line);
                        const char* p(line), * surf(*p == '"' ? ++p : p);
                        const bool quoted = p != line;
                        p = _strchr_n(p, quoted ? '"' : ',', 1) + quoted;
                        if (p - surf - quoted > max_plen) {
                            std::stringstream ss;
                            ss << "increase MAX_PATTERN_BITS not to skip: " << std::string(line, p - line).c_str();
                            throw std::runtime_error(ss.str());
                        }
                        const int pi = pbag.to_i(std::make_pair(std::string(surf, p - surf - quoted), -1));
                        char* f = const_cast <char*> (_strchr_n(++p, ',', 3));
                        *f = '\t'; // POS starts with '\t'
                        p = _strchr_n(f, ',', NUM_POS_FIELD);
                        const int ti(_tbag.to_i(f, p - f)), fi(_fbag.to_i(f, line + len - f));
                        si2ti2fi.resize(pbag.size());
                        si2ti2fi[pi].insert(std::make_pair(ti, fi)); // may not unique
                    }
                    std::fclose(fp);
                }
                std::fprintf(stderr, "done; %ld words, %ld features\n", si2ti2fi.size(), _fbag.size());
            }
            const int num_seed = static_cast <int> (pbag.size());
            std::fprintf(stderr, "registering concatenating chars and symbols as seed patterns...");
            for (int i(0), b(0); chars_[i]; ++i) // seed from numeric / alpha / kana
                for (const char* p = &chars_[i][0]; *p; pbag.to_i(std::make_pair(std::string(p, b), -1)), p += b) {
                    size_t u = unicode(p, b);
                    if (u < char_t.size()) {
                        char_t[u] = 1 << i;
                    }
                    else {
                        std::fprintf(stderr, "(%ld) char_t index out of bounds:%ld\n", __LINE__, u);
                    }
                }
            for (int i = 0; UC_SYMBOL_RANGE[i][0]; ++i)
                for (int j = UC_SYMBOL_RANGE[i][0]; j <= UC_SYMBOL_RANGE[i][1]; ++j) {
                    int k(j), b((j > 0xffff) + (j > 0x7ff) + (j > 0x7f));
                    char c[5] = { "\0\xc0\xe0\xf0"[b] };
                    for (c[0] |= k >> (6 * b); b; k >>= 6)
                        c[b--] = 0x80 | (k & 0x3f);
                    pbag.to_i(std::make_pair(&c[0], -1));
                }
            std::fprintf(stderr, "done.\n");
            ti2c.resize(_tbag.size(), 0);
            pi2sfic.resize(pbag.size());
            std::fprintf(stderr, "mining patterns from training data...");
            { // notations follow https://aclanthology.org/2023.acl-short.2/
                std::string cs; // sequence of characters
                std::vector <std::pair <size_t, std::string> > ss; // tokens <len(w), t>
                FILE* fp = _fopen(train.c_str(), "r");
                if (!fp) {
                    std::stringstream ss;
                    ss << "cannot read from: " << train.c_str();
                    throw std::runtime_error(ss.str());
                }
                for (char line[BUF_SIZE]; std::fgets(line, BUF_SIZE, fp); ) {
                    const size_t len = std::strlen(line);
                    if (std::strncmp(line, "EOS\n", 4) == 0) {
                        for (size_t i(0), j(0), ti(0), ti_prev(0); j < ss.size(); i += ss[j].first, ti_prev = ti, ++j) {
                            const long shift(ss[j].first), fi(_fbag.to_i(ss[j].second));
                            if (!fp) {
                                std::stringstream ss;
                                ss << "increase MAX_PATTERN_BITS not to skip: " << cs.substr(i, shift).c_str();
                                throw std::runtime_error(ss.str());
                            }
                            for (int k = shift; i + k <= cs.size() && k <= max_plen; k += u8_len(&cs[i + k])) {
                                const int pi_max = pbag.size();
                                const int pi = pbag.to_i(std::make_pair(cs.substr(i, k), -1));
                                const int pi_ = pbag.to_i(std::make_pair(cs.substr(i, k), ti_prev));
                                pi2sfic.resize(pbag.size());
                                ++pi2sfic[pi][std::make_pair(shift, fi)];
                                ++pi2sfic[pi_][std::make_pair(shift, fi)];
                                if (pi >= pi_max) break; // skip pattern extension; heuristics
                            }
                            const char* fs = ss[j].second.c_str();
                            const int n = pbag.to_i(std::make_pair(cs.substr(i, shift), -1));
                            ti = _tbag.to_i(fs, _strchr_n(fs, ',', NUM_POS_FIELD) - fs);
                            if (n >= num_seed && check_ctype(&cs[i], shift, char_t) != NUM) { // for unseen tokens
                                ti2c.resize(_tbag.size(), 0); // fi -> _fbag.size (); bug fix
                                ++ti2c[ti];
                                const int pi = pbag.to_i(std::make_pair("", ti_prev));
                                const int fi_unk = _fbag.to_i(_tbag.to_s(ti) + ",*,*,*\n");
                                pi2sfic.resize(pbag.size());
                                ++pi2sfic[pi][std::make_pair(0, fi_unk)];
                            }
                        }
                        cs.clear();
                        ss.clear();
                    }
                    else { // token
                        const char* const p(line), * f(_strchr_n(p, '\t', 1));
                        cs += std::string(p, f - p);
                        ss.push_back(std::make_pair(f - p, std::string(f, p + len - f)));
                    }
                }
                std::fclose(fp);
            }
            std::fprintf(stderr, "done; %ld pattern candidates\n", pbag.size());
            { // pruning patterns
                ccedar::da <char, int> patterns;
                for (size_t i = 0; i < CP_MAX + 1 + _tbag.size(); ++i) {
                    _ccnt.push_back(std::make_pair(0, _ccnt.size()));
                }
                std::fprintf(stderr, "pruning patterns...");
                long max_ti = std::max_element(ti2c.begin(), ti2c.end()) - ti2c.begin();
                for (bag_t <std::pair <std::string, int> >::iter it = pbag.begin(); it != pbag.end(); ++it) {

                    const std::pair <std::string, int>& p = it->first;
                    const std::string& c = p.first;
                    int pi(it->second), ti_prev(it->first.second), shift(c.size()), fi(0), count(0);
                    if (pi2sfic[pi].empty()) { // unseen patterns
                        if (pi < num_seed) { // dictionary words
                            const std::map <int, int>& ti2fi = si2ti2fi[pi];
                            int ti = 0;
                            for (std::map <int, int>::const_iterator jt = ti2fi.begin();
                                jt != ti2fi.end(); ++jt)
                                if (ti2c[jt->first] >= ti2c[ti])
                                    ti = jt->first;
                            fi = ti2fi.find(ti)->second;
                        }
                        else if (check_ctype(c.c_str(), shift, char_t) == NUM)
                            fi = _fbag.to_i(std::string(FEAT_NUM) + ",*,*,*\n");
                        else if (check_ctype(c.c_str(), shift, char_t) != OTHER)
                            fi = _fbag.to_i(_tbag.to_s(max_ti) + "," + c + "," + c + ",*\n");
                        else
                            fi = _fbag.to_i(std::string(FEAT_SYMBOL) + ",*,*,*\n");
                    }
                    else { // perform pruning for seen patterns
                        const std::map <std::pair <int, int>, int>& sfi2c = pi2sfic[pi];
                        std::vector <int> s2c(max_plen + 1, 0);
                        for (std::map <std::pair <int, int>, int>::const_iterator jt = sfi2c.begin();
                            jt != sfi2c.end(); ++jt)
                            s2c[jt->first.first] += jt->second;
                        shift = -std::distance(s2c.rend(), std::max_element(s2c.rbegin(), s2c.rend())) - 1;
                        for (std::map <std::pair <int, int>, int>::const_iterator jt = sfi2c.begin();
                            jt != sfi2c.end(); ++jt)
                            if (jt->first.first == shift && jt->second > count)
                                count = jt->second, // used to s2c
                                fi = jt->first.second;
                        const pat_info_t* r = 0;
                        for (size_t from(0), pos(0); pos < c.size(); ) {
                            const int n_ = patterns.traverse(c.c_str(), from, pos, pos + 1);
                            if (n_ == ccedar::NO_VALUE) continue;
                            if (n_ == ccedar::NO_PATH)  break;
                            r = &_pi2sf[n_];
                        }
                        if (r && shift == r->shift && fi == r->fi) continue;
                    }
                    const int ctype = check_ctype(c.c_str(), shift, char_t, shift ? ANY : OTHER); // NUM -> OTHER
                    // count each character and prev POS for count-based indexing
                    for (int i(0), b(0), len(c.size()); i < len; i += b) {
                        size_t u = unicode(&c[i], b);
                        if (u < _ccnt.size()) {
                            _ccnt[u].first += count + 1;
                        }
                        else {
                            std::fprintf(stderr, "(%ld) _ccnt index out of bounds:%ld\n", __LINE__, u);
                        }
                    }
                    if (ti_prev != -1) {
                        size_t u = CP_MAX + 1 + ti_prev;
                        if (u < _ccnt.size()) {
                            _ccnt[u].first += count + 1;
                        }
                        else {
                            std::fprintf(stderr, "(%ld) _ccnt index out of bounds:%ld\n", __LINE__, u);
                        }
                    }
                    else {// record surface-only patterns for pruning
                        patterns.update(c.c_str(), c.size()) = static_cast <int> (_pi2sf.size());
                    }
                    _pi2sf.push_back(pat_info_t(c, p.second, count, shift, ctype, fi));
                }
            }
            std::fprintf(stderr, "done; %ld -> %ld patterns\n", pi2sfic.size(), _pi2sf.size());
        }

        void write_patterns(const std::string& m) { // output compiled patterns
            auto startTime = std::chrono::high_resolution_clock::now();
            std::fprintf(stderr, "building DA trie from patterns..");

            bag_t <std::pair <int, int> > fsbag;
            bag_t <std::string> fbag; // core (sorted, compressed)
            ccedar::da_ da;
            IF_COMPACT(fbag.to_i(",*,*,*\n")); // f0: features for unk (lex)
            IF_NOT_COMPACT(fbag.to_i(std::string(FEAT_UNK) + ",*,*,*\n")); // f0: unk
            fsbag.to_i(std::make_pair(0, 1)); // unk <f0, t1>
            // save c2i
            std::sort(_ccnt.rbegin(), _ccnt.rend() - 1);
            std::vector <uint16_t> c2i(_ccnt.size());
            for (size_t i = 1; i < _ccnt.size() && _ccnt[i].first; ++i) {
                size_t u = _ccnt[i].second;
                if (u < _ccnt.size()) {
                    c2i[u] = static_cast <uint16_t> (i);
                }
                else {
                    std::fprintf(stderr, "(%ld) c2i index out of bounds:%ld\n", __LINE__, u);
                }
            }
            _write_array(c2i.data(), CP_MAX + 2, m + ".c2i"); // chop POS except BOS
            FILE* writer = _fopen(m.c_str(), "w");

            std::sort(_pi2sf.rbegin(), _pi2sf.rend());//this takes time!

            for (std::vector <pat_info_t>::iterator it = _pi2sf.begin(); it != _pi2sf.end(); ++it) { // output pattern

                it->print(writer, _tbag, _fbag);
                const std::string& fs = _fbag.to_s(it->fi);
                const int ti_prev = it->ti_prev;
                size_t pos = _strchr_n(fs.c_str(), ',', NUM_POS_FIELD) - fs.c_str();
                const int ti = _tbag.to_i(fs.substr(0, pos)); // core
                IF_NOT_COMPACT(pos = 0);                       // lemma -> core + lemma
                const int fi = fbag.to_i(fs.substr(pos));
                const int pi = fsbag.to_i(std::make_pair(fi, ti));
                // save pattern trie
                std::vector <int> pv;
                for (int i(0), b(0), len(it->surf.size()); i < len; i += b) {
                    size_t u = unicode(&it->surf[i], b);
                    if (u < c2i.size()) {
                        pv.push_back(c2i[u]);
                    }
                    else {
                        std::fprintf(stderr, "(%ld) c2i index out of bounds:%ld\n", __LINE__, u);
                    }
                }
                if (ti_prev + 1) {
                    size_t u = CP_MAX + 1 + ti_prev;
                    if (u < c2i.size()) {
                        pv.push_back(c2i[u]);
                    }
                    else {
                        std::fprintf(stderr, "(%ld) c2i index out of bounds:%ld\n", __LINE__, u);
                    }
                }
                union { struct { uint32_t shift : MAX_PATTERN_BITS, ctype : 4, id : 20; bool : 1; }; int r; } s = { { it->shift, it->ctype, static_cast <uint32_t> (pi) } };

                if (pv.size() != 0) {
                    da.update(&pv[0], pv.size()) = s.r;
                }else{
                        std::fprintf(stderr, "(%ld) attempt to insert zero-length key\n", __LINE__);
                }

            }
            std::fclose(writer);
            _write_array(da.array(), da.size(), m + ".da");
            // save feature strings
            std::vector <size_t> offsets, offsets_;
            writer = _fopen((m + ".fs").c_str(), "wb");
            IF_COMPACT(const size_t base_offset = _tbag.serialize(writer, offsets_));
            fbag.serialize(writer, offsets); // (core +) lemma
            fflush(writer);
            std::fclose(writer);
            // save mapping from feature ID to feature strings
            feat_info_t finfo = { 0 };
            std::vector <feat_info_t> p2f(fsbag.size(), finfo);
            for (size_t pi = 0; pi < fsbag.size(); ++pi) {
                const int fi(fsbag.to_s(pi).first), ti(fsbag.to_s(pi).second);
                p2f[pi].ti = c2i[CP_MAX + 1 + ti];
                p2f[pi].core_feat_len = _tbag.to_s(ti).size();
                p2f[pi].feat_len = fbag.to_s(fi).size();
                IF_COMPACT(p2f[pi].core_feat_offset = offsets_[ti]);
                IF_COMPACT(p2f[pi].feat_offset = base_offset + offsets[fi]);
                IF_NOT_COMPACT(p2f[pi].feat_offset = offsets[fi]);
            }
            _write_array(p2f.data(), p2f.size(), m + ".p2f");
            std::fprintf(stderr, "done.\n");
        }
    };
}

static void getParentDir(std::string& parentdir) {

#if __APPLE__
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::vector<char> buffer(size);
    _NSGetExecutablePath(buffer.data(), &size);
    std::string executablepath = std::string(buffer.data());
    parentdir = std::filesystem::path(executablepath).parent_path().parent_path().string();
#else
    wchar_t buffer[_MAX_PATH] = { 0 };
    GetModuleFileNameW(nullptr, buffer, _MAX_PATH);
    std::wstring executablepath(buffer);
    wide_to_utf8(std::filesystem::path(executablepath).parent_path().parent_path().wstring().c_str(), parentdir);
#endif
    parentdir += SEP;
}

static bool create_folder(std::string& m) {

    bool success = false;

#if __APPLE__
    NSString* path = (NSString*)CFStringCreateWithFileSystemRepresentation(kCFAllocatorDefault, m.c_str());
    NSFileManager* fm = [[NSFileManager alloc]init];
    success = [fm createDirectoryAtPath : path
        withIntermediateDirectories : YES
        attributes : nil
        error : NULL];
    [path release] ;
    [fm release] ;
#else
    wchar_t    buf[_MAX_PATH];
    if (MultiByteToWideChar(CP_UTF8, 0, m.c_str(), -1, (LPWSTR)buf, _MAX_PATH))
    {
        success = SHCreateDirectory(NULL, (PCWSTR)buf);
    }

#endif

    return success;
}

int main (int argc, char** argv) {
  std::string m, u, train;

  std::string d;
  getParentDir(d);
  d += ("dict" SEP DEFAULT_DICNAME SEP "dict");

  std::vector <std::string> dict;
  { // options (minimal)
    extern char *optarg;
    extern int optind;
    for (int opt = 0; (opt = getopt (argc, argv, "m:d:u:")) != -1; )
      switch (opt) {
      case 'm':
      {
          m = optarg;
          if (!m.empty()) {
              char lastChar = m.back();
              if (lastChar == SEPCHAR) {
                  m.pop_back();
              }
              m += SEP;
          }
          break;
      }
        case 'd': 
        {
            d = optarg;
            if (!d.empty()) {
                char lastChar = d.back();
                if (lastChar == SEPCHAR) {
                    d.pop_back();
                }
                dict.insert(dict.begin(), d);
            }
            break;
        }
        case 'u': 
        {
            u = optarg;
            if (!u.empty()) {
                char lastChar = u.back();
                if (lastChar == SEPCHAR) {
                    u.pop_back();
                }
                dict.push_back(optarg);
            }
            break;
        }
      }
    if (optind == argc || m.empty ()) errx (1, "Extract patterns for Jagger from dictionary and training data\nCopyright (c) 2023- Naoki Yoshinaga, All rights reserved.\n\nUsage: %s [-m dir -d dict -u dict] train\n\nOptions:\n -m dir \tdirectory to store patterns\n -d dict\tdictionary in CSV format\n -u user_dict\tuser-defined dictionary in CSV format\n", argv[0]);
    train = argv[optind];
  }
 
  jagger::pattern_builder builder;

  //try{
      builder.extract_patterns(train, dict);
      create_folder(m);
      builder.write_patterns(m + "patterns");
  //}
  //catch (const std::runtime_error& e) {
  
  //}

  return 0;
}
