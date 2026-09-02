using System.Windows;

namespace HostsEditor_GUI.Views;

public partial class MoveWindow : Window
{
    public MoveWindow(IEnumerable<string> groups)
    {
        InitializeComponent();

        GroupBox.ItemsSource = groups;
        GroupBox.SelectedIndex = groups.Any() ? 0 : -1;

        Loaded += (_, _) => GroupBox.Focus();
    }

    public string GroupName => GroupBox.Text.Trim();

    private void OnOk(object sender, RoutedEventArgs e)
    {
        if (GroupName.Length == 0)
        {
            MessageBox.Show("请选择目标分组。", "提示", MessageBoxButton.OK, MessageBoxImage.Information);
            return;
        }
        DialogResult = true;
    }

    private void OnCancel(object sender, RoutedEventArgs e) => DialogResult = false;
}
