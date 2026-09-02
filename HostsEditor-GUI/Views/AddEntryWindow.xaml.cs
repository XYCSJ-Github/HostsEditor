using System.Windows;

namespace HostsEditor_GUI.Views;

public partial class AddEntryWindow : Window
{
    public AddEntryWindow(IEnumerable<string> groups, string presetGroup = "")
    {
        InitializeComponent();

        GroupBox.ItemsSource = groups;
        if (presetGroup.Length > 0) GroupBox.Text = presetGroup;

        Loaded += (_, _) => IpBox.Focus();
    }

    public string Ip => IpBox.Text.Trim();

    public string HostName => HostBox.Text.Trim();

    public string GroupName => GroupBox.Text.Trim();

    private void OnOk(object sender, RoutedEventArgs e)
    {
        if (Ip.Length == 0 || HostName.Length == 0)
        {
            MessageBox.Show("请输入 IP 地址和主机名。", "提示", MessageBoxButton.OK, MessageBoxImage.Information);
            return;
        }
        DialogResult = true;
    }

    private void OnCancel(object sender, RoutedEventArgs e) => DialogResult = false;
}
