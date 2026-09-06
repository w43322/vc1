# Virtual City decomp agent entry point

本项目仍是早期 decomp；可编译产物是参考 Squirrel 和小型验证程序，不是完整游戏。**整个反编译过程必须同时受 Ghidra MCP 的原版证据和 reccmp 的重编译差异约束。** 任一侧没有验证，都不能把函数标为完成。证据写在源码注释里，不要另写文档。

## 开始前

1. 检查 `git status --short`；保留用户及其他任务的修改，不擅自 reset/clean。
2. 原版必须匹配 README 中的 SHA-256。所有分析操作显式指定 `VirtualCity.Unwrapped.exe`。
3. 每次选一个有地址的函数或紧密关联的小组。下一步从源码 `// FUNCTION: VC1` 注释和未映射的 Ghidra 地址继续。

## Ghidra MCP

- 每轮先 `list_instances`，再连接项目名 `VirtualCity`；每个读写请求都显式指定 program `VirtualCity.Unwrapped.exe`。
- 写 C++ 前必须看函数边界、原始汇编、xrefs、caller/callee 和相关常量/字符串。伪代码只作辅助。
- 参数位置、返回寄存器、栈平衡、callee-saved 寄存器和函数指针 ABI 必须从汇编及至少一个调用方核实。VC9 LTCG 可能使用 EAX、ESI、EDI 等自定义传参。
- 确认后才写入描述性名称、精确类型、自定义 storage、结构字段、plate comment 和 `VC1_INFERRED`/`SQ223_IDENTIFIED` 标签。推定名必须注明不是原符号。
- 源码映射后的函数名/类型用 `tools/import-ghidra.ps1`（官方 `reccmp-ghidra-import`）写回 Ghidra，不要另写同步脚本。headless 导入前必须关掉 Ghidra GUI。
- 修改后必须 `save_program`。Ghidra 数据库不入 Git。

## reccmp

- 地址身份确认后，游戏源码使用 `// FUNCTION: VC1 0x...`；原样第三方函数使用 `config/verified.csv`。
- 每次新增或修改映射函数后运行 `tools/build-vc9.cmd`，再运行 `tools/compare-reccmp.ps1`，阅读该函数 diff。
- 不要修改 `third_party/squirrel223`、加入内联汇编或伪造映射。已恢复源码组织和编译参数可以按原版证据调整；保留 `/GL /LTCG`。
- 只报告具体函数匹配率，不称为全游戏进度。

## 不变量

- VC9 SP1 x86，不要用现代 C++ 语法。
- 不允许只在 Ghidra 留名字，也不允许只在 `src/` 写伪代码。
- 默认不创建子 agent，不上传游戏资产，不提交本地工具、构建物和数据库。
