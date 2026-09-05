# Virtual City 1 — decomp / preservation

以本地 Windows 版 Virtual City 1 为参照，逐步恢复程序结构、脚本格式与游戏行为，为跨平台移植、preservation 和学习建立可复现的工程基础。目前处于分析起步阶段，尚无可编译的游戏重实现。

## 已完成

- 创建本地 Ghidra 工程 `ghidra/VirtualCity.gpr`，导入并分析 `VirtualCity.Unwrapped.exe`。
- Ghidra 12.1.2 / `x86:LE:32:default` / `windows`；初始分析识别 3,615 个函数。
- 接通 Ghidra MCP 7.0.0，并保存首个经源码核对的函数 `sq_base_register`（`0x00485f40`）。
- 建立本地 Git 仓库、1,573 个输入文件的 SHA-256 清单、分析证据和重建脚本。

## 工具链结论

| 项目 | 结论 | 可信度 |
| --- | --- | --- |
| Squirrel | **2.2.3 stable** | 版本字符串及官方源码行为吻合；不排除游戏方修改 |
| MSVC | **Visual Studio 2008 SP1 / VC9，build 30729** | Rich Header 的 C/C++/linker 记录互相印证 |
| 参考编译器 | `cl 15.00.30729.01`，`link 9.00.30729.01` | 对应 SP1 基线；PE 无法确定补丁末位 |
| ABI | Win32 x86，小端；`SQChar=1`、`SQInteger=4`、`SQFloat=4` | PE 与 `sq_base_register` 汇编直接证据 |
| Squirrel 宏 | 不定义 `SQUNICODE` / `_UNICODE` / `SQUSEDOUBLE` / `_SQ64` | 按官方 2.2.3 头文件重建上述 ABI |
| 优化 | 存在 MSVC LTCG C++ 代码；需考虑 `/GL` + `/LTCG` | Rich Header + EAX 自定义传参支持 |
| CRT | 静态 CRT，优先尝试 `/MT` | 强推断；无 MSVC CRT DLL 导入且含 CRT 实现 |
| 精确参数 | `/O2`、`/Ob2`、`/Oy`、`/GS`、`/EHsc`、`/GR` 等仍需实验 | 不把候选参数当作原始命令行 |

完整证据、地址、版本来源和 Function ID 构建策略见 [docs/toolchain.md](docs/toolchain.md)。

## 目录

```text
Virtual-City_Win_EN_Full.zip        原始外层归档（本地，不入 Git）
Virtual-City_Win_EN_Full/
  VirtualCity.Unwrapped.exe        分析目标
  VirtualCity.RWG                  同尺寸的另一输入，独立保留指纹
  VirtualCity.bar                  实测为 7z/LZMA 资源包（扩展名 .bar）
  VirtualCity/                    .bar 的解压结果，不是第二个安装根目录
    scripts/                      626 个 .nut 均为 Squirrel 字节码，不是明文源码
ghidra/                           本地 Ghidra 数据库，不入 Git
vendor/                           本地上游参考源码与下载包，不入 Git
analysis/                         可提交的指纹、汇编/反编译证据及输入清单
docs/                             研究结论与后续路线
tools/                            指纹提取、Ghidra 创建及初始标注重放脚本
```

Git 只跟踪原创工程文件、研究记录及必要的分析证据。原始游戏文件、资源与大型 Ghidra 数据库留在本地；从仓库重建分析环境仍需提供同一份输入。未设置远程仓库、未上传任何内容。原游戏及第三方代码的权利归原权利人，上游 Squirrel 使用其自带许可；本仓库暂未为后续重实现选择许可证。

## 使用

在仓库根目录运行：

```powershell
python -m pip install -r tools/requirements.txt
python tools/inspect_binary.py
# 当前工程已经存在，只需打开；若它已打开，直接使用当前窗口。
./tools/open-ghidra.ps1
# 仅在没有 ghidra/VirtualCity.gpr 的新 checkout 中使用：
./tools/init-ghidra.ps1
```

脚本默认 Ghidra 安装在 `D:\ghidra_12.1.2_PUBLIC`，可用 `-GhidraHome` 覆盖。创建脚本会拒绝覆盖已有工程，并在分析后重放已核实的 `sq_base_register` 标注。不要同时用 headless 和 GUI 写入同一 Ghidra 工程；备份整个 `ghidra` 目录前先关闭工程。

MCP 操作顺序：`list_instances` → `connect_instance(project="VirtualCity")` → 必要时 `tool_launch_codebrowser(path="/VirtualCity.Unwrapped.exe")` → `get_metadata(program="VirtualCity.Unwrapped.exe")`。修改后调用 `save_program`。本次实际端口为 **8090**，8089 被另一个实例占用并返回 404；以后应按项目发现实例，不硬编码端口。

所有地址以目标 SHA-256 `bdc3bc9f075a813d12078fd2eaaaa1131ba74616b11aa1444cdd5a5b83e08695`、image base `0x00400000` 为准。自动反编译文本可能误判参数和返回值，尤其是 LTCG 函数；不能直接当作可编译源码。

## 后续顺序

1. 配置 VC9 SP1 x86 工具链，构建 Squirrel 2.2.3 的符号参考样本与 Function ID 数据库。
2. 用已确认的 Squirrel 锚点校验匹配质量，逐步区分 Squirrel、CRT、其他库与游戏代码。
3. 解析 `.nut` 字节码，恢复脚本结构、常量、类和 native binding 接口。
4. 建立资源加载、渲染（D3D8）、音频（BASS）、输入及存档边界，再设计平台抽象层。
5. 用原版行为、资源和存档样本验证重实现，再推进跨平台构建。
