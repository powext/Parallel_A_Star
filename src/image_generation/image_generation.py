import json
import re

from PIL import Image, ImageDraw

data = None
with open("output/output.json", "r") as f:
    data = json.load(f)

# Parameters
grid_size = data["grid_size"]
n_chunks = data["n_chunks"]
chunk_size = grid_size // n_chunks
obstacles = list(map(int, data["obstacles"].split(",")))

# Create an image
img = Image.new('RGB', (grid_size, grid_size), color='white').convert('RGBA')
draw = ImageDraw.Draw(img)

# Draw obstacles
for obstacle in obstacles:
    row = obstacle // grid_size
    col = obstacle % grid_size
    draw.rectangle([(col, row), (col, row)], fill='grey')


# Draw exit points for each chunk
for i, chunk in enumerate(data["chunks"]):
    exit_points = zip(*(iter(list(map(int, chunk["exit_points"].split(",")))),) * 2)
    for j, exit_point in enumerate(exit_points):
        if exit_point != -1:
            draw.rectangle([(exit_point[0], exit_point[1]), (exit_point[0], exit_point[1])], fill=(255, 0, 0, 200))

# Save the image
img.save('output/game_matrix.png')
img.show()
