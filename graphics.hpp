// All of this will be workified by autogen.py

#define NA_uint8_t uint8_t

void draw_pixel(uint16_t xPos, uint16_t yPos, uint8_t R, uint8_t G, uint8_t B, NA_uint8_t *framebuffer = frame_buf, unsigned int videowidth = VIDEO_WIDTH, unsigned int videoheight = VIDEO_HEIGHT){
    xPos %= videowidth; // Make the screen borders scroll
    yPos %= videoheight; // Ditto
    unsigned long pixelPos = (yPos * VIDEO_WIDTH + xPos) * 4;
    framebuffer[pixelPos] = B;
    framebuffer[pixelPos + 1] = G;
    framebuffer[pixelPos + 2] = R;
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

notAutogen draw_fb(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t* fb){
    for (uint32_t i = 0; i < w * h; i ++){
        unsigned int ofX = (i - i % w) / w;
        unsigned int ofY = (i % w);
        ((uint32_t *)frame_buf)[((y + ofY) * VIDEO_HEIGHT + x + ofX)] = fb[i];
    }
}

void draw_letter(uint16_t x, uint16_t y, uint8_t fontsize, char* letter, uint8_t R, uint8_t G, uint8_t B){
    // Because of python ishyness, we know that it will have to be a char* no matter what.
    uint8_t *frame = (uint8_t*)malloc(LETTER_WIDTH * LETTER_HEIGHT * sizeof(uint32_t));
    draw_fb(0, 0, LETTER_WIDTH, LETTER_HEIGHT, frame);
    switch (letter[0]){
        case '0':
            for (uint8_t i = 0; i < LETTER_WIDTH; i ++){
                for (uint8_t j = 0; j < LETTER_HEIGHT; j ++){
                    draw_pixel(x + i, y + j, R, G, B, frame);
                }
            }
            break;
    }
}
