#include "common.h"


#include <ft2build.h>
#include FT_FREETYPE_H



/*
 * TODO : For some reason fonts were rendering black and red, I fixed it by adding an
 *        unneccesary complexity to the base shader. You might wanna fix this in the future.
 */



/* Create and Free Fonts */

Font* SBRE_create_font(const char* font_path, uint32_t font_size, uint32_t filter) {

    /* Initialize Freetype and create a font face from given ttf file */

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {

        printf("[Freetype ERROR][Freetype could not be initialized.]\n");
        return NULL;
    }


    FT_Face face;
    if(FT_New_Face(ft, font_path, 0, &face)) {

        printf("[Freetype ERROR][Failed to load font \"%s\".]\n", font_path);
        return NULL;
    }
    FT_Set_Pixel_Sizes(face, 0, font_size);


    /* Create an empty texture */

    int width = face->bbox.xMax;
    int height = face->bbox.yMax;
    uint32_t texture_id;

    unsigned char* local_buffer = (unsigned char*) calloc(width * height, sizeof(unsigned char));

    glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, local_buffer);
	glBindTexture(GL_TEXTURE_2D, 0);
    free(local_buffer);
	

	Texture* t = (Texture*) malloc(sizeof(Texture));
	*t = (Texture){
		
		.texture_id = texture_id,
		.color = SBRE_COLOR(255, 255, 255, 255),
		.initial_width  = width,
		.initial_height = height,
		.width  = width,
		.height = height,
		.bpp = 0
	};


    /* Offset */

    int offset_x = 0;
    int offset_y = 0;
    int offset_y_increase = 0;
    int biggest_width  = 0;
    int biggest_height = 0;


    /* Find a size which will fit all glyphs */

    for (unsigned char c = 0; c < 128; ++c) {
        
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            continue;
        
        if (face->glyph->bitmap.width > biggest_width)
            biggest_width = face->glyph->bitmap.width;
        if (face->glyph->bitmap.rows > biggest_height)
            biggest_height = face->glyph->bitmap.rows;
    }


    /* Create Font */

    Font* f = (Font*) malloc(sizeof(Font));
    *f = (Font){

        .font_size = font_size,
        .font_atlas = t,
        .biggest_char = SBRE_VEC2(biggest_width, biggest_height),
        ._characters = (SBRE_Character*) calloc(128, sizeof(SBRE_Character)),
    };


    /* Create font_atlas and create each character */

    glBindTexture(GL_TEXTURE_2D, t->texture_id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


    for (unsigned char c = 0; c < 128; ++c) {

        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {

            printf("[Freetype ERROR][Failed to load glyph : %c]\n", c);
            continue;
        }


        /* Flip the glyph vertically and save it to atlas */

        unsigned char* data  = face->glyph->bitmap.buffer;
        uint32_t buffer_size = face->glyph->bitmap.width * face->glyph->bitmap.rows;

        unsigned char* flipped_image = (unsigned char*) calloc(buffer_size, sizeof(unsigned char));

        for (int y = 0; y < face->glyph->bitmap.rows; ++y) {

            for (int x = 0; x < face->glyph->bitmap.width; ++x) {

                uint32_t initial_pos = (y * face->glyph->bitmap.width) + x;
                uint32_t new_pos     = (((face->glyph->bitmap.rows - 1) - y) * face->glyph->bitmap.width) + x;

                flipped_image[new_pos] = data[initial_pos];
            }
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, offset_x, offset_y, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, flipped_image); // face->glyph->bitmap.buffer
        free(flipped_image);


        /* Save the character */

        f->_characters[c] = (SBRE_Character) {
            .character = c,
            .render_offset = SBRE_VEC2(offset_x, offset_y),
            .size = SBRE_VEC2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            .bearing = SBRE_VEC2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            .advance = face->glyph->advance.x
        };


        /* Calculate the next characters offset */

        offset_x += biggest_width;
        if (offset_x + biggest_width >= width) {

            offset_x = 0;
            offset_y_increase += biggest_height;
            offset_y = (height - biggest_height - 1) - offset_y_increase;
        }
    }


    /* Cleanup */

    FT_Done_Face(face);
    FT_Done_FreeType(ft);


    return f;
}



void SBRE_free_font(Font* font) {

    glDeleteTextures(1, &font->font_atlas->texture_id);
    free(font->font_atlas);
    free(font->_characters);
    free(font);
    
    font = NULL;
}



Vec2 SBRE_font_get_text_size(Font* font, const char* text) {

    float width = 0;
    float height = 0;


    uint32_t text_len = strlen(text);
	for (int i = 0; i < text_len; ++i) {
		
		SBRE_Character crnt_char = font->_characters[text[i]];


        if (i < text_len - 1)
            width += crnt_char.bearing.x + (crnt_char.advance >> 6);
        else 
            width += crnt_char.size.x + crnt_char.bearing.x;


        //float char_height = font->biggest_char.y - crnt_char.size.y; // TODO : Remove this line
        if (crnt_char.size.y > height)
            height = crnt_char.size.y;
	}


    return (Vec2) { width, height };
}