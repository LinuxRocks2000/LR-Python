import lrpython

tileset = [
    [0, 0, 500, 50]
]

x = 0
y = 0

def begin():
    print("Started Successfully")

def run():
    y += 0.01
    for i in tileset:
        lrpython.draw_stroke_rectangle(x + i[0], y + i[1], i[2], i[3], 10, 255, 0, 0);
