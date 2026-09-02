using System.Diagnostics;
using System.IO;
using System.Text;

namespace HostsEditor_GUI.Services;

public sealed class CmdRunner
{
    private static readonly UTF8Encoding Utf8Strict = new(false, true);

    public string ExecutablePath { get; }

    public CmdRunner(string executablePath)
    {
        ExecutablePath = executablePath;
    }

    public static string DefaultExecutablePath()
        => Path.Combine(AppContext.BaseDirectory, "HostsEditor-Cmd.exe");

    public async Task<string> RunAsync(string arguments, CancellationToken ct = default)
    {
        if (!File.Exists(ExecutablePath))
            throw new FileNotFoundException("未找到 HostsEditor-Cmd.exe，请重新生成项目。", ExecutablePath);

        var psi = new ProcessStartInfo
        {
            FileName = ExecutablePath,
            Arguments = arguments,
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            CreateNoWindow = true,
        };

        using var process = new Process { StartInfo = psi };
        process.Start();

        var outStream = new MemoryStream();
        var errStream = new MemoryStream();
        var copyOut = process.StandardOutput.BaseStream.CopyToAsync(outStream);
        var copyErr = process.StandardError.BaseStream.CopyToAsync(errStream);

        await process.WaitForExitAsync(ct);
        await Task.WhenAll(copyOut, copyErr);

        var stdout = Decode(outStream.ToArray());
        var stderr = Decode(errStream.ToArray());

        if (process.ExitCode != 0)
            throw new InvalidOperationException($"命令执行失败（退出码 {process.ExitCode}）{Environment.NewLine}{stderr}".Trim());

        return stdout;
    }

    private static string Decode(byte[] data)
    {
        if (data.Length == 0) return "";
        try
        {
            return Utf8Strict.GetString(data);
        }
        catch (DecoderFallbackException)
        {
            return Encoding.GetEncoding(936).GetString(data);
        }
    }
}
