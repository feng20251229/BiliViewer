using System;
using System.Runtime.InteropServices;
using System.Text;

namespace BiliAnalyzer
{
    /// <summary>
    /// 对应 C++ 的 bili_export.h
    /// </summary>
    public static class BiliNative
    {
        private const string DLL = "BiliNative.dll";

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern int Bili_Init(int fromWeb);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Bili_Shutdown();

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern int Bili_Count();

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern long Bili_ViewAt(int i);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern int Bili_Progress(int i);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern int Bili_Duration(int i);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern int Bili_Title(int i, StringBuilder buf, int len);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern int Bili_Tag(int i, StringBuilder buf, int len);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern int Bili_Bvid(int i, StringBuilder buf, int len);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern int Bili_Business(int i, StringBuilder buf, int len);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern int Bili_TimeSeconds(string start, string end);

        [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern double Bili_CompletionRate();

        // ---- 封装一下，方便调用 ----

        public static string GetTitle(int i)
        {
            var sb = new StringBuilder(512);
            Bili_Title(i, sb, sb.Capacity);
            return sb.ToString();
        }

        public static string GetTag(int i)
        {
            var sb = new StringBuilder(256);
            Bili_Tag(i, sb, sb.Capacity);
            return sb.ToString();
        }

        public static string GetBvid(int i)
        {
            var sb = new StringBuilder(64);
            Bili_Bvid(i, sb, sb.Capacity);
            return sb.ToString();
        }

        public static string GetBusiness(int i)
        {
            var sb = new StringBuilder(64);
            Bili_Business(i, sb, sb.Capacity);
            return sb.ToString();
        }

        /// <summary>
        /// 时间戳(秒) -> "yyyy-MM-dd HH:mm"
        /// </summary>
        public static string FormatTime(long ts)
        {
            return DateTimeOffset.FromUnixTimeSeconds(ts)
                                 .LocalDateTime
                                 .ToString("yyyy-MM-dd HH:mm");
        }
    }
}