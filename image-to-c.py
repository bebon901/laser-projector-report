from PIL import Image
import numpy as np

IMG_ROWS = 1000
IMG_COLS = 1000

img = Image.open("parrot.jpg")

img = img.resize((IMG_COLS, IMG_ROWS))
img = img.convert("RGB")
data = np.array(img)
print(data.shape)
# 4. Scale values to 4-bit (0-15) for your DAC
# (value / 255) * 15


for i in [1, 2, 3, 4, 6, 8, 16]:
    data_4bit = (data / (np.max(data)) * (i**2 - 1)).astype(int)
    imgout = Image.fromarray((data_4bit / (i**2 - 1) * 255).astype(np.uint8))
    imgout.save(f"parrot_{i}bit.jpg")

def cap(colorval):
    if colorval > 15:
        return 15
    elif colorval < 0:
        return 0
    else:
        return colorval

def scale_value(value, old_min=0, old_max=16, new_min=5, new_max=9):
    return new_min + ((value - old_min) * (new_max - new_min)) / (old_max - old_min)

## Green Channel Scaling!

for x in range(IMG_COLS):
    for y in range(IMG_ROWS):
        data_4bit[y][x][1] = scale_value(data_4bit[y][x][1])


imgout = Image.fromarray((data_4bit / 15 * 255).astype(np.uint8))
imgout.save("output_4bit-G_ADJUSTED.jpg")


output = ""

output += "#define IMG_ROWS " + str(IMG_ROWS) + "\n" + "#define IMG_COLS " + str(IMG_COLS) + "\n"
output += """int image_data[IMG_ROWS][IMG_COLS] = {\n"""

for i in range(IMG_ROWS):
    output += "    {"
    for j in range(IMG_COLS):
        ## Need to map each RGB to binary outputs. Dealing with R_B at the moment
        output += "0b" + str(f"{data_4bit[i][j][0]:04b}") + str(f"{data_4bit[i][j][1]:04b}")  + str(f"{data_4bit[i][j][2]:04b}") + ", "
    output = output[:-2]
    output += "},\n"

output = output[:-2]
output += "\n};"


print(output)
f = open("image_to_c.txt", "w+")
f.write(output)
f.close()