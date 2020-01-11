#ifndef FONT_H
#define FONT_H



enum textjusty_t {
	TEXT_LEFT,
	TEXT_CENTER,
	TEXT_RIGHT,
	TEXT_TOP,
	TEXT_BOTTOM
};

struct glyphlist_t {
	TTF_Font *font;
	unsigned size;
	
	// keep track of our textures
	GLuint textures[256];
	unsigned tex_w[256];
	unsigned tex_h[256];
	
	// things we'll need for deciding where to draw out textures
	unsigned font_h;					// max hieght for font
	unsigned font_h_diff;			// diff between texture h and font h
	int glyph_advance[256];
	
	glyphlist_t *next;
};

struct fontlist_t {
	char *name;
	char *path;
	
	glyphlist_t *glyphs;
	fontlist_t *next;
};



// set pointers to NULL, etc
void initText();

// load a font at the given path, associate with the given handle
// creates a fontlist_t object for each font
void loadFont(char *,char *);

// set text color, font and size
// will create new glyphlist_t things when the size is new
void setTextProps(float,float,float,char *,unsigned);

// place the cursor at the specified coords
void setTextStart(float,float);

// place the cursor at the specified coords and prepare to print rotated text
void setTextStart(float,float,float);

// set the horiz,vert justification of the text
void setTextJust(textjusty_t,textjusty_t);

// adjust the location of the cursor relative to where it is
void shiftTextStart(float,float);

// determine how wide the given string will be when printed
int textWidth(char *);

// determine how tall the tallest string would be when printed
int textHieght();

// write a string of text to the screen, creating glyphs as needed
void renderText(char *);



#endif
