import os
import re
import base64
import subprocess

script_dir = os.path.dirname(os.path.abspath(__file__))
report_md_path = os.path.join(script_dir, "REPORT.md")
html_out_path = os.path.join(script_dir, "report_temp.html")
pdf_out_path = os.path.join(script_dir, "CUDA_Tutorial_Report.pdf")

with open(report_md_path, "r", encoding="utf-8") as f:
    md_content = f.read()

# Helper to encode local images to base64 if any exist
def img_replacer(match):
    alt_text = match.group(1)
    img_rel_path = match.group(2)
    full_path = os.path.join(script_dir, img_rel_path)
    if os.path.exists(full_path):
        with open(full_path, "rb") as img_file:
            b64_data = base64.b64encode(img_file.read()).decode("utf-8")
        ext = os.path.splitext(full_path)[1].lower().replace(".", "")
        mime = f"image/{ext}" if ext in ["png", "jpg", "jpeg", "webp"] else "image/png"
        return f'<div class="img-container"><img src="data:{mime};base64,{b64_data}" alt="{alt_text}" /><p class="caption">{alt_text}</p></div>'
    return match.group(0)

# Replace image markdown ![alt](path)
md_content = re.sub(r'!\[(.*?)\]\((.*?)\)', img_replacer, md_content)

# Convert markdown to HTML
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

if in_table:
    html_lines.append('<div class="table-container"><table>')
    for idx, r in enumerate(table_buffer):
        cols = [c.strip() for c in r.split("|")[1:-1]]
        if idx == 0:
            html_lines.append("<thead><tr>" + "".join(f"<th>{c}</th>" for c in cols) + "</tr></thead><tbody>")
        else:
            html_lines.append("<tr>" + "".join(f"<td>{c}</td>" for c in cols) + "</tr>")
    html_lines.append("</tbody></table></div>")

full_body = "\n".join(html_lines)

# Inline formatting: bold, code, links
full_body = re.sub(r'\*\*(.*?)\*\*', r'<strong>\1</strong>', full_body)
full_body = re.sub(r'`(.*?)`', r'<code>\1</code>', full_body)
full_body = re.sub(r'\[(.*?)\]\((.*?)\)', r'<a href="\2">\1</a>', full_body)

html_template = f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>CUDA Tutorial Assessment Report</title>
<style>
  @page {{
    size: A4;
    margin: 18mm 15mm 18mm 15mm;
  }}
  body {{
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
    color: #1f2937;
    line-height: 1.6;
    font-size: 10pt;
    margin: 0;
    padding: 0;
  }}
  .doc-title {{
    color: #107c41;
    border-bottom: 3px solid #107c41;
    padding-bottom: 0.3em;
    font-size: 20pt;
    margin-top: 0;
    margin-bottom: 0.8em;
  }}
  .section-title {{
    color: #111827;
    border-bottom: 1.5px solid #e5e7eb;
    padding-bottom: 0.25em;
    font-size: 14pt;
    margin-top: 1.6em;
    page-break-after: avoid;
  }}
  .subsection-title {{
    color: #1f2937;
    font-size: 11.5pt;
    margin-top: 1.2em;
    page-break-after: avoid;
  }}
  .sub-subsection-title {{
    color: #374151;
    font-size: 10.5pt;
    margin-top: 1em;
    page-break-after: avoid;
  }}
  p {{
    margin: 0.5em 0;
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
    font-size: 9pt;
    margin-bottom: 1em;
  }}
  th, td {{
    border: 1px solid #d1d5db;
    padding: 6px 9px;
    text-align: left;
  }}
  th {{
    background-color: #f3f4f6;
    font-weight: 600;
    color: #111827;
  }}
  tr:nth-child(even) {{
    background-color: #f9fafb;
  }}
  .code-block {{
    background-color: #f8fafc;
    border: 1px solid #e2e8f0;
    border-left: 4px solid #107c41;
    border-radius: 4px;
    padding: 8px 12px;
    font-family: Consolas, "Liberation Mono", Menlo, Courier, monospace;
    font-size: 8.5pt;
    line-height: 1.45;
    overflow-x: auto;
    page-break-inside: avoid;
    white-space: pre-wrap;
    word-break: break-all;
  }}
  code {{
    font-family: Consolas, "Liberation Mono", Menlo, Courier, monospace;
    background-color: rgba(16, 124, 65, 0.08);
    color: #0f5132;
    padding: 0.15em 0.35em;
    border-radius: 3px;
    font-size: 9pt;
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
    font-size: 8.5pt;
    color: #6b7280;
    margin-top: 5px;
    font-style: italic;
    text-align: center;
  }}
  blockquote {{
    border-left: 4px solid #107c41;
    padding: 0.4em 1em;
    color: #374151;
    margin: 0.8em 0;
    background: #f0fdf4;
    border-radius: 0 4px 4px 0;
  }}
  hr {{
    border: 0;
    height: 1px;
    background: #e5e7eb;
    margin: 1.4em 0;
  }}
  li {{
    margin: 0.25em 0;
  }}
  .spacer {{
    height: 0.35em;
  }}
  a {{
    color: #0d9488;
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

# Run Microsoft Edge in Headless Mode to generate PDF
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

print(f"Converting HTML to PDF via Headless Browser: {browser_exe}...")
res = subprocess.run(cmd, capture_output=True, text=True)
if res.returncode == 0 and os.path.exists(pdf_out_path):
    print(f"SUCCESS: Created PDF at {pdf_out_path} ({os.path.getsize(pdf_out_path)} bytes)")
else:
    print(f"Browser return code: {res.returncode}")
    print("Stderr:", res.stderr)
