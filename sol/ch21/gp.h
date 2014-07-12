/////////////////////////////////////////////////////////////////////
// Constants
//

// Screen pitch
#define PITCH (gScreen->pitch / 4)
// Screen width
#define WIDTH 480
// Screen height
#define HEIGHT 320
// Color of player's ball
#define BALLCOLOR 0x007fff
// Radius of the player's ball
#define RADIUS 12
// Background color
#define BGCOLOR 0x5f0000
// Definition of PI
#define PI 3.1415926535897932384626433832795f

// Width of level data
#define LEVELWIDTH 15
// Height of level data
#define LEVELHEIGHT 10
// Fall color
#define FALLCOLOR 0x000000
// Tile size (width and height)
#define TILESIZE 32

// How many seconds does the player have to solve the level
#define TIMELIMIT 10

// Player's thrust value
#define THRUST 0.1f
// Sliding tile thrust power
#define SLIDEPOWER 0.075f
// Slowdown due to friction, etc.
#define SLOWDOWN 0.99f
// Slowdown due to collision
#define COLLISIONSLOWDOWN 0.9f

// Color of the start tile
#define STARTCOLOR 0x001f7f
// Color of the goal tile
#define ENDCOLOR 0x3f7f1f
// Radius of a collectible item
#define COLLECTIBLERADIUS 8
// Color of a collectible item
#define COLLECTIBLECOLOR 0xffff00

// Physics iterations per second
#define PHYSICSFPS 100

/////////////////////////////////////////////////////////////////////
// Enumerations
//

// Tile values
enum leveldataenum
{
  LEVEL_DROP = 0,
  LEVEL_GROUND = 1,
  LEVEL_START = 2,
  LEVEL_END = 3,
  LEVEL_COLLECTIBLE = 5,
  LEVEL_LEFT = 10,
  LEVEL_RIGHT = 11,
  LEVEL_UP = 12,
  LEVEL_DOWN = 13
};

/////////////////////////////////////////////////////////////////////
// Structures
//

struct collectible
{
  float mX;
  float mY;
  int mColor;
  int mRadius;
  int mTaken;
};

/////////////////////////////////////////////////////////////////////
// Globals
//

// Screen surface
extern SDL_Surface *gScreen;
// Surface that contains the tiles
extern SDL_Surface *gTiles;
// Surface that contains the font
extern SDL_Surface *gFont;

// Pointer to the joystick interface
extern SDL_Joystick *gJoystick;

// Total number of collectibles
extern int gCollectibleCount;
// Number of collectibles taken
extern int gCollectiblesTaken;
// Array of collectibles
extern collectible *gCollectible;

// Last iteration's tick value
extern int gLastTick;
// Level start tick
extern int gLevelStartTick;
// Current level
extern int gCurrentLevel;

// Player's position
extern float gXPos;
extern float gYPos;

// Player's motion vector
extern float gXMov;
extern float gYMov;

// Player's start position
extern float gStartX;
extern float gStartY;

// Player's key status
extern int gKeyLeft;
extern int gKeyRight;
extern int gKeyUp;
extern int gKeyDown;

// Player's score
extern int gScore;

// Level data
extern unsigned char gLevel[];  

// Offset of character data per character
extern int gFontOfs[];
// Width of character data per character
extern int gFontWidth[];

/////////////////////////////////////////////////////////////////////
// Functions
//

extern void collectiblecollision();
extern void reset();
extern void rendergame();
extern unsigned int blend_avg(unsigned int source, unsigned int target);
extern unsigned int blend_mul(unsigned int source, unsigned int target);
extern unsigned int blend_add(unsigned int source, unsigned int target);
extern void drawcircle(int x, int y, int r, int c);
extern void drawrect(int x, int y, int width, int height, int c);
extern void drawtile(int x, int y, int tile);
extern void drawcharacter(int x, int y, int character, int blend);
extern void drawstring(int x, int y, char *s);
