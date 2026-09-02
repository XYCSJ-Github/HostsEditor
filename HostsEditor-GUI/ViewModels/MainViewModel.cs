using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Input;
using HostsEditor_GUI.Models;
using HostsEditor_GUI.Services;
using HostsEditor_GUI.Views;

namespace HostsEditor_GUI.ViewModels;

public sealed class ThemeOption
{
    public AppTheme Value { get; }
    public string Name { get; }

    public ThemeOption(AppTheme value, string name)
    {
        Value = value;
        Name = name;
    }

    public override string ToString() => Name;
}

public sealed class MainViewModel : ViewModelBase
{
    private const string DefaultHostsPath = @"C:\Windows\System32\drivers\etc\hosts";

    private readonly CmdRunner _runner;
    private readonly ObservableCollection<HostGroup> _groups = new();
    private readonly AsyncRelayCommand _enableSelectedCommand;
    private readonly AsyncRelayCommand _disableSelectedCommand;
    private readonly AsyncRelayCommand _removeSelectedCommand;
    private readonly AsyncRelayCommand _moveSelectedCommand;

    private object? _selectedItem;
    private bool _isBusy;
    private string _statusMessage = "";
    private string _hostsFilePath = DefaultHostsPath;
    private ThemeOption _selectedThemeOption;

    public MainViewModel()
    {
        _runner = new CmdRunner(CmdRunner.DefaultExecutablePath());
        _selectedThemeOption = ThemeOptions.First();

        RefreshCommand = new AsyncRelayCommand(_ => LoadAsync());
        AddCommand = new AsyncRelayCommand(_ => AddAsync());
        _enableSelectedCommand = new AsyncRelayCommand(_ => SetEnabledAsync(SelectedItem, true), CanOperateOnSelection);
        _disableSelectedCommand = new AsyncRelayCommand(_ => SetEnabledAsync(SelectedItem, false), CanOperateOnSelection);
        _removeSelectedCommand = new AsyncRelayCommand(_ => RemoveEntryAsync(SelectedItem as HostEntry), p => p is HostEntry);
        _moveSelectedCommand = new AsyncRelayCommand(_ => MoveEntryAsync(SelectedItem as HostEntry), p => p is HostEntry);

        EnableTargetCommand = new AsyncRelayCommand(p => SetEnabledAsync(p, true));
        DisableTargetCommand = new AsyncRelayCommand(p => SetEnabledAsync(p, false));
        RemoveTargetCommand = new AsyncRelayCommand(p => RemoveEntryAsync(p as HostEntry));
        MoveTargetCommand = new AsyncRelayCommand(p => MoveEntryAsync(p as HostEntry));
        AddToGroupCommand = new AsyncRelayCommand(p => AddAsync(p as HostGroup));

        ThemeManager.SetMode(_selectedThemeOption.Value);
        _ = LoadAsync();
    }

    public ObservableCollection<HostGroup> Groups => _groups;

    public AsyncRelayCommand RefreshCommand { get; }
    public AsyncRelayCommand AddCommand { get; }
    public ICommand EnableSelectedCommand => _enableSelectedCommand;
    public ICommand DisableSelectedCommand => _disableSelectedCommand;
    public ICommand RemoveSelectedCommand => _removeSelectedCommand;
    public ICommand MoveSelectedCommand => _moveSelectedCommand;

    public ICommand EnableTargetCommand { get; }
    public ICommand DisableTargetCommand { get; }
    public ICommand RemoveTargetCommand { get; }
    public ICommand MoveTargetCommand { get; }
    public ICommand AddToGroupCommand { get; }

    public object? SelectedItem
    {
        get => _selectedItem;
        set
        {
            if (SetProperty(ref _selectedItem, value))
            {
                _enableSelectedCommand.RaiseCanExecuteChanged();
                _disableSelectedCommand.RaiseCanExecuteChanged();
                _removeSelectedCommand.RaiseCanExecuteChanged();
                _moveSelectedCommand.RaiseCanExecuteChanged();
            }
        }
    }

    public bool IsBusy
    {
        get => _isBusy;
        private set
        {
            if (SetProperty(ref _isBusy, value))
            {
                RefreshCommand.RaiseCanExecuteChanged();
                AddCommand.RaiseCanExecuteChanged();
                _enableSelectedCommand.RaiseCanExecuteChanged();
                _disableSelectedCommand.RaiseCanExecuteChanged();
                _removeSelectedCommand.RaiseCanExecuteChanged();
                _moveSelectedCommand.RaiseCanExecuteChanged();
            }
        }
    }

    public string StatusMessage
    {
        get => _statusMessage;
        private set => SetProperty(ref _statusMessage, value);
    }

    public string HostsFilePath
    {
        get => _hostsFilePath;
        set
        {
            if (SetProperty(ref _hostsFilePath, value))
            {
                _ = LoadAsync();
            }
        }
    }

    public IReadOnlyList<ThemeOption> ThemeOptions { get; } =
    [
        new ThemeOption(AppTheme.System, "跟随系统"),
        new ThemeOption(AppTheme.Light, "浅色"),
        new ThemeOption(AppTheme.Dark, "深色"),
    ];

    public ThemeOption SelectedThemeOption
    {
        get => _selectedThemeOption;
        set
        {
            if (SetProperty(ref _selectedThemeOption, value) && value is not null)
            {
                ThemeManager.SetMode(value.Value);
            }
        }
    }

    private bool CanOperateOnSelection(object? parameter)
        => !IsBusy && (SelectedItem is HostGroup || SelectedItem is HostEntry);

    public async Task LoadAsync()
    {
        if (IsBusy) return;
        IsBusy = true;
        try
        {
            var output = await RunAsync("show");
            var groups = HostsParser.Parse(output);

            Groups.Clear();
            foreach (var group in groups)
            {
                Groups.Add(group);
            }

            StatusMessage = $"已加载 {groups.Count} 个分组，共 {groups.Sum(g => g.Entries.Count)} 条记录";
        }
        catch (Exception ex)
        {
            StatusMessage = ex.Message;
        }
        finally
        {
            IsBusy = false;
        }
    }

    public async Task ToggleEntryAsync(HostEntry entry)
    {
        await SetEnabledAsync(entry, entry.IsEnabled);
    }

    private async Task AddAsync(HostGroup? presetGroup = null)
    {
        if (IsBusy) return;

        var groupNames = Groups.Select(g => g.Name).Distinct().ToList();
        var dialog = new AddEntryWindow(groupNames, presetGroup?.Name ?? "")
        {
            Owner = Application.Current.MainWindow,
        };

        if (dialog.ShowDialog() != true) return;

        var args = new List<string> { "add", dialog.Ip, dialog.HostName };
        if (dialog.GroupName.Length > 0) args.Add(dialog.GroupName);

        await RunModifyAsync(args);
    }

    private async Task SetEnabledAsync(object? target, bool enable)
    {
        if (target is null) return;
        if (IsBusy) return;

        var name = target switch
        {
            HostGroup group => group.Name,
            HostEntry entry => entry.HostName,
            _ => "",
        };

        if (name.Length == 0) return;

        await RunModifyAsync([enable ? "enable" : "disable", name]);
    }

    private async Task RemoveEntryAsync(HostEntry? entry)
    {
        if (entry is null || IsBusy) return;
        await RunModifyAsync(["remove", entry.HostName]);
    }

    private async Task MoveEntryAsync(HostEntry? entry)
    {
        if (entry is null || IsBusy) return;

        var groupNames = Groups
            .Select(g => g.Name)
            .Where(n => !string.Equals(n, entry.HostName, StringComparison.OrdinalIgnoreCase))
            .Distinct()
            .ToList();

        var dialog = new MoveWindow(groupNames)
        {
            Owner = Application.Current.MainWindow,
        };

        if (dialog.ShowDialog() != true) return;

        await RunModifyAsync(["moveto", entry.HostName, dialog.GroupName]);
    }

    private async Task RunModifyAsync(IReadOnlyList<string> args)
    {
        IsBusy = true;
        try
        {
            var output = await RunAsync(args.ToArray());
            var message = output.Trim();
            StatusMessage = message.Length > 0 ? message : "操作完成";
        }
        catch (Exception ex)
        {
            StatusMessage = ex.Message;
            MessageBox.Show(ex.Message, "操作失败", MessageBoxButton.OK, MessageBoxImage.Error);
        }
        finally
        {
            IsBusy = false;
        }

        await LoadAsync();
    }

    private async Task<string> RunAsync(params string[] args)
    {
        var allArgs = new[] { "--file", HostsFilePath }.Concat(args).Select(Quote);
        return await _runner.RunAsync(string.Join(" ", allArgs));
    }

    private static string Quote(string arg)
        => "\"" + arg.Replace("\"", "\\\"") + "\"";
}
