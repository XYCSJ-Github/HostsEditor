# HostsEditor

一个面向 Windows 的 hosts 文件分组管理工具。采用「C++ 命令行引擎 + WPF 图形界面」的双层架构，并对发布产物做自解压封装：最终只需分发一个 `HostsEditor.exe`，运行时自动在 `%LOCALAPPDATA%\HostsEditor` 释放全部资源并启动图形界面，程序退出后自动清理，不留残留。

## 功能特性

- **分组管理**：自动识别 hosts 文件中 `#组名 Start` / `#组名 End` 标记并按组展示，未分组的零散内容归入“默认分组”。
- **启用 / 禁用**：对单个条目或整个分组一键启用/禁用，仅切换注释状态，不破坏文件结构。
- **新增 / 删除**：向指定分组（或默认分组）添加新条目，或删除已有条目。
- **移动条目**：将条目移动到其他分组，便于重新组织规则。
- **实时刷新**：所有修改立即写回系统 hosts 文件并刷新列表，即时生效。
- **命令行操作**：内置 C++ 命令行引擎，支持脚本化执行 `show`、`enable`、`disable`、`add`、`moveto`、`remove` 等操作，GUI 亦复用它完成读写。
- **自解压发布**：单文件启动器，双击即用，免安装、退出即清理。
- **跟随系统主题**：GUI 使用 WPF 原生控件与系统 Fluent 主题（浅色/深色自动切换）。

## 仓库结构

```
HostsEditor/
├── HostsEditor.slnx                  # 解决方案（.slnx 格式）
├── HostsEditor-Cmd/                  # C++ 命令行引擎（控制台程序）
│   ├── main.cpp                      # 命令解析与业务入口
│   ├── hosts_io.cpp/.h               # hosts 文件读取 / 分组分析 / 序列化写回
│   ├── hosts_group.cpp/.h            # 分组数据结构与行转换
│   ├── host_struct.h                 # IP/主机名 与分组基础结构
│   ├── bin_cache.cpp/.h              # 可选二进制缓存（--cache 时写入）
│   └── Logout/                       # 带颜色的控制台日志模块
├── HostsEditor-GUI/                  # WPF 图形界面（.NET 10）
│   ├── Models/                       # HostEntry / HostGroup
│   ├── ViewModels/                   # MainViewModel（MVVM）
│   ├── Services/                     # CmdRunner（调用引擎）/ HostsParser（解析 show 输出）
│   ├── Views/                        # 添加条目 / 移动分组 对话框
│   ├── Themes/                       # 主题相关资源
│   └── app.manifest                  # requireAdministrator 清单
└── HostsEditor/                      # 自解压启动器（C++，Windows 子系统）
    ├── main.cpp                      # 释放资源 → 运行 GUI → 退出清理
    └── HostsEditor.vcxproj           # 构建期自动发布 GUI 并内嵌资源
```

## hosts 分组语法约定

工具使用带标记的分组注释来划分条目，格式约定：

```text
# MyGroup Start
127.0.0.1  example.com
0.0.0.0   ads.example.com
# MyGroup End
```

- 分组以 `# <组名> Start` 开始、`# <组名> End` 结束。
- 未处于任何标记块内的内容视为**默认分组**（文件头部说明等散落内容与条目均归入其中）。
- 被注释掉的条目（行首 `#`）视为“已禁用”，仍保留在分组内可重新启用。

> 写回说明：工具在写回时会按「解析出的条目」重新生成文件。形如 `IP 域名` 的行被解析为条目并规范化输出；而纯注释、空行等不符合该配对格式的内容不会原样保留（部分注释行甚至会被当作“已禁用条目”解析）。涉及注释的原始排版无法精确还原，请勿在 hosts 中用注释或特殊格式承载关键信息。

## HostsEditor-Cmd（命令行引擎）

```
用法: HostsEditor-Cmd [--file <路径>] <命令> [参数...]

命令:
  show                    解析并打印 hosts 分组内容 (默认)
  apply                   序列化并写回 hosts 文件
  enable <host|group>     启用匹配的主机或整个分组并写回
  disable <host|group>    禁用匹配的主机或整个分组并写回
  add <ip> <host> [group] 新增启用条目到指定分组(默认Default)并写回
  moveto <host> <group>   移动条目到指定分组并写回
  remove <host>           删除匹配的条目并写回
  help                    显示本帮助

选项:
  --file <路径>   指定 hosts 文件路径 (默认 C:\Windows\System32\drivers\etc\hosts)
  --cache         命令成功后写入 bin 缓存 (默认不写)
```

`show` 输出示例（GUI 的解析依据）：

```text
Default:
IP:127.0.0.1|Host:localhost|Enable:Yes

MyGroup:
IP:127.0.0.1|Host:example.com|Enable:Yes
IP:0.0.0.0|Host:ads.example.com|Enable:No
```

> 注意：修改系统 hosts 需要管理员权限。若以普通用户运行，请先以管理员身份打开终端。

## HostsEditor-GUI（图形界面）

- 以**分组树**展示全部分组与条目，分组行显示分组名与条目计数。
- 顶部工具栏提供：刷新、添加、启用、禁用、移动到、删除。
- 条目通过复选框快速切换启用/禁用，禁用条目以半透明显示。
- 分组右键菜单：启用分组、禁用分组、添加条目…；条目右键菜单：启用、禁用、移动到分组…、删除。
- 底部状态栏可查看/修改 hosts 文件路径与操作结果消息。
- 界面依赖同目录下的 `HostsEditor-Cmd.exe` 完成读写。

## HostsEditor（自解压启动器）

最终交付物为单个 `HostsEditor.exe`，构建时自动把 GUI 的 win-x64 自包含发布产物及 `HostsEditor-Cmd.exe` 内嵌为资源。运行逻辑：

1. 以管理员身份启动（清单 `requireAdministrator`，单实例互斥）。
2. 释放全部内嵌资源到 `%LOCALAPPDATA%\HostsEditor`（先清理历史残留）。
3. 启动 `HostsEditor-GUI.exe` 并等待其退出。
4. 退出后自动删除整个 `%LOCALAPPDATA%\HostsEditor` 目录（多次重试，占用时安排重启后删除）。

### 启动参数

| 参数 | 行为 |
| --- | --- |
| （无） | 默认流程：释放到 `%LOCALAPPDATA%\HostsEditor` → 运行 GUI → 退出清理 |
| `-i` / `/i` | 就地释放：把内嵌资源解包到 `HostsEditor.exe` 所在目录，**不运行 GUI、不清理**（用于便携部署） |

## 构建环境

| 依赖 | 说明 |
| --- | --- |
| Visual Studio 2026（v18）| C++ 工具集 `v145`，需安装“使用 C++ 的桌面开发” |
| Windows SDK | 10.0 |
| .NET SDK | 10.x（GUI 为 `net10.0-windows`） |

打开根目录的 `HostsEditor.slnx`，选择 `Release | x64` 直接生成即可。构建顺序：

1. `HostsEditor-Cmd` —— C++ 命令行引擎；
2. `HostsEditor-GUI` —— WPF 界面（构建后自动把 Cmd 产物复制到输出目录）；
3. `HostsEditor` —— 启动器（构建前自动以 win-x64 自包含 single-file 发布 GUI，并把非 `.pdb` 发布产物内嵌为 RCDATA 资源）。

> 首次构建启动器会触发一次完整的 GUI 自包含发布（耗时较长）；此后仅当 GUI/Cmd 源码变更时才重新发布。

## 发布与产物

- 最终产物：`x64\Release\HostsEditor.exe`（单文件，约 150+ MB，内含 GUI、Cmd 与运行所需 native 库）。
- GUI 采用 **win-x64 / 自包含 / single-file** 发布，目标机无需预装 .NET 运行时。
- 便携部署：`HostsEditor.exe -i` 即可在当前目录解包出可独立运行的文件集。
- 普通双击：自动走「临时释放 → 运行 → 清理」流程，系统不残留应用目录。

## 说明与限制

- hosts 文件为系统关键文件，程序清单要求管理员权限，首次运行会出现 UAC 提示。
- 写回按 `IP 域名` 条目重建文件，原始注释与排版无法精确保留，建议备份后再编辑，条目使用纯 `IP 域名` 格式。
- `bin_cache` 为可选功能，默认不写入；GUI 通过标准输出交互，不依赖缓存文件。

---

*署名 DSV4-Flash*
