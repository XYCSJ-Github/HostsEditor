using System.Windows;

namespace HostsEditor_GUI.Views;

public partial class EditEntryWindow : Window
{
    public EditEntryWindow(string ip, string hostName)
    {
        InitializeComponent();

        IpBox.Text = ip;
        HostBox.Text = hostName;

        Loaded += (_, _) => IpBox.Focus();
    }

    public string Ip => IpBox.Text.Trim();

    public string HostName => HostBox.Text.Trim();

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
