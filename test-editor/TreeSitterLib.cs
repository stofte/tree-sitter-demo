using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace test_editor
{
    public static unsafe partial class TreeSitterLib
    {
        public enum Language
        {
            NONE,
            JAVASCRIPT,
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct TSPoint
        {
            uint row;
            uint column;
        };

        public enum TSInputEncoding
        {
            TSInputEncodingUTF8,
            TSInputEncodingUTF16,
        }

        [LibraryImport("tslib.dll")]
        public static partial IntPtr initialize([MarshalAs(UnmanagedType.Bool)] bool log_to_stdout);

        [LibraryImport("tslib.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static partial bool set_language(IntPtr ctx, Language language, [MarshalAs(UnmanagedType.LPStr)] string str);

        [LibraryImport("tslib.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static partial bool parse_string(IntPtr ctx, [MarshalAs(UnmanagedType.LPStr)] string str, uint str_length, TSInputEncoding encoding);

        [LibraryImport("tslib.dll")]
        public static partial void print_syntax_tree(IntPtr ctx);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public delegate string? edit_string_callback(IntPtr payload, uint byte_index, TSPoint position, ref uint bytes_read);

        [LibraryImport("tslib.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static partial bool edit_string(
            IntPtr ctx,
            uint start_byte,
            uint old_end_byte,
            uint new_end_byte,
            uint start_point_row,
            uint start_point_column,
            uint old_end_point_row,
            uint old_end_point_column,
            uint new_end_point_row,
            uint new_end_point_column,
            edit_string_callback bufferReader,
            TSInputEncoding encoding
        );

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void highlights_callback(uint byte_index, uint byte_length, [MarshalAs(UnmanagedType.LPStr)] string capture_name);

        [LibraryImport("tslib.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static partial bool get_highlights(IntPtr ctx, uint byte_offset, uint byte_length, highlights_callback callback);

        [LibraryImport("tslib.dll")]
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static partial string syntax_tree(IntPtr ctx);
    }
}
