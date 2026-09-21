using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Windows;
using System.Windows.Input;
using HostsEditor_GUI.Models;
using HostsEditor_GUI.Services;
using HostsEditor_GUI.Views;

namespace HostsEditor_GUI.ViewModels;

public sealed class MainViewModel : ViewModelBase
{
    private const string DefaultHostsPath = @"C:\Windows\System32\drivers\etc\hosts";

    private readonly CmdRunner _runner;
    private readonly ObservableCollection<HostGroup> _groups = new();
    private readonly AsyncRelayCommand _enableSelectedCommand;
    private readonly AsyncRelayCommand _disableSelectedCommand;
    private readonly AsyncRelayCommand _removeSelectedCommand;
    private readonly AsyncRelayCommand _moveSelectedCommand;
    private readonly AsyncRelayCommand _editSelectedCommand;

    private bool _isBusy;
    private string _statusMessage = "";
    private string _hostsFilePath = DefaultHostsPath;

    public MainViewModel()
    {
        _runner = new CmdRunner(CmdRunner.DefaultExecutablePath());

        RefreshCommand = new AsyncRelayCommand(_ => LoadAsync());
        AddCommand = new AsyncRelayCommand(_ => AddAsync());
        _enableSelectedCommand = new AsyncRelayCommand(_ => SetEnabledManyAsync(GetSelectedItems(), true), _ => HasSelection && !IsBusy);
        _disableSelectedCommand = new AsyncRelayCommand(_ => SetEnabledManyAsync(GetSelectedItems(), false), _ => HasSelection && !IsBusy);
        _removeSelectedCommand = new AsyncRelayCommand(_ => RemoveEntriesAsync(GetSelectedEntries()), _ => HasEntrySelection && !IsBusy);
        _moveSelectedCommand = new AsyncRelayCommand(_ => MoveEntriesAsync(GetSelectedEntries()), _ => HasEntrySelection && !IsBusy);
        _editSelectedCommand = new AsyncRelayCommand(_ => EditEntryAsync(GetSelectedEntries().FirstOrDefault()), _ => SelectedEntryCount == 1 && !IsBusy);

        EnableTargetCommand = new AsyncRelayCommand(p => SetEnabledManyAsync(ToSingle(p), true));
        DisableTargetCommand = new AsyncRelayCommand(p => SetEnabledManyAsync(ToSingle(p), false));
        RemoveTargetCommand = new AsyncRelayCommand(p => RemoveEntriesAsync(p is HostEntry entry ? new[] { entry } : Array.Empty<HostEntry>()));
        MoveTargetCommand = new AsyncRelayCommand(p => MoveEntriesAsync(p is HostEntry entry ? new[] { entry } : Array.Empty<HostEntry>()));
        EditTargetCommand = new AsyncRelayCommand(p => EditEntryAsync(p as HostEntry));
        AddToGroupCommand = new AsyncRelayCommand(p => AddAsync(p as HostGroup));

        OpenFileCommand = new RelayCommand(_ => OpenHostsFile());
        OpenFolderCommand = new RelayCommand(_ => OpenHostsFolder());

        _ = LoadAsync();
    }

    public ObservableCollection<HostGroup> Groups => _groups;

    public AsyncRelayCommand RefreshCommand { get; }
    public AsyncRelayCommand AddCommand { get; }
    public ICommand EnableSelectedCommand => _enableSelectedCommand;
    public ICommand DisableSelectedCommand => _disableSelectedCommand;
    public ICommand RemoveSelectedCommand => _removeSelectedCommand;
    public ICommand MoveSelectedCommand => _moveSelectedCommand;
    public ICommand EditSelectedCommand => _editSelectedCommand;

    public ICommand EnableTargetCommand { get; }
    public ICommand DisableTargetCommand { get; }
    public ICommand RemoveTargetCommand { get; }
    public ICommand MoveTargetCommand { get; }
    public ICommand EditTargetCommand { get; }
    public ICommand AddToGroupCommand { get; }

    public ICommand OpenFileCommand { get; }
    public ICommand OpenFolderCommand { get; }

    public bool IsBusy
    {
        get => _isBusy;
        private set
        {
            if (SetProperty(ref _isBusy, value))
            {
                RefreshCommand.RaiseCanExecuteChanged();
                AddCommand.RaiseCanExecuteChanged();
                RaiseSelectionCommands();
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

    private bool HasSelection => _groups.Any(g => g.IsSelected || g.Entries.Any(e => e.IsSelected));

    private bool HasEntrySelection => _groups.Any(g => g.Entries.Any(e => e.IsSelected));

    private int SelectedEntryCount => _groups.Sum(g => g.Entries.Count(e => e.IsSelected));

    public IReadOnlyList<object> GetSelectedItems()
    {
        var list = new List<object>();
        foreach (var group in _groups)
        {
            if (group.IsSelected) list.Add(group);
            foreach (var entry in group.Entries)
            {
                if (entry.IsSelected) list.Add(entry);
            }
        }
        return list;
    }

    public IReadOnlyList<HostEntry> GetSelectedEntries()
        => _groups.SelectMany(g => g.Entries).Where(e => e.IsSelected).ToList();

    private static IEnumerable<object> ToSingle(object? target)
        => target is null ? Array.Empty<object>() : new[] { target };

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
                AttachSelectionTracking(group);
                Groups.Add(group);
            }

            RaiseSelectionCommands();
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

    private void AttachSelectionTracking(HostGroup group)
    {
        group.PropertyChanged += OnModelPropertyChanged;
        foreach (var entry in group.Entries)
        {
            entry.PropertyChanged += OnModelPropertyChanged;
        }
    }

    private void OnModelPropertyChanged(object? sender, PropertyChangedEventArgs e)
    {
        if (e.PropertyName == nameof(HostEntry.IsSelected) || e.PropertyName == nameof(HostGroup.IsSelected))
        {
            RaiseSelectionCommands();
        }
    }

    private void RaiseSelectionCommands()
    {
        _enableSelectedCommand.RaiseCanExecuteChanged();
        _disableSelectedCommand.RaiseCanExecuteChanged();
        _removeSelectedCommand.RaiseCanExecuteChanged();
        _moveSelectedCommand.RaiseCanExecuteChanged();
        _editSelectedCommand.RaiseCanExecuteChanged();
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

    private async Task SetEnabledManyAsync(IEnumerable<object> targets, bool enable)
    {
        if (IsBusy) return;

        var names = new List<string>();
        foreach (var target in targets)
        {
            switch (target)
            {
                case HostGroup group when group.Name.Length > 0:
                    names.Add(group.Name);
                    break;
                case HostEntry entry when entry.HostName.Length > 0:
                    names.Add(entry.HostName);
                    break;
            }
        }

        names = names.Distinct(StringComparer.OrdinalIgnoreCase).ToList();
        if (names.Count == 0) return;

        var args = new List<string> { enable ? "enable" : "disable" };
        args.AddRange(names);
        await RunModifyAsync(args);
    }

    private async Task RemoveEntriesAsync(IEnumerable<HostEntry> entries)
    {
        if (IsBusy) return;

        var hosts = entries
            .Select(e => e.HostName)
            .Where(h => h.Length > 0)
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .ToList();

        if (hosts.Count == 0) return;

        var args = new List<string> { "remove" };
        args.AddRange(hosts);
        await RunModifyAsync(args);
    }

    private async Task MoveEntriesAsync(IEnumerable<HostEntry> entries)
    {
        if (IsBusy) return;

        var hosts = entries
            .Select(e => e.HostName)
            .Where(h => h.Length > 0)
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .ToList();

        if (hosts.Count == 0) return;

        var groupNames = Groups.Select(g => g.Name).Distinct(StringComparer.OrdinalIgnoreCase).ToList();
        var dialog = new MoveWindow(groupNames)
        {
            Owner = Application.Current.MainWindow,
        };

        if (dialog.ShowDialog() != true) return;

        var args = new List<string> { "movetomany", dialog.GroupName };
        args.AddRange(hosts);
        await RunModifyAsync(args);
    }

    private async Task EditEntryAsync(HostEntry? entry)
    {
        if (entry is null || IsBusy) return;

        var dialog = new EditEntryWindow(entry.Ip, entry.HostName)
        {
            Owner = Application.Current.MainWindow,
        };

        if (dialog.ShowDialog() != true) return;

        if (entry.Ip == dialog.Ip && entry.HostName == dialog.HostName)
        {
            StatusMessage = "未修改";
            return;
        }

        await RunModifyAsync(new[] { "edit", entry.Ip, entry.HostName, dialog.Ip, dialog.HostName });
    }

    private void OpenHostsFile()
    {
        try
        {
            Process.Start(new ProcessStartInfo
            {
                FileName = "notepad.exe",
                Arguments = Quote(HostsFilePath),
                UseShellExecute = true,
            });
            StatusMessage = $"已用记事本打开: {HostsFilePath}";
        }
        catch (Exception ex)
        {
            StatusMessage = ex.Message;
            MessageBox.Show(ex.Message, "打开失败", MessageBoxButton.OK, MessageBoxImage.Error);
        }
    }

    private void OpenHostsFolder()
    {
        try
        {
            var fullPath = Path.GetFullPath(HostsFilePath);
            var folder = Path.GetDirectoryName(fullPath) ?? fullPath;
            var exists = File.Exists(fullPath);
            var arguments = exists ? "/select," + Quote(fullPath) : Quote(folder);

            Process.Start(new ProcessStartInfo
            {
                FileName = "explorer.exe",
                Arguments = arguments,
                UseShellExecute = true,
            });
            StatusMessage = exists ? $"已在资源管理器中定位: {fullPath}" : $"已打开文件夹: {folder}";
        }
        catch (Exception ex)
        {
            StatusMessage = ex.Message;
            MessageBox.Show(ex.Message, "打开失败", MessageBoxButton.OK, MessageBoxImage.Error);
        }
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
