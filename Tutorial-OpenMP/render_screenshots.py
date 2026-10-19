import os
from PIL import Image, ImageDraw, ImageFont

os.makedirs("screenshots", exist_ok=True)

results_map = {
    "results/q1_output.txt": "screenshots/q1_race_condition.png",
    "results/q2_output.txt": "screenshots/q2_scheduling_nowait.png",
    "results/q3_output.txt": "screenshots/q3_monte_carlo_pi.png",
    "results/q4_output.txt": "screenshots/q4_binary_tree_sum.png",
    "results/q5_output.txt": "screenshots/q5_bfs_dfs.png",
    "results/q6_output.txt": "screenshots/q6_default_schedule.png",
    "results/optional_heat_output.txt": "screenshots/optional_heat_conduction.png"
}

# Try to find a monospace font
font = None
font_candidates = [
    "C:/Windows/Fonts/consola.ttf",
    "C:/Windows/Fonts/cour.ttf",
    "C:/Windows/Fonts/lucon.ttf"
]
for fc in font_candidates:
    if os.path.exists(fc):
        try:
            font = ImageFont.truetype(fc, 15)
            break
        except Exception:
            pass

if font is None:
    font = ImageFont.load_default()

for txt_path, out_img in results_map.items():
    if not os.path.exists(txt_path):
        continue
    with open(txt_path, "r", encoding="utf-8", errors="replace") as f:
        lines = [line.rstrip() for line in f.readlines()]
    
    if not lines:
        continue

    # Calculate image dimensions
    line_height = 22
    max_len = max(len(l) for l in lines)
    char_width = 9.2
    img_width = max(800, int(max_len * char_width) + 60)
    header_height = 42
    img_height = header_height + len(lines) * line_height + 40

    img = Image.new("RGB", (img_width, img_height), color="#1e1e1e")
    draw = ImageDraw.Draw(img)

    # Title bar
    draw.rectangle([(0, 0), (img_width, header_height)], fill="#2d2d2d")
    # Window controls (mac/linux style dots)
    draw.ellipse([(14, 15), (26, 27)], fill="#ff5f56")
    draw.ellipse([(34, 15), (46, 27)], fill="#ffbd2e")
    draw.ellipse([(54, 15), (66, 27)], fill="#27c93f")

    # Title text
    title = f"Terminal - {os.path.basename(out_img).replace('.png', '')}"
    draw.text((80, 13), title, fill="#cccccc", font=font)

    # Draw terminal text
    y = header_height + 15
    for line in lines:
        text_color = "#d4d4d4"
        if "====" in line or "----" in line:
            text_color = "#569cd6"
        elif "CORRECT" in line or "PASSED" in line or "MATCH" in line or "STATIC" in line:
            text_color = "#4ec9b0"
        elif "INCORRECT" in line or "MISMATCH" in line or "FAILED" in line:
            text_color = "#f44747"
        elif "Time" in line or "Speedup" in line or "Threads" in line:
            text_color = "#ce9178"
        
        draw.text((25, y), line, fill=text_color, font=font)
        y += line_height

    img.save(out_img)
    print(f"Saved terminal screenshot: {out_img}")

print("All screenshots generated successfully.")
