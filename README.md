# LR-Python

A simplistic LibRetro core designed to "emulate" python scripts. Adding and not subtracting features, you have access to the entire system from the python- there is no sandboxing. Thus, anything is possible!

The only thing it does is give LibRetro support. Use `import lrpython` (not on PyPi, it magically appears when you load a Python ROM) and define `run()` (more magic) - you have a script! `run` runs at about 60 fps, hopefully. The interface is similar to Processing.js, as in you can use `lrpython.draw_filled_rectangle(x, y, width, height, color_r, color_g, color_b)` to draw a rectangle. Example: `lrpython.draw_filled_rectangle(100, 100, 200, 50, 255, 0, 0)` draws a red 200x50 rectangle at (100, 100).

## Why

I write this because I really want to develop games for my RetroPie setup.