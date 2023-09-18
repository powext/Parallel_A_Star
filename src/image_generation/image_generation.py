import json
import random
import math

from PIL import Image, ImageDraw


def generate_random_color():
    return (random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))


def color_distance(color1, color2):
    r_diff = color1[0] - color2[0]
    g_diff = color1[1] - color2[1]
    b_diff = color1[2] - color2[2]
    return math.sqrt(r_diff**2 + g_diff**2 + b_diff**2)


def blend_with_background(color, opacity, background=(255, 255, 255)):
    return tuple(
        int(color[i] * opacity + background[i] * (1 - opacity))
        for i in range(3)
    )


def get_distinguishable_color(fixed_colors, threshold=100, opacity=0.5):
    while True:
        random_color = generate_random_color()
        if all(color_distance(random_color, fixed_color) > threshold for fixed_color in fixed_colors):
            return blend_with_background(random_color, opacity)


data = None
with open("output/output.json", "r") as f:
    data = json.load(f)

# Parameters
grid_size = data["grid_size"]
n_chunks = data["n_chunks"]
chunk_size = grid_size // n_chunks
obstacles = []
if data["obstacles"]:
    obstacles = list(map(int, data["obstacles"].split(",")))

# Create an image
img = Image.new('RGB', (grid_size, grid_size), color='white').convert('RGBA')
draw = ImageDraw.Draw(img)

fixed_colors = [(0, 255, 0), (255, 0, 0), (255, 255, 255), (128, 128, 128)]

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


# Draw starting and destination points
def get_coordinates(point_str):
    x, y = map(int, point_str.split(","))
    return (x, y)

starting_point = get_coordinates(data["starting_point"])
destination_point = get_coordinates(data["destination_point"])

draw.rectangle([starting_point, starting_point], fill=(0, 255, 0, 200))
draw.rectangle([destination_point, destination_point], fill=(0, 0, 255, 200))


for i, chunk in enumerate(data["chunks"]):
    new_color = get_distinguishable_color(fixed_colors, opacity=0.2)
    for j, path in enumerate(chunk["paths"]):
        path_points = zip(*(iter(list(map(int, path.split(",")))),) * 2)
        for k, path_point in enumerate(path_points):
            if path_point != -1:
                draw.rectangle([(path_point[0], path_point[1]), (path_point[0], path_point[1])], fill=new_color)


if (data["final_path"]):
    exit_points = zip(*(iter(list(map(int, data["final_path"].split(",")))),) * 2)
    for j, exit_point in enumerate(exit_points):
        if exit_point != -1:
            draw.rectangle([(exit_point[0], exit_point[1]), (exit_point[0], exit_point[1])], fill=(50, 205, 50))

# Save the image
img.save('output/game_matrix.png')
