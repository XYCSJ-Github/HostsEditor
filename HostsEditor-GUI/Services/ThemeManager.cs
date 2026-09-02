using System.Windows;
using System.Windows.Threading;
using Microsoft.Win32;

namespace HostsEditor_GUI.Services;

public enum AppTheme
{
    System,
    Light,
    Dark,
}

public static class ThemeManager
{
    private const string SystemThemeRegKey = @"Software\Microsoft\Windows\CurrentVersion\Themes\Personalize";
    private static readonly Uri LightUri = new("pack://application:,,,/Themes/Light.xaml");
    private static readonly Uri DarkUri = new("pack://application:,,,/Themes/Dark.xaml");
    private static DispatcherTimer? _timer;

    public static AppTheme Mode { get; set; } = AppTheme.System;

    public static bool IsSystemDark { get; private set; } = DetectSystemDark();

    public static bool IsDarkApplied
    {
        get
        {
            var current = FindThemeDictionary();
            return current is not null && current.Source == DarkUri;
        }
    }

    public static void Initialize()
    {
        Apply();

        _timer = new DispatcherTimer { Interval = TimeSpan.FromSeconds(2) };
        _timer.Tick += (_, _) =>
        {
            bool dark = DetectSystemDark();
            if (dark == IsSystemDark) return;
            IsSystemDark = dark;
            if (Mode == AppTheme.System) Apply();
        };
        _timer.Start();
    }

    public static void SetMode(AppTheme mode)
    {
        Mode = mode;
        Apply();
    }

    public static void Apply()
    {
        if (Application.Current is null) return;

        bool dark = Mode == AppTheme.Dark || (Mode == AppTheme.System && IsSystemDark);
        var target = dark ? DarkUri : LightUri;

        var dictionaries = Application.Current.Resources.MergedDictionaries;
        var current = FindThemeDictionary();
        if (current is not null && current.Source == target) return;

        if (current is not null) dictionaries.Remove(current);
        dictionaries.Add(new ResourceDictionary { Source = target });
    }

    private static ResourceDictionary? FindThemeDictionary()
    {
        var dictionaries = Application.Current?.Resources.MergedDictionaries;
        if (dictionaries is null) return null;
        return dictionaries.FirstOrDefault(d =>
            d.Source is not null && (d.Source == LightUri || d.Source == DarkUri));
    }

    private static bool DetectSystemDark()
    {
        try
        {
            using var key = Registry.CurrentUser.OpenSubKey(SystemThemeRegKey);
            if (key?.GetValue("AppsUseLightTheme") is int value)
            {
                return value == 0;
            }
        }
        catch
        {
            // 注册表读取失败时使用默认浅色
        }
        return false;
    }
}

