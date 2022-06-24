using System;
using System.IO;
using System.IO.Compression;
using System.Reflection;
using System.Runtime.InteropServices;

namespace UsmapCompressor;

public class Program
{
    public static void Main(string[]? args)
    {
        args ??= Array.Empty<string>();
        var usmapFilePath = string.Empty;

        if (args.Length == 0)
        {
            Console.Out.Write("Help");
        }

        foreach (var arg in args)
        {
            if (arg.StartsWith("-path="))
            {
                var path = arg.Split('=')[1];
                if (File.Exists(path)) usmapFilePath = path;
            }

            if (arg.StartsWith("-compression=") && !string.IsNullOrEmpty(usmapFilePath))
            {
                var compressionStr = arg.Split('=')[1];
                var compressions = compressionStr.Contains(',') ? compressionStr.Split(',') : new[] { compressionStr };

                if (compressions.Length > 0)
                {
                    var file = File.OpenRead(usmapFilePath);
                    byte[] data;

                    using (var memoryStream = new MemoryStream())
                    {
                        file.CopyTo(memoryStream);
                        data = memoryStream.ToArray();
                    }

                    foreach (var compression in compressions)
                    {
                        if (compression.Contains("oodle", StringComparison.OrdinalIgnoreCase))
                        {
                            LoadUnmanagedLibraryFromResource(Assembly.GetExecutingAssembly(), "UsmapCompressor.Resources.oo2core_4_win64.dll", "oo2core_4_win64.dll");

                            var output = Compress(data, data.Length, OodleFormat.Mermaid, OodleCompressionLevel.Optimal5);
                            File.WriteAllBytes(@"C:\Users\GMatrixGames\Desktop\MappingsTest\Oodle.usmap", output);
                        }
                        else if (compression.Contains("brotli", StringComparison.OrdinalIgnoreCase))
                        {
                            using var memory = new MemoryStream();
                            using (var brotli = new BrotliStream(memory, CompressionLevel.Optimal))
                            {
                                brotli.Write(data, 0, data.Length);
                            }

                            var output = memory.ToArray();
                            File.WriteAllBytes(@"C:\Users\GMatrixGames\Desktop\MappingsTest\Brotli.usmap", output);
                        }
                    }
                }
            }
        }
    }

    [DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
    public static extern IntPtr LoadLibrary(string dllToLoad);

    [DllImport("oo2core_4_win64.dll")]
    private static extern int OodleLZ_Compress(OodleFormat format, byte[] buffer, long bufferSize, byte[] outputBuffer, OodleCompressionLevel level, uint unused1, uint unused2, uint unused3);

    public static byte[] Compress(byte[] buffer, int size, OodleFormat format, OodleCompressionLevel level)
    {
        var compressedBufferSize = GetCompressionBound((uint) size);
        var compressedBuffer = new byte[compressedBufferSize];

        var compressedCount = OodleLZ_Compress(format, buffer, size, compressedBuffer, level, 0, 0, 0);

        var outputBuffer = new byte[compressedCount];
        Buffer.BlockCopy(compressedBuffer, 0, outputBuffer, 0, compressedCount);

        return outputBuffer;
    }

    private static uint GetCompressionBound(uint bufferSize)
    {
        return bufferSize + 274 * ((bufferSize + 0x3FFFF) / 0x40000);
    }

    public static void LoadUnmanagedLibraryFromResource(Assembly assembly,
        string libraryResourceName,
        string libraryName)
    {
        using (var s = assembly.GetManifestResourceStream(libraryResourceName))
        {
            var data = new BinaryReader(s).ReadBytes((int) s.Length);

            var assemblyPath = Path.GetDirectoryName(assembly.Location);
            var tempDllPath = Path.Combine(assemblyPath ?? "", libraryName);
            File.WriteAllBytes(tempDllPath, data);
        }

        LoadLibrary(libraryName);
    }
}

public enum OodleFormat : uint
{
    LZH,
    LZHLW,
    LZNIB,
    None,
    LZB16,
    LZBLW,
    LZA,
    LZNA,
    Kraken,
    Mermaid,
    BitKnit,
    Selkie,
    Akkorokamui
}

public enum OodleCompressionLevel : ulong
{
    None,
    SuperFast,
    VeryFast,
    Fast,
    Normal,
    Optimal1,
    Optimal2,
    Optimal3,
    Optimal4,
    Optimal5
}