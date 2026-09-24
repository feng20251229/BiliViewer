#include "bili_export.h"
#include "connect.h"
#include "data.h"
#include <cstring>
#include <string>
#include <cstdio>

static std::vector<BiliRecord>* g_records = nullptr;

static void DbgLog(const char* msg) {
    FILE* f = fopen("debug.log", "a");
    if (f) { fprintf(f, "[BiliNative] %s\n", msg); fclose(f); }
}

// MinGW 没有 strncpy_s，自己写一个
static void SafeCopy(char* buf, int len, const std::string& s) {
    if (len <= 0) return;
    size_t n = s.size();
    if (n > (size_t)(len - 1)) n = (size_t)(len - 1);
    memcpy(buf, s.data(), n);
    buf[n] = '\0';
}

BILI_API int Bili_Init(int fromWeb) {
    DbgLog("enter Bili_Init");

    if (g_records) {
        DbgLog("deleting old g_records");
        delete g_records;
        g_records = nullptr;
    }

    try {
        {
            DbgLog("before constructing Gethist");
            Gethist g(fromWeb ? Web : Data);
            DbgLog("after constructing Gethist");

            g_records = new std::vector<BiliRecord>(g.loadrecords());
            DbgLog("after copying records");
        }
        DbgLog("after Gethist destruct");

        char buf[64];
        sprintf(buf, "records count = %d", (int)g_records->size());
        DbgLog(buf);

        return (int)g_records->size();
    }
    catch (const std::exception& e) {
        DbgLog("C++ exception caught!");
        DbgLog(e.what());
        return -1;
    }
    catch (...) {
        DbgLog("unknown C++ exception");
        return -1;
    }
}

BILI_API void Bili_Shutdown() {
    delete g_records;
    g_records = nullptr;
}

BILI_API int Bili_Count() {
    return g_records ? (int)g_records->size() : 0;
}

BILI_API long long Bili_ViewAt(int i)     { return (*g_records)[i].view_timestamp; }
BILI_API int Bili_Progress(int i)         { return (*g_records)[i].progress; }
BILI_API int Bili_Duration(int i)         { return (*g_records)[i].duration; }

BILI_API int Bili_Title(int i, char* buf, int len) {
    SafeCopy(buf, len, (*g_records)[i].title);
    return (int)(*g_records)[i].title.size();
}

BILI_API int Bili_Tag(int i, char* buf, int len) {
    SafeCopy(buf, len, (*g_records)[i].tag);
    return (int)(*g_records)[i].tag.size();
}

BILI_API int Bili_Bvid(int i, char* buf, int len) {
    SafeCopy(buf, len, (*g_records)[i].bvid);
    return (int)(*g_records)[i].bvid.size();
}

BILI_API int Bili_Business(int i, char* buf, int len) {
    SafeCopy(buf, len, (*g_records)[i].business);
    return (int)(*g_records)[i].business.size();
}


BILI_API int Bili_TimeSeconds(const char* start, const char* end) {
    if (!g_records) return 0;
    Danalysis d(*g_records);
    return d.getTimeFromRecords(start, end);
}


BILI_API double Bili_CompletionRate() {
    if (!g_records) return 0;
    Danalysis d(*g_records);
    double total = 0; int cnt = 0;
    for (const auto& r : *g_records) {
        if (r.duration <= 0) continue;
        if (r.progress == -1) { total += 1; cnt++; continue; }
        total += (double)r.progress / r.duration;
        cnt++;
    }
    return cnt ? total / cnt * 100.0 : 0;
}