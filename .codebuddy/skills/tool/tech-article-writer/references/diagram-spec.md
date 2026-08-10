# 图表样式规范参考

> 本文档是从 SKILL.md §八 提取的详细图表绘制规范，作为 `drawio-generator` 技能的输入约束。
> 核心约束和图表类型选择规则见 SKILL.md §七。

---

## 1. 配色方案

| 元素 | 填充色 | 边框色 | 文字色 | 用途 |
|------|--------|--------|--------|------|
| 🔵 主要类/核心节点 | `#D6E8FA` | `#4A90D9` | `#1A1A2E` | 系统核心类、关键组件 |
| 🟢 辅助类/扩展节点 | `#D5F5E3` | `#27AE60` | `#1A1A2E` | 辅助类、扩展点 |
| 🟡 外部系统/接口 | `#FCF3CF` | `#F1C40F` | `#1A1A2E` | 外部依赖、引擎系统 |
| 🟠 数据/消息 | `#FAD7A1` | `#E67E22` | `#1A1A2E` | 数据结构、消息体 |
| ⚪ 注释/说明框 | `#F4F6F6` | `#BDC3C7` | `#7F8C8D` | 补充注释、图例 |
| 🔴 关键/警告 | `#FADBD8` | `#E74C3C` | `#1A1A2E` | 重点关注、易错点 |

## 2. 字体规范

- **中文字体**：`Microsoft YaHei`（微软雅黑）
- **英文字体**：`Consolas` 或 `Monaco`
- **节点标题字号**：14pt，加粗
- **节点正文字号**：12pt，常规
- **连线标签字号**：10pt，常规

### 2.1 文本框尺寸约束（强制）

**原则：文本框宽高必须容纳全部文字内容，不允许文字溢出边界。**

| 字号 | 中文单字符宽度 | 英文单字符宽度 | 单行最小高度 |
|------|-------------|-------------|------------|
| 9pt  | ~10px | ~6px  | 16px |
| 10pt | ~11px | ~7px  | 20px |
| 11pt | ~12px | ~8px  | 22px |
| 12pt | ~13px | ~8px  | 24px |
| 13pt | ~14px | ~9px  | 26px |
| 14pt | ~15px | ~10px | 28px |
| 18pt | ~20px | ~12px | 34px |

**宽度计算公式**：

```
所需宽度 = 中文字符数 × 中文单字符宽度 + 英文/数字字符数 × 英文单字符宽度
```

**高度计算公式**：

```
所需高度 = 行数 × 单行最小高度
其中 行数 = ceil(所需宽度 ÷ 实际框宽度)
```

**风格设置**：

```xml
<!-- ✅ 正确：whiteSpace=wrap + 充足高度，不用 overflow=hidden -->
<mxCell id="text1" value="中文文本内容需要自动换行"
  style="text;html=1;strokeColor=none;fillColor=none;fontSize=11;align=left;whiteSpace=wrap;"
  vertex="1" parent="1">
  <mxGeometry x="100" y="100" width="300" height="22" as="geometry"/>
</mxCell>

<!-- ❌ 错误：overflow=hidden 对中文换行不友好，会在字符边界处裁剪 -->
<mxCell id="text2" value="中文文本内容需要自动换行"
  style="text;html=1;...;overflow=hidden;" vertex="1" parent="1">
</mxCell>

<!-- ❌ 错误：高度不足，文字被底部裁剪 -->
<mxCell id="text3" value="这是可能换行的长文本内容"
  style="text;html=1;...;whiteSpace=wrap;" vertex="1" parent="1">
  <mxGeometry x="100" y="100" width="250" height="16" as="geometry"/>
</mxCell>
```

**检查清单**：

- [ ] 每个文本框：中文字符数 × 中文字宽 + 英文字符数 × 英文字宽 ≤ 框宽度？
- [ ] 如果文字宽度 > 框宽度，是否会换行？换行后的高度是否 ≥ 行数 × 单行高度？
- [ ] 混合中英文的标签尤其容易低估宽度——逐条检查，不依赖直觉
- [ ] 凡是包含 `whiteSpace=wrap` 的文本框，**不添加 `overflow=hidden`**（中文无空格，overflow=hidden 会在错误位置裁剪）

## 3. 线条与箭头

- **继承关系**：空心三角箭头 + 实线（`#7F8C8D`）
- **组合/聚合关系**：菱形 + 实线（`#4A90D9`）
- **调用/依赖关系**：实心箭头 + 虚线（`#E67E22`）
- **数据流**：实心箭头 + 实线（`#27AE60`）
- **线条宽度**：统一使用 1.5pt

### 3.1 连线走线规则（强制）

**原则：能用直线绝不用折线，能用一段折线绝不用多段。**

| 场景 | 风格 | `edgeStyle` 值 | 说明 |
|------|------|---------------|------|
| 源和目标在同一水平线（y 坐标差 ≤ 20px） | **纯水平直线** | `none` | 直接 `sourcePoint` → `targetPoint`，不要用正交路由弯折 |
| 源和目标在同一垂直线（x 坐标差 ≤ 20px） | **纯垂直直线** | `none` | 直接 `sourcePoint` → `targetPoint` |
| 源和目标不在同一行列，需要拐弯 | 正交折线 | `orthogonalEdgeStyle` | 单折点（└┐）优先，两个折点次之。**弯折次数越少越好** |
| 源和目标不在同一行列，但空间宽敞 | **斜直线** | `none` | 如果不会穿过其他图框，直接用斜线比多段折线更清晰 |

> **关键约束**：画完后检查每条线。如果发现用了 `orthogonalEdgeStyle` 但实际只产生了一段直线，必须改为 `edgeStyle=none` 并移除多余的 waypoint。

**XML 示例**：

```xml
<!-- ✅ 正确：同水平线 → 纯直线 -->
<mxCell id="e1" value="" style="endArrow=classic;html=1;strokeWidth=2;edgeStyle=none;" edge="1" parent="1">
  <mxGeometry relative="1" as="geometry">
    <mxPoint x="400" y="326" as="sourcePoint"/>
    <mxPoint x="650" y="326" as="targetPoint"/>
  </mxGeometry>
</mxCell>

<!-- ❌ 错误：同水平线却用了正交风格 + waypoint，产生无意义弯折 -->
<mxCell id="e2" value="" style="edgeStyle=orthogonalEdgeStyle;..." edge="1" parent="1" source="a" target="b">
  <mxGeometry relative="1" as="geometry">
    <Array as="points">
      <mxPoint x="500" y="326"/>   <!-- 多余的弯折点 -->
    </Array>
  </mxGeometry>
</mxCell>
```

## 4. 图框与布局

布局规则以 `drawio-generator` 强制规则为准，以下为对齐后的规范：

- 所有节点使用**圆角矩形**（圆角半径 6px）
- **水平间距**：≥120px（类图/架构图并列时至少留这个间距，避免连线重叠）
- **垂直间距**：≥100px（时序图各 lifeline 之间、流程图各步骤之间）
- 节点之间不得重叠，连线不得穿过表头或类名称区域
- **整体布局优先矩形化**：用多列网格（如 2×3、3×3、3×4）分摊节点，避免极端窄高或扁平。页面宽高比控制在 **1:1 ~ 1.6:1**（接近 4:3 或 16:10），杜绝"一根柱子"或"一根面条"式的长图
- 同类节点对齐，同层级元素保持统一坐标（如多个类的 y 坐标一致）
- 关系线尽量**水平或垂直**，避免斜线；需要拐弯时用正交路由（orthogonal edge）

> **注意**：drawio-generator 的间距规则（≥120px / ≥100px）优先级高于此处建议值，实际生成时以 drawio-generator 规则为准。

## 5. 连线避让（强制约束）

所有关系线**不得穿过任何图框（swimlane / rect / rounded rect）的内部区域**。这是硬性约束，绘制完成后必须逐线检查。

**核心思路**：利用图框之间的**天然间隙**（gap）走线，而非一律绕到布局最外围——折线应该"见缝插针"，就近穿过空闲通道。

### 5.1 间隙寻路算法

**Step 1：建立障碍地图**

为每个图框计算"禁区"矩形（含 20px margin）：

```
禁区 = [x - 20, y - 20, x + w + 20, y + h + 20]
```

所有图框的禁区合集 = 连线不可穿越的区域。

**Step 2：计算可用的间隙通道**

- **垂直通道**：相邻两列图框之间的 x 区间（水平间距 ≥120px 的区域），从页面顶部贯穿到底部
- **水平通道**：相邻两行图框之间的 y 区间（垂直间距 ≥100px 的区域），从页面左侧贯穿到右侧

对每个通道，记录它的坐标范围 `[start, end]` 和中心线坐标 `center`。

```
示例布局（3 列 2 行，间距 120px / 100px）：

        col-gap-1       col-gap-2
        (x=420~540)     (x=960~1080)
   ┌─────────┐   ┌─────────┐   ┌─────────┐
   │ Box A   │   │ Box D   │   │ Box G   │  ← row 0, y=100
   │ x=0~300 │   │ x=540~840│   │ x=1080~1380│
   └─────────┘   └─────────┘   └─────────┘
        | row-gap-1 (y=360~460)
   ┌─────────┐   ┌─────────┐   ┌─────────┐
   │ Box B   │   │ Box E   │   │ Box H   │  ← row 1, y=460
   └─────────┘   └─────────┘   └─────────┘
        | row-gap-2 (y=720~820)
   ┌─────────┐   ┌─────────┐   ┌─────────┐
   │ Box C   │   │ Box F   │   │ Box I   │  ← row 2, y=820
   └─────────┘   └─────────┘   └─────────┘

可用垂直通道: v1=(420,540,center=480), v2=(960,1080,center=1020)
可用水平通道: h1=(360,460,center=410), h2=(720,820,center=770)
```

**Step 3：为每条边选择路由**（按优先级尝试，命中即停止）

| 优先级 | 策略 | 条件 | 折线形状 |
|--------|------|------|----------|
| **P1** | 直连 | 源和目标在同一列（x 重叠 ≥40px）且纵轴上无障碍 | `│` 或 `─`（零折点） |
| **P1** | 直连 | 源和目标在同一行（y 重叠 ≥40px）且横轴上无障碍 | `─`（零折点） |
| **P2** | 单折 | 源和目标不在同行/同列，但存在一条垂直通道 v 使得 (sx → v → tx) 全程无障碍 | `└┐`（一个折点） |
| **P3** | 双折·垂直通道 | 源和目标在不同列，取**距离两端中点最近**的垂直间隙 v，走 s→(v, sy)→(v, ty)→t | `│─│`（两个折点） |
| **P4** | 双折·水平通道 | 源和目标在不同行，取**距离两端中点最近**的水平间隙 h，走 s→(sx, h)→(tx, h)→t | `──┐└──`（两个折点） |
| **P5** | 外围绕行 | 以上均失败（布局极度拥挤），走最左侧或最右侧外围 | 兜底方案 |

> **关键原则**：优先 P3/P4 的"就近间隙"方案，而非 P5 的"一刀切外围绕行"。折线应该在图框之间的合理空隙中穿行，保持视觉紧凑。

**Step 4：生成 waypoint XML**

确定了走线方案后，将折点坐标写为 `<Array as="points">`：

```xml
<!-- P3 示例：Box B(左列, y=560) → Box G(右列, y=100)
     选取 col-gap-1 (x=480) 作为垂直通道 -->
<mxCell id="edge-1" value="uses"
  style="edgeStyle=orthogonalEdgeStyle;...;exitX=1;exitY=0.5;entryX=0;entryY=0.5;"
  edge="1" parent="1" source="box-B" target="box-G">
  <mxGeometry relative="1" as="geometry">
    <Array as="points">
      <mxPoint x="480" y="560"/>   <!-- 从 Box B 右侧横走到 col-gap-1 -->
      <mxPoint x="480" y="100"/>   <!-- 沿 col-gap-1 垂直走到 Box G 高度 -->
    </Array>
  </mxGeometry>
</mxCell>
```

### 5.2 折点坐标计算规则

折点必须落在**通道安全区**内：

- **水平段**：y 坐标在水平通道范围内，两端各留 10px margin
- **垂直段**：x 坐标在垂直通道范围内，两端各留 10px margin
- **折点**：x 取垂直通道中心线，y 取水平通道中心线，确保折点本身不在任何禁区上

### 5.3 连线美化

- 多条线共用一个垂直通道时，在通道内**均匀分布**（如通道宽 120px，3 条线各间隔 30px），避免视觉重叠
- `exitX/exitY` 和 `entryX/entryY` 设置为源/目标框距通道最近的那个面的中心

### 5.4 连线文字约束

连线上的标签文字（如 `uses`、`manages`、`implements`）默认放在线段中间位置，但**必须确保文字 bounding box 不与任何图框重叠**。

**放置规则**：

| 线段类型 | 默认位置 | 冲突时调整 |
|----------|---------|-----------|
| 水平段 | 线段中点，文字在线**上方**（y 偏移 -8px） | 若上方有图框，改到下方（y 偏移 +8px） |
| 垂直段 | 线段中点，文字在线**左侧**（x 偏移 -8px） | 若左侧有图框，改到右侧（x 偏移 +8px） |
| 折点附近 | 放在不会与折角线重叠的一侧 | 远离折角 20px 以上 |

**冲突检测**：文字标签的 bounding box ≈ 文字宽度 × 14px 高度。计算标签矩形是否与任何图框的禁区矩形相交。若相交，尝试以下调整（按顺序）：

1. **换侧**：从线上方换到下方（或左换右）
2. **沿线段平移**：保持同侧，沿线段方向往远离冲突点移动 20~40px，直到脱离禁区
3. **移到相邻段**：当前线段全部处于禁区 → 标签移到相邻折线段上

**XML 实现**：通过 label 的 `mxGeometry` offset 控制位置：

```xml
<mxCell id="edge-1" value="uses"
  style="edgeStyle=orthogonalEdgeStyle;...;labelBackgroundColor=none;"
  edge="1" parent="1" source="box-A" target="box-B">
  <mxGeometry relative="1" as="geometry">
    <Array as="points">
      <mxPoint x="480" y="560"/>
      <mxPoint x="480" y="100"/>
    </Array>
    <!-- 标签放在首段水平线中点上方 -->
    <mxPoint x="390" y="552" as="offset"/>
  </mxGeometry>
</mxCell>
```

> `labelBackgroundColor=none` 保持透明背景，避免标签遮挡线条。如果走线区域背景复杂（图框密集），可设置为 `labelBackgroundColor=#FFFFFF` 增加可读性。

### 5.5 检查清单

- [ ] 每条连线的每一段（折点之间）是否穿入了任何图框的禁区矩形？
- [ ] 折点坐标是否在间隙通道内（不在任何图框内部）？
- [ ] 连线上的文字标签是否与任何图框重叠？（逐标签检查 bounding box vs 禁区）
- [ ] 标签冲突时是否已按 §5.4 规则调整位置（换侧 → 平移 → 移段）？
- [ ] 多条线共用同一通道时是否有 ≥20px 间距？
- [ ] `entryX/entryY` 和 `exitX/exitY` 是否与 waypoint 路径一致（不冲突）？

## 6. 导出设置

- **格式**：PNG
- **缩放**：200%（2x 分辨率，保证 Retina 屏幕清晰度）
- **导出位置**：与 .drawio 源文件同一目录
- **macOS**：`/Applications/draw.io.app/Contents/MacOS/draw.io -x -f png --scale 2 -o <output>.png <input>.drawio`
- **Windows**：`& "$env:LOCALAPPDATA\Programs\draw.io\draw.io.exe" -x -f png --scale 2 -o <output>.png <input>.drawio`
