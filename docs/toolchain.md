# 目标构建指纹与 Squirrel 参考构建

调查日期：2026-09-06。静态分析，没有运行目标游戏。原始字节、反编译推断与参考构建建议分开记录。

## 1. 输入身份

- 文件：`Virtual-City_Win_EN_Full/VirtualCity.Unwrapped.exe`，1,117,696 字节。
- SHA-256：`bdc3bc9f075a813d12078fd2eaaaa1131ba74616b11aa1444cdd5a5b83e08695`。
- PE32 / machine `0x014c`；image base `0x00400000`；入口 `0x0042f5d4`；GUI 子系统。
- COFF 时间戳：`2009-12-21T19:47:52Z`。这是头部记录，不保证真实编译时间。
- CodeView RSDS：PDB GUID `67A6CE4E-87E7-4C09-A1BD-E1278DDB1B64`，age 1。
- PDB 路径：`d:\Bamboo-Home\xml-data\build-dir\MOVEALOT-WIN32\bin\VirtualCity.pdb`，位于文件偏移 `0xe2f28` / VA `0x004e4528`。当前未发现该 PDB。

完整 PE 数据见 `analysis/binary-fingerprint.json`；所有本地原始输入的 hash 见 `analysis/input-manifest.json`。它们可由 `tools/inspect_binary.py` 重算。

资源归档需更正初始假设：本地 `VirtualCity.bar` 头部为 `37 7A BC AF 27 1C`，是 **7z**，不是 ZIP。签名定义见 [7-Zip 官方格式说明](https://www.7-zip.org/recover.html)。Python ZipFile 拒绝此文件是格式不符，不代表归档损坏。外层 `Virtual-City_Win_EN_Full.zip` 与内层 `.bar` 应分别处理；7-Zip 校验输出保存在 `analysis/bar-test.txt`。

## 2. Squirrel 版本与 ABI

文件偏移 `0xd27b4` / VA `0x004d3db4` 是 `Squirrel 2.2.3 stable`。该字符串被 `0x00486078` 引用，所属函数入口 `0x00485f40`。

与官方 2.2.3 的 `squirrel/sqbaselib.cpp:248` 核对，此函数的注册表遍历、`_version_` 和三个大小常量的顺序吻合 `sq_base_register`，现已按上游名称标注并保存。汇编证据见 `analysis/sq_base_register.asm.json`；初始自动反编译快照见 `analysis/sq_base_register.decomp.json`，其中旧名与错误的 `void(void)` 是标注前输出。

| 常量 | 字符串 VA | 对应机器码 | 值 |
| --- | --- | --- | --- |
| `_charsize_` | `0x004d3dcc` | `0x004860b7`: 写入立即数 1 | 1 |
| `_intsize_` | `0x004d3dd8` | `0x00486108`: `ESI=4`，`0x0048610d`: 写入 ESI | 4 |
| `_floatsize_` | `0x004d3de4` | `0x0048615a`: 写入同一个 ESI | 4 |

因此参考构建使用 32 位指针、8 字节 SQObject、窄字符和单精度浮点。官方 `include/squirrel.h` 由 `SQUSEDOUBLE` 控制浮点宽度，`SQUNICODE` 控制字符宽度，`_SQ64` 控制整数宽度；`_UNICODE` 也会启用 `SQUNICODE`，所以同样不定义。此处指 Squirrel 配置，不推断整个游戏的所有 Windows API 字符集选项。

二进制含 `collectgarbage` 注册相关字符串，支持保留默认 GC（不定义 `NO_GARBAGE_COLLECTOR`），但本轮没有完整验证回收算法。版本字符串和这个函数的吻合不能证明整个 Squirrel 库未经修改。

全部 626 个 `.nut` 文件头均为 `FA FA 52 49 51 53 01 00 00 00 54 52 41 50`：字节码标记、`SQIR` 四字符常量的小端表示、SQChar 大小 1 和 `PART`。这与 2.2.3 `SQClosure::Save/Load` 匹配。2.2.3 的这里仅保存字符大小，不能只凭这个头判断整数和浮点大小；后两者来自上述机器码。

## 3. MSVC

Rich Header 的关键原始记录：

| comp.id | product | build | count |
| --- | --- | --- | --- |
| `00847809` | Utc1500_CPP | 30729 | 47 |
| `00837809` | Utc1500_C | 30729 | 177 |
| `008a7809` | Utc1500_LTCG_CPP | 30729 | 167 |
| `00957809` | Masm900 | 30729 | 60 |
| `00917809` | Linker900 | 30729 | 1 |
| `0094521e` | Cvtres900 | 21022 | 1 |

PE optional header 的 linker version 是 9.0。共同指向 **VC9 / VS2008 SP1，build 30729**。参考编译器首选 `cl 15.00.30729.01` / `link 9.00.30729.01`；Rich Header 不编码完整补丁末位，因此无法声称 `.01` 精确命中。`_MSC_VER` 对应 1500。

其他记录含旧版 import libraries、未标记对象与 AliasObj；它们说明链接输入混合，不等于游戏主代码采用多个旧编译器。Rich count 也不是源文件或最终函数的精确计数，不能把 167 个 LTCG 对象全部归给 Squirrel。

映射来源：[Rich parser 作者的 product-id 表](https://gist.github.com/skochinsky/07c8e95e33d9429d81a75622b5d24c8b)、[richprint 的 build 表](https://github.com/dishather/richprint/blob/master/comp_id.txt)、[Microsoft 编译器版本说明](https://learn.microsoft.com/en-us/cpp/overview/compiler-versions)。Rich Header 属未公开格式，表格是逆向工程资料；保留原始 comp.id 便于以后重新判断。

## 4. 参数能够判断到什么程度

| 参数/行为 | 结论与限制 |
| --- | --- |
| `/GL` + `/LTCG` | 存在 LTCG C++ 记录。`sq_base_register` 用 EAX 接收 VM 指针（`0x00485f45 MOV EDI,EAX`），且若干 API 已内联，支持 Squirrel 参与全程序优化。不能恢复完整原始命令行。 |
| `/MT` | 无 MSVCR/MSVCP/UCRT 导入或 delay import，EXE 含 CRT 错误消息和实现。静态 CRT 强推断；`/MT` 是参考构建首选，不排除各库不同设置。 |
| `/O2 /Ob2 /Oy` | 机器码有内联及省略帧指针；无法唯一归因于这些开关，`/O1` 等也可能产生类似行为。作为实验维度。 |
| `/Zi` 或 `/Z7`，link `/DEBUG` | RSDS/PDB 记录支持链接时生成调试信息；无法区分编译时 `/Zi` 与 `/Z7`，也不表示 Debug 构建。 |
| `/GS`、`/EHsc`、`/GR`、`/fp`、`/Gd`、`/Gy`、`/GF`、`/Zp` | 本轮未确定；不要根据默认值假定原始设置。特别是优化后的 EAX 传参不能写成普通 `__fastcall`。 |
| `/DYNAMICBASE`、`/NXCOMPAT` | DllCharacteristics=`0x8000`，这两个位未设置；没有 `.reloc` 节。只记录映像状态，不唯一还原 `/FIXED` 等参数。 |

## 5. 已取得的官方参考源码

- 发布目录：[Squirrel 2.2.3 stable](https://sourceforge.net/projects/squirrel/files/squirrel2/squirrel%202.2.3%20stable/)，发布记录日期 2009-06-30。
- 文件：`squirrel_2.2.3_stable.tar.gz`，419,005 字节。
- 本次下载 SHA-256：`a822def76b177024e45a013017df43e390cbbc7fac970a131b50855b54eded37`（本地固定值，并非独立签名认证）。
- 本地位置：`vendor/squirrel_2.2.3_stable.tar.gz`，解压到 `vendor/SQUIRREL2/`；保留上游 copyright/许可。
- 可用下载 URL：`https://downloads.sourceforge.net/project/squirrel/squirrel2/squirrel%202.2.3%20stable/squirrel_2.2.3_stable.tar.gz?download=1`。本次普通镜像链接曾返回 HTML，应检查 gzip magic 与 hash 后解包。

官方归档自带 VC6 格式 `.dsp`，Release 使用 `/O2`。这只是上游工程设置，不能据此把游戏误判成 VC6，也不能拿它直接代表游戏构建参数。另有调查时下载的 `vendor/squirrel-git/`（现代上游）留在本地；**目标参考源码使用 SQUIRREL2 归档，不使用该 master**。

## 6. 下轮编译与 Function ID

本轮完成版本/ABI定位，尚未编译 Squirrel 或生成 FID 数据库，也未安装 VC9 工具链。

1. 准备 VC9 SP1 x86 编译器、link、lib 和对应头文件/静态 CRT。记录工具本身 hash、`cl`/`link` banner 与环境；不要用 VS2012/2013 或现代 MSVC 替代代码匹配样本。
2. 首个机器码参考库：官方 2.2.3，Win32，`/MT /O2 /EHsc /DNDEBUG`，上述 ABI 宏保持关闭，GC 默认开启。这里的 `/O2 /EHsc` 是候选起点。构建 `squirrel` 与 `sqstdlib`，保存每个 `.obj`、`.lib`、命令行。
3. **为 FID 保留一套 `/GL-` 的普通 COFF `.obj/.lib`**：有正常函数机器码和符号，便于 Ghidra 分析与建立 `.fidb`。不直接拿 `/GL` 中间表示对象当普通机器码导入。
4. 另做 `/GL` + `/LTCG` 的已链接样本，输出 PDB、MAP，确保所需函数被引用并保留，检查链接裁剪结果。LTCG 样本更接近目标，但内联、合并、代码裁剪和自定义寄存器传参都可能降低 FID 覆盖率；不要承诺全自动匹配。
5. 将普通对象或带符号已链接样本导入单独的参考 Ghidra 工程。CodeBrowser 启用 Function ID 插件后，用 **Tools → Function ID → Create new empty FidDb**，再 **Populate FidDb from programs**；选择已分析且符号正确的参考程序目录。可使用安装目录 `Ghidra/Features/FunctionID/ghidra_scripts` 中的脚本批处理，执行前核对参数。
6. 库标识记录为 Squirrel / 2.2.3 / vc9-sp1-x86-mt-o2-gl-off（或实际配置）。将数据库加入目标分析器的活动数据库，重新运行 Function ID，并从 `sq_base_register`、native registration tables 等人工锚点校验结果。
7. 优先试 `/O1`、`/O2` 与内联差异，再根据失败模式调整 `/GS`、`/GR` 等；每个配置分开归档，避免无依据排列组合。FID 名称和原型必须人工复核，短函数与折叠函数尤其容易误匹配。

用户所说的 functionidmap 在此按 Ghidra **Function ID 数据库**理解；常规用户生成文件为 `.fidb`，发行库还可有 `.fidbf`，不是只导入一个文本地址映射表。Ghidra 本地帮助和脚本是操作依据。

## 7. Ghidra 初始分析限制

导入和保存均成功。初始日志提示未找到 PDB、嵌入 PNG `0x004e85ec` 解析失败，外部 DLL 没有作为独立程序导入；不影响当前 Squirrel 版本证据，但不代表完整语义分析无误。自动加载的 `windows_vs12_32` 类型库是 Ghidra 的分析选择，**不是编译器版本证据**。

目前唯一人工命名的源码锚点是 `sq_base_register`。其名称保留上游拼写，MCP 的 PascalCase 建议不适用于第三方原名还原。初始标注可用 `tools/SeedVirtualCity.java` 重放，脚本先验证目标 SHA-256，避免把地址标注套到其他版本。
