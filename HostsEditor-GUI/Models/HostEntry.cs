namespace HostsEditor_GUI.Models;

public class HostEntry : ObservableObject
{
    private string _ip = "";
    private string _hostName = "";
    private bool _isEnabled = true;
    private bool _isSelected;

    public string Ip
    {
        get => _ip;
        set => SetProperty(ref _ip, value);
    }

    public string HostName
    {
        get => _hostName;
        set => SetProperty(ref _hostName, value);
    }

    public bool IsEnabled
    {
        get => _isEnabled;
        set => SetProperty(ref _isEnabled, value);
    }

    public bool IsSelected
    {
        get => _isSelected;
        set => SetProperty(ref _isSelected, value);
    }
}
