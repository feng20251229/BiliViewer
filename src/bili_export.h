#pragma once
#ifdef BILI_EXPORTS
#define BILI_API __declspec(dllexport)
#else
#define BILI_API __declspec(dllimport)
#endif

extern "C" {
    BILI_API int  Bili_Init(int fromWeb);
    BILI_API void Bili_Shutdown();
    BILI_API int  Bili_Count();
    BILI_API long long Bili_ViewAt(int i);
    BILI_API int  Bili_Progress(int i);
    BILI_API int  Bili_Duration(int i);
    BILI_API int  Bili_Title(int i, char* buf, int len);
    BILI_API int  Bili_Tag(int i, char* buf, int len);
    BILI_API int  Bili_Bvid(int i, char* buf, int len);
    BILI_API int  Bili_Business(int i, char* buf, int len);
    BILI_API int  Bili_TimeSeconds(const char* start, const char* end);
    BILI_API double Bili_CompletionRate();
}