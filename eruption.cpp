
//Using SDL, SDL_image, standard IO, and, strings
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <math.h>
#include <sstream>
#include <cstdlib>
#include <time.h>
#include <vector>
#include <iostream>
#include <bitset>
#include <algorithm>
#include <stdlib.h>
#include <limits>

//Texture wrapper class
class LTexture
{
public:

    LTexture();
    ~LTexture();

    //Loads image at specified path
    bool loadFromFile( std::string path );

    //Creates image from font string
    bool loadFromRenderedText( std::string textureText, SDL_Color textColor );

    //Creates blank texture
    bool createBlank( int width, int height, SDL_TextureAccess = SDL_TEXTUREACCESS_STREAMING );

    //Deallocates texture
    void free();

    //Set color modulation
    void setColor( Uint8 red, Uint8 green, Uint8 blue );

    //Set blending
    void setBlendMode( SDL_BlendMode blending );

    //Set alpha modulation
    void setAlpha( Uint8 alpha );

    //Renders texture at given point
    void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

    //Set self as render target
    void setAsRenderTarget();

    //Gets image dimensions
    int getWidth();
    int getHeight();

    //Pixel manipulators
    bool lockTexture();
    bool unlockTexture();
    void* getPixels();
    void copyPixels( void* pixels );
    int getPitch();
    Uint32 getPixel32( unsigned int x, unsigned int y );

private:
    //The actual hardware texture
    SDL_Texture* mTexture;
    void* mPixels;
    int mPitch;

    //Image dimensions
    int mWidth;
    int mHeight;
};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Screen dimensions
SDL_Rect gScreenRect = { 0, 0, 320, 240 };

//Globally used font
TTF_Font *gFont = NULL;
LTexture gTextTexture;
LTexture gTextTexture2;
LTexture gTextTexture3;
LTexture gTextTexture4;

LTexture::LTexture()
{
    //Initialize
    mTexture = NULL;
    mWidth = 0;
    mHeight = 0;
    mPixels = NULL;
    mPitch = 0;
}

LTexture::~LTexture()
{
    //Deallocate
    free();
}

bool LTexture::loadFromFile( std::string path )
{
    //Get rid of preexisting texture
    free();

    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );

    //Convert surface to display format
    SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat(loadedSurface, SDL_PIXELFORMAT_RGBA8888, 0 );

    //Create blank streamable texture
    newTexture = SDL_CreateTexture( gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
                                    formattedSurface->w, formattedSurface->h );



    //Enable blending on texture
    SDL_SetTextureBlendMode( newTexture, SDL_BLENDMODE_BLEND );

    //Lock texture for manipulation
    SDL_LockTexture( newTexture, &formattedSurface->clip_rect, &mPixels, &mPitch );

    //Copy loaded/formatted surface pixels
    memcpy( mPixels, formattedSurface->pixels, formattedSurface->pitch * formattedSurface->h );

    //Get image dimensions
    mWidth = formattedSurface->w;
    mHeight = formattedSurface->h;

    //Get pixel data in editable format
    Uint32* pixels = (Uint32*)mPixels;
    int pixelCount = ( mPitch / 4 ) * mHeight;

    //Map colors
    Uint32 colorKey = SDL_MapRGB( formattedSurface->format, 0, 0xFF, 0xFF );
    Uint32 transparent = SDL_MapRGBA( formattedSurface->format, 0x00, 0xFF, 0xFF, 0x00 );

    //Color key pixels
    for( int i = 0; i < pixelCount; ++i )
    {
        if( pixels[ i ] == colorKey )
        {
            pixels[ i ] = transparent;
        }
    }

    //Unlock texture to update
    SDL_UnlockTexture( newTexture );
    mPixels = NULL;

    //Get rid of old formatted surface
    SDL_FreeSurface( formattedSurface );

    //Get rid of old loaded surface
    SDL_FreeSurface( loadedSurface );

    //Return success
    mTexture = newTexture;
    return mTexture != NULL;
}

bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{
    //Get rid of preexisting texture
    free();

    //Render text surface
    SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
    if( textSurface != NULL )
    {
        //Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
        if( mTexture == NULL )
        {
            SDL_Log( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            mWidth = textSurface->w;
            mHeight = textSurface->h;
        }

        //Get rid of old surface
        SDL_FreeSurface( textSurface );
    }
    else
    {
        SDL_Log( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
    }

    //Return success
    return mTexture != NULL;
}

bool LTexture::createBlank( int width, int height, SDL_TextureAccess access )
{
    //Create uninitialized texture
    mTexture = SDL_CreateTexture( gRenderer, SDL_PIXELFORMAT_RGBA8888, access, width, height );

    mWidth = width;
    mHeight = height;

    return mTexture != NULL;
}

void LTexture::free()
{
    //Free texture if it exists
    if( mTexture != NULL )
    {
        SDL_DestroyTexture( mTexture );
        mTexture = NULL;
        mWidth = 0;
        mHeight = 0;
        mPixels = NULL;
        mPitch = 0;
    }
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
    //Modulate texture rgb
    SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
    //Set blending function
    SDL_SetTextureBlendMode( mTexture, blending );
}

void LTexture::setAlpha( Uint8 alpha )
{
    //Modulate texture alpha
    SDL_SetTextureAlphaMod( mTexture, alpha );
}

float scalex = 1;
float scaley = 1;
int sky = 0;

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
    //Set rendering space and render to screen
    SDL_Rect renderQuad;
    if (scalex == 1 && scaley == 1) {
        SDL_Rect renderQuad2 = {x, y, mWidth, mHeight}; renderQuad = renderQuad2;
    }else{
        SDL_Rect renderQuad2 = {x, y, static_cast<int>(mWidth*scalex), static_cast<int>(mWidth*scaley)}; renderQuad = renderQuad2;
    }
    if (sky == 1){
        SDL_Rect renderQuad2 = {x, y, gScreenRect.w, gScreenRect.h}; renderQuad = renderQuad2;
    }

    //Set clip rendering dimensions
    if( clip != NULL )
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }

    //Render to screen
    SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

void LTexture::setAsRenderTarget()
{
    //Make self render target
    SDL_SetRenderTarget( gRenderer, mTexture );
}

int LTexture::getWidth()
{
    return mWidth;
}

int LTexture::getHeight()
{
    return mHeight;
}

bool LTexture::lockTexture()
{
    bool success = true;

    //Texture is already locked
    if( mPixels != NULL )
    {
        SDL_Log( "Texture is already locked!\n" );
        success = false;
    }
        //Lock texture
    else
    {
        if( SDL_LockTexture( mTexture, NULL, &mPixels, &mPitch ) != 0 )
        {
            SDL_Log( "Unable to lock texture! %s\n", SDL_GetError() );
            success = false;
        }
    }

    return success;
}

bool LTexture::unlockTexture()
{
    bool success = true;

    //Texture is not locked
    if( mPixels == NULL )
    {
        SDL_Log( "Texture is not locked!\n" );
        success = false;
    }
        //Unlock texture
    else
    {
        SDL_UnlockTexture( mTexture );
        mPixels = NULL;
        mPitch = 0;
    }

    return success;
}

void* LTexture::getPixels()
{
    return mPixels;
}

void LTexture::copyPixels( void* pixels )
{
    //Texture is locked
    if( mPixels != NULL )
    {
        //Copy to locked pixels
        memcpy( mPixels, pixels, mPitch * mHeight );
    }
}

int LTexture::getPitch()
{
    return mPitch;
}

Uint32 LTexture::getPixel32( unsigned int x, unsigned int y )
{
    //Convert the pixels to 32 bit
    Uint32 *pixels = (Uint32*)mPixels;

    //Get the pixel requested
    return pixels[ ( y * ( mPitch / 4 ) ) + x ];
}

bool init()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    SDL_Init( SDL_INIT_VIDEO );

    //Set texture filtering to linear
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );

    //Get device display mode
    SDL_DisplayMode displayMode;
    if( SDL_GetCurrentDisplayMode( 0, &displayMode ) == 0 )
    {
        gScreenRect.w = displayMode.w;
        gScreenRect.h = displayMode.h;
        if (gScreenRect.w > gScreenRect.h){
            double temp = gScreenRect.w;
            gScreenRect.w = gScreenRect.h;
            gScreenRect.h = temp;
        }
    }

    //Create window
    gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, gScreenRect.w, gScreenRect.h, SDL_WINDOW_SHOWN );

    //Create renderer for window
    gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );

    //Initialize renderer color
    SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

    //Initialize PNG loading
    int imgFlags = IMG_INIT_PNG;

    TTF_Init();

    return success;
}

LTexture gTargetTexture;
LTexture gArt;
LTexture gSkyBlue;

SDL_Rect gTileClips[48][48];
SDL_Rect gAnim[3][4];
SDL_Rect gItemClip[2][4];

bool loadMedia()
{
    //Loading success flag
    bool success = true;

    //Load scene textures
    gSkyBlue.loadFromFile("eruption/skyblue.png");

    gTargetTexture.createBlank( gScreenRect.w,gScreenRect.h, SDL_TEXTUREACCESS_TARGET );
    gArt.createBlank( gScreenRect.w,gScreenRect.h, SDL_TEXTUREACCESS_TARGET );

    //Fonts
    gFont = TTF_OpenFont( "eruption/clacon.ttf", 56 ); //Font Size
    SDL_Color textColor = { 255,255,255 };
    gTextTexture.loadFromRenderedText("Error", textColor);
    gTextTexture2.loadFromRenderedText("Error", textColor);
    gTextTexture3.loadFromRenderedText("Error", textColor);
    gTextTexture4.loadFromRenderedText("Error", textColor);

    return success;
}

void close()
{
    //Free loaded images
    gSkyBlue.free();

    gTextTexture.free();
    gTextTexture2.free();
    gTextTexture3.free();
    gTextTexture4.free();

    TTF_CloseFont( gFont );
    gFont = NULL;

    //Destroy window
    SDL_DestroyRenderer( gRenderer );
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;
    gRenderer = NULL;

    //Quit SDL subsystems
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}


const int NUMBER = 0,
          VARIABLE = 2,
          OPERATOR = 1,
          VECTOR = 3;


struct Node{

    int kind;
    double number;
    Node* r;
    Node* g;
    Node* b;
    std::string op;

    Node* left;
    Node* right;

    Node( double val ) {
        kind = NUMBER;
        number = val;
        this->left = NULL;
        this->right = NULL;
    }

    Node( Node* r, Node* g, Node* b ) {
        kind = VECTOR;
        this->r = r;
        this->g = g;
        this->b = b;
        this->left = NULL;
        this->right = NULL;
    }


    Node( std::string val, Node* none) {
        kind = VARIABLE;
        op = val;
        this->left = NULL;
        this->right = NULL;
    }

    Node( std::string op, Node *left, Node *right ) {
        kind = OPERATOR;
        this->op = op;
        this->left = left;
        this->right = right;
    }

    double get_Number(){
        return number;
    }

    std::string get_Op(){
        return op;
    }

};

union data{
    double input;
    unsigned long long output;
};


double frag_x = 0;
double frag_y = 0;

//Tree


Node* x_var = new Node("X",NULL);
Node* y_var = new Node("Y",NULL);
/*
Node* l1 = new Node("*", new Node("Y",NULL),  new Node(2));
Node* r1 = new Node("+", y_var,  new Node(.8));
Node* root = new Node("/", l1, r1);
*/

int color_num = 0;
//double rgb1[3] = {.2,.6,.6};
Node* root = new Node("Log", y_var, x_var );
//Node* root = new Node(0);

Node* current = root;
std::vector<std::string> v;


Node* recur(Node* prev){
    current = prev;

    if (prev->kind == OPERATOR) { v.push_back("("); }
    if (prev->kind == VECTOR) { v.push_back("#["); }

    if (prev->left != NULL && prev->kind == OPERATOR){
        recur(prev->left);
    }

    if (prev->kind == OPERATOR){
        std::stringstream ss;
        ss << prev->get_Op();
        v.push_back( ss.str() );
    }

    if (prev->kind == VECTOR){
        //std::stringstream ss;
        recur(prev->r);
        v.push_back(",");
        recur(prev->g);
        v.push_back(",");
        recur(prev->b);
        //ss << prev->r << "," << prev->g << "," << prev->b;
        //v.push_back( ss.str() );
    }

    if (prev->right != NULL && prev->kind == OPERATOR){
        recur(prev->right);
    }

    //If is not Operator, could be a number of a variable
    if (prev->kind == NUMBER){
        std::stringstream ss;
        ss << prev->get_Number();
        v.push_back( ss.str() );
    }
    if (prev->kind == VARIABLE){
        std::stringstream ss;
        ss << prev->get_Op();
        v.push_back( ss.str() );
    }

    if (prev->kind == OPERATOR) { v.push_back(")"); }
    if (prev->kind == VECTOR) { v.push_back("]"); }

    return prev;
}


/*
 * Delete Tree Recursively
 */
void  deleteTree(Node* prev){
    if (prev->left != NULL && prev->kind == OPERATOR){
        recur(prev->left);
    }
    if (prev->right != NULL && prev->kind == OPERATOR){
        recur(prev->right);
    }
    if (prev->kind == VECTOR){
        recur(prev->r);
        recur(prev->g);
        recur(prev->b);
    }
    delete prev;
}

std::bitset<sizeof(double) * CHAR_BIT> message;
std::string messages;


/*
 * Calculate Equation
 */
double getValue( Node *node ) {

    if ( node->kind == NUMBER ) {
        return node->number;
    }

    if ( node->kind == VECTOR ) {
        if (color_num == 0){ return getValue(node->r); }
        if (color_num == 1){ return getValue(node->g); }
        return getValue(node->b);
    }

    if ( node->kind == VARIABLE ) {
        if ( node-> op == "X"){
            return frag_x;
        }
        if ( node-> op == "Y"){
            return frag_y;
        }
    }

    double leftVal, rightVal;
    if (node->left != NULL) {
        leftVal = getValue(node->left);
    }
    if (node->right != NULL) {
        rightVal = getValue(node->right);
    }

    std::string opt = node->op;
    if (opt == "+") return leftVal + rightVal;
    if (opt == "-") return leftVal - rightVal;
    if (opt == "*") return leftVal * rightVal;
    if (opt == "/") return leftVal / rightVal;
    if (opt == "Mod") return std::fmod(leftVal, rightVal);
    if (opt == "Min") return std::min(leftVal, rightVal);
    if (opt == "Max") return std::max(leftVal, rightVal);
    if (opt == "And"){
        unsigned long long tempL = reinterpret_cast<unsigned long long &>(leftVal);
        unsigned long long tempR = reinterpret_cast<unsigned long long &>(rightVal);
        auto tempF = tempR & tempL;
        double value = *(double*)&tempF;
        return value;
    }
    if (opt == "Or"){
        unsigned long long tempL = reinterpret_cast<unsigned long long &>(leftVal);
        unsigned long long tempR = reinterpret_cast<unsigned long long &>(rightVal);
        auto tempF = tempR | tempL;
        double value = *(double*)&tempF;
        return value;
    }
    if (opt == "Xor"){
        unsigned long long tempL = reinterpret_cast<unsigned long long &>(leftVal);
        unsigned long long tempR = reinterpret_cast<unsigned long long &>(rightVal);
        auto tempF = tempR ^ tempL;
        double value = *(double*)&tempF;
        return value;
    }

    //Only Right Value Matters
    if (opt == "Abs") return abs(rightVal);
    if (opt == "Round") return round(rightVal);
    if (opt == "Expt") return exp(rightVal);
    if (opt == "Log") return log(rightVal);
    if (opt == "Sin") return (sin(rightVal * 12)+1.0) / 2.0;
    if (opt == "Cos") return (cos(rightVal * 12)+1.0) / 2.0;
    if (opt == "aTan") return atan(rightVal* 12);
    if (opt == "Invert") {
        unsigned long long tempR = reinterpret_cast<unsigned long long &>(rightVal);
        auto tempF = ~tempR;
        double value = *(double *)&tempF;
        return value;
    }

}

std::string randomOp(){
    int r = rand() % 18;
    if (r == 0) return "+";
    if (r == 1) return "-";
    if (r == 2) return "*";
    if (r == 3) return "/";
    if (r == 4) return "Mod";
    if (r == 5) return "Min";
    if (r == 6) return "Max";
    if (r == 7) return "And";
    if (r == 8) return "Or";
    if (r == 9) return "Xor";

    if (r == 10) return "Abs";
    if (r == 11) return "Round";
    if (r == 12) return "Expt";
    if (r == 13) return "Log";
    if (r == 14) return "Sin";
    if (r == 15) return "Cos";
    if (r == 16) return "aTan";
    if (r == 17) return "Invert";
    return "*";
}

/*
 * DPS to mutate random operators into new types
 */
Node* mutateExpression(Node *prev, int depth){
    current = prev;
    if (depth > 4){
        return NULL;
    }

    if (prev->kind == OPERATOR){
        if (prev->left != NULL){
            mutateExpression(prev->left, depth+1);
        }else{
            prev->left =  new Node(rand() % 100 / 100.0);
        }
        if (prev->right != NULL){
            mutateExpression(prev->right, depth+1);
        }else{
            prev->right =  new Node(rand() % 100 / 100.0);
        }
        int r = rand() % 10;
        if (r <= 1) {
            prev->op = randomOp();
        }
        return prev;
    }


    if (prev->kind == NUMBER && rand()%2 == 0){
        int r = rand() % 10;
        if (r <= 2){
            double rgb_new[3] = {rand() % 100 / 100.0, rand() % 100 / 100.0, rand() % 100 / 100.0};
            prev->kind = VECTOR;
            prev->r = new Node(rgb_new[0]);
            prev->g = new Node(rgb_new[1]);
            prev->b = new Node(rgb_new[2]);
            return prev;
        }else if (r <= 5){
            prev->kind = VARIABLE;
            int c = rand() % 2;
            if (c == 0) prev->op = "X";
            if (c == 1) prev->op = "Y";
            return prev;
        }
        prev->number = rand()%100/100.0;
    }

    if (prev->kind == VARIABLE && rand()%2 == 0) {
        int c = rand() % 3;
        if (c == 0) prev->op = "X";
        if (c == 1) prev->op = "Y";
        /*if (c == 2){
            prev->kind = OPERATOR;
            prev->left = x_var;
            prev->right = new Node(rand()%100/100.0);
            prev->op = randomOp();

        }*/
    }

    if (prev->kind == VECTOR && rand()%2 == 0){
        int r = rand() % 10;
        if (r <= 1){
            deleteTree(prev->r);
            deleteTree(prev->g);
            deleteTree(prev->b);
            prev->kind = NUMBER;
            prev->number = rand()%100;
            return prev;
        }else if (r <= 3){
            mutateExpression(prev->r, depth + 1);
            mutateExpression(prev->g, depth + 1);
            mutateExpression(prev->b, depth + 1);
            return prev;
        }
    }

    int r = rand() % 10;
    if (r <= 2) {
        prev->kind = OPERATOR;
        prev->left = new Node(rand()%100 / 100.0);
        prev->right = new Node(rand()%100 / 100.0);
        prev->op = randomOp();
        return prev;
    }

    return prev;
}





int main( int argc, char* args[] )
{

    srand(time(0));
    init();

    loadMedia();

    //Main loop flag
    bool quit = false;

    //Event handler
    SDL_Event e;

    //Touch variables
    SDL_Point touchLocation = { gScreenRect.w / 2, gScreenRect.h / 2 };
    //LTexture* currentTexture = &gItemSpark;

    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);

    bool holding = false;
    bool clearAll = true;
    int fade = 0;
    int delay = 0;
    clock_t start, diff;
    int msec;
    srand (time(NULL));

    double line_y = 0;
    double line_y_old = 0;
    double r_x = gScreenRect.w * .25;
    double r_y = gScreenRect.w * .25;

    //While application is running
    while( !quit )
    {
        start = clock();
        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 )
        {
            //User requests quit
            if( e.type == SDL_QUIT )
            {
                quit = true;
            }
                //Window event
            else if( e.type == SDL_WINDOWEVENT )
            {

            }
                //Touch down
            else if( e.type == SDL_FINGERDOWN )
            {
                touchLocation.x = e.tfinger.x * gScreenRect.w;
                touchLocation.y = e.tfinger.y * gScreenRect.h;
            }
                //Touch motion
            else if( e.type == SDL_FINGERMOTION )
            {
                touchLocation.x = e.tfinger.x * gScreenRect.w;
                touchLocation.y = e.tfinger.y * gScreenRect.h;
                holding = true;
            }
                //Touch release
            else if( e.type == SDL_FINGERUP )
            {
                touchLocation.x = e.tfinger.x * gScreenRect.w;
                touchLocation.y = e.tfinger.y * gScreenRect.h;
                holding = false;
            }
        }

        gTargetTexture.setAsRenderTarget();


        if (clearAll == true) {
            //Clear screen
            SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0xFF );
            SDL_RenderClear( gRenderer );

            sky = 1;
            gSkyBlue.render(0, 0);
            sky = 0;
            clearAll = false;
        }

        if (holding == true){
            fade = 100;
        }else{
            if (fade > 0) fade -= 4;
        }
        if (delay > 0){ delay-=1; }

        /*
        if (fade > 0) {
            int s = 200;
            float a = 1.5;
            SDL_Rect fillRect = {0, 0, 0, 0};
            //top left
            if (touchLocation.x < s*a && touchLocation.y > gScreenRect.h-s*2 && touchLocation.y < gScreenRect.h-s){
                SDL_Rect f = {0, gScreenRect.h-s*2, s*a,s}; fillRect = f;
                if (holding == true && delay <= 0){
                    delay = 5;
                }
            }
            //bottom left
            if (touchLocation.x < s*a && touchLocation.y > gScreenRect.h-s){
                SDL_Rect f = {0, gScreenRect.h-s, s*a,s}; fillRect = f;
                if (holding == true && delay <= 0){
                    delay = 5;
                }
            }
            //top right
            if (touchLocation.x > s*a && touchLocation.x < s*2*a && touchLocation.y > gScreenRect.h-s*2 && touchLocation.y < gScreenRect.h-s){
                SDL_Rect f = {s*a, gScreenRect.h-s*2, s*a,s}; fillRect = f;
                if (holding == true && delay <= 0){
                    delay = 5;
                }
            }
            //bottom right
            if (touchLocation.x > s*a && touchLocation.x < s*2*a && touchLocation.y > gScreenRect.h-s){
                SDL_Rect f = {s*a, gScreenRect.h-s, s*a,s}; fillRect = f;
                if (holding == true && delay <= 0){
                    delay = 5;
                }
            }
            SDL_SetRenderDrawColor(gRenderer, 255,255,255, fade);
            SDL_RenderFillRect(gRenderer, &fillRect);
        }
        */

        //Draw Points
        SDL_RenderSetScale( gRenderer, 4.0, 4.0);

        if (line_y < r_y) {
            //for (double y = 0; y < r_x; y++) {
            for (double y = line_y_old; y <= line_y; y++) {
                for (double x = 0; x < r_y; x++) {

                    frag_x = (x - r_x * .5) / (r_x * .5);
                    frag_y = (y - r_y * .5) / (r_y * .5);

                    color_num = 0; double r = getValue(root) * 255;
                    color_num = 1; double g = getValue(root) * 255;
                    color_num = 2; double b = getValue(root) * 255;

                    SDL_SetRenderDrawColor(gRenderer,
                                           r,
                                           g,
                                           b,
                                           255);

                                                 //Resize value
                    int tx = x + gScreenRect.w * (.25) * .5 - r_x * .5;
                    int ty = y + gScreenRect.h * (.25) * .5 - r_y * .5;
                    SDL_RenderDrawPoint(gRenderer, tx, ty);

                }
            }
        }
        //Next line y-axis
        line_y_old = line_y;
        line_y += 1;
        //if (line_y > r_y) {
        //line_y = 0;
        //}

        //SDL_RenderSetScale( gRenderer, 1.0, 1.0);

        //Full screen fade
        SDL_Rect fillRect = {0,0, gScreenRect.w, gScreenRect.h};
        SDL_SetRenderDrawColor(gRenderer, 0,0,0, 0);
        SDL_RenderFillRect(gRenderer, &fillRect);

        //Render primitives and Background
        SDL_SetRenderTarget( gRenderer, NULL );
        gTargetTexture.render( 0, 0 );
        SDL_RenderSetScale( gRenderer, 1.0, 1.0);



        //Debug Text
        std::ostringstream temp;
        std::ostringstream temp2;
        std::ostringstream temp3, temp4;
        recur(root);
        for (int n = 0; n < v.size(); n++){
            if (n < 25)
                temp << v[n];
            else if (n < 50)
                temp2 << v[n];
            else if (n < 75)
                temp3 << v[n];
            else
                temp4 << v[n];
        }
        v.clear();

        diff = clock() - start;
        msec = diff * 1000 / CLOCKS_PER_SEC;
        temp4 << " " << msec << " "; //<< getValue(root);
        //Completed an Image
        if (line_y > r_y){
            temp4 << "Click...";
        }
        if (holding == 1 && delay <= 0) {
            clearAll = true;
            line_y = 0;

            if (touchLocation.y < gScreenRect.h / 4.0){
                deleteTree(root);
                root = new Node(0);
            }else {

                while (true) {
                    auto save_root = root;
                    mutateExpression(root, 0);

                    //Compare if all corners of image are the same to avoid boring 1-color art
                    int strikes = 0;
                    for (int n = 0; n < 3; n++) {
                        color_num = n;
                        frag_x = 0;
                        frag_y = 0;
                        double compare1 = getValue(root) * 255;
                        frag_x = 1;
                        frag_y = 0;
                        double compare2 = getValue(root) * 255;
                        frag_x = 0;
                        frag_y = 1;
                        double compare3 = getValue(root) * 255;
                        frag_x = 1;
                        frag_y = 1;
                        double compare4 = getValue(root) * 255;
                        if (compare1 == compare2 == compare3 == compare4) {
                            strikes += 1;
                        }
                    }
                    if (strikes == 3) {
                        temp4 << strikes;
                        deleteTree(root);
                        root = save_root;
                        continue;
                    }
                    temp4 << strikes;
                    break;

                }
            }
            delay = 10;
        }
        //temp << messages;
        temp << " "; temp2 << " "; temp3 << " "; temp4 << " ";
        SDL_Color textColor = { 255,255,255 };
        gTextTexture.loadFromRenderedText(temp.str(), textColor);
        gTextTexture.render(0, gScreenRect.h-200);
        gTextTexture2.loadFromRenderedText(temp2.str(), textColor);
        gTextTexture2.render(0, gScreenRect.h-150);
        gTextTexture3.loadFromRenderedText(temp3.str(), textColor);
        gTextTexture3.render(0, gScreenRect.h-100);
        gTextTexture4.loadFromRenderedText(temp4.str(), textColor);
        gTextTexture4.render(0, gScreenRect.h-50);

        //Update screen
        SDL_RenderPresent( gRenderer );

    }

    close();

    return 0;
}
