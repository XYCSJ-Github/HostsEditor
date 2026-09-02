using System.Text;
using System.Windows;
using HostsEditor_GUI.Services;

namespace HostsEditor_GUI
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        protected override void OnStartup(StartupEventArgs e)
        {
            Encoding.RegisterProvider(CodePagesEncodingProvider.Instance);

            ThemeManager.Initialize();

            base.OnStartup(e);
        }
    }
}
