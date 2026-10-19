import os
import re
import base64
import subprocess

report_md_path = "REPORT.md"
html_out_path = "report/report_temp.html"
pdf_out_path = os.path.abspath("report/OpenMP_Tutorial05_Report.pdf")

with open(report_md_path, "r", encoding="utf-8") as f:
    md_content = f.read()

# Helper to encode local images to base64
def img_replacer(match):
    alt_text = match.group(1)
    img_rel_path = match.group(2)
    full_path = os.path.abspath(img_rel_path)
    if os.path.exists(full_path):
        with open(full_path, "rb") as img_file:
            b64_data = base64.b64encode(img_file.read()).decode("utf-8")
        ext = os.path.splitext(full_path)[1].lower().replace(".", "")
        mime = f"image/{ext}" if ext in ["png", "jpg", "jpeg", "webp"] else "image/png"
        return f'<div class="img-container"><img src="data:{mime};base64,{b64_data}" alt="{alt_text}" /><p class="caption">{alt_text}</p></div>'
    return match.group(0)

# Replace image markdown ![alt](path)
md_content = re.sub(r'!\[(.*?)\]\((.*?)\)', img_replacer, md_content)

# Convert markdown to HTML manually and cleanly
lines = md_content.split("\n")
html_lines = []
in_code = False
code_lang = ""
code_buffer = []
in_table = False
table_buffer = []

for line in lines:
    # Code block
    if line.startswith("```"):
        if in_code:
            in_code = False
            code_str = "\n".join(code_buffer)
            # escape html
            code_str = code_str.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
            html_lines.append(f'<pre class="code-block"><code class="language-{code_lang}">{code_str}</code></pre>')
            code_buffer = []
        else:
            in_code = True
            code_lang = line[3:].strip()
        continue

    if in_code:
        code_buffer.append(line)
        continue

    # Tables
    if "|" in line:
        # Check if table separator
        if re.match(r'^\s*\|?\s*:?-+:?\s*\|.*', line):
            continue
        if not in_table:
            in_table = True
            table_buffer = []
        table_buffer.append(line)
        continue
    else:
        if in_table:
            in_table = False
            # Render table
            html_lines.append('<div class="table-container"><table>')
            for idx, r in enumerate(table_buffer):
                cols = [c.strip() for c in r.split("|")[1:-1]]
                if idx == 0:
                    html_lines.append("<thead><tr>" + "".join(f"<th>{c}</th>" for c in cols) + "</tr></thead><tbody>")
                else:
                    html_lines.append("<tr>" + "".join(f"<td>{c}</td>" for c in cols) + "</tr>")
            html_lines.append("</tbody></table></div>")
            table_buffer = []

    # Headings
    if line.startswith("# "):
        html_lines.append(f'<h1 class="doc-title">{line[2:].strip()}</h1>')
    elif line.startswith("## "):
        html_lines.append(f'<h2 class="section-title">{line[3:].strip()}</h2>')
    elif line.startswith("### "):
        html_lines.append(f'<h3 class="subsection-title">{line[4:].strip()}</h3>')
    elif line.startswith("#### "):
        html_lines.append(f'<h4 class="sub-subsection-title">{line[5:].strip()}</h4>')
    elif line.startswith("> "):
        html_lines.append(f'<blockquote>{line[2:].strip()}</blockquote>')
    elif line.startswith("---"):
        html_lines.append('<hr />')
    elif line.startswith("- "):
        html_lines.append(f'<li class="bullet">{line[2:].strip()}</li>')
    elif re.match(r'^\d+\.\s', line):
        html_lines.append(f'<li class="num-item">{re.sub(r"^\d+\.\s", "", line).strip()}</li>')
    elif line.strip() == "":
        html_lines.append('<div class="spacer"></div>')
    else:
        html_lines.append(f'<p>{line.strip()}</p>')

full_body = "\n".join(html_lines)

# Inline formatting: bold, code, links
full_body = re.sub(r'\*\*(.*?)\*\*', r'<strong>\1</strong>', full_body)
full_body = re.sub(r'`(.*?)`', r'<code>\1</code>', full_body)
full_body = re.sub(r'\[(.*?)\]\((.*?)\)', r'<a href="\2">\1</a>', full_body)

html_template = f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>OpenMP Tutorial 05 Assessment Report</title>
<style>
  @page {{
    size: A4;
    margin: 20mm 15mm 20mm 15mm;
  }}
  body {{
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
    color: #24292e;
    line-height: 1.6;
    font-size: 10.5pt;
    margin: 0;
    padding: 0;
  }}
  .doc-title {{
    color: #0366d6;
    border-bottom: 2px solid #eaecef;
    padding-bottom: 0.3em;
    font-size: 22pt;
    margin-top: 0;
  }}
  .section-title {{
    color: #1a202c;
    border-bottom: 1px solid #eaecef;
    padding-bottom: 0.2em;
    font-size: 15pt;
    margin-top: 1.5em;
    page-break-after: avoid;
  }}
  .subsection-title {{
    color: #2d3748;
    font-size: 12.5pt;
    margin-top: 1.2em;
    page-break-after: avoid;
  }}
  .sub-subsection-title {{
    color: #4a5568;
    font-size: 11pt;
    margin-top: 1em;
    page-break-after: avoid;
  }}
  p {{
    margin: 0.6em 0;
    text-align: justify;
  }}
  .table-container {{
    margin: 1em 0;
    width: 100%;
    overflow-x: auto;
    page-break-inside: avoid;
  }}
  table {{
    width: 100%;
    border-collapse: collapse;
    font-size: 9.5pt;
    margin-bottom: 1em;
  }}
  th, td {{
    border: 1px solid #d1d5db;
    padding: 6px 10px;
    text-align: left;
  }}
  th {{
    background-color: #f3f4f6;
    font-weight: 600;
    color: #111827;
  }}
  tr:nth-child(even) {{
    background-color: #fafbfc;
  }}
  .code-block {{
    background-color: #f6f8fa;
    border: 1px solid #e1e4e8;
    border-radius: 6px;
    padding: 10px 14px;
    font-family: Consolas, "Liberation Mono", Menlo, Courier, monospace;
    font-size: 9pt;
    line-height: 1.45;
    overflow-x: auto;
    page-break-inside: avoid;
    white-space: pre-wrap;
    word-break: break-all;
  }}
  code {{
    font-family: Consolas, "Liberation Mono", Menlo, Courier, monospace;
    background-color: rgba(27,31,35,0.05);
    padding: 0.2em 0.4em;
    border-radius: 3px;
    font-size: 9.5pt;
  }}
  .img-container {{
    text-align: center;
    margin: 1.5em 0;
    page-break-inside: avoid;
  }}
  .img-container img {{
    max-width: 95%;
    height: auto;
    border: 1px solid #e1e4e8;
    border-radius: 6px;
    box-shadow: 0 2px 4px rgba(0,0,0,0.1);
  }}
  .caption {{
    font-size: 9pt;
    color: #586069;
    margin-top: 6px;
    font-style: italic;
    text-align: center;
  }}
  blockquote {{
    border-left: 4px solid #0366d6;
    padding: 0 1em;
    color: #6a737d;
    margin: 0.8em 0;
    background: #f1f8ff;
    border-radius: 0 4px 4px 0;
  }}
  hr {{
    border: 0;
    height: 1px;
    background: #eaecef;
    margin: 1.5em 0;
  }}
  li {{
    margin: 0.3em 0;
  }}
  .spacer {{
    height: 0.4em;
  }}
  a {{
    color: #0366d6;
    text-decoration: none;
  }}
</style>
</head>
<body>
{full_body}
</body>
</html>
"""

with open(html_out_path, "w", encoding="utf-8") as f:
    f.write(html_template)

print(f"Generated intermediate HTML at {html_out_path}")

# Run Microsoft Edge or Chrome in Headless Mode to print to PDF
edge_path = r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe"
chrome_path = r"C:\Program Files\Google\Chrome\Application\chrome.exe"
browser_exe = edge_path if os.path.exists(edge_path) else chrome_path

abs_html = os.path.abspath(html_out_path)
cmd = [
    browser_exe,
    "--headless",
    "--disable-gpu",
    "--run-all-compositor-stages-before-draw",
    "--no-pdf-header-footer",
    f"--print-to-pdf={pdf_out_path}",
    f"file:///{abs_html.replace(os.sep, '/')}"
]

print("Converting HTML to PDF via Headless Browser...")
res = subprocess.run(cmd, capture_output=True, text=True)
if res.returncode == 0 and os.path.exists(pdf_out_path):
    print(f"SUCCESS: Created PDF at {pdf_out_path} ({os.path.getsize(pdf_out_path)} bytes)")
else:
    print(f"Browser return code: {res.returncode}")
    print("Stderr:", res.stderr)
