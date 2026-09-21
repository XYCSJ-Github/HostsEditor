using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Media3D;
using HostsEditor_GUI.Models;
using HostsEditor_GUI.ViewModels;

namespace HostsEditor_GUI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private object? _selectionAnchor;

        public MainWindow()
        {
            InitializeComponent();
            DataContext = new MainViewModel();
        }

        // 选择由复选框表达，这里吞掉行上的左键按下，避免 TreeView 自带选中高亮（蓝底）。
        // 复选框与展开/收起箭头（ToggleButton）以及滚动条不受影响。
        private void OnTreeViewPreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (e.OriginalSource is not DependencyObject origin) return;
            if (sender is not TreeView treeView) return;
            if (ItemsControl.ContainerFromElement(treeView, origin) is not TreeViewItem) return;
            if (IsWithinToggleButton(origin)) return;

            e.Handled = true;
        }

        private static bool IsWithinToggleButton(DependencyObject origin)
        {
            DependencyObject? current = origin;
            while (current is not null)
            {
                if (current is System.Windows.Controls.Primitives.ToggleButton) return true;

                current = current is Visual or Visual3D
                    ? VisualTreeHelper.GetParent(current)
                    : LogicalTreeHelper.GetParent(current);
            }
            return false;
        }

        // 复选框是唯一的选择入口：本地勾选互不干扰，天然支持多选。
        // 分组复选框会连带勾选/取消组内全部条目；按住 Shift 可范围连选。
        private void OnSelectionCheckBoxClick(object sender, RoutedEventArgs e)
        {
            if (sender is not CheckBox { DataContext: { } model }) return;

            if (model is HostGroup group)
            {
                foreach (var entry in group.Entries)
                {
                    entry.IsSelected = group.IsSelected;
                }

                _selectionAnchor = group;
                return;
            }

            if ((Keyboard.Modifiers & ModifierKeys.Shift) == ModifierKeys.Shift && _selectionAnchor is not null)
            {
                SelectRange(model);
            }
            else
            {
                _selectionAnchor = model;
            }
        }

        private static void SetSelected(object model, bool selected)
        {
            switch (model)
            {
                case HostEntry entry:
                    entry.IsSelected = selected;
                    break;
                case HostGroup group:
                    group.IsSelected = selected;
                    break;
            }
        }

        private void ClearSelection()
        {
            if (DataContext is not MainViewModel viewModel) return;
            foreach (var group in viewModel.Groups)
            {
                group.IsSelected = false;
                foreach (var entry in group.Entries)
                {
                    entry.IsSelected = false;
                }
            }
        }

        private void SelectRange(object target)
        {
            if (DataContext is not MainViewModel viewModel) return;

            var flat = new List<object>();
            foreach (var group in viewModel.Groups)
            {
                flat.Add(group);
                foreach (var entry in group.Entries)
                {
                    flat.Add(entry);
                }
            }

            var start = _selectionAnchor is null ? -1 : flat.IndexOf(_selectionAnchor);
            var end = flat.IndexOf(target);
            if (start < 0 || end < 0)
            {
                SetSelected(target, true);
                return;
            }

            ClearSelection();
            var lo = Math.Min(start, end);
            var hi = Math.Max(start, end);
            for (var i = lo; i <= hi; i++)
            {
                SetSelected(flat[i], true);
            }
        }
    }
}
