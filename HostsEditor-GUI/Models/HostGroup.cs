using System.Collections.ObjectModel;

namespace HostsEditor_GUI.Models;

public class HostGroup : ObservableObject
{
    private string _name = "";
    private bool _isDefault;

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

    public ObservableCollection<HostEntry> Entries { get; } = new();
}
