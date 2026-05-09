# Fallen Knight 拆分版

这是从原始单文件程序拆出来的多文件结构，尽量保持原逻辑不变。

## 文件职责

* `Common.h/.cpp`：公共 include、常量、`Rect`、AABB、透明贴图函数、全局音频错误信息。
* `InputManager.h`：键盘输入状态。
* `Animator.h`：精灵表动画播放。
* `SoundManager.h`：BGM 和音效播放。
* `GameAssets.h`：图片资源加载。
* `GameObject.h` / `Entity.h`：游戏对象和带血量/物理的实体基类。
* `Projectile.h`：Boss 发射物。
* `Player.h/.cpp`：玩家类、输入、攻击、防御、移动和动画切换。
* `Boss.h`：Boss 状态机和 AI。为了最小改动，Boss 的实现保留在头文件内。
* `UI.h/.cpp`：文字和菜单按钮绘制。
* `Game.h/.cpp`：游戏状态、主更新、碰撞、绘制、循环。
* `Main.cpp`：程序入口。

## 使用方法

1. 新建一个 Visual Studio C++ 项目。
2. 安装并配置 EasyX。
3. 把这些 `.h` / `.cpp` 文件全部加入项目。
4. 保持原来的素材目录不变：`bg.png`、`knight\_r.png`、`knight\_l.png`、`boss\_r.png`、`boss\_l.png`、`arm\_projectile\_r.png`、`arm\_projectile\_l.png`、`Laser\_sheet\_r.png`、`Laser\_sheet\_l.png`，以及 `sounds` 文件夹。
5. 编译运行。

