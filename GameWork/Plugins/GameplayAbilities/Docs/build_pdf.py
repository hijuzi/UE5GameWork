# -*- coding: utf-8 -*-
"""
把 Docs 下的策划文档渲染成带【重点 / 关键】标记的 PDF。

用法：
    python build_pdf.py                    # 默认处理 Buff配置说明-策划版.md
    python build_pdf.py 其它文档.md

依赖：
    pip install markdown pypdf reportlab

流程：
    Markdown -> HTML（套样式 + 自动标记重点）-> Chrome/Edge 无头打印 -> 叠加页码
"""
import datetime
import io
import re
import subprocess
import sys
import tempfile
import threading
from pathlib import Path

try:
    sys.stdout.reconfigure(encoding="utf-8")
except Exception:
    pass

# Windows 控制台默认 GBK 时，读取子进程输出会出现解码噪声；它不影响构建结果。
# 本脚本的线程只用于读取子进程输出，这里统一吞掉，避免刷屏。
threading.excepthook = lambda args: None

import markdown
from pypdf import PdfReader, PdfWriter
from reportlab.pdfgen import canvas

DOCS = Path(__file__).resolve().parent
DEFAULT_MD = "Buff配置说明-策划版.md"

# PDF 首页信息条，按需修改
DOC_META = ["适用对象：策划", "引擎基线：UE 5.8"]

BROWSERS = [
    r"C:\Program Files\Google\Chrome\Application\chrome.exe",
    r"C:\Program Files (x86)\Google\Chrome\Application\chrome.exe",
    r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe",
    r"C:\Program Files\Microsoft\Edge\Application\msedge.exe",
]

# 负面 / 易错规则 -> 红框「重点」
WARN_RE = re.compile(
    r"不要选|必须填|最常见的错误|最容易|容易被漏掉|不配的话|忘了|否则|会提前"
    r"|会变成|完全不是一回事|永远是一|看起来就像|就不掉血|失效"
)
# 正向关键结论 -> 蓝框「关键」
KEY_RE = re.compile(
    r"区别只有一条|就这些|就可以|都是这个套路|唯一的|不在 GE 配置范围内"
    r"|写在效果计算里|留空.{0,2}的意思|正好到期|就定死了"
)
# 整章都是易错清单的标题
DANGER_TITLE_RE = re.compile(r"<h2>([^<]*容易配错[^<]*)</h2>", re.S)

CSS = r"""
@page { size: A4; margin: 17mm 15mm 16mm 15mm; }
* { box-sizing: border-box; }
body {
  font-family: "Microsoft YaHei", "微软雅黑", "PingFang SC", "Segoe UI", sans-serif;
  font-size: 10.5pt; line-height: 1.75; color: #1f2328; margin: 0;
}
h1 {
  font-size: 21pt; color: #0b4f9e; text-align: center; letter-spacing: 1px;
  margin: 0 0 8px; padding-bottom: 10px; border-bottom: 3px solid #0b4f9e;
}
.docmeta {
  display: flex; justify-content: center; gap: 20px; font-size: 9pt;
  color: #57606a; margin-bottom: 20px; padding-bottom: 10px;
  border-bottom: 1px dashed #d0d7de;
}
h2 {
  font-size: 15pt; color: #0b4f9e; margin: 24px 0 10px;
  padding: 5px 0 5px 11px; border-left: 6px solid #0b4f9e;
  background: linear-gradient(90deg, #eaf2fd, #ffffff);
  page-break-after: avoid;
}
h2.danger {
  color: #b31a26; border-left-color: #cf222e;
  background: linear-gradient(90deg, #ffeeed, #ffffff);
}
h3 {
  font-size: 12.5pt; color: #24292f; margin: 18px 0 8px; padding-bottom: 4px;
  border-bottom: 1px dashed #c8d1da; page-break-after: avoid;
}
p { margin: 8px 0; }
strong { color: #b32d0f; font-weight: 700; }
code {
  font-family: "Cascadia Mono", Consolas, "Courier New", monospace;
  background: #f0f3f6; color: #b32d0f; padding: 1px 5px; border-radius: 3px;
  font-size: .9em; white-space: nowrap;
}
td code { white-space: normal; }
table {
  width: 100%; border-collapse: collapse; margin: 12px 0;
  font-size: 9.8pt; page-break-inside: avoid;
}
th {
  background: #0b4f9e; color: #fff; font-weight: 600; text-align: left;
  padding: 7px 9px; border: 1px solid #0b4f9e;
}
td { padding: 6px 9px; border: 1px solid #d5dde5; vertical-align: top; }
tbody tr:nth-child(even) td { background: #f7f9fc; }
blockquote {
  margin: 12px 0; padding: 9px 13px; background: #fff8e1;
  border-left: 5px solid #d9a406; color: #6b4c00; font-size: 9.8pt;
}
blockquote p { margin: 3px 0; }
img {
  display: block; max-width: 100%; margin: 12px auto;
  border: 1px solid #c8d1da; border-radius: 5px; page-break-inside: avoid;
}
hr { border: none; border-top: 1px solid #dde3e9; margin: 18px 0; }
.warn, .key { margin: 12px 0; padding: 9px 13px; border-radius: 4px; font-size: 10pt; }
.warn { background: #fff1f0; border: 1px solid #ffc9c4; border-left: 6px solid #cf222e; color: #8a1420; }
.key  { background: #eef6ff; border: 1px solid #bcd7f5; border-left: 6px solid #0b4f9e; color: #0a3d73; }
.warn::before, .key::before {
  display: inline-block; font-size: 8.5pt; font-weight: 700; color: #fff;
  padding: 1px 7px; border-radius: 3px; margin-right: 7px;
  vertical-align: 2px; letter-spacing: .5px;
}
.warn::before { content: "重点"; background: #cf222e; }
.key::before  { content: "关键"; background: #0b4f9e; }
.warn code, .key code { background: rgba(255, 255, 255, .8); }
"""

TEMPLATE = """<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="utf-8">
<title>{title}</title>
<style>{css}</style>
</head>
<body>
{body}
</body>
</html>
"""


def find_browser():
    for path in BROWSERS:
        if Path(path).exists():
            return path
    raise SystemExit("未找到 Chrome / Edge，无法打印 PDF")


def decorate(match):
    inner = match.group(1)
    if "<img" in inner:
        return match.group(0)
    if WARN_RE.search(inner):
        return '<p class="warn">{}</p>'.format(inner)
    if KEY_RE.search(inner):
        return '<p class="key">{}</p>'.format(inner)
    return match.group(0)


def mark_danger_sections(body):
    """把「容易配错」整章的段落全部升级为红框。"""

    def repl(match):
        title, sec = match.group(1), match.group(2)
        sec = sec.replace('<p class="key">', '<p class="warn">')
        sec = re.sub(r"<p>", '<p class="warn">', sec)
        return '<h2 class="danger">{}</h2>{}'.format(title, sec)

    pattern = re.compile(
        r"<h2>([^<]*容易配错[^<]*)</h2>(.*?)(?=<hr|<h2|$)", re.S
    )
    return pattern.sub(repl, body)


def build_html(md_path, html_path):
    md_text = md_path.read_text(encoding="utf-8")
    body = markdown.markdown(
        md_text,
        extensions=["tables", "fenced_code", "attr_list", "sane_lists"],
    )
    body = re.sub(r"<p>(.*?)</p>", decorate, body, flags=re.S)
    body = mark_danger_sections(body)

    heading = re.search(r"^#\s+(.+)$", md_text, re.M)
    title = heading.group(1).strip() if heading else md_path.stem

    meta = '<div class="docmeta">{}</div>'.format(
        "".join("<span>{}</span>".format(x) for x in DOC_META)
        + "<span>生成日期：{}</span>".format(datetime.date.today().isoformat())
    )
    body = body.replace("</h1>", "</h1>\n" + meta, 1)

    html_path.write_text(
        TEMPLATE.format(title=title, css=CSS, body=body), encoding="utf-8"
    )
    return title


def print_pdf(html_path, pdf_path, browser):
    cmd = [
        browser,
        "--headless=new",
        "--disable-gpu",
        "--no-pdf-header-footer",
        "--virtual-time-budget=6000",
        "--user-data-dir={}".format(Path(tempfile.gettempdir()) / "cb_pdf_profile"),
        "--print-to-pdf={}".format(pdf_path),
        html_path.as_uri(),
    ]
    # Chrome 的输出是 UTF-8；显式指定编码，避免 Windows 默认 GBK 参与解码
    subprocess.run(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        encoding="utf-8",
        errors="replace",
    )
    if not pdf_path.exists():
        raise SystemExit("PDF 生成失败：{}".format(pdf_path))


def _overlay(width, height, text):
    buf = io.BytesIO()
    c = canvas.Canvas(buf, pagesize=(width, height))
    c.setFont("Helvetica", 8)
    c.setFillColorRGB(0.45, 0.47, 0.5)
    c.drawCentredString(width / 2.0, 22, text)
    c.save()
    buf.seek(0)
    return PdfReader(buf).pages[0]


def add_page_numbers(src_pdf, dst_pdf, title):
    reader = PdfReader(str(src_pdf))
    total = len(reader.pages)
    writer = PdfWriter()
    for i, page in enumerate(reader.pages, start=1):
        box = page.mediabox
        page.merge_page(
            _overlay(float(box.width), float(box.height), "%d / %d" % (i, total))
        )
        writer.add_page(page)
    writer.add_metadata({"/Title": title, "/Creator": "GameplayAbilities Docs"})
    with dst_pdf.open("wb") as f:
        writer.write(f)
    return total


def main():
    name = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_MD
    md_path = DOCS / name
    if not md_path.exists():
        raise SystemExit("找不到文档：{}".format(md_path))

    out_pdf = md_path.with_suffix(".pdf")
    tmp_html = DOCS / (md_path.stem + ".__build__.html")
    tmp_pdf = DOCS / (md_path.stem + ".__build__.pdf")

    title = build_html(md_path, tmp_html)
    print_pdf(tmp_html, tmp_pdf, find_browser())
    total = add_page_numbers(tmp_pdf, out_pdf, title)

    tmp_html.unlink(missing_ok=True)
    tmp_pdf.unlink(missing_ok=True)
    print("OK -> {} ({} pages)".format(out_pdf.name, total))


if __name__ == "__main__":
    main()
