#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "timer.h"

#include <font_bmp.h>
#include <logo_bmp.h>
#include <razor_bmp.h>
#include <scanline_bmp.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_BMP
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_NO_STDIO
#include "stb_image.h"

#define DEBUG 0						// debug [0/1]
#define START 4						// part to start
#define PI 3.14159265358979323846f // pi
#define PID PI/180.0f			// pi ratio
#define CR 1.0f/256.0f		// color ratio
#define SNG true					// music flag

Timer *timer;
float timer_global=0;
float timer_global_previous=0;
float timer_global_buffer=0;
float timer_fps=0;
float timer_fps_total=0;
float timer_fps_average=0;
float timer_fps_min=32768;
float timer_fps_max=0;
int timer_frame=0;
int frame_counter=0;
int frame_total=0;
bool done=false;

#ifdef SNG
// miniaudio
#define MINIAUDIO_IMPLEMENTATION
#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_NO_GENERATION
#define MA_NO_ENCODING
#define MA_NO_WAV
#define MA_NO_FLAC
#if _WIN32
#define MA_ENABLE_DSOUND
#elif __linux__
#define MA_ENABLE_ALSA
#else
#error "Sound is not supported on this platform"
#endif

#include "miniaudio.h"
#include <lpr231_mp3.h>

#endif

#ifdef SNG
	ma_engine engine;
	ma_sound sound;
	bool mod_play=false;	// flag
	float mod_pos=0;
	float mod_prv_pos=0;
#endif

#define GEAR1_LIST 1
#define GEAR2_LIST 2
#define GEAR3_LIST 3
#define GEAR4_LIST 4
#define GEAR5_LIST 5
#define GEAR6_LIST 6
#define GEAR7_LIST 7
#define GEAR2D1_LIST 8
#define GEAR2D2_LIST 9
#define GEAR2D3_LIST 10
#define GEAR2D4_LIST 11
#define GRADIENT_LIST 12
#define BORDER_LIST 13

GLuint razor_id;
GLuint scanline_id;
GLuint font_id;
GLuint logo_id;

GLFWwindow* window=NULL;

int  keys[256];					// keyboard array
int	 active=true;				// window active flag
bool fullscreen=DEBUG?false:true;	// fullscreen flag
bool dempause=false;				// pause flag
float fov_base=70;			// base fov angle
float fov=fov_base;			// field of view angle
float nearplane=0.5f;		// nearplane
float farplane=1000.0f;	// farplane
bool polygon=true;			// polygon mode
/* window variable			*/
int window_w=1280;				// width
int window_h=720;				// height
int screen_w;						// width
int screen_h;						// height
int screen_average;			// average
int window_color=32;		// color depth
int window_depth=16;		// depth buffer
/* object variable			*/
float	p_x=0;						// position x
float	p_y=0;						// position y
float	p_z=0;						// position z
float	a_x=0;						// angle x
float	a_y=0;						// angle y
float	a_z=0;						// angle z
float	main_angle;				// main angle
/* color variable				*/
float color_inc=0.025f;	// color incrementation
float base_r=0.1f;			// base r
float base_g=0.125f;		// base g
float base_b=0.1f;			// base b
float bgd_base_r=base_r;// red base value
float bgd_base_g=base_g;// green base value
float bgd_base_b=base_b;// blue base value
float bgd_r=0;					// red value
float bgd_g=0;					// green value
float bgd_b=0;					// blue value
/* fog variable					*/
float fog_color[]={base_r,base_g,base_b,1.0f};	// fog color definition
float f_v1;							// level 1
float f_v2;							// level 2
float f_n;							// level (new)
float f_t1;							// synchro time 1
float f_t2;							// synchro time 2
/* liner variable				*/
bool liner_flag=false;	// liner flag
int car;								// ascii code
int liner_length;				// text length
int l_n;								// line number
int liner_max;					// line max length
int liner_line;					// line increment
int liner_i;						// char increment
int liner_count=0;			// liner counter
int liner_count_start=0;// liner counter start
int liner_count_wait=0;	// wait between two update
float liner_angle;			// angle
float l_w;							// width
float l_h;							// height
float liner_size=0.1f;	// size
float liner_z=-8.0f;    // position z
float liner_radius=0;		// radius x
float liner_jump=0.03f;	// jump radius
float liner_r=1.0f;			// color r
float liner_g=1.0f;			// color g
float liner_b=1.0f;			// color b
float liner_color;			// color increment
/* text variable				*/
const char *txt;
const char *name="Razor 1911 - StarCraft ][";
const char *txt_info1="\r\r\r   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\rÄÄÄ´ ğ=- RAZOR 1911 PRESENTS -=ğ ÃÄÄÄ\r   ÀÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÙ\r    ³                           ³\r    ³        StarCraft][        ³\r    ³   ğ Wings of Liberty ğ    ³\r    ³                           ³\r    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\r                                 ";
const char *txt_info2="\r\r\rÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\r³        < Informations >        ³\rÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´\r³ Supplied By     \007 *** *****    ³\r³ Cracked By      \007 ******       ³\r³ Release Date    \007 31 July 2010 ³\r³ Protection Type \007 Battle.net   ³\rÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\r                                  ";
const char *txt_info3="\r\r\r   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\r   ³        < Greetings >        ³\r   ÀÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÙ\r     ³    -=ğ BLIZZARD\003 ğ=-    ³\r     ³ \007 Please, try again \002 \007 ³\r   ÚÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄ¿\rÄÄÄ´ ğğğ NO INTERNET NEEDED! ğğğ ÃÄÄÄ\r   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\r                                  ";
const char *txt_info4="\r\r\r ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\r ³ IF YOU LIKED THIS PRODUCT ³\r ÀÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÙ\rÄÄ´  -=ğ PLEASE BUY IT ğ=-  ÃÄÄ\r ÚÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁ¿\r ³ THE PUBLISHERS OF QUALITY ³\r ³ SOFTWARE DESERVE SUPPORT! ³\r ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\r                              ";
const char *txt_info5="\r\r\rÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\r³ You reach the end of this ³\r³ little intro. Thank a lot ³\r³ for reading everything! \002 ³\rÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´\r³ As a gift, you can hit F2 ³\r³ key for an hidden message ³\rÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\r                             ";

const char *txt_info6="\r\r\rÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\r³ I would like to thank you rez ³\r³   for releasing the source    ³\r³    of this wonderful demo  \002  ³\rÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´\r³     and looper231 for this    ³\r³  remix/cover of the original  ³\r³    'intro number 61' module   ³\rÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\r                             ";

const char *txt_info7="\r\r\rÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\r³                               ³\r³ You can now press ESC to quit ³\r³   or stay to watch it again   ³\r³                               ³\rÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\r                             ";

const char *txt_logo="\r\r\r/\\______  /\\______  ____/\\______     __\r\\____   \\/  \\__   \\/  _ \\____   \\ __/  \\____\r / _/  _/    \\/   /   /  / _/  _// / / / / /\r/  \\   \\  /\\ /   /\\  /  /  \\   \\/ /\\  / / /\r\\__/\\   \\/RTX______\\___/\\__/\\   \\/ / /_/_/\rğğğğğ\\___)ğğğ\\__)ğğğğğğğğğğğğ\\___)ğ\\/ğğğğğğ\rRAZOR 1911 \007 SHARPENING THE BLADE 1985,2010 \r                                            ";
const char *txt_credits[]={"PIRATES OF THE 777 SEAS ","","","","CODE","REZ","","LOGO","KENET","","MUSIC","WOTW","LOOPER231"};
const char *txt_intro="";
const char *txt_intro1="\003 WE ARE PROUD TO PRESENT \003";
const char *txt_intro2="\002 A BRAND NEW RELEASE BY \002";
const char *txt_hidden[]={"  \003 RAZOR 1911 \003    \003 RAZOR 1911 \003    \003 RAZOR 1911 \003 ","  \002 \007WE RULEZ\007 \002    \002 \007WE RULEZ\007 \002    \002 \007WE RULEZ\007 \002 ","  SKID ROW SUCKS    SKID ROW SUCKS    SKID ROW SUCKS "};
const char *txt_crack="\003 Happy birthday Razor 1911 \003 Sharpening the blade since 1985! \016 We also want to wish an happy birthday to the Commodore Amiga who turned 25 years old, just like us! -=*=- We already cracked the following protection: 3PLock, ActiveMARK, Alcatraz, Alpha-DVD, Alpha-ROM, CD Lock, CD-Cops, CD-Lock, CD-Protect, CodeLok, CopyLok, CrypKey, DBB, DiscGuard, DVD-Cops, FADE, HexaLock, JoWood X-Prot, LaserLock, LockBlocks, PhenoProtect, ProRing, ProtecDISC, ProtectCD, Ring PROTECH, Roxxe, SafeCast, SafeDisc, SecuROM, SmartE, SmarteCD, SoftLock, Solidshield, StarForce, SVK Protector, TAGES, UbiSoft DRM, VOB Protect \002 SVKP, Themidea, VMProtect, Armadillo, EXECryptor, ACProtect, ASPack, FSG, MEW, MoleBox, Morphine, Obsidium, PeCompact, PeLock, SafeCast, SDProtector, tElock, WinLicense, Yodas Crypter, Yodas Protector and still counting! \002";
/* gear variable				*/
bool gear_flag=false;		// flag
float gear_x=0;					// position x
float gear_y=0;					// position y
float gear_h=0.5f;			// height
float gear_r=0.45f;			// color r
float gear_g=0.4f;			// color g
float gear_b=0.35f;			// color b
float gear_radius=0;		// radius x
float triforce_r=0.75f;	// color r
float triforce_g=0.625f;// color g
float triforce_b=0.5f;	// color b
float triforce_i=0.25f;	// color increment
float screw_r=0.45f;		// color r
float screw_g=0.425f;		// color g
float screw_b=0.375f;		// color b
float screw_i=0.125f;		// color increment
float gear1=5.0f;
float gear2=3.0f;
float gear3=10.0f;
float gear4=2.0f;
float gear5=3.0f;
float gear6=4.0f;
float gear7=12.0f;
/* hidden */
bool hidden_flag=false;	// flag
int hidden_type=1;			// hidden text type
int hidden_length=0;		// text length
float hidden_w=1.2f;		// width
float hidden_h=1.5f;		// height
float hidden_m=0.04f;
float hidden_inc=0.01f;	// color incrementation
float hidden_base_r;		// base r
float hidden_base_g;		// base g
float hidden_base_b;		// base b
float hidden_r;					// color r
float hidden_g;					// color g
float hidden_b;					// color b
float hidden_x;					// position x
float hidden_y;					// position y
float hidden_z=0.075f;	// position z
int hidden_n=6;					// repeat
/* credits variable			*/
bool credits_flag=false;// flag
bool credits_change=false;// flag
float credits_x=0;			// position x
float credits_y;				// position y
float credits_w;				// width
float credits_angle;		// angle
float credits_color;		// color
int credits_length;			// text length
int credits_n=1;				// number
/* crack variable				*/
bool crack_flag=true;		// flag
int crack_length=(int)strlen(txt_crack);
float crack_w;					// width
float crack_h;					// height
float crack_x;					// char x
float crack_px;					// position x
int crack_n;						// char n by line
int crack_counter=-1000;// pointer
/* intro variable				*/
bool intro_flag=false;	// flag
float intro_size=0.1f;	// char size
float intro_x=2.45f;		// position x
float intro_y=-1.5f;		// position y
float intro_z=-2.7f;		// position z
float intro_h=0;				// height
float intro_r=1.0f;			// color r
float intro_g=1.0f;			// color g
float intro_b=1.0f;			// color b
float intro_angle=0;		// angle
int	intro_light=0;			// text light
int	intro_length=0;			// text length
/* razor variable				*/
bool razor_flag1=false;	// flag
bool razor_flag2=false;	// flag
bool razor_close=false;	// flag
int razor_w=600;				// width
int razor_h=272;				// height
int razor_nx=razor_w/30;
int razor_ny=razor_h/34;
float razor_color=1.0f;
float razor_angle=0;
float razor_angle_transition=0;
float razor_angle_out=0;
float razor_angle_close=0;
float razor_size=0.075f;
float razor_width=razor_size*0.2f;
float razor_y=0;
float razor_z=0;
float razor_radius=-3.5f;
float razor_zoom=0.1f;
/* transition variable	*/
int transition_n=16;
int transition_i=0;
int transition_w;
/* cube variable				*/
bool cube_flag=false;		// flag
int cube_n=256;					// number
float cube_size=0.125f;	// size
float cube_position[256][3];
float cube_rotation[256][4];
float cube_color[256][3];
/* logo variable				*/
bool logo_flag=false;		// flag
int logo_w=32;					// width
int logo_h=32;					// height
int logo_margin=6;			// margin
/* gradient variable		*/
bool gradient_flag=false;	// flag
/* flash variable				*/
bool flash_flag=false;	// flag
float flash_angle=0;	// angle
/* decrunch variable		*/
bool decrunch_flag=DEBUG?false:true;// flag
int decrunch_h=0;				// height
int decrunch_y=0;				// top
int decrunch_split=0;		// bar split random
int decrunch_split_w;		// bar split w
float decrunch_time=DEBUG?0:0.75f;
/* debug variable				*/
bool debug_flag=DEBUG?true:false;	// flag
bool debug_test=true;		// test
/* border variable			*/
bool border_flag=false;	// flag
int border_h;						// height
/* scanline variable		*/
bool scanline_flag=true;// flag
/* synchro variable			*/
bool synchro_flag=false;// flag
float synchro_angle=0;	// angle
float synchro_value=0;	// value
/* fade variable				*/
bool fade_flag=false;		// flag
float fade_angle=0;			// angle
float fade_value=0;			// value
int i,j;
float x,y,z;
float r,g,b,c;
float angle,radius,scale;

#ifdef SNG

void load_song() {
	ma_result result;
	
	result = ma_engine_init(NULL, &engine);
	if (result != MA_SUCCESS)
		exit(0);

	result = ma_resource_manager_register_encoded_data(
		ma_engine_get_resource_manager(&engine), "music", lpr231_mp3, lpr231_mp3_size);
	if (result != MA_SUCCESS)
		exit(0);

	result = ma_sound_init_from_file(&engine, "music", 0, NULL, NULL, &sound);
	if (result != MA_SUCCESS)
		exit(0);
}

#endif

int load_tex(unsigned char *file, int size, GLint filter, GLint clamp) {
	GLuint TexID;
	int x, y, cpf;

	stbi_set_flip_vertically_on_load(1);
	unsigned char *tex = stbi_load_from_memory(file,size,&x,&y,&cpf,0);
	assert(tex);

	glGenTextures(1,&TexID);
	glPixelStorei(GL_UNPACK_ALIGNMENT,4);
	glBindTexture(GL_TEXTURE_2D,TexID);
	glTexImage2D(GL_TEXTURE_2D,0,3,x,y,0,GL_RGB,GL_UNSIGNED_BYTE,tex);
	gluBuild2DMipmaps(GL_TEXTURE_2D,3,x,y,GL_RGB,GL_UNSIGNED_BYTE,tex);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,filter);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,clamp);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,clamp);
	stbi_image_free(tex);
	return TexID;
}

void init3d(GLsizei width,GLsizei height)
	{
	glViewport(0,0,width,height);	// reset viewport
	glMatrixMode(GL_PROJECTION);	// select projection matrix
	glLoadIdentity();							// reset projection matrix
	gluPerspective(fov,(float)((float)width/(float)height),nearplane,farplane); // aspect ratio
	glMatrixMode(GL_MODELVIEW);		// select modelview matrix
	glLoadIdentity();							// reset modelview matrix
	}

void init2d(GLsizei width,GLsizei height)
	{
	glViewport(0,0,width,height);	// reset viewport
	glMatrixMode(GL_PROJECTION);	// select projection matrix
	glLoadIdentity();							// reset projection matrix
	gluOrtho2D(0,width,height,0);	// init orthographic mode
	glMatrixMode(GL_MODELVIEW);		// select modelview matrix
	glLoadIdentity();							// reset modelview matrix
	}

void calc_txt()
	{
	liner_length=strlen(txt);
	liner_count=0;
	liner_angle=main_angle;
	l_n=0;
	liner_max=0;
	liner_i=0;
	for(i=0;i<liner_length;i++)
		{
		if(txt[i]!=13)
			{
			liner_i++;
			}
		else
			{
			if(liner_i>liner_max) liner_max=liner_i;
			l_n++;
			liner_i=0;
			}
		}
	if(liner_i>liner_max) liner_max=liner_i;
	fade_value=1.0f;
	}

void draw_char(char car,float w,float h,float m)
	{
	if(car!=32)
		{
		l_w=(car%16)/16.0f;
		l_h=(car-car%16)/256.0f;
		glBegin(GL_QUADS);
			glTexCoord2f(l_w+0.0620f,-l_h-0.0620f); glVertex2f( w*0.5f-m, h*0.5f);
			glTexCoord2f(l_w+0.0620f,-l_h-0.0005f); glVertex2f( w*0.5f+m,-h*0.5f);
			glTexCoord2f(l_w+0.0005f,-l_h-0.0005f); glVertex2f(-w*0.5f-m,-h*0.5f);
			glTexCoord2f(l_w+0.0005f,-l_h-0.0620f); glVertex2f(-w*0.5f+m, h*0.5f);
		glEnd();
		}
	}

void flash()
	{
	flash_flag=true;
	flash_angle=main_angle;
	}

void synchro()
	{
	synchro_flag=true;
	synchro_angle=main_angle;
	intro_light=0;
	}

void fade()
	{
	fade_flag=true;
	fade_angle=main_angle;
	}

void cube(float size,float r,float g,float b,float inc,bool mode)
	{
	glBegin(GL_QUADS);
		if(mode)
			{
			glColor3f(r+inc,g+inc,b+inc);
			glVertex3f( size, size, size); // front
			glVertex3f(-size, size, size);
			glVertex3f(-size,-size, size);
			glVertex3f( size,-size, size);
			glVertex3f( size, size,-size); // back
			glVertex3f( size,-size,-size);
			glVertex3f(-size,-size,-size);
			glVertex3f(-size, size,-size);
			glColor3f(r,g,b);
			glVertex3f(-size, size, size); // left
			glVertex3f(-size, size,-size);
			glVertex3f(-size,-size,-size);
			glVertex3f(-size,-size, size);
			glVertex3f( size, size, size); // right
			glVertex3f( size,-size, size);
			glVertex3f( size,-size,-size);
			glVertex3f( size, size,-size);
			glColor3f(r-inc,g-inc,b-inc);
			glVertex3f( size,-size, size); // top
			glVertex3f(-size,-size, size);
			glVertex3f(-size,-size,-size);
			glVertex3f( size,-size,-size);
			glVertex3f( size, size, size); // bottom
			glVertex3f( size, size,-size);
			glVertex3f(-size, size,-size);
			glVertex3f(-size, size, size);
			}
		else
			{
			glColor3f(r-inc,g-inc,b-inc);
			glVertex3f( size, size,-size); // front
			glVertex3f(-size, size,-size);
			glVertex3f(-size,-size,-size);
			glVertex3f( size,-size,-size);
			glVertex3f( size, size, size); // back
			glVertex3f( size,-size, size);
			glVertex3f(-size,-size, size);
			glVertex3f(-size, size, size);
			glColor3f(r,g,b);
			glVertex3f( size, size, size); // left
			glVertex3f( size, size,-size);
			glVertex3f( size,-size,-size);
			glVertex3f( size,-size, size);
			glVertex3f(-size, size, size); // right
			glVertex3f(-size,-size, size);
			glVertex3f(-size,-size,-size);
			glVertex3f(-size, size,-size);
			glColor3f(r+inc,g+inc,b+inc);
			glVertex3f( size, size, size); // top
			glVertex3f(-size, size, size);
			glVertex3f(-size, size,-size);
			glVertex3f( size, size,-size);
			glVertex3f( size,-size, size); // bottom
			glVertex3f( size,-size,-size);
			glVertex3f(-size,-size,-size);
			glVertex3f(-size,-size, size);
			}
	glEnd();
	}

void screw(float radius1,float radius2,float h1,float h2,float r,float g,float b,float inc)
	{
	float x1,x2,y1,y2;
	int n1=6;
	int n2=8;
	for(i=0;i<n1;i++)
		{
		x1=radius1*cosf(360.0f/n1*i*PID);
		y1=radius1*sinf(360.0f/n1*i*PID);
		x2=radius1*cosf(360.0f/n1*(i+1)*PID);
		y2=radius1*sinf(360.0f/n1*(i+1)*PID);
		glColor3f(r,g,b);
		glBegin(GL_TRIANGLES);
			glVertex3f( 0, 0, h1);
			glVertex3f(x1,y1, h1);
			glVertex3f(x2,y2, h1);
		glEnd();
		glColor3f(r-inc,g-inc,b-inc);
		glBegin(GL_QUADS);
			glVertex3f(x1,y1,-h1);
			glVertex3f(x2,y2,-h1);
			glVertex3f(x2,y2, h1);
			glVertex3f(x1,y1, h1);
		glEnd();
		}
	for(i=0;i<n2;i++)
		{
		x1=radius2*cosf(360.0f/n2*i*PID);
		y1=radius2*sinf(360.0f/n2*i*PID);
		x2=radius2*cosf(360.0f/n2*(i+1)*PID);
		y2=radius2*sinf(360.0f/n2*(i+1)*PID);
		glColor3f(r+inc,g+inc,b+inc);
		glBegin(GL_TRIANGLES);
			glVertex3f( 0, 0,h1+h2);
			glVertex3f(x1,y1,h1+h2);
			glVertex3f(x2,y2,h1+h2);
		glEnd();
		glColor3f(r-inc,g-inc,b-inc);
		glBegin(GL_QUADS);
			glVertex3f(x1,y1,h1   );
			glVertex3f(x2,y2,h1   );
			glVertex3f(x2,y2,h1+h2);
			glVertex3f(x1,y1,h1+h2);
		glEnd();
		}
	}

void gear(float tooth_n,float radius1,float radius5,float radius3,float radius6,float border,float height1,float height3,float height2,float r,float g,float b,float inc)
	{
	float radius2=radius1-border;
	float radius4=radius3-border;
	float a1=360.0f/(tooth_n*2.0f)*PID;
	float a2=a1*0.325f;
	float a3=a1*0.2f;
	float cos1,cos2,cos3,cos4,cos5,cos6,sin1,sin2,sin3,sin4,sin5,sin6;
	float x1,x2,x3,x4,x5,x6,x7,x8;
	float y1,y2,y3,y4,y5,y6,y7,y8;
	glBegin(GL_QUADS);
	for(i=0;i<tooth_n*2;i++)
		{
		cos1=cosf(i*a1);
		sin1=sinf(i*a1);
		cos2=cosf(i*a1+a1);
		sin2=sinf(i*a1+a1);
		cos3=cosf(i*a1+a3);
		sin3=sinf(i*a1+a3);
		cos4=cosf(i*a1+a1-a3);
		sin4=sinf(i*a1+a1-a3);
		cos5=cosf(i*a1+a2);
		sin5=sinf(i*a1+a2);
		cos6=cosf(i*a1+a1-a2);
		sin6=sinf(i*a1+a1-a2);
		x1=radius1*cos1;
		y1=radius1*sin1;
		x2=radius1*cos2;
		y2=radius1*sin2;
		x3=radius2*cos1;
		y3=radius2*sin1;
		x4=radius2*cos2;
		y4=radius2*sin2;
		x5=radius3*cos1;
		y5=radius3*sin1;
		x6=radius3*cos2;
		y6=radius3*sin2;
		x7=radius4*cos1;
		y7=radius4*sin1;
		x8=radius4*cos2;
		y8=radius4*sin2;
		glColor3f(r,g,b);
		glVertex3f(x4,y4, height1); // border 1 top
		glVertex3f(x3,y3, height1);
		glVertex3f(x1,y1, height1);
		glVertex3f(x2,y2, height1);
		glColor3f(r-inc,g-inc,b-inc);
		glVertex3f(x3,y3, height1); // border 1 back
		glVertex3f(x4,y4, height1);
		glVertex3f(x4,y4, height2);
		glVertex3f(x3,y3, height2);
		glColor3f(r+inc,g+inc,b+inc);
		glVertex3f(x6,y6, height2); // border top
		glVertex3f(x5,y5, height2);
		glVertex3f(x3,y3, height2);
		glVertex3f(x4,y4, height2);
		glColor3f(r-inc,g-inc,b-inc);
		glVertex3f(x6,y6, height1); // border 2 front
		glVertex3f(x5,y5, height1);
		glVertex3f(x5,y5, height2);
		glVertex3f(x6,y6, height2);
		glColor3f(r,g,b);
		glVertex3f(x8,y8, height1); // border 2 top
		glVertex3f(x7,y7, height1);
		glVertex3f(x5,y5, height1);
		glVertex3f(x6,y6, height1);
		if(radius6!=0&&i%2==0)
			{
			x3=(radius6+0.1f)*cos3;
			y3=(radius6+0.1f)*sin3;
			x4=(radius6+0.1f)*cos4;
			y4=(radius6+0.1f)*sin4;
			x5=radius6*cos5;
			y5=radius6*sin5;
			x6=radius6*cos6;
			y6=radius6*sin6;
			glColor3f(r-inc,g-inc,b-inc);
			glVertex3f(x5,y5, height3); // tooth front
			glVertex3f(x6,y6, height3);
			glVertex3f(x6,y6,-height3);
			glVertex3f(x5,y5,-height3);
			glColor3f(r+inc,g+inc,b+inc);
			glVertex3f(x4,y4,-height3); // tooth left
			glVertex3f(x4,y4, height3);
			glVertex3f(x8,y8, height1);
			glVertex3f(x8,y8,-height1);
			glVertex3f(x7,y7,-height1); // tooth right
			glVertex3f(x7,y7, height1);
			glVertex3f(x3,y3, height3);
			glVertex3f(x3,y3,-height3);
			glColor3f(r,g,b);
			glVertex3f(x6,y6,-height3); // tooth left
			glVertex3f(x6,y6, height3);
			glVertex3f(x4,y4, height3);
			glVertex3f(x4,y4,-height3);
			glVertex3f(x3,y3,-height3); // tooth right
			glVertex3f(x3,y3, height3);
			glVertex3f(x5,y5, height3);
			glVertex3f(x5,y5,-height3);
			glColor3f(r,g,b);
			glVertex3f(x4,y4, height3); // tooth top
			glVertex3f(x3,y3, height3);
			glVertex3f(x7,y7, height1);
			glVertex3f(x8,y8, height1);
			glVertex3f(x6,y6, height3);
			glVertex3f(x5,y5, height3);
			glVertex3f(x3,y3, height3);
			glVertex3f(x4,y4, height3);
			}
		else
			{
			glColor3f(r-inc,g-inc,b-inc);
			glVertex3f(x7,y7, height1); // border 2 back
			glVertex3f(x8,y8, height1);
			glVertex3f(x8,y8,-height1);
			glVertex3f(x7,y7,-height1);
			}
		if(i%2==0)
			{
			x3=(radius5-0.1f)*cos3;
			y3=(radius5-0.1f)*sin3;
			x4=(radius5-0.1f)*cos4;
			y4=(radius5-0.1f)*sin4;
			x5=radius5*cos5;
			y5=radius5*sin5;
			x6=radius5*cos6;
			y6=radius5*sin6;
			glColor3f(r-inc,g-inc,b-inc);
			glVertex3f(x6,y6, height3); // tooth front
			glVertex3f(x5,y5, height3);
			glVertex3f(x5,y5,-height3);
			glVertex3f(x6,y6,-height3);
			glColor3f(r+inc,g+inc,b+inc);
			glVertex3f(x2,y2,-height1); // tooth left
			glVertex3f(x2,y2, height1);
			glVertex3f(x4,y4, height3);
			glVertex3f(x4,y4,-height3);
			glVertex3f(x3,y3,-height3); // tooth right
			glVertex3f(x3,y3, height3);
			glVertex3f(x1,y1, height1);
			glVertex3f(x1,y1,-height1);
			glColor3f(r,g,b);
			glVertex3f(x4,y4,-height3); // tooth left
			glVertex3f(x4,y4, height3);
			glVertex3f(x6,y6, height3);
			glVertex3f(x6,y6,-height3);
			glVertex3f(x5,y5,-height3); // tooth right
			glVertex3f(x5,y5, height3);
			glVertex3f(x3,y3, height3);
			glVertex3f(x3,y3,-height3);
			glColor3f(r,g,b);
			glVertex3f(x2,y2, height1); // tooth top
			glVertex3f(x1,y1, height1);
			glVertex3f(x3,y3, height3);
			glVertex3f(x4,y4, height3);
			glVertex3f(x4,y4, height3);
			glVertex3f(x3,y3, height3);
			glVertex3f(x5,y5, height3);
			glVertex3f(x6,y6, height3);
			}
		else
			{
			glColor3f(r-inc,g-inc,b-inc);
			glVertex3f(x2,y2, height1); // front
			glVertex3f(x1,y1, height1);
			glVertex3f(x1,y1,-height1);
			glVertex3f(x2,y2,-height1);
			}
		}
	glEnd();
	}

void gear2d(float tooth_n,float radius1,float radius2,float radius3,float r,float g,float b)
	{
	float a1=360.0f/(tooth_n*2.0f)*PID;
	float a2=a1*0.325f;
	float a3=a1*0.25f;
	float cos1,cos2,cos3,cos4,sin1,sin2,sin3,sin4;
	float x1,x2,x3,x4,x5,x6;
	float y1,y2,y3,y4,y5,y6;
	glBegin(GL_QUADS);
		for(i=0;i<tooth_n*2;i++)
			{
			cos1=cosf(i*a1);
			sin1=sinf(i*a1);
			cos2=cosf(i*a1+a1);
			sin2=sinf(i*a1+a1);
			cos3=cosf(i*a1+a3);
			sin3=sinf(i*a1+a3);
			cos4=cosf(i*a1+a1-a3);
			sin4=sinf(i*a1+a1-a3);
			x1=radius1*cos1;
			y1=radius1*sin1;
			x2=radius1*cos2;
			y2=radius1*sin2;
			x3=radius2*cos3;
			y3=radius2*sin3;
			x4=radius2*cos4;
			y4=radius2*sin4;
			x5=radius3*cos1;
			y5=radius3*sin1;
			x6=radius3*cos2;
			y6=radius3*sin2;
			glColor3f(r,g,b);
			glVertex3f(x2,y2,0);
			glVertex3f(x1,y1,0);
			glVertex3f(x5,y5,0);
			glVertex3f(x6,y6,0);
			if(i%2==0)
				{
				glVertex3f(x1,y1,0);
				glVertex3f(x2,y2,0);
				glVertex3f(x4,y4,0);
				glVertex3f(x3,y3,0);
				}
			}
	glEnd();
	}

void triforce(bool mode,float radius2,float z,float r,float g,float b,float inc)
	{
	float radius1=radius2*0.5f;
	float x1=radius1*cosf(PID* 30.0f);
	float y1=radius1*sinf(PID* 30.0f);
	float x2=radius1*cosf(PID*150.0f);
	float y2=radius1*sinf(PID*150.0f);
	float x3=radius1*cosf(PID*270.0f);
	float y3=radius1*sinf(PID*270.0f);
	float x4=radius2*cosf(PID* 90.0f);
	float y4=radius2*sinf(PID* 90.0f);
	float x5=radius2*cosf(PID*210.0f);
	float y5=radius2*sinf(PID*210.0f);
	float x6=radius2*cosf(PID*330.0f);
	float y6=radius2*sinf(PID*330.0f);
	glColor3f(r,g,b);
	glBegin(GL_TRIANGLES);
		if(mode)
			{
			glVertex3f(x1,y1,z);
			glVertex3f(x4,y4,z);
			glVertex3f(x2,y2,z);
			glVertex3f(x2,y2,z);
			glVertex3f(x5,y5,z);
			glVertex3f(x3,y3,z);
			glVertex3f(x3,y3,z);
			glVertex3f(x6,y6,z);
			glVertex3f(x1,y1,z);
			glVertex3f(x1,y1,0);
			glVertex3f(x4,y4,0);
			glVertex3f(x2,y2,0);
			glVertex3f(x2,y2,0);
			glVertex3f(x5,y5,0);
			glVertex3f(x3,y3,0);
			glVertex3f(x3,y3,0);
			glVertex3f(x6,y6,0);
			glVertex3f(x1,y1,0);
			}
		else
			{
			glVertex3f(x2,y2,z);
			glVertex3f(x4,y4,z);
			glVertex3f(x1,y1,z);
			glVertex3f(x3,y3,z);
			glVertex3f(x5,y5,z);
			glVertex3f(x2,y2,z);
			glVertex3f(x1,y1,z);
			glVertex3f(x6,y6,z);
			glVertex3f(x3,y3,z);
			glVertex3f(x2,y2,0);
			glVertex3f(x4,y4,0);
			glVertex3f(x1,y1,0);
			glVertex3f(x3,y3,0);
			glVertex3f(x5,y5,0);
			glVertex3f(x2,y2,0);
			glVertex3f(x1,y1,0);
			glVertex3f(x6,y6,0);
			glVertex3f(x3,y3,0);
			}
	glEnd();
	glColor3f(r-inc,g-inc,b-inc);
	glBegin(GL_QUADS);
		if(mode)
			{
			glVertex3f(x1,y1,z);
			glVertex3f(x2,y2,z);
			glVertex3f(x2,y2,0);
			glVertex3f(x1,y1,0);
			glVertex3f(x2,y2,z);
			glVertex3f(x4,y4,z);
			glVertex3f(x4,y4,0);
			glVertex3f(x2,y2,0);
			glVertex3f(x4,y4,z);
			glVertex3f(x1,y1,z);
			glVertex3f(x1,y1,0);
			glVertex3f(x4,y4,0);
			glVertex3f(x2,y2,z);
			glVertex3f(x3,y3,z);
			glVertex3f(x3,y3,0);
			glVertex3f(x2,y2,0);
			glVertex3f(x3,y3,z);
			glVertex3f(x5,y5,z);
			glVertex3f(x5,y5,0);
			glVertex3f(x3,y3,0);
			glVertex3f(x5,y5,z);
			glVertex3f(x2,y2,z);
			glVertex3f(x2,y2,0);
			glVertex3f(x5,y5,0);
			glVertex3f(x3,y3,z);
			glVertex3f(x1,y1,z);
			glVertex3f(x1,y1,0);
			glVertex3f(x3,y3,0);
			glVertex3f(x1,y1,z);
			glVertex3f(x6,y6,z);
			glVertex3f(x6,y6,0);
			glVertex3f(x1,y1,0);
			glVertex3f(x6,y6,z);
			glVertex3f(x3,y3,z);
			glVertex3f(x3,y3,0);
			glVertex3f(x6,y6,0);
			}
		else
			{
			glVertex3f(x1,y1,0);
			glVertex3f(x2,y2,0);
			glVertex3f(x2,y2,z);
			glVertex3f(x1,y1,z);
			glVertex3f(x2,y2,0);
			glVertex3f(x4,y4,0);
			glVertex3f(x4,y4,z);
			glVertex3f(x2,y2,z);
			glVertex3f(x4,y4,0);
			glVertex3f(x1,y1,0);
			glVertex3f(x1,y1,z);
			glVertex3f(x4,y4,z);
			glVertex3f(x2,y2,0);
			glVertex3f(x3,y3,0);
			glVertex3f(x3,y3,z);
			glVertex3f(x2,y2,z);
			glVertex3f(x3,y3,0);
			glVertex3f(x5,y5,0);
			glVertex3f(x5,y5,z);
			glVertex3f(x3,y3,z);
			glVertex3f(x5,y5,0);
			glVertex3f(x2,y2,0);
			glVertex3f(x2,y2,z);
			glVertex3f(x5,y5,z);
			glVertex3f(x3,y3,0);
			glVertex3f(x1,y1,0);
			glVertex3f(x1,y1,z);
			glVertex3f(x3,y3,z);
			glVertex3f(x1,y1,0);
			glVertex3f(x6,y6,0);
			glVertex3f(x6,y6,z);
			glVertex3f(x1,y1,z);
			glVertex3f(x6,y6,0);
			glVertex3f(x3,y3,0);
			glVertex3f(x3,y3,z);
			glVertex3f(x6,y6,z);
			}
	glEnd();
	}

void hidden_color()
	{
	hidden_type=hidden_flag?2:((hidden_type==0)?1:0);
	hidden_length=(int)strlen(txt_hidden[hidden_type]);
	hidden_r=1.0f;
	hidden_g=1.0f;
	hidden_b=1.0f;
	switch(hidden_type)
		{
		case 0:
			hidden_base_r=0.625f;
			hidden_base_g=0.25f;
			hidden_base_b=0.25f;
			break;
		case 1:
			hidden_base_r=0.25f;
			hidden_base_g=0.5f;
			hidden_base_b=0.75f;
			break;
		case 2:
			hidden_base_r=0.75f;
			hidden_base_g=0.5f;
			hidden_base_b=0;
			break;
		}
	}

int InitGL(void)
	{
	glClearDepth(1.0f);						// set depth buffer
	glDepthMask(GL_TRUE);					// do not write z-buffer
	glEnable(GL_CULL_FACE);				// disable cull face
	glCullFace(GL_BACK);					// don't draw front face
	glDisable(GL_BLEND);					// enable blending mode
	// fog
	glFogi(GL_FOG_MODE,GL_LINEAR);		// fog mode
	glFogfv(GL_FOG_COLOR,fog_color);	// fog color
	glFogf(GL_FOG_DENSITY,1.0f);			// fog density
	glHint(GL_FOG_HINT,GL_NICEST);		// fog hint value
	glFogf(GL_FOG_START,2.0f);				// fog start depth
	glFogf(GL_FOG_END,32.0f);					// fog end depth
	// load texture
	font_id = load_tex(font_bmp, font_bmp_size,GL_LINEAR,GL_REPEAT);
	razor_id = load_tex(razor_bmp, razor_bmp_size,GL_LINEAR,GL_CLAMP);
	logo_id = load_tex(logo_bmp, logo_bmp_size,GL_LINEAR,GL_CLAMP);
	scanline_id = load_tex(scanline_bmp, scanline_bmp_size,GL_LINEAR,GL_REPEAT);
	// generate list
	glNewList(GEAR1_LIST,GL_COMPILE);
		//glPushMatrix();
    //glPushAttrib(GL_CURRENT_BIT);
			gear(15,gear1,gear1+0.75f,gear1*0.5f,0,0.5f,0.75f,0.5f,0.5f,gear_r,gear_g,gear_b,0.1f);
			glTranslatef(0,3.2f,0.625f);
			triforce(true,1.0f,0.125f,triforce_r,triforce_g,triforce_b,triforce_i);
			glTranslatef(0,-3.2f,-0.625f);
			glRotatef(120,0,0,1.0f);
			glTranslatef(0,3.2f,0.625f);
			triforce(true,1.0f,0.125f,triforce_r,triforce_g,triforce_b,triforce_i);
			glTranslatef(0,-3.2f,-0.625f);
			glRotatef(120,0,0,1.0f);
			glTranslatef(0,3.2f,0.625f);
			triforce(true,1.0f,0.125f,triforce_r,triforce_g,triforce_b,triforce_i);
			glTranslatef(0,-3.2f,-0.625f);
			glRotatef(-180,0,0,1.0f);
			glTranslatef(0,3.5f,0.625f);
			screw(0.625f,0.25f,0.2f,0.125f,screw_r,screw_g,screw_b,screw_i);
			glTranslatef(0,-3.5f,-0.625f);
			glRotatef(120,0,0,1.0f);
			glTranslatef(0,3.5f,0.625f);
			screw(0.625f,0.25f,0.2f,0.125f,screw_r,screw_g,screw_b,screw_i);
			glTranslatef(0,-3.5f,-0.625f);
			glRotatef(120,0,0,1.0f);
			glTranslatef(0,3.5f,0.625f);
			screw(0.625f,0.25f,0.2f,0.125f,screw_r,screw_g,screw_b,screw_i);
		//glPopAttrib();
    //glPopMatrix();
	glEndList();
	glNewList(GEAR2_LIST,GL_COMPILE);
		gear(10,gear2,gear2+0.75f,gear2*0.235f,0,0.25f,0.75f,0.5f,0.5f,gear_r,gear_g,gear_b,0.1f);
		glTranslatef(0,0,0.5f);
		triforce(true,gear2-0.25f,0.125f,triforce_r,triforce_g,triforce_b,triforce_i);
		glTranslatef(0,0,-0.5f);
		glRotatef(60,0,0,1.0f);
		glTranslatef(0,2.05f,0.75f);
		screw(0.4f,0.2f,0.1f,0.1f,screw_r,screw_g,screw_b,screw_i);
		glTranslatef(0,-2.05f,-0.75f);
		glRotatef(120,0,0,1.0f);
		glTranslatef(0,2.05f,0.75f);
		screw(0.4f,0.2f,0.1f,0.1f,screw_r,screw_g,screw_b,screw_i);
		glTranslatef(0,-2.05f,-0.75f);
		glRotatef(120,0,0,1.0f);
		glTranslatef(0,2.05f,0.75f);
		screw(0.4f,0.2f,0.1f,0.1f,screw_r,screw_g,screw_b,screw_i);
	glEndList();
	glNewList(GEAR3_LIST,GL_COMPILE);
		gear(30,gear3,gear3+0.75f,gear3-2.0f,gear3-2.75f,0.25f,0.75f,0.5f,0.625f,gear_r,gear_g,gear_b,0.1f);
		glTranslatef(0,8.65f,0.625f);
		triforce(true,1.0f,0.125f,triforce_r,triforce_g,triforce_b,triforce_i);
		glTranslatef(0,-8.65f,0);
		glRotatef(120,0,0,1.0f);
		glTranslatef(0,8.65f,0);
		triforce(true,1.0f,0.125f,triforce_r,triforce_g,triforce_b,triforce_i);
		glTranslatef(0,-8.65f,0);
		glRotatef(120,0,0,1.0f);
		glTranslatef(0,8.65f,0);
		triforce(true,1.0f,0.125f,triforce_r,triforce_g,triforce_b,triforce_i);
	glEndList();
	glNewList(GEAR4_LIST,GL_COMPILE);
		gear(7,gear4,gear4+0.75f,gear4-0.75f,0,0.25f,0.75f,0.5f,0.625f,gear_r,gear_g,gear_b,0.1f);
	glEndList();
	glNewList(GEAR5_LIST,GL_COMPILE);
		gear(14,gear5,gear5+0.5f,gear5-1.0f,0,0.25f,0.75f,0.5f,0.5f,gear_r,gear_g,gear_b,0.1f);
		glRotatef(6.5,0,0,1.0f);
		glTranslatef(0,2.35f,0.625f);
		for(int i=0;i<7;i++)
			{
			glTranslatef(0,-2.35f,0);
			glRotatef(360.0f/7.0f,0,0,1.0f);
			glTranslatef(0,2.35f,0);
			screw(0.3f,0.125f,0.125f,0.1f,screw_r,screw_g,screw_b,screw_i);
			}
	glEndList();
	glNewList(GEAR6_LIST,GL_COMPILE);
		gear(12,gear6,gear6+0.75f,gear6-1.25f,0,0.25f,0.75f,0.5f,0.625f,gear_r,gear_g,gear_b,0.1f);
		glRotatef(360.0f/48+360.0f/24,0,0,1.0f);
		glTranslatef(0,gear6-0.8f,0.75f);
		screw(0.35f,0.125f,0.1f,0.1f,screw_r,screw_g,screw_b,screw_i);
		for(i=1;i<6;i++)
			{
			glTranslatef(0,-gear6+0.8f,0);
			glRotatef(360.0f/6.0f,0,0,1.0f);
			glTranslatef(0,gear6-0.8f,0);
			screw(0.35f,0.125f,0.1f,0.1f,screw_r,screw_g,screw_b,screw_i);
			}
	glEndList();
	glNewList(GEAR7_LIST,GL_COMPILE);
		gear(60,gear7,gear7+0.25f,gear7-1.0f,gear7-1.5f,0.2f,1.5f,1.375f,1.25f,gear_r,gear_g,gear_b,0.1f);
	glEndList();
	glNewList(GEAR2D1_LIST,GL_COMPILE);
		gear2d(10,screen_average*0.03f,screen_average*0.04f,screen_average*0.02f,0.1f,0.1f,0.1f);
	glEndList();
	glNewList(GEAR2D2_LIST,GL_COMPILE);
		gear2d(13,screen_average*0.05f,screen_average*0.06f,screen_average*0.035f,0.1f,0.1f,0.1f);
	glEndList();
	glNewList(GEAR2D3_LIST,GL_COMPILE);
		gear2d(8,screen_average*0.025f,screen_average*0.035f,screen_average*0.01625f,0.1f,0.1f,0.1f);
	glEndList();
	glNewList(GEAR2D4_LIST,GL_COMPILE);
		gear2d(6,screen_average*0.019f,screen_average*0.0275f,screen_average*0.01f,0.1f,0.1f,0.1f);
	glEndList();
	glNewList(GRADIENT_LIST,GL_COMPILE);
		glBlendFunc(GL_SRC_COLOR,GL_SRC_ALPHA);
		glBegin(GL_QUADS);
			glColor4f(0   ,   0,   0,1.0f); glVertex2i(screen_w,0);
			glColor4f(0.1f,   0,0.3f,1.0f); glVertex2i(       0,0);
			glColor4f(0.4f,0.5f,0.6f,1.0f); glVertex2i(       0,screen_h/2);
			glColor4f(0.6f,0.5f,0.4f,1.0f); glVertex2i(screen_w,screen_h/2);
			glColor4f(0.6f,0.5f,0.4f,1.0f); glVertex2i(screen_w,screen_h/2);
			glColor4f(0.4f,0.5f,0.6f,1.0f); glVertex2i(       0,screen_h/2);
			glColor4f(   0,   0,   0,1.0f); glVertex2i(       0,screen_h);
			glColor4f(0.3f,   0,0.1f,1.0f); glVertex2i(screen_w,screen_h);
		glEnd();
	glEndList();
	glNewList(BORDER_LIST,GL_COMPILE);
		glLoadIdentity();
		glBlendFunc(GL_SRC_COLOR,GL_SRC_ALPHA);
		glBegin(GL_QUADS);
			glColor4f(0,0,0,   0); glVertex2i(screen_w,0);
			glColor4f(0,0,0,   0); glVertex2i(       0,0);
			glColor4f(0,0,0,1.0f); glVertex2i(       0,border_h);
			glColor4f(0,0,0,1.0f); glVertex2i(screen_w,border_h);
			glColor4f(0,0,0,1.0f); glVertex2i(screen_w,screen_h-border_h);
			glColor4f(0,0,0,1.0f); glVertex2i(       0,screen_h-border_h);
			glColor4f(0,0,0,   0); glVertex2i(       0,screen_h);
			glColor4f(0,0,0,   0); glVertex2i(screen_w,screen_h);
		glEnd();
	glEndList();
	// initialize cube variable
	cube_size=0.5f;
	for(i=0;i<cube_n;i++)
		{
		radius=15.0f+(float)(rand()%50)*0.1f;
		//angle=360.0f*PID/cube_n*i;
		angle=(rand()%36000)*0.01f*PID;
		cube_position[i][0]=radius/screen_average*screen_w*cosf(angle);
		cube_position[i][1]=radius/screen_average*screen_h*sinf(angle);
		cube_position[i][2]=-8.0f-(float)(rand()%160)*0.1f;
		cube_rotation[i][0]=-1.0f+(rand()%200)*0.01f;
		cube_rotation[i][1]=-1.0f+(rand()%200)*0.01f;
		cube_rotation[i][2]=-1.0f+(rand()%200)*0.01f;
		cube_rotation[i][3]=(float)(rand()%1800)*0.1f;
		cube_color[i][0]=(40+rand()%20)*0.01f;
		cube_color[i][1]=(40+rand()%20)*0.01f;
		cube_color[i][2]=(40+rand()%20)*0.01f;
		}
	// initialize some variable
	hidden_color();
	timer=new Timer();
	return true;
	}

int DrawGLScene(void) // draw scene
	{
	// synchro
	if(!dempause)
		{
		frame_total++;
		frame_counter++;
		timer->update();
		timer_global_previous=timer_global;
		timer_global=timer->elapsed;
		timer_fps=1.0f/(timer_global-timer_global_previous);
		timer_fps_total+=timer_fps;
		timer_fps_average=timer_fps_total/frame_total;
		if(timer_fps<timer_fps_min) timer_fps_min=timer_fps;
		if(timer_fps>timer_fps_max) timer_fps_max=timer_fps;
		// compute rotation
		main_angle=100.0f*timer_global*PID;
		}
	if(timer_global-timer_global_buffer>0.01f)
		{
		timer_global_buffer=timer_global;
		timer_frame=frame_counter;
		frame_counter=0;
		// start music
		if(!mod_play&&timer_global>decrunch_time)
			{
			mod_play=true;
			if(SNG) ma_sound_start(&sound);
			}
		/*if(liner_count_i!=0)
			{
			liner_count_wait++;
			if(liner_count_wait%2==0&&liner_count_i>0)
				{
				liner_count+=liner_count_i;
				if(liner_count>liner_length) liner_count=liner_length;
				}
			}*/
		if(intro_flag)
			{
			if(intro_light<intro_length) intro_light++;
			}
		if(gear_flag)
			{
			fov=fov_base+15.0f*sinf(main_angle*0.25f);
			bgd_base_r=base_r+0.1f*cosf(main_angle*0.125f);
			bgd_base_g=base_g;
			bgd_base_b=base_b+0.1f*cosf(main_angle*0.125f);
			fog_color[0]=bgd_base_r;
			fog_color[1]=bgd_base_g;
			fog_color[2]=bgd_base_b;
			glFogfv(GL_FOG_COLOR,fog_color);
			hidden_r=(hidden_r>hidden_base_r)?hidden_r-hidden_inc:hidden_base_r;
			hidden_g=(hidden_g>hidden_base_g)?hidden_g-hidden_inc:hidden_base_g;
			hidden_b=(hidden_b>hidden_base_b)?hidden_b-hidden_inc:hidden_base_b;
			}
		if(synchro_flag)
			{
			angle=(main_angle-synchro_angle)*1.5f;
			if(angle>90.0f*PID) synchro_flag=false;
			synchro_value=1.0f-sinf(angle);
			}
		if(fade_flag)
			{
			angle=(main_angle-fade_angle)*1.25f;
			if(angle>90.0f*PID) fade_flag=false;
			fade_value=1.0f-sinf(angle);
			}
		}
	if(SNG&&mod_play)
		{
		mod_prv_pos = mod_pos;
		ma_sound_get_cursor_in_seconds(&sound, &mod_pos);
		mod_pos = round(mod_pos * 10)/10;
#if 0
			if(mod_row==0)
				{
				#if !DEBUG
					mod_ord=FMUSIC_GetOrder();
				#endif
				#if DEBUG
					if(debug_test)
						{
						mod_ord=START;
						debug_test=false;
						}
					else
						{
						mod_ord++;
						}
				#endif
				}
#endif
		if (mod_pos != mod_prv_pos) {
			if(fmodf(mod_pos, 1)==0.5f) synchro();
			if (mod_pos == 2.0f) {
				border_flag=true;
				razor_flag1=false;
				razor_flag2=false;
				razor_close=false;
				cube_flag=false;
				gradient_flag=false;
				credits_flag=false;
				logo_flag=false;
				gear_flag=false;
				liner_flag=false;
				intro_angle=0;
				decrunch_flag=false;
				border_flag=true;
				intro_flag=true;
				intro_h=0;
				txt_intro=txt_intro1;
				intro_length=(int)strlen(txt_intro);
				bgd_base_r=0;
				bgd_base_g=0;
				bgd_base_b=0;
				fog_color[0]=bgd_base_r;
				fog_color[1]=bgd_base_g;
				fog_color[2]=bgd_base_b;
				glFogfv(GL_FOG_COLOR,fog_color);
				flash();
			}
			else if(mod_pos == 6.0f) {
				intro_h=0;
				txt_intro=txt_intro2;
				intro_length=(int)strlen(txt_intro);
				flash();
			}
			else if(mod_pos == 8.5f) {
				intro_angle=main_angle;
			}
			else if(mod_pos == 9.5f) {
				intro_flag=false;
				border_flag=true;
				razor_flag1=true;
				cube_flag=true;
				bgd_base_r=0.125f;
				bgd_base_g=0.05f;
				bgd_base_b=0.075f;
				fog_color[0]=bgd_base_r;
				fog_color[1]=bgd_base_g;
				fog_color[2]=bgd_base_b;
				glFogfv(GL_FOG_COLOR,fog_color);
				razor_angle=main_angle;
				razor_angle_transition=main_angle;
			}
			else if(mod_pos == 16.4f) {
				razor_flag1=false;
				razor_flag2=true;
				razor_angle_out=main_angle;
			}
			else if(mod_pos == 16.7f) {
				razor_angle_close=main_angle;
				razor_close=true;
			}
			else if(mod_pos == 17.3f) {
				border_flag=true;
				razor_flag2=false;
				razor_close=false;
				cube_flag=false;
				gradient_flag=true;
				credits_flag=true;
				logo_flag=true;
				gear_flag=true;
				liner_flag=true;
				txt=txt_info1;
				calc_txt();
				flash();
			}
			else if(mod_pos == 32.8f) {
				txt=txt_info2;
				calc_txt();
				hidden_color();
				//flash();
			}
			else if(mod_pos == 48.0f) {
				txt=txt_logo;
				calc_txt();
				hidden_color();
				//flash();
			}
			else if(mod_pos == 63.3f) {
				txt=txt_info3;
				calc_txt();
				hidden_color();
				//flash();
			}
			else if(mod_pos == 80.5f) {
				txt=txt_info4;
				calc_txt();
				hidden_color();
				//flash();
			}
			else if(mod_pos == 96.0f) {
				txt=txt_info5;
				calc_txt();
				hidden_color();
				//flash();
			}
			else if(mod_pos == 113.0f) {
				txt=txt_info6;
				calc_txt();
				hidden_color();
				//flash();
			}
			else if(mod_pos == 144.0f) {
				txt=txt_info7;
				calc_txt();
				hidden_color();
				//flash();
			}
		}

		if((mod_pos>=28.5f && mod_pos<28.6f) ||
			(mod_pos>=44.0f && mod_pos<44.1f ) ||
			(mod_pos>=60.0f && mod_pos<60.1f ) ||
			(mod_pos>=75.0f && mod_pos<75.1f ) ||
			(mod_pos>=94.0f && mod_pos<94.1f ) ||
			(mod_pos>=110.0f && mod_pos<110.1f ||
			(mod_pos>=130.0f && mod_pos<130.1f ))) {
			fade();
		}

		if(ma_sound_at_end(&sound)) {
			ma_sound_reset_stop_time_and_fade(&sound);
			ma_sound_seek_to_second(&sound, 0);
			ma_sound_start(&sound);
		}

		}

	// clear screen and depth buffer
	init2d(screen_w,screen_h);
	glClearColor(bgd_base_r,bgd_base_g,bgd_base_b,1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_FOG);
	glDisable(GL_TEXTURE_2D);
	// draw gradient
	if(gradient_flag) glCallList(GRADIENT_LIST);
	init3d(screen_w,screen_h);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	// draw gear
	if(gear_flag)
		{
		angle=cosf(main_angle*0.25f);
		a_y=40.0f*angle;
		p_x=3.0f+4.0f*angle;
		p_y=-6.0f-1.0f*angle;
		p_z=-18.0f-1.0f*angle;
		// gear 1
		glLoadIdentity();
		glTranslatef(p_x,p_y,p_z);
		glRotatef(a_y,0.5f,1.0f,0);
		glRotatef(main_angle*(360.0f/15.0f),0,0,1.0f);
		glCallList(GEAR1_LIST);
		// gear 2
		gear_x=(float)((gear1+gear2+0.8f)*cos(0));
		gear_y=(float)((gear1+gear2+0.8f)*sin(0));
		glLoadIdentity();
		glTranslatef(p_x,p_y,p_z);
		glRotatef(a_y,0.5f,1.0f,0);
		glTranslatef(gear_x,gear_y,0);
		glRotatef(-main_angle*(360.0f/10.0f)-5.0f,0,0,1.0f);
		glCallList(GEAR2_LIST);
		// gear 3
		gear_x=(float)((gear1+gear3+0.8f)*cos(135.0f*PID));
		gear_y=(float)((gear1+gear3+0.8f)*sin(135.0f*PID));
		hidden_x=gear_x;
		hidden_y=gear_y;
		glLoadIdentity();
		glTranslatef(p_x,p_y,p_z);
		glRotatef(a_y,0.5f,1.0f,0);
		glTranslatef(gear_x,gear_y,0);
		glRotatef(-main_angle*(360.0f/30.0f),0,0,1.0f);
		glCallList(GEAR3_LIST);
		// gear 4
		gear_x=(float)((gear1+gear4+0.8f)*cos(60.0f*PID));
		gear_y=(float)((gear1+gear4+0.8f)*sin(60.0f*PID));
		glLoadIdentity();
		glTranslatef(p_x,p_y,p_z);
		glRotatef(a_y,0.5f,1.0f,0);
		glTranslatef(gear_x,gear_y,0);
		glRotatef(-main_angle*(360.0f/7.0f)+15.5f,0,0,1.0f);
		glCallList(GEAR4_LIST);
		// gear 5
		gear_x=(float)((gear3+1.625f)*cos(135.0f*PID));
		gear_y=(float)((gear3+1.625f)*sin(135.0f*PID));
		glLoadIdentity();
		glTranslatef(p_x,p_y,p_z);
		glRotatef(a_y,0.5f,1.0f,0);
		glTranslatef(gear_x,gear_y,0);
		glRotatef(-main_angle*(360.0f/14.0f)-9.0f,0,0,1.0f);
		glCallList(GEAR5_LIST);
		// gear 6
		gear_x=(float)((gear1+gear2+gear6+2.625f)*cos(60.0f*PID));
		gear_y=(float)((gear1+gear2+gear6+2.625f)*sin(60.0f*PID));
		glLoadIdentity();
		glTranslatef(p_x,p_y,p_z);
		glRotatef(a_y,0.5f,1.0f,0);
		glTranslatef(gear_x,gear_y,0);
		glRotatef(main_angle*(360.0f/12.0f)-8.0f,0,0,1.0f);
		glCallList(GEAR6_LIST);
		// gear 7
		gear_x=2.0f;
		gear_y=7.0f;
		glLoadIdentity();
		glTranslatef(p_x,p_y,p_z-8.0f);
		glRotatef(a_y,0.5f,1.0f,0);
		glTranslatef(gear_x,gear_y,0);
		glRotatef(main_angle*(360.0f/40.0f),0,0,1.0f);
		glCallList(GEAR7_LIST);
		glTranslatef(0,0,-1.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_COLOR,GL_DST_ALPHA);
		glRotatef(10.5f,0,0,1.0f);
		triforce(false,gear7*0.9f,2.0f,triforce_r,triforce_g,triforce_b,0.2f);
		triforce(true,gear7*0.9f,2.0f,triforce_r,triforce_g,triforce_b,0.2f);
		glDisable(GL_BLEND);
		}
	// draw cube
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_COLOR,GL_ONE);
	if(cube_flag)
		{
		z=razor_radius*cosf((main_angle-razor_angle)*0.2f);
		fov=fov_base-30.0f*cosf((main_angle-razor_angle)*0.25f+90.0f*PID);
		for(i=0;i<cube_n;i++)
			{
			glLoadIdentity();
			glTranslatef(cube_position[i][0],cube_position[i][1],cube_position[i][2]+z*5.0f);
			glRotatef(cube_rotation[i][3]+main_angle*20,cube_rotation[i][0],cube_rotation[i][1],cube_rotation[i][2]);
			r=cube_color[i][0]+bgd_r;
			g=cube_color[i][1]+bgd_g;
			b=cube_color[i][2]+bgd_b;
			cube(cube_size,r,g,b,0.2f,true);
			cube(cube_size,b,r,g,0.2f,false);
			}
		}
	glDisable(GL_FOG);
	glBlendFunc(GL_SRC_COLOR,GL_ONE);//GL_SRC_COLOR,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	// draw razor
	if(razor_flag1)
		{
		float tex_coord=0.5f;
		if(polygon) glBindTexture(GL_TEXTURE_2D,razor_id);
		razor_z=razor_radius*cosf((main_angle-razor_angle)*razor_zoom);
		glColor3f(razor_color,razor_color,razor_color);
		for(i=0;i<razor_nx;i++)
			{
			for(j=0;j<razor_ny;j++)
				{
				glLoadIdentity();
				angle=(main_angle-razor_angle)*80.0f-(i*20.0f+j*10.0f);
				if(angle<0) angle=0;
				if(angle>180.0f) angle=180.0f;
				glTranslatef(-((razor_nx-1)*razor_size)+razor_size*2.0f*i,razor_y-((razor_ny-1)*razor_size)+razor_size*2.0f*j,razor_z);
				glRotatef(180.0f+angle,1.0f,-1.0f,0);
				glTranslatef(0,0,0.125f);
				glBegin(GL_QUADS);
					//glColor3f(razor_color,razor_color,razor_color);
					glTexCoord2f(1.0f/razor_nx*(i+1),1.0f/razor_ny*(j+1)); glVertex3f( razor_size, razor_size,razor_width);
					glTexCoord2f(1.0f/razor_nx*i    ,1.0f/razor_ny*(j+1)); glVertex3f(-razor_size, razor_size,razor_width);
					glTexCoord2f(1.0f/razor_nx*i    ,1.0f/razor_ny*j    ); glVertex3f(-razor_size,-razor_size,razor_width);
					glTexCoord2f(1.0f/razor_nx*(i+1),1.0f/razor_ny*j    ); glVertex3f( razor_size,-razor_size,razor_width);
					//glColor3f(0.25f,razor_color*0.25f,razor_color*0.25f);
					/*
					glColor3f(0,0,0);
					glTexCoord2f(1.0f/razor_nx*(i+tex_coord),1.0f/razor_ny*(j+tex_coord)); glVertex3f( razor_size, razor_size,-razor_width);
					glTexCoord2f(1.0f/razor_nx*(i+tex_coord),1.0f/razor_ny*(j+tex_coord)); glVertex3f( razor_size,-razor_size,-razor_width);
					glTexCoord2f(1.0f/razor_nx*(i+tex_coord),1.0f/razor_ny*(j+tex_coord)); glVertex3f(-razor_size,-razor_size,-razor_width);
					glTexCoord2f(1.0f/razor_nx*(i+tex_coord),1.0f/razor_ny*(j+tex_coord)); glVertex3f(-razor_size, razor_size,-razor_width);
					glColor3f(razor_color*0.625f,razor_color*0.625f,razor_color*0.625f);
					glVertex3f(-razor_size, razor_size, razor_width);
					glVertex3f(-razor_size, razor_size,-razor_width);
					glVertex3f(-razor_size,-razor_size,-razor_width);
					glVertex3f(-razor_size,-razor_size, razor_width);
					glVertex3f( razor_size, razor_size, razor_width);
					glVertex3f( razor_size,-razor_size, razor_width);
					glVertex3f( razor_size,-razor_size,-razor_width);
					glVertex3f( razor_size, razor_size,-razor_width);
					glColor3f(razor_color*0.375f,razor_color*0.375f,razor_color*0.375f);
					glVertex3f( razor_size,-razor_size, razor_width);
					glVertex3f(-razor_size,-razor_size, razor_width);
					glVertex3f(-razor_size,-razor_size,-razor_width);
					glVertex3f( razor_size,-razor_size,-razor_width);
					glVertex3f( razor_size, razor_size, razor_width);
					glVertex3f( razor_size, razor_size,-razor_width);
					glVertex3f(-razor_size, razor_size,-razor_width);
					glVertex3f(-razor_size, razor_size, razor_width);
					*/
				glEnd();
				}
			}
		}
	if(razor_flag2)
		{
		if(polygon) glBindTexture(GL_TEXTURE_2D,razor_id);
		razor_z=razor_radius*cosf((main_angle-razor_angle)*razor_zoom);
		glLoadIdentity();
		glRotatef(5.0f,1.0f,0,0);
		glTranslatef(0,razor_y,razor_z+0.125f);
		glColor3f(razor_color,razor_color,razor_color);
		for(i=0;i<razor_nx*2;i++)
			{
			angle=(main_angle-razor_angle_out)*120;
			if(angle<0) angle=0;
			if(angle>180.0f) angle=180.0f;
			glLoadIdentity();
			glTranslatef(-((razor_nx*2-1)*razor_size*0.5f)+razor_size*1.0f*i,razor_y,razor_z+0.125f);
			glRotatef(-angle,0,1.0f,0);
			glBegin(GL_QUADS);
				glTexCoord2f(1.0f/(razor_nx*2)*(i+1),1.0f); glVertex3f( razor_size*0.5f, razor_size*razor_ny,razor_width);
				glTexCoord2f(1.0f/(razor_nx*2)*i    ,1.0f); glVertex3f(-razor_size*0.5f, razor_size*razor_ny,razor_width);
				glTexCoord2f(1.0f/(razor_nx*2)*i    ,   0); glVertex3f(-razor_size*0.5f,-razor_size*razor_ny,razor_width);
				glTexCoord2f(1.0f/(razor_nx*2)*(i+1),   0); glVertex3f( razor_size*0.5f,-razor_size*razor_ny,razor_width);
			glEnd();
			}
		}
	if(polygon) glBindTexture(GL_TEXTURE_2D,font_id);
	glBlendFunc(GL_ONE,GL_ONE);
	// draw hidden
	if(gear_flag)
		{
		for(i=0;i<hidden_length;i++)
			{
			car=txt_hidden[hidden_type][i];
			if(car!=32)
				{
				glLoadIdentity();
				glTranslatef(p_x,p_y,p_z);
				glRotatef(a_y,0.5f,1.0f,0);
				glTranslatef(hidden_x,hidden_y,0);
				glRotatef(-main_angle*(360.0f/30.0f)+i*(360.0f/hidden_length),0,0,1.0f);
				glTranslatef(0,8.9f,0.65f);
				scale=1.0f+synchro_value*cosf(i*1.0f+synchro_value*8.0f)*0.5f;
				glScalef(1.0f,scale,1.0f);
				glBegin(GL_QUADS);
				l_w=(car%16)/16.0f;
				l_h=(car-car%16)/256.0f;
				for(j=0;j<hidden_n;j++)
					{
					if(j<hidden_n-1)
						{
						c=0.15f;//0.5f-0.5f/hidden_n*j;
						r=gear_r*c*fade_value;
						g=gear_g*c*fade_value;
						b=gear_b*c*fade_value;
						}
					else
						{
						c=1.15f;
						r=hidden_r*c*fade_value;
						g=hidden_g*c*fade_value;
						b=hidden_b*c*fade_value;
						}
					glColor3f(r      ,g      ,b      ); glTexCoord2f(l_w+0.0620f,-l_h-0.0620f ); glVertex3f(-hidden_w*0.5f-hidden_m*0.5f, hidden_h*0.5f,j*hidden_z);
					glColor3f(g*0.25f,g*0.25f,g*0.25f); glTexCoord2f(l_w+0.0620f,-l_h-0.03125f); glVertex3f(-hidden_w*0.5f+hidden_m     ,	            0,j*hidden_z);
					glColor3f(r*0.25f,g*0.25f,b*0.25f); glTexCoord2f(l_w+0.0005f,-l_h-0.03125f); glVertex3f( hidden_w*0.5f-hidden_m     ,             0,j*hidden_z);
					glColor3f(r      ,g      ,b      ); glTexCoord2f(l_w+0.0005f,-l_h-0.0620f ); glVertex3f( hidden_w*0.5f+hidden_m*0.5f, hidden_h*0.5f,j*hidden_z);
					glColor3f(g*0.25f,g*0.25f,g*0.25f); glTexCoord2f(l_w+0.0620f,-l_h-0.03125f); glVertex3f(-hidden_w*0.5f-hidden_m     ,             0,j*hidden_z);
					glColor3f(g      ,g      ,b      ); glTexCoord2f(l_w+0.0620f,-l_h-0.0005f ); glVertex3f(-hidden_w*0.5f+hidden_m*0.5f,-hidden_h*0.5f,j*hidden_z);
					glColor3f(g      ,g      ,b      ); glTexCoord2f(l_w+0.0005f,-l_h-0.0005f ); glVertex3f( hidden_w*0.5f-hidden_m*0.5f,-hidden_h*0.5f,j*hidden_z);
					glColor3f(g*0.25f,g*0.25f,b*0.25f); glTexCoord2f(l_w+0.0005f,-l_h-0.03125f); glVertex3f( hidden_w*0.5f+hidden_m     ,             0,j*hidden_z);
					}
				glEnd();
				}
			}
		}
	// draw liner
	if(liner_flag)
		{
		liner_line=-1;
		liner_count=(int)((main_angle-liner_angle)*20.0f)-10;
		if(liner_count>liner_length) liner_count=liner_length;
		for(i=0;i<liner_count;i++)
			{
			angle=main_angle+liner_i*0.02f;
			car=txt[i];
			liner_r=0.5f;
			liner_g=0.4f;
			liner_b=0.3f;
			if(car<32)
				{
				liner_r=1.0f;
				liner_g=0.375f;
				liner_b=0.375f;
				}
			if((car>47&&car<58)||(car>64&&car<91)||(car>96&&car<123))
				{
				liner_r=1.0f;
				liner_g=1.0f;
				liner_b=1.0f;
				}
			if(car>160||i==liner_count)
				{
				liner_r=0.375f+0.625f*cosf(angle*2.0f);
				liner_g=0.5f;
				liner_b=0.375f+0.625f*sinf(angle*2.0f);
				}
			if(car!=13)
				{
				// compute position
				p_x+=liner_size*1.75f;
				if(car!=32)
					{
					liner_color=0.5f-(liner_count-i)*(0.02f); if(liner_color<0.08f) liner_color=0.08f;
					glLoadIdentity();
					glRotatef(-25.0f,1.0f,1.0f,0);
					glTranslatef(-2.5f,2.25f,1.5f);
					glTranslatef(p_x,p_y,liner_z+1.0f*sinf(angle));
					a_z=30.0f*cosf(angle);
					glRotatef(a_z,0,-1.0f,0);
					l_w=(car%16)/16.0f;
					l_h=(car-car%16)/256.0f;
					glBegin(GL_QUADS);
						glColor3f(liner_color*fade_value,liner_color*0.625f*fade_value,0.075f*fade_value);
						for(j=0;j<10;j++)
							{
							glTexCoord2f(l_w+0.0620f,-l_h-0.0005f); glVertex3f( liner_size, liner_size+liner_size,-0.15f+j*0.025f);
							glTexCoord2f(l_w+0.0005f,-l_h-0.0005f); glVertex3f(-liner_size, liner_size+liner_size,-0.15f+j*0.025f);
							glTexCoord2f(l_w+0.0005f,-l_h-0.0620f); glVertex3f(-liner_size,-liner_size-liner_size,-0.15f+j*0.025f);
							glTexCoord2f(l_w+0.0620f,-l_h-0.0620f); glVertex3f( liner_size,-liner_size-liner_size,-0.15f+j*0.025f);
							}
						glColor3f(liner_r*fade_value,liner_g*fade_value,liner_b*fade_value);
						glTexCoord2f(l_w+0.0620f,-l_h-0.0005f); glVertex3f( liner_size, liner_size+liner_size,0.15f);
						glTexCoord2f(l_w+0.0005f,-l_h-0.0005f); glVertex3f(-liner_size, liner_size+liner_size,0.15f);
						glTexCoord2f(l_w+0.0005f,-l_h-0.0620f); glVertex3f(-liner_size,-liner_size-liner_size,0.15f);
						glTexCoord2f(l_w+0.0620f,-l_h-0.0620f); glVertex3f( liner_size,-liner_size-liner_size,0.15f);
					glEnd();
					}
				liner_i+=5;
				}
			else
				{
				liner_line++;
				liner_i=0;
				p_x=-(liner_max*liner_size*1.75f)*0.5f;
				p_y=(l_n*liner_size*1.75f)-liner_line*liner_size*3.5f;
				}
			}
		}
	glDisable(GL_TEXTURE_2D);
	// draw intro
	if(intro_flag)
		{
		glBlendFunc(GL_ONE,GL_ONE);
		glEnable(GL_TEXTURE_2D);
		x=intro_x-intro_length*intro_size*1.75f*0.5f;
		if(intro_angle>0)
			{
			angle=(main_angle-intro_angle)*1.25f;
			if(angle>90.0f*PID) angle=90.0f*PID;
			intro_h=intro_size*sinf(angle);
			}
		for(i=0;i<intro_length;i++)
			{
			angle=main_angle*1.5f+i*0.2f;
			y=intro_size*2.825f+intro_size*2.0f*cosf(angle);
			z=(intro_size-intro_h)*synchro_value*cosf(i*1.0f+synchro_value*8.0f)*0.375f;
			for(j=0;j<24;j++)
				{
				c=0.0625f;
				car=txt_intro[i];
				switch(car)
					{
					case 2:
						intro_r=0.375f;
						intro_g=0.5f;
						intro_b=0.625f;
						break;
					case 3:
						intro_r=1.0f;
						intro_g=0.25f;
						intro_b=0.25f;
						break;
					default:
						intro_r=0.5f+0.25f*cosf(angle*2.5f);
						intro_g=0.5f;
						intro_b=0.5f+0.25f*sinf(angle*2.5f);
					}
				if(i==intro_light-1&&i>0&&i<intro_length-1)
					{
					intro_r+=0.25f;
					intro_g+=0.25f;
					intro_b+=0.25f;
					}
				if(i==intro_light&&i>0&&i<intro_length-1)
					{
					intro_r+=0.5f;
					intro_g+=0.5f;
					intro_b+=0.5f;
					}
				if(j!=0)
					{
					intro_r*=c;
					intro_g*=c;
					intro_b*=c;
					}
				l_w=(car%16)/16.0f;
				l_h=(car-car%16)/256.0f;
				glLoadIdentity();
				glColor3f(intro_r,intro_g,intro_b);
				glRotatef(45.0f,1.0f,1.0f,0);
				glTranslatef(x+i*intro_size*1.75f,intro_y+y,intro_z+(12-j)*intro_size*0.125f);
				glBegin(GL_QUADS);
					glTexCoord2f(l_w+0.0610f,-l_h-0.0015f); glVertex3f( intro_size, intro_size-z-intro_h,0);
					glTexCoord2f(l_w+0.0015f,-l_h-0.0015f); glVertex3f(-intro_size, intro_size-z-intro_h,0);
					glTexCoord2f(l_w+0.0015f,-l_h-0.0610f); glVertex3f(-intro_size,-intro_size+z+intro_h,0);
					glTexCoord2f(l_w+0.0610f,-l_h-0.0610f); glVertex3f( intro_size,-intro_size+z+intro_h,0);
				glEnd();
				glLoadIdentity();
				glColor3f(intro_r*0.375f,intro_g*0.375f,intro_b*0.375f);
				glRotatef(45.0f,1.0f,1.0f,0);
				glTranslatef(x+i*intro_size*1.75f,intro_y-y,intro_z+(12-j)*intro_size*0.125f);
				glBegin(GL_QUADS);
					glTexCoord2f(l_w+0.0610f,-l_h-0.0610f); glVertex3f( intro_size, intro_size-z-intro_h,0);
					glTexCoord2f(l_w+0.0015f,-l_h-0.0610f); glVertex3f(-intro_size, intro_size-z-intro_h,0);
					glTexCoord2f(l_w+0.0015f,-l_h-0.0015f); glVertex3f(-intro_size,-intro_size+z+intro_h,0);
					glTexCoord2f(l_w+0.0610f,-l_h-0.0015f); glVertex3f( intro_size,-intro_size+z+intro_h,0);
				glEnd();
				}
			}
		glDisable(GL_TEXTURE_2D);
		int intro_triforce_n=24;
		radius=intro_size*32.0f;
		for(i=0;i<intro_triforce_n;i++)
			{
			angle=main_angle*0.125f+360.0f*PID/intro_triforce_n*i;
			x=radius*cosf(angle);
			y=synchro_value*cosf(i*4.0f+synchro_value*8.0f)*0.05f;
			z=radius*sinf(angle);
			r=(0.1f+0.05f*sinf(angle))/(radius*1.25f)*(radius*1.25f+z);
			g=0.1f/(radius*1.5f)*(radius*1.5f+z);
			b=(0.1f+0.05f*cosf(angle))/(radius*1.25f)*(radius*1.25f+z);
			r-=r/intro_size*intro_h;
			g-=g/intro_size*intro_h;
			b-=b/intro_size*intro_h;
			glLoadIdentity();
			glRotatef(45.0f,1.0f,1.0f,0);
			glTranslatef(intro_x+x,(float)(intro_y+intro_size*1.25f+fabs(y)),intro_z+z-radius*0.25f);
			glRotatef(-main_angle*40.0f,0,1.0f,0);
			glRotatef(y*200.0f,0,0,1.0f);
			glTranslatef(0,0,-intro_size*0.125f);
			triforce(true,intro_size*2.0f,intro_size*0.25f,r,g,b,0.1f);
			triforce(false,intro_size*2.0f,intro_size*0.25f,r,g,b,0.1f);
			glLoadIdentity();
			glRotatef(45.0f,1.0f,1.0f,0);
			glTranslatef(intro_x+x,intro_y-intro_size*1.25f-y,intro_z+z-radius*0.25f);
			glRotatef(-180,1.0f,0,0);
			glRotatef(-main_angle*40.0f,0,-1.0f,0);
			glRotatef(y*200.0f,0,0,1.0f);
			glTranslatef(0,0,-intro_size*0.125f);
			triforce(true,intro_size*2.0f,intro_size*0.25f,r*0.375f,g*0.375f,b*0.375f,0.0375f);
			triforce(false,intro_size*2.0f,intro_size*0.25f,r*0.375f,g*0.375f,b*0.375f,0.0375f);
			}
		}
	init2d(screen_w,screen_h);
	// draw decrunch
	if(decrunch_flag)
		{
		glLoadIdentity();
		glBlendFunc(GL_SRC_COLOR,GL_DST_COLOR);
		glBegin(GL_QUADS);
		glColor3f((float)(CR*(144+rand()%112)),(float)(CR*(144+rand()%112)),(float)(CR*(144+rand()%112)));
		for(i=0;i<screen_h;i++)
			{
			decrunch_y=i*decrunch_h;
			decrunch_split=rand()%4;
			if(decrunch_split==0)
				{
				if(rand()%4==0) glColor3f((float)(CR*(144+rand()%112)),(float)(CR*(144+rand()%112)),(float)(CR*(144+rand()%112)));
				decrunch_split_w=rand()%(screen_w/16)*16;
				glVertex2i(decrunch_split_w,decrunch_y-decrunch_h);
				glVertex2i(0               ,decrunch_y-decrunch_h);
				glVertex2i(0               ,decrunch_y);
				glVertex2i(decrunch_split_w,decrunch_y);
				if(rand()%8==0) glColor3f((float)(CR*(144+rand()%112)),(float)(CR*(144+rand()%112)),(float)(CR*(144+rand()%112)));
				glVertex2i(screen_w        ,decrunch_y-decrunch_h);
				glVertex2i(decrunch_split_w,decrunch_y-decrunch_h);
				glVertex2i(decrunch_split_w,decrunch_y);
				glVertex2i(screen_w        ,decrunch_y);
				}
			else
				{
				glVertex2i(screen_w        ,decrunch_y-decrunch_h);
				glVertex2i(0               ,decrunch_y-decrunch_h);
				glVertex2i(0               ,decrunch_y);
				glVertex2i(screen_w        ,decrunch_y);
				}
			}
		glEnd();	
		}
	// draw border
	if(border_flag) glCallList(BORDER_LIST);
	// draw credits
	if(credits_flag)
		{
		credits_w=screen_average*0.025f;
		credits_x=(float)fmod(main_angle*credits_w,credits_w);
		int n=(int)(screen_w/credits_w+2);
		glBlendFunc(GL_ONE,GL_ONE);
		glColor3f(0.1f,0.1f,0.1f);
		glLoadIdentity();
		glTranslatef(0,credits_y+screen_average*0.06325f,0);
		glBegin(GL_QUADS);
			glVertex2f((float)screen_w,0);
			glVertex2f(              0,0);
			glVertex2f(              0,screen_average*0.01f);
			glVertex2f((float)screen_w,screen_average*0.01f);
		glEnd();
		glLoadIdentity();
		glTranslatef(credits_x-credits_w-credits_w-screen_average*0.000625f,screen_average*0.01f,0);
		for(i=0;i<n;i++)
			{
			glTranslatef(credits_w,0,0);
			glBegin(GL_QUADS);
				glVertex2f( screen_average*0.007f,0);
				glVertex2f(-screen_average*0.007f,0);
				glVertex2f(-screen_average*0.004f,screen_average*0.01f);
				glVertex2f( screen_average*0.004f,screen_average*0.01f);
			glEnd();
			}
		glLoadIdentity();
		glTranslatef(screen_w-screen_average*0.0525f,screen_average*0.0525f,0);
		glRotatef(main_angle*(360.0f/10.0f)-11.0f,0,0,1.0f);
		glCallList(GEAR2D1_LIST);
		// ---
		credits_y=screen_h-screen_average*0.07125f;
		glLoadIdentity();
		glBegin(GL_QUADS);
			glVertex2f((float)screen_w,0);
			glVertex2f(              0,0);
			glVertex2f(              0,screen_average*0.01f);
			glVertex2f((float)screen_w,screen_average*0.01f);
		glEnd();
		glLoadIdentity();
		glTranslatef(-credits_x-credits_w+screen_average*0.00475f,credits_y+screen_average*0.05325f,0);
		for(i=0;i<n;i++)
			{
			glTranslatef(credits_w,0,0);
			glBegin(GL_QUADS);
				glVertex2f( screen_average*0.004f,0);
				glVertex2f(-screen_average*0.004f,0);
				glVertex2f(-screen_average*0.007f,screen_average*0.01f);
				glVertex2f( screen_average*0.007f,screen_average*0.01f);
			glEnd();
			}
		glLoadIdentity();
		glTranslatef(screen_average*0.075f,credits_y,0);
		glRotatef(main_angle*(360.0f/13.0f)+11.5f,0,0,1.0f);
		glCallList(GEAR2D2_LIST);
		glLoadIdentity();
		glTranslatef(screen_average*0.075f+screen_average*0.0775f,credits_y-screen_average*0.0405f,0);
		glRotatef(-main_angle*(360.0f/8.0f)-8.0f,0,0,1.0f);
		glCallList(GEAR2D3_LIST);
		glLoadIdentity();
		glTranslatef(screen_average*0.05f,screen_average*0.04f,0);
		glRotatef(main_angle*(360.0f/6.0f)-9.5f,0,0,1.0f);
		glCallList(GEAR2D4_LIST);
		// credits text
		glEnable(GL_TEXTURE_2D);
		int txt_length=(int)strlen(txt_credits[0]);
		glColor3f(0.3f,0.25f,0.2f);
		for(i=0;i<txt_length;i++)
			{
			glLoadIdentity();
			glTranslatef(screen_average*0.075f,credits_y,0);
			glRotatef(main_angle*(360.0f/12.0f)-i*(360.0f/txt_length),0,0,1.0f);
			glTranslatef(0,screen_average*0.0425f,0);
			draw_char(txt_credits[0][i],screen_average*0.012f,screen_average*0.0125f,-screen_average*0.000625f);
			}
		credits_color=0.625f+0.75f*cosf(main_angle); if(credits_color>0.625f) credits_color=0.625f;
		if(credits_color<0)
			{
			if(credits_change)
				{
				credits_n+=3; if(credits_n>10) credits_n=1;
				credits_change=false;
				}
			credits_color=0;
			}
		else
			{
			credits_change=true;
			}
		// credits text 1
		credits_length=(int)strlen(txt_credits[credits_n]);
		glColor3f(credits_color*0.75f,credits_color*0.375f,credits_color*0.5f);
		for(i=0;i<credits_length;i++)
			{
			glLoadIdentity();
			glTranslatef(screen_average*0.075f-((credits_length-1)*screen_average*0.007f)*0.5f+i*screen_average*0.007f,credits_y-screen_average*0.015f,0);
			draw_char(txt_credits[credits_n][i],screen_average*0.0075f,screen_average*0.0125f,0);
			}
		// credits text 2
		credits_length=(int)strlen(txt_credits[credits_n+1]);
		glColor3f(credits_color,credits_color,credits_color);
		for(i=0;i<credits_length;i++)
			{
			glLoadIdentity();
			glTranslatef(screen_average*0.075f-((credits_length-1)*screen_average*0.013f)*0.5f+i*screen_average*0.013f,credits_y,0);
			draw_char(txt_credits[credits_n+1][i],screen_average*0.015f,screen_average*0.02f,screen_average*0.001f);
			}
		// credits text 3
		credits_length=(int)strlen(txt_credits[credits_n+2]);
		for(i=0;i<credits_length;i++)
			{
			glLoadIdentity();
			glTranslatef(screen_average*0.075f-((credits_length-1)*screen_average*0.007f)*0.5f+i*screen_average*0.007f,credits_y+screen_average*0.013f,0);
			draw_char(txt_credits[credits_n+2][i],screen_average*0.0075f,screen_average*0.0125f,0);
			}
		// draw crack
		if(crack_flag)
			{
			crack_w=screen_average*0.02f;
			crack_h=screen_average*0.04f;
			crack_px=crack_w*0.85f;
			crack_n=(int)(screen_w/crack_px)+2;
			x=crack_x;
			crack_x=(float)fmod(main_angle*0.125f*screen_average,crack_px);
			if(crack_counter<-crack_n-4) crack_counter=-crack_n-4;
			if(crack_x-x<0.0f) crack_counter++; if(crack_counter>crack_length) crack_counter=-crack_n;
			for(i=0;i<crack_n;i++)
				{
				car=(crack_counter+i<0||crack_counter+i>crack_length)?32:txt_crack[crack_counter+i];
				if(car!=32)
					{
					r=0.375f;
					g=0.375f;
					b=0.375f;
					if(car<32)
						{
						r=0.5f;
						g=0.2f;
						b=0.1f;
						}
					if((car>47&&car<58)||(car>64&&car<91)||(car>96&&car<123))
						{
						r=0.25f;
						g=0.375f;
						b=0.5f;
						}
					float mul=(i-8)*0.2f; if(mul>1.0f) mul=1.0f;
					r*=mul;
					g*=mul;
					b*=mul;
					l_w=(car%16)/16.0f;
					l_h=(car-car%16)/256.0f;
					angle=main_angle*2.0f+(float)((crack_counter+i)*PID*8.0f);
					y=credits_y+screen_average*0.02f-screen_average*0.02f*cosf(angle);//-screen_average*0.005f*synchro_value*cosf(i*1.0f+synchro_value*8.0f);
					scale=0.5f+0.125f*cosf(angle);//+0.25f*synchro_value*cosf(i*0.2f+synchro_value*8.0f);
					glLoadIdentity();
					glTranslatef(-crack_x+i*crack_px+screen_average*0.01f*sinf(angle),y,0);
					glRotatef(9.0f*sinf(angle),0,0,1.0f);
					glBegin(GL_QUADS);
					/*glColor3f(r*0.1f,g*0.1f,b*0.1f); glTexCoord2f(l_w+0.0620f,-l_h-0.0620f); glVertex2f( crack_w*0.5f, crack_h*scale);
						glColor3f(r     ,g     ,b     ); glTexCoord2f(l_w+0.0620f,-l_h-0.0005f); glVertex2f( crack_w*0.5f,-crack_h*scale);
						glColor3f(r     ,g     ,b     ); glTexCoord2f(l_w+0.0005f,-l_h-0.0005f); glVertex2f(-crack_w*0.5f,-crack_h*scale);
						glColor3f(r*0.1f,g*0.1f,b*0.1f); glTexCoord2f(l_w+0.0005f,-l_h-0.0620f); glVertex2f(-crack_w*0.5f, crack_h*scale);*/
						glColor3f(g     ,g     ,g     ); glTexCoord2f(l_w+0.0620f,-l_h-0.0620f ); glVertex2f( crack_w*0.5f, crack_h*scale);
						glColor3f(r*0.2f,g*0.2f,b*0.2f); glTexCoord2f(l_w+0.0620f,-l_h-0.03125f); glVertex2f( crack_w*0.5f,             0);
						glColor3f(r*0.2f,g*0.2f,b*0.2f); glTexCoord2f(l_w+0.0005f,-l_h-0.03125f); glVertex2f(-crack_w*0.5f,             0);
						glColor3f(0     ,0     ,0     ); glTexCoord2f(l_w+0.0005f,-l_h-0.0620f ); glVertex2f(-crack_w*0.5f, crack_h*scale);
						glColor3f(g,g,b);
						glColor3f(r*0.2f,g*0.2f,b*0.2f); glTexCoord2f(l_w+0.0620f,-l_h-0.03125f); glVertex2f( crack_w*0.5f,             0);
						glColor3f(r*2.0f,g*2.0f,b*2.0f); glTexCoord2f(l_w+0.0620f,-l_h-0.0005f ); glVertex2f( crack_w*0.5f,-crack_h*scale);
						glColor3f(r     ,g     ,b     ); glTexCoord2f(l_w+0.0005f,-l_h-0.0005f ); glVertex2f(-crack_w*0.5f,-crack_h*scale);
						glColor3f(r*0.2f,g*0.2f,b*0.2f); glTexCoord2f(l_w+0.0005f,-l_h-0.03125f); glVertex2f(-crack_w*0.5f,             0);
					glEnd();
					}
				}
			}
		glDisable(GL_TEXTURE_2D);
		}
	// draw transition
	if(razor_flag1)
		{
		glBlendFunc(GL_SRC_COLOR,GL_SRC_ALPHA);
		transition_w=screen_w/transition_n;
		angle=(main_angle-razor_angle_transition)*2.0f;
		if(angle>30.0f*PID)
			{
			angle=0;
			transition_i++;
			razor_angle_transition=main_angle;
			}
		glLoadIdentity();
		glTranslatef(-(float)transition_w,0,0);
		for(i=0;i<transition_n;i++)
			{
			if(i<transition_i) y=1.0f;
			if(i==transition_i-2) y=1.0f-cosf(angle+60.0f*PID);
			if(i==transition_i-1) y=1.0f-cosf(angle+30.0f*PID);
			if(i==transition_i) y=1.0f-cosf(angle);
			if(i>transition_i) y=0;
			glColor4f(0,0,0,y);
			glTranslatef((float)transition_w,0,0);
			glBegin(GL_QUADS);
				glVertex2i(transition_w,       0);
				glVertex2i(           0,       0);
				glVertex2i(           0,screen_h);
				glVertex2i(transition_w,screen_h);
			glEnd();
			}
		}
	// draw transition
	if(razor_close)
		{
		glBlendFunc(GL_SRC_COLOR,GL_SRC_ALPHA);
		angle=(main_angle-razor_angle_close)*4.0f;
		if(angle>90.0f*PID) angle=90.0f*PID;
		y=(screen_h/2)*sinf(angle);
		c=1.0f/(screen_h/2)*y;
		glColor3f(c,c,c);
		glLoadIdentity();
		glTranslatef(0,-(screen_h/2)+y,0);
		glBegin(GL_QUADS);
			glVertex2i(screen_w,         0);
			glVertex2i(       0,         0);
			glVertex2i(       0,screen_h/2);
			glVertex2i(screen_w,screen_h/2);
		glEnd();
		glLoadIdentity();
		glTranslatef(0,screen_h-y,0);
		glBegin(GL_QUADS);
			glVertex2i(screen_w,         0);
			glVertex2i(       0,         0);
			glVertex2i(       0,screen_h/2);
			glVertex2i(screen_w,screen_h/2);
		glEnd();
		}
	// draw flash
	if(flash_flag)
		{
		glBlendFunc(GL_ONE,GL_ONE);
		angle=(main_angle-flash_angle)*2.0f;
		if(angle>90.0f*PID) flash_flag=false;
		c=1.0f-sinf(angle);
		glColor3f(c,c,c);
		glLoadIdentity();
		glBegin(GL_QUADS);
			glVertex2i(screen_w,       0);
			glVertex2i(       0,       0);
			glVertex2i(       0,screen_h);
			glVertex2i(screen_w,screen_h);
		glEnd();
		}
	glEnable(GL_TEXTURE_2D);
	// draw logo
	if(logo_flag)
		{
		glLoadIdentity();
		if(polygon) glBindTexture(GL_TEXTURE_2D,logo_id);
		glBlendFunc(GL_SRC_COLOR,GL_ONE);
		glTranslatef((float)(screen_w-logo_w-logo_margin),(float)(screen_h-logo_h-logo_margin),0);
		glColor3f(1.0f,1.0f,1.0f);
		glBegin(GL_QUADS);
			glTexCoord2f(1.0f,1.0f); glVertex2i(logo_w,     0);
			glTexCoord2f(   0,1.0f); glVertex2i(     0,     0);
			glTexCoord2f(   0,   0); glVertex2i(     0,logo_h);
			glTexCoord2f(1.0f,   0); glVertex2i(logo_w,logo_h);
		glEnd();
		}
	// draw debug
	if(debug_flag)
		{
		float debug_w=screen_average*0.0125f;
		float debug_h=screen_average*0.025f;
		if(polygon) glBindTexture(GL_TEXTURE_2D,font_id);
		glBlendFunc(GL_ONE,GL_ONE);
		glColor3f(0.25f,0.225f,0.2f);
		char debug[192];
		sprintf(debug,"*** DEBUG ***\nscreen=%d*%d\nfps=%3.1f\naverage=%3.1f\nfps min=%3.1f\nfps max=%3.1f\n%d frame(s)\nmusic=%.1fs",screen_w,screen_h,timer_fps,timer_fps_average,timer_fps_min,timer_fps_max,timer_frame,mod_pos);
		glLoadIdentity();
		glTranslatef(0,debug_h*0.65f,0);
		j=0;
		for(i=0;i<(int)strlen(debug);i++)
			{
			j++;
			car=debug[i];
			if(car==10)
				{
				glTranslatef(-j*debug_w*0.85f,debug_h*0.85f,0);
				j=0;
				}
			glTranslatef(debug_w*0.85f,0,0);
			draw_char(car,debug_w,debug_h,0);
			}
		}
	// draw scanline
	if(scanline_flag)
		{
		glLoadIdentity();
		if(polygon) glBindTexture(GL_TEXTURE_2D,scanline_id);
		glBlendFunc(GL_DST_COLOR,GL_SRC_ALPHA);
		glColor4f(1.0f,1.0f,1.0f,0.75f);
		glBegin(GL_QUADS);
			glTexCoord2f(1.0f,                     0); glVertex2i(screen_w,       0);
			glTexCoord2f(   0,                     0); glVertex2i(       0,       0);
			glTexCoord2f(   0,(float)(screen_h*0.5f)); glVertex2i(       0,screen_h);
			glTexCoord2f(1.0f,(float)(screen_h*0.5f)); glVertex2i(screen_w,screen_h);
		glEnd();
		}
	return true;
	}

void window_close_callback(GLFWwindow* window)
{
	done=true;
}

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			done=true;
			break;
		case GLFW_KEY_F1:
			debug_flag=!debug_flag;
			break;
		case GLFW_KEY_F2:
			hidden_flag=!hidden_flag;
			hidden_color();
			break;
		case GLFW_KEY_F3:
			polygon=!polygon;
			glPolygonMode(GL_FRONT,polygon?GL_FILL:GL_LINE);
			//if(polygon) glDisable(GL_LINE_SMOOTH); else glEnable(GL_LINE_SMOOTH);
			break;
		#if DEBUG
		case GLFW_KEY_BACKSPACE:
			dempause=!dempause;
			break;
		case GLFW_KEY_TAB:
			synchro();
			break;
		#endif
		case GLFW_KEY_ENTER:
			#if DEBUG
				flash();
				hidden_color();
				calc_txt();
				razor_color=1.0f;
				razor_radius=0;
			#endif
			frame_total=0;
			timer_fps_total=0;
			timer_fps_min=32768;
			timer_fps_max=0;
			break;
		}
	}
}
	
void KillGLWindow(void)							// kill window
{
	if (window)
		glfwDestroyWindow(window);

	glfwTerminate();
	delete timer;
}

int CreateGLWindow(const char* title)
	{
	GLFWmonitor* primary = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primary);
	GLuint PixelFormat;												// pixel format result
	int w=mode->width;
	int h=mode->height;
	screen_w=fullscreen?w:window_w;
	screen_h=fullscreen?h:window_h;
	screen_average=(screen_w+screen_h)/2;
	timer_fps_min=32768;
	timer_fps_max=0;
	border_h=(int)(screen_h*0.125f);
	decrunch_h=(int)(screen_h*0.01f);

	glfwWindowHint(GLFW_DEPTH_BITS, window_color);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	
	// create window
	if (fullscreen) {;
		window = glfwCreateWindow(screen_w, screen_h, title, primary, NULL);
	} else
		window = glfwCreateWindow(screen_w, screen_h, title, NULL, NULL);

	if (!window)
		return false;

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSetWindowCloseCallback(window, window_close_callback);
	glfwSetKeyCallback(window, key_callback);
	
	glfwMakeContextCurrent(window);
	
	init3d(screen_w,screen_h);	// set up perspective of GL screen
	if(!InitGL())								// initialize GL window
	{
		KillGLWindow();
		return false;
	}
	return true;
}

// instance,previous instance,command line parameters,window show state
int main(int argc, char *argv[]) {
	done=false;																	// exit loop

	glfwSetErrorCallback(error_callback);
	
	if (!glfwInit())
		return 0;

#if !DEBUG
	// ask for fullscreen mode
	if (isatty(STDOUT_FILENO)) {
		printf("Run in fullscreen mode ? (y/n) ");
		(getchar() == 121) ? fullscreen=true : fullscreen=false;
	}
#endif

	// create openGL window
	if(!CreateGLWindow(name)) return 0;					// quit if window not created
	// load and play music
	if(SNG) load_song();
	// main loop
	while(!done) {
		// draw the scene, watch for escape key and quit message from DrawGLScene()
		if((active&&!DrawGLScene())) done=true; else glfwSwapBuffers(window);	// exit or swap buffers
		glfwPollEvents();
	}
	// shutdown
	if(SNG) {
		ma_sound_stop(&sound);
		ma_sound_uninit(&sound);
		ma_engine_uninit(&engine);
	}
	KillGLWindow();								// kill the window
	return 0;					// exit the program
}
