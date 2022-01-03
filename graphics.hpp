// All of this will be workified by autogen.py

void draw_pixel(uint16_t xPos, uint16_t yPos, uint8_t R, uint8_t G, uint8_t B){
    xPos %= VIDEO_WIDTH; // Make the screen borders scroll
    yPos %= VIDEO_HEIGHT; // Ditto
    unsigned long pixelPos = (yPos * VIDEO_WIDTH + xPos) * 4;
    frame_buf[pixelPos] = B;
    frame_buf[pixelPos + 1] = G;
    frame_buf[pixelPos + 2] = R;
}

void draw_filled_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t R, uint8_t G, uint8_t B){
    for (unsigned int i = x; i < x + w; i ++){
        for (unsigned int j = y; j < y + h; j ++){
            draw_pixel(i, j, R, G, B);
        }
    }
}

void draw_stroke_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t sw, uint8_t R, uint8_t G, uint8_t B){
    for (unsigned int i = x; i < x + w; i ++){
        for (unsigned int j = y; j < y + h; j ++){
            if (j < y + sw || i < x + sw || j >= y + h - sw || i >= x + w - sw){ // XORs mean that it works better
                draw_pixel(i, j, R, G, B);
            }
        }
    }
}

void draw_filled_circle(uint16_t cx, uint16_t cy, uint16_t r, uint8_t R, uint8_t G, uint8_t B){
    for(int y = -r; y <= r; y++){
        for(int x = -r; x <= r; x++){
            if(x * x + y * y <= r * r){
                draw_pixel(cx + x, cy + y, R, G, B);
            }
        }
    }
}

void draw_stroke_circle(uint16_t cx, uint16_t cy, uint16_t r, uint8_t sw, uint8_t R, uint8_t G, uint8_t B){
    for (int y = -r; y <= r; y++){
        for (int x = -r; x <= r; x++){
            int pythagoreanDist = x * x + y * y;
            int pythagoreanOuterRadius = r * r;
            int pythagoreanInnerRadius = (r - sw) * (r - sw);
            if (pythagoreanDist <= pythagoreanOuterRadius && pythagoreanDist >= pythagoreanInnerRadius){
                draw_pixel(cx + x, cy + y, R, G, B);
            }
        }
    }
}
