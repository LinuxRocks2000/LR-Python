import lrpython

tileset = [
    [0, 0, 500, 50]
]

x = 0
y = 0
xv = 0
yv = 0

def begin():
    print("Started Successfully")

def run():
    global x
    global y
    global xv
    global yv
    x += xv
    y += yv
    lrpython.draw_filled_rectangle(x, y, 100, 100, 0, 0, 0);
    for i in tileset:
        lrpython.draw_stroke_rectangle(x + i[0], y + i[1], i[2] - i[0], i[3] - i[1], 10, 255, 0, 0);

def keydown_Right():
    global xv
    xv += 1

def keydown_Up():
    global yv
    yv -= 1

def keydown_Down():
    global yv
    yv += 1

def keydown_Left():
    global xv
    xv -= 1
