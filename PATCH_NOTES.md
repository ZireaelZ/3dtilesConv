# 补丁版 3dtiles 说明

基线：`fanvanzh/3dtiles` master 分支 `acbcf603f33f`（2026-04-13）。

## 包含的修改

1. **坐标转换线程安全**（`src/coordinate_transformer.h/.cpp`）
   每个工作线程按源坐标定义懒创建独立的 `OGRCoordinateTransformation`，不再共享同一个 GDAL/PROJ 转换对象。
2. **转换卡死超时收尾**（`src/osgb.rs`）
   顶层瓦片目录由固定工作线程池并发转换，主线程按 5 秒粒度收结果；连续 `OTS_ENGINE_TILE_IDLE` 秒（默认 900，0 表示关闭）没有新的顶层目录完成时，写出已完成部分的 `tileset.json` 并退出，不再无限等待。
3. **KTX2 压缩线程开关**（`src/mesh_processor.cpp`）
   basisu 编码器内部多线程默认开启；设置 `OTS_KTX2_THREADED=0` 可关闭，用于排查嵌套线程导致的卡顿。
4. **Rust 1.99 兼容**（`src/shape.rs`）
   使用 `f64::INFINITY` / `f64::NEG_INFINITY` 常量，移除已弃用的 `std::f64` 导入。
5. **Windows 本地构建支持**（`build.rs`）
   通过 CMake 变量 `VCPKG_INSTALLED_DIR` 指定依赖目录并关闭 manifest 模式，构建过程不创建符号链接，不需要管理员权限。

## 构建（Windows x64）

```bat
git submodule update --init --recursive
set VCPKG_ROOT=<vcpkg 路径>
cargo build --release
```

依赖用 vcpkg 按 `vcpkg.json` 安装到仓库根目录的 `vcpkg_installed\x64-windows`（manifest 模式关闭，依赖一次性预装）。产物为 `target\release\_3dtile.exe`，同目录下的 `gdal\`、`proj\`、`osgPlugins-3.6.5\` 与 vcpkg 运行库 DLL 需一并分发。

## 运行参数

| 环境变量 | 默认值 | 说明 |
|---|---|---|
| `OTS_ENGINE_TILE_IDLE` | 900 | 顶层目录空闲超时秒数，超时后按已完成部分收尾退出；0 关闭 |
| `OTS_KTX2_THREADED` | 1 | basisu 内部多线程开关 |
| `RAYON_NUM_THREADS` | CPU 核数 | 转换工作线程数 |
