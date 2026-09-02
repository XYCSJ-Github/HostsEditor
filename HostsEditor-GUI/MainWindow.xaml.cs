using System.Windows;
using System.Windows.Controls;
using HostsEditor_GUI.Models;
using HostsEditor_GUI.ViewModels;

namespace HostsEditor_GUI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            DataContext = new MainViewModel();
        }

        private void OnTreeViewSelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            if (DataContext is MainViewModel viewModel)
            {
                viewModel.SelectedItem = e.NewValue;
            }
        }

        private async void OnEntryCheckBoxClicked(object sender, RoutedEventArgs e)
        {
            if (DataContext is not MainViewModel viewModel) return;
            if (sender is CheckBox { DataContext: HostEntry entry })
            {
                await viewModel.ToggleEntryAsync(entry);
            }
        }
    }
}
