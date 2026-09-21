using System.Collections.ObjectModel;

namespace HostsEditor_GUI.Models;

public class HostGroup : ObservableObject
{
    private string _name = "";
    private bool _isDefault;
    private bool _isSelected;

    public string Name
    {
        get => _name;
        set => SetProperty(ref _name, value);
    }

    public bool IsDefault
    {
        get => _isDefault;
        set => SetProperty(ref _isDefault, value);
    }

    public bool IsSelected
    {
        get => _isSelected;
        set => SetProperty(ref _isSelected, value);
    }

    /// <summary>组内条目是否全部禁用（用于整组删除线展示）。</summary>
    public bool AllDisabled => Entries.Count > 0 && Entries.All(e => !e.IsEnabled);

    public ObservableCollection<HostEntry> Entries { get; } = new();
}
