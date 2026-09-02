namespace HostsEditor_GUI.Models;

public class HostEntry : ObservableObject
{
    private string _ip = "";
    private string _hostName = "";
    private bool _isEnabled = true;

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
}
