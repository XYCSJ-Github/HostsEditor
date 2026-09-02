using HostsEditor_GUI.Models;

namespace HostsEditor_GUI.Services;

public static class HostsParser
{
    public static List<HostGroup> Parse(string output)
    {
        var groups = new List<HostGroup>();
        HostGroup? current = null;

        foreach (var raw in output.Split('\n'))
        {
            var line = raw.TrimEnd('\r').Trim();
            if (line.Length == 0) continue;

            if (line.StartsWith("IP:", StringComparison.Ordinal))
            {
                if (current is null)
                {
                    current = new HostGroup { Name = "Default", IsDefault = true };
                    groups.Add(current);
                }
                current.Entries.Add(ParseEntry(line));
            }
            else if (line.EndsWith(":", StringComparison.Ordinal))
            {
                var name = line[..^1].Trim();
                if (name.Length == 0) name = "Default";
                current = new HostGroup { Name = name, IsDefault = name.Equals("Default", StringComparison.OrdinalIgnoreCase) };
                groups.Add(current);
            }
        }

        return groups;
    }

    private static HostEntry ParseEntry(string line)
    {
        var entry = new HostEntry();
        foreach (var segment in line.Split('|'))
        {
            int idx = segment.IndexOf(':');
            if (idx < 0) continue;

            var key = segment[..idx];
            var value = segment[(idx + 1)..];

            switch (key)
            {
                case "IP":
                    entry.Ip = value;
                    break;
                case "Host":
                    entry.HostName = value;
                    break;
                case "Enable":
                    entry.IsEnabled = value.Equals("Yes", StringComparison.OrdinalIgnoreCase);
                    break;
            }
        }
        return entry;
    }
}
