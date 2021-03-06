#include <TFT.h>

Framebuffer::Framebuffer() {
    _width = _height = _masterWidth = _masterHeight = 0;
    sprites = NULL;
    buffer = NULL;
    setColor(0, Color::Black);
    setColor(1, Color::DarkBlue);
    setColor(2, Color::DarkRed);
    setColor(3, Color::DarkMagenta);
    setColor(4, Color::DarkGreen);
    setColor(5, Color::DarkCyan);
    setColor(6, Color::Orange);
    setColor(7, Color::Gray75);
    setColor(8, Color::Gray50);
    setColor(9, Color::Blue);
    setColor(10, Color::Red);
    setColor(11, Color::Magenta);
    setColor(12, Color::Green);
    setColor(13, Color::Cyan);
    setColor(14, Color::Yellow);
    setColor(15, Color::White);

    _maxX = _maxY = 0;
    _minX = _width;
    _minY = _height;
}

Framebuffer::Framebuffer(int16_t w, int16_t h, DataStore *b) {
    _width = w;
    _height = h;
    _masterWidth = w;
    _masterHeight = h;
    sprites = NULL;
    buffer = b;
    setColor(0, Color::Black);
    setColor(1, Color::DarkBlue);
    setColor(2, Color::DarkRed);
    setColor(3, Color::DarkMagenta);
    setColor(4, Color::DarkGreen);
    setColor(5, Color::DarkCyan);
    setColor(6, Color::Orange);
    setColor(7, Color::Gray75);
    setColor(8, Color::Gray50);
    setColor(9, Color::Blue);
    setColor(10, Color::Red);
    setColor(11, Color::Magenta);
    setColor(12, Color::Green);
    setColor(13, Color::Cyan);
    setColor(14, Color::Yellow);
    setColor(15, Color::White);
}

void Framebuffer::initializeDevice() {
    sprites = NULL;
	buffer->initializeDevice();
}

void Framebuffer::translateCoordinates(int16_t *x, int16_t *y) {
    int16_t t;
    switch (rotation) {
        case 1:
            t = *x;
            *x = _width - *y;
            *y = t;
            break;
        case 2:
            *x = _width - *x;
            *y = _height - *y;
            break;
        case 3:
            t = *x;
            *x = *y;
            *y = _height - t;
            break;
    }
}

void Framebuffer::setPixel(int16_t x, int16_t y, uint16_t color) {
    translateCoordinates(&x, &y);

    if (x < 0 || x >= _width || y < 0 || y >= _height) {
        return;
    }
    
    uint8_t pcol = color & 0xFF;
    uint32_t pos = y * _width + x;
    bufferWrite(pos, pcol);

    if (x < _minX) _minX = x;
    if (x > _maxX) _maxX = x;
    if (y < _minY) _minY = y;
    if (y > _maxY) _maxY = y;
}

void Framebuffer::fillScreen(uint16_t color) {
    _minX = _minY = 0;
    _maxX = _width-1;
    _maxY = _height-1;
    buffer->setAll8(color);
}

void Framebuffer::drawVerticalLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    int16_t w = 1;
    if (!clipToScreen(x, y, w, h)) {
        return;
    }
    for (int y1 = y; y1 < y + h; y1++) {
        uint8_t pcol = color & 0xFF;
        uint32_t pos = y1 * _width + x;
        bufferWrite(pos, pcol);
    }
}

void Framebuffer::drawHorizontalLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    int16_t h = 1;
    if (!clipToScreen(x, y, w, h)) {
        return;
    }
    for (int x1 = x; x1 < x + w; x1++) {
        uint8_t pcol = color & 0xFF;
        uint32_t pos = y * _width + x1;
        bufferWrite(pos, pcol);
    }
}

void Framebuffer::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (!clipToScreen(x, y, w, h)) {
        return;
    }
    for (int y1 = y; y1 < y + h; y1++) {
        for (int x1 = x; x1 < x + w; x1++) {
            setPixel(x1, y1, color);
        }
    }
}

void Framebuffer::setColor(uint8_t color, uint16_t rgb) {
    palette[color] = rgb;
}

void Framebuffer::setColor(uint8_t color, uint8_t r, uint8_t g, uint8_t b) {
    palette[color] = color565(r, g, b);
}

void Framebuffer::loadPalette(const uint16_t *p) {
    for (int i = 0; i < 256; i++) {
        setColor(i, p[i]);
    }
}

void Framebuffer::loadPalette(const uint8_t p[256][3]) {
    for (int i = 0; i < 256; i++) {
        setColor(i, p[i][0], p[i][1], p[i][2]);
    }
}

void Framebuffer::loadPalette(Framebuffer *fb) {
    memcpy(palette, fb->palette, 256);
}

struct sprite * Framebuffer::addSprite(const uint8_t *data, uint16_t w, uint16_t h, uint8_t t, uint8_t f) {
    struct sprite *scan;
    struct sprite *s = (struct sprite *)malloc(sizeof(struct sprite));
    if (!s) {
        return NULL;
    }
    s->data = data;
    s->width = w;
    s->height = h;
    s->xpos = _width+1;
    s->ypos = _height+1;
    s->transparent = t;
    s->frames = f;
    s->currentframe = 0;
    s->animdir = 1;
    s->next = NULL;
    if (sprites == NULL) {
        sprites = s;
        return s;
    }
    for (scan = sprites; scan->next; scan = scan->next);
    scan->next = s;
    return s;
}

void Framebuffer::removeSprite(struct sprite *s) {
    struct sprite *scan;
    struct sprite *last = NULL;

    if (sprites == s) {
        scan = s;
        sprites = sprites->next;
        free(s);
        return;
    }

    for (scan = sprites; scan; scan = scan->next) {
        if (scan == s) {
            if (last) {
                last->next = scan->next;
                free(s);
                return;
            }
        }
        last = scan;
    }
}

struct sprite *Framebuffer::spriteAt(int16_t x, int16_t y) {
    struct sprite *scan;
    if (sprites == NULL) {
        return NULL;
    }
    for (scan = sprites; scan; scan = scan->next) {
        if (scan->xpos <= x && scan->ypos <= y && (scan->xpos + scan->width) > x && (scan->ypos + scan->height) > y) {
            uint32_t offset = scan->width * scan->height * scan->currentframe;
            uint8_t color = scan->data[offset + (y - scan->ypos) * scan->width + (x - scan->xpos)];
            if (color != scan->transparent) {
                return scan;
            }
        }
    }
    return NULL;
}

uint16_t Framebuffer::colorAt(int16_t x, int16_t y) {
    if (x < 0 || y < 0 || x >= _width || y >= _height) {
        return 0;
    }
    struct sprite *s = spriteAt(x, y);
    if (s) {
        uint32_t offset = s->width * s->height * s->currentframe;
        uint8_t color = s->data[offset + (y - s->ypos) * s->width + (x - s->xpos)];
        return palette[color];
    }

    uint32_t pos = y * _width + x;
    return palette[bufferRead(pos)];
}

uint16_t Framebuffer::bgColorAt(int16_t x, int16_t y) {
    if (x < 0 || y < 0 || x >= _width || y >= _height) {
        return 0;
    }
    uint32_t pos = y * _width + x;
    return palette[bufferRead(pos)];
}

void Framebuffer::getScanLine(uint16_t y, uint16_t *data) {
    getScanLine(y, 0, getWidth(), data);
}

void Framebuffer::getScanLine(uint16_t y, uint16_t x, uint16_t w, uint16_t *data) {
	uint8_t bufferdata[w];
	buffer->read8(y * getWidth() + x, bufferdata, w);
	for (uint16_t px = 0; px < w; px++) {
        if (sprites != NULL) {
            struct sprite *s = spriteAt(x + px, y);
            if (s) {
                uint32_t offset = s->width * s->height * s->currentframe;
                uint8_t color = s->data[offset + (y - s->ypos) * s->width + (px - s->xpos)];
                data[px] = palette[color];
            } else {
                data[px] = palette[bufferdata[px]];
            }
        } else {
            data[px] = palette[bufferdata[px]];
        }
	}
}

void Framebuffer::animate(struct sprite *s) {
    s->xpos += s->dx;
    s->ypos += s->dy;
    s->currentframe++;
    if (s->currentframe >= s->frames) {
        s->currentframe = 0;
    }
}

void Framebuffer::animatePingPong(struct sprite *s) {
    s->xpos += s->dx;
    s->ypos += s->dy;
    s->currentframe += s->animdir;
    if (s->currentframe >= s->frames) {
        s->animdir = -1;
        s->currentframe = s->frames-1;
    }
    if (s->currentframe < 0) {
        s->animdir = 1;
        s->currentframe = 0;
    }
}

struct sprite *Framebuffer::collidesWith(struct sprite *s) {
    struct sprite *scan;
    int16_t sl = s->xpos;
    int16_t st = s->ypos;
    int16_t sr = s->xpos + s->width;
    int16_t sb = s->ypos + s->height;
    for (scan = sprites; scan; scan = scan->next) {
        if (scan == s) continue;
        int16_t dl= scan->xpos;
        int16_t dt = scan->ypos;
        int16_t dr = scan->xpos + scan->width;
        int16_t db = scan->ypos + scan->height;

        if (sl <= dr && sr >= dl && st <= db && sb >= dt) {
            return scan;
        }
    }
    return NULL;
}

void Framebuffer::moveTo(struct sprite *s, int16_t x, int16_t y) {
    s->xpos = x;
    s->ypos = y;
}

void Framebuffer::moveBy(struct sprite *s, int16_t dx, int16_t dy) {
    s->xpos += dx;
    s->ypos += dy;
}

struct sprite *Framebuffer::firstSprite() {
    selectedSprite = sprites;
    return selectedSprite;
}

struct sprite *Framebuffer::nextSprite() {
    if (selectedSprite == NULL) {
        return NULL;
    }
    selectedSprite = selectedSprite->next;
    return selectedSprite;
}

int8_t Framebuffer::getSprite(struct sprite *s, uint8_t n) {
    n = n & 0x07;
    return s->store[n];
}

void Framebuffer::setSprite(struct sprite *s, uint8_t n, int8_t v) {
    n = n & 0x07;
    s->store[n] = v;
}

void Framebuffer::copyRect(int16_t dx, int16_t dy, int16_t sx, int16_t sy, uint16_t w, uint16_t h) {
    uint32_t dpos;
    uint32_t spos;
    translateCoordinates(&dx, &dy);
    for (int y = 0; y < w; y++) {
        for (int x = 0; x < h; x++) {
            if (sx > dx && sy > dy) {
                dpos = (y + dy) * _width + (x + dx);
                spos = (y + sy) * _width + (x + sx);
            } else if(sx <= dx && sy > dy) {
                dpos = (y + dy) * _width + ((w - x) + dx);
                spos = (y + sy) * _width + ((w - x) + sx);
            } else if(sx > dx && sy <= dy) {
                dpos = ((h - y) + dy) * _width + (x + dx);
                spos = ((h - y) + sy) * _width + (x + sx);
            } else if(sx <= dx && sy <= dy) {
                dpos = ((h - y) + dy) * _width + ((w - x) + dx);
                spos = ((h - y) + sy) * _width + ((w - x) + sx);
            }
            bufferWrite(dpos, bufferRead(spos));
        }
    }
}

uint8_t Framebuffer::bufferRead(uint32_t addr) {
    return buffer->read8(addr);
}

void Framebuffer::bufferWrite(uint32_t addr, uint8_t value) {
    buffer->write8(addr, value);
}

void Framebuffer::setRotation(uint8_t rot) {
    rotation = rot % 4;
}

uint16_t Framebuffer::getWidth() {
    if (rotation == 0 || rotation == 2) {
        return _width;
    }
    return _height;
}

uint16_t Framebuffer::getHeight() {
    if (rotation == 0 || rotation == 2) {
        return _height;
    }
    return _width;
}

uint8_t Framebuffer::getClosestColor(uint16_t c) {
    uint32_t cdiff = 0xFFFFFFFF;

    uint8_t best;
    for (uint16_t x = 0; x < 256; x++) {
        uint32_t d = deltaOrth(palette[x], c);
        if (d < cdiff) {
            cdiff = d;
            best = x;
        }
    }
    return best;
}

void Framebuffer::scroll(int16_t dx, int16_t dy) {
    uint16_t w = getWidth();
    uint16_t h = getHeight();

    uint16_t ax = abs(dx);
    uint16_t ay = abs(dy);

    if (ay == 0) {
        for (uint16_t y = 0; y < h; y++) {
            if (dx < 0) {
                for (uint16_t x = 0; x < w - ax; x++) {
                    uint16_t col = bgColorAt(x + ax, y);
                    setPixel(x, y, col);
                }
            } else {
                for (uint16_t x = 0; x < w - ax; x++) {
                    uint16_t col = bgColorAt(w - (x + ax), y);
                    setPixel(w - x, y, col);
                }
            }
        }
    }
}

void Framebuffer::update(TFT *tft) {
    update(tft, 0, 0);
}

void Framebuffer::update(TFT *tft, int16_t x0, int16_t y0) {
    tft->openWindow(x0, y0, _width, _height);
    uint16_t buf[_width];
    for (uint32_t y = 0; y < _height; y++) {
        uint32_t line = y * _width;
        for (uint32_t x = 0; x < _width; x++) {
            buf[x] = palette[buffer->read8(line + x)];
        }
        tft->windowData(buf, _width);
    }
    tft->closeWindow();
}

void Framebuffer::setDirection(struct sprite *spr, int16_t dx, int16_t dy) {
    spr->dx = dx;
    spr->dy = dy;
}
    

