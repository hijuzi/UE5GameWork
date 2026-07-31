# Humanizer 使用指南

## 功能

去除 AI 写作痕迹，让文本更自然、更像人类书写。基于维基百科 "AI 写作特征" 指南。

## 使用方式

```
"用 humanizer 润色这段文字"
"让这段文档读起来更自然"
```

## 识别并修复的 24 种 AI 模式

| 类别 | 典型问题 |
|------|---------|
| **内容膨胀** | 夸大重要性、虚假宣传语、肤浅 -ing 分析句 |
| **AI 词汇** | "crucial"、"showcase"、"landscape"、"tapestry" 等高频词 |
| **句法模式** | 破折号滥用、三连排比、否定并列（"不仅...更是..."） |
| **风格问题** | 标题大写、emoji 装饰、粗体滥用、弯引号 |

## 使用示例

### 示例 1：润色技术文档

> **你**: "帮我润色这段 README，去掉 AI 味：
> *'This project serves as a testament to modern engineering excellence, showcasing a seamless and intuitive architecture that ensures optimal performance across diverse environments.'*"

> **AI**: *"This project is a backend service for processing video files. It handles upload, transcoding, and delivery across three cloud providers."*

### 示例 2：润色 API 注释

> **你**: "这段注释太 AI 了，润色一下：
> *'Moreover, this function plays a pivotal role in the data pipeline by highlighting critical transformations and underscoring the system's commitment to reliability.'*"

> **AI**: *"This function converts raw sensor data into normalized readings and logs any values outside the expected range."*

### 示例 3：批量处理文档

> **你**: "把 `docs/api/` 下所有 md 文件用 humanizer 过一遍"

> **AI**: 逐个文件扫描 AI 模式并输出润色结果。

## 输出格式

```
## 润色结果
[改写后的文本]

## 修改摘要（可选）
- 移除 "serves as a testament"（夸大重要性）
- 移除 "Moreover"（AI 词汇）
- 简化 "seamless and intuitive"（宣传语）
- 替换泛泛而谈为具体细节
```

## 适用场景

- 技术文档发布前的最后润色
- AI 生成的 PR 描述改为自然语言
- 项目 README / Wiki 文章去 AI 味
- 任何需要读起来像人写的文本
