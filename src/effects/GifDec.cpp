#include "GifDec.h"

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

typedef struct Entry {
    uint16_t length;
    uint16_t prefix;
    uint8_t  suffix;
} Entry;

typedef struct Table {
    int bulk;
    int nentries;
    Entry *entries;
} Table;

static uint16_t
read_num(File *f)
{
    uint8_t bytes[2];

    f->read(bytes, 2);
    return bytes[0] + (((uint16_t) bytes[1]) << 8);
}

void printDir(const char* path) {
    Dir dir = LittleFS.openDir(path);
    while (dir.next()) {
        if (dir.isDirectory()) {
            String tmp = String(path) + String(dir.fileName()) + String("/");
            printDir(tmp.c_str());
        } else {
            String fullPath = String(path) + String(dir.fileName());
            Serial.printf("'%s' %d\n", fullPath.c_str(), dir.fileSize());
        }
    }
}

gd_GIF *
gd_open_gif(const char *fname)
{
    int fd;
    uint8_t sigver[3];
    uint16_t width, height, depth;
    uint8_t fdsz, bgidx, aspect;
    int i;
    int gct_sz;
    gd_GIF *gif;

    printDir("/");

    File *f = new File();
    *f = LittleFS.open(fname, "r");

    if (!*f) {
        return NULL;
    }
    /* Header */
    f->read((uint8_t*)sigver, 3);
    if (memcmp(sigver, "GIF", 3) != 0) {
        Serial.println("invalid signature");
        goto fail;
    }
    /* Version */
    f->read((uint8_t*)sigver, 3);
    if (memcmp(sigver, "89a", 3) != 0) {
        Serial.println("invalid version");
        goto fail;
    }
    /* Width x Height */
    width  = read_num(f);
    height = read_num(f);
    /* FDSZ */
    f->read((uint8_t*)&fdsz, 1);
    /* Presence of GCT */
    if (!(fdsz & 0x80)) {
        Serial.println("no global color table");
        goto fail;
    }
    /* Color Space's Depth */
    depth = ((fdsz >> 4) & 7) + 1;
    /* Ignore Sort Flag. */
    /* GCT Size */
    gct_sz = 1 << ((fdsz & 0x07) + 1);
    /* Background Color Index */
    f->read((uint8_t*)&bgidx, 1);
    /* Aspect Ratio */
    f->read((uint8_t*)&aspect, 1);
    /* Create gd_GIF Structure. */
    gif = (gd_GIF*)calloc(1, sizeof(*gif));
    if (!gif) {
        Serial.println("gif is null");
        goto fail;
    }

    gif->fd = f;

    gif->width  = width;
    gif->height = height;
    gif->depth  = depth;
    /* Read GCT */
    gif->gct.size = gct_sz;
    f->read((uint8_t*)gif->gct.colors, 3 * gif->gct.size);
    gif->palette = &gif->gct;
    gif->bgindex = bgidx;
    gif->frame = (uint8_t*)malloc(width * height);
    if (!gif->frame) {
        free(gif);
        Serial.println("gif->frame is null");
        goto fail;
    }
    if (gif->bgindex)
        memset(gif->frame, gif->bgindex, gif->width * gif->height);
    gif->anim_start = f->position();
    goto ok;
fail:
    Serial.println("FAIL");
    f->close();
    return 0;
ok:
    return gif;
}

static void
discard_sub_blocks(gd_GIF *gif)
{
    uint8_t size;

    do {
        gif->fd->read((uint8_t*)&size, 1);
        gif->fd->seek(size, SeekCur);
    } while (size);
}

static void
read_plain_text_ext(gd_GIF *gif)
{
    if (gif->plain_text) {
        uint16_t tx, ty, tw, th;
        uint8_t cw, ch, fg, bg;
        off_t sub_block;
        gif->fd->seek(1, SeekCur); /* block size = 12 */
        tx = read_num(gif->fd);
        ty = read_num(gif->fd);
        tw = read_num(gif->fd);
        th = read_num(gif->fd);
        gif->fd->read((uint8_t*)&cw, 1);
        gif->fd->read((uint8_t*)&ch, 1);
        gif->fd->read((uint8_t*)&fg, 1);
        gif->fd->read((uint8_t*)&bg, 1);
        sub_block = gif->fd->position();
        gif->plain_text(gif, tx, ty, tw, th, cw, ch, fg, bg);
        gif->fd->seek(sub_block, SeekSet);
    } else {
        /* Discard plain text metadata. */
        gif->fd->seek(13, SeekCur);
    }
    /* Discard plain text sub-blocks. */
    discard_sub_blocks(gif);
}

static void
read_graphic_control_ext(gd_GIF *gif)
{
    uint8_t rdit;

    /* Discard block size (always 0x04). */
    gif->fd->seek(1, SeekCur);
    gif->fd->read((uint8_t*)&rdit, 1);
    gif->gce.disposal = (rdit >> 2) & 3;
    gif->gce.input = rdit & 2;
    gif->gce.transparency = rdit & 1;
    gif->gce.delay = read_num(gif->fd);
    gif->fd->read((uint8_t*)&gif->gce.tindex, 1);
    /* Skip block terminator. */
    gif->fd->seek(1, SeekCur);
}

static void
read_comment_ext(gd_GIF *gif)
{
    if (gif->comment) {
        off_t sub_block = gif->fd->position();
        gif->comment(gif);
        gif->fd->seek(sub_block, SeekSet);
    }
    /* Discard comment sub-blocks. */
    discard_sub_blocks(gif);
}

static void
read_application_ext(gd_GIF *gif)
{
    char app_id[8];
    char app_auth_code[3];

    /* Discard block size (always 0x0B). */
    gif->fd->seek(1, SeekCur);
    /* Application Identifier. */
    gif->fd->read((uint8_t*)app_id, 8);
    /* Application Authentication Code. */
    gif->fd->read((uint8_t*)app_auth_code, 3);

    if (!strncmp(app_id, "NETSCAPE", sizeof(app_id))) {
        /* Discard block size (0x03) and constant byte (0x01). */
        gif->fd->seek(2, SeekCur);
        gif->loop_count = read_num(gif->fd);
        /* Skip block terminator. */
        gif->fd->seek(1, SeekCur);
    } else if (gif->application) {
        off_t sub_block = gif->fd->position();
        gif->application(gif, app_id, app_auth_code);
        gif->fd->seek(sub_block, SeekSet);
        discard_sub_blocks(gif);
    } else {
        discard_sub_blocks(gif);
    }
}

static void
read_ext(gd_GIF *gif)
{
    uint8_t label;

    gif->fd->read(&label, 1);
    switch (label) {
    case 0x01:
        read_plain_text_ext(gif);
        break;
    case 0xF9:
        read_graphic_control_ext(gif);
        break;
    case 0xFE:
        read_comment_ext(gif);
        break;
    case 0xFF:
        read_application_ext(gif);
        break;
    default:
        Serial.print("unknown extension: ");
        Serial.println(label);
    }
}

static Table *
new_table(int key_size)
{
    int key;
    int init_bulk = MAX(1 << (key_size + 1), 0x100);
    Table *table = (Table*)malloc(sizeof(*table) + sizeof(Entry) * init_bulk);
    if (table) {
        table->bulk = init_bulk;
        table->nentries = (1 << key_size) + 2;
        table->entries = (Entry *) &table[1];
        for (key = 0; key < (1 << key_size); key++)
            table->entries[key] = (Entry) {1, 0xFFF, key};
    }
    return table;
}

/* Add table entry. Return value:
 *  0 on success
 *  +1 if key size must be incremented after this addition
 *  -1 if could not realloc table */
static int
add_entry(Table **tablep, uint16_t length, uint16_t prefix, uint8_t suffix)
{
    Table *table = *tablep;
    if (table->nentries == table->bulk) {
        table->bulk *= 2;
        table = (Table*)realloc(table, sizeof(*table) + sizeof(Entry) * table->bulk);
        if (!table) return -1;
        table->entries = (Entry *) &table[1];
        *tablep = table;
    }
    table->entries[table->nentries] = (Entry) {length, prefix, suffix};
    table->nentries++;
    if ((table->nentries & (table->nentries - 1)) == 0)
        return 1;
    return 0;
}

static uint16_t
get_key(gd_GIF *gif, int key_size, uint8_t *sub_len, uint8_t *shift, uint8_t *byte)
{
    int bits_read;
    int rpad;
    int frag_size;
    uint16_t key;

    key = 0;
    for (bits_read = 0; bits_read < key_size; bits_read += frag_size) {
        rpad = (*shift + bits_read) % 8;
        if (rpad == 0) {
            /* Update byte. */
            if (*sub_len == 0) {
                gif->fd->read((uint8_t*)sub_len, 1); /* Must be nonzero! */
                if (*sub_len == 0)
                    return 0x1000;
            }
            gif->fd->read((uint8_t*)byte, 1);
            (*sub_len)--;
        }
        frag_size = MIN(key_size - bits_read, 8 - rpad);
        key |= ((uint16_t) ((*byte) >> rpad)) << bits_read;
    }
    /* Clear extra bits to the left. */
    key &= (1 << key_size) - 1;
    *shift = (*shift + key_size) % 8;
    return key;
}

/* Compute output index of y-th input line, in frame of height h. */
static int
interlaced_line_index(int h, int y)
{
    int p; /* number of lines in current pass */

    p = (h - 1) / 8 + 1;
    if (y < p) /* pass 1 */
        return y * 8;
    y -= p;
    p = (h - 5) / 8 + 1;
    if (y < p) /* pass 2 */
        return y * 8 + 4;
    y -= p;
    p = (h - 3) / 4 + 1;
    if (y < p) /* pass 3 */
        return y * 4 + 2;
    y -= p;
    /* pass 4 */
    return y * 2 + 1;
}

/* Decompress image pixels.
 * Return 0 on success or -1 on out-of-memory (w.r.t. LZW code table). */
static int
read_image_data(gd_GIF *gif, int interlace)
{
    uint8_t sub_len, shift, byte;
    int init_key_size, key_size, table_is_full;
    int frm_off, frm_size, str_len, i, p, x, y;
    uint16_t key, clear, stop;
    int ret;
    Table *table;
    Entry entry;
    off_t start, end;

    gif->fd->read((uint8_t*)&byte, 1);
    key_size = (int) byte;
    if (key_size < 2 || key_size > 8)
        return -1;
    
    start = gif->fd->position();
    discard_sub_blocks(gif);
    end = gif->fd->position();
    gif->fd->seek(start, SeekSet);
    clear = 1 << key_size;
    stop = clear + 1;
    table = new_table(key_size);
    key_size++;
    init_key_size = key_size;
    sub_len = shift = 0;
    key = get_key(gif, key_size, &sub_len, &shift, &byte); /* clear code */
    frm_off = 0;
    ret = 0;
    frm_size = gif->fw*gif->fh;
    while (frm_off < frm_size) {
        if (key == clear) {
            key_size = init_key_size;
            table->nentries = (1 << (key_size - 1)) + 2;
            table_is_full = 0;
        } else if (!table_is_full) {
            ret = add_entry(&table, str_len + 1, key, entry.suffix);
            if (ret == -1) {
                free(table);
                return -1;
            }
            if (table->nentries == 0x1000) {
                ret = 0;
                table_is_full = 1;
            }
        }
        key = get_key(gif, key_size, &sub_len, &shift, &byte);
        if (key == clear) continue;
        if (key == stop || key == 0x1000) break;
        if (ret == 1) key_size++;
        entry = table->entries[key];
        str_len = entry.length;
        for (i = 0; i < str_len; i++) {
            p = frm_off + entry.length - 1;
            x = p % gif->fw;
            y = p / gif->fw;
            if (interlace)
                y = interlaced_line_index((int) gif->fh, y);
            gif->frame[(gif->fy + y) * gif->width + gif->fx + x] = entry.suffix;
            if (entry.prefix == 0xFFF)
                break;
            else
                entry = table->entries[entry.prefix];
        }
        frm_off += str_len;
        if (key < table->nentries - 1 && !table_is_full)
            table->entries[table->nentries - 1].suffix = entry.suffix;
    }
    free(table);
    if (key == stop)
        gif->fd->read((uint8_t*)&sub_len, 1); /* Must be zero! */
    gif->fd->seek(end, SeekSet);
    return 0;
}

/* Read image.
 * Return 0 on success or -1 on out-of-memory (w.r.t. LZW code table). */
static int
read_image(gd_GIF *gif)
{
    uint8_t fisrz;
    int interlace;

    /* Image Descriptor. */
    gif->fx = read_num(gif->fd);
    gif->fy = read_num(gif->fd);
    
    if (gif->fx >= gif->width || gif->fy >= gif->height)
        return -1;
    
    gif->fw = read_num(gif->fd);
    gif->fh = read_num(gif->fd);
    
    gif->fw = MIN(gif->fw, gif->width - gif->fx);
    gif->fh = MIN(gif->fh, gif->height - gif->fy);
    
    gif->fd->read((uint8_t*)&fisrz, 1);
    interlace = fisrz & 0x40;
    /* Ignore Sort Flag. */
    /* Local Color Table? */
    if (fisrz & 0x80) {
        /* Read LCT */
        gif->lct.size = 1 << ((fisrz & 0x07) + 1);
        gif->fd->read((uint8_t*)gif->lct.colors, 3 * gif->lct.size);
        gif->palette = &gif->lct;
    } else
        gif->palette = &gif->gct;
    /* Image Data. */
    return read_image_data(gif, interlace);
}

/* Return 1 if got a frame; 0 if got GIF trailer; -1 if error. */
int
gd_get_frame(gd_GIF *gif)
{
    char sep;

    gif->fd->read((uint8_t*)&sep, 1);
    
    while (sep != ',') {
        if (sep == ';') {
            return 0;
        }
        if (sep == '!') {
            read_ext(gif);
        }
        else {
            return -1;
        }
        gif->fd->read((uint8_t*)&sep, 1);
    }
    if (read_image(gif) == -1)
        return -1;

    return 1;
}

void gd_get_color_at(gd_GIF *gif, int x, int y, uint8_t *buf) {
    if (x >= gif->fx && x < gif->fx + gif->fw && y >= gif->fy && y < gif->fy + gif->fh) {
        // point is inside frame
        uint8_t index, *color;
        index = gif->frame[y * gif->width + x];
        color = &gif->palette->colors[index * 3];
        if (!gif->gce.transparency || index != gif->gce.tindex) {
            memcpy(buf, color, 3);
        } else {
            // TODO: ...
            // for now just do the same
            memcpy(buf, color, 3);
        }
    } else {
        // point is outside frame
        uint8_t *bgcolor = &gif->palette->colors[gif->bgindex * 3];
        memcpy(buf, bgcolor, 3);
    }
}

int
gd_is_bgcolor(gd_GIF *gif, uint8_t color[3])
{
    return !memcmp(&gif->palette->colors[gif->bgindex*3], color, 3);
}

void
gd_rewind(gd_GIF *gif)
{
    gif->fd->seek(gif->anim_start, SeekSet);
}

void
gd_close_gif(gd_GIF *gif)
{
    gif->fd->close();
    free(gif->frame);    
    free(gif);
}