/*
 *---------------------------------------------
 *
 *                      Game.cpp
 *
 * Date 2013-12-13
 *
 * Creator RS
 *---------------------------------------------*/
#include "Game.h"

// Allocate memory for coord
SDL_Rect baseclass::coord;

//---------Constructor: initilaize the Game with proper values----------
Game::Game()
{
    Game_Window=SDL_CreateWindow("My Game Window",
                                 SDL_WINDOWPOS_UNDEFINED,
                                 SDL_WINDOWPOS_UNDEFINED,
                                 SCREEN_WIDTH,
                                 SCREEN_HEIGHT,
                                 0);
    Game_Renderer = SDL_CreateRenderer(Game_Window, -1,
                                       SDL_RENDERER_ACCELERATED);

    //Start position for window on level
	SDL_Rect startpos;
	startpos.x = 0;
	startpos.y = 10*baseclass::TILE_SIZE;


    baseclass::coord.x=startpos.x;
    baseclass::coord.y=startpos.y;

    //Size of window
    baseclass::coord.w=SCREEN_WIDTH;
    baseclass::coord.h=SCREEN_HEIGHT;

    direction[0]=direction[1]=0;
    running=true;

    // Initialize The levels and player
    level1=new Level("level1_map.map","Bana1.bmp",
                     "solid_block1.bmp","level1_E_M.map",
                     load_texture("hiss1.bmp"), load_texture("enemy1.bmp"));

    level2=new Level("level2_map.map","Strandbana.bmp",
                     "solid_block2.bmp","level2_E_M.map",
                     load_texture("Moln.bmp"),load_texture("Gubbe.bmp"));

    playerInit=new player(load_texture("player.bmp"));

    player1=playerInit;

    // Textures displayed on bigger events
    prolog1 = load_texture("text_prolog.bmp");
    kap1 = load_texture("text_kapitel1.bmp");
    epi = load_texture("text_epilog.bmp");
    die_text = load_texture("die_text.bmp");
    win_text = load_texture("win_text.bmp");

}


//------Destructor: Keep the memory clean ----------
Game::~Game()
{
    //Destroy textures
    SDL_DestroyTexture(die_text);
    SDL_DestroyTexture(win_text);

    //Delete non trivial objects
    delete curLevel;
    delete level1;
    delete level2;
    delete player1;
    SDL_Quit();
}

//---------load_texture: Return a textrure from a file inpute (BMP)----------
SDL_Texture* Game::load_texture(const char* filename)
{
    SDL_Surface* tmp=SDL_LoadBMP(filename);
    //If the load failes you are given a message
    std::cout << SDL_GetError()<< std::endl;

    //The color white isnt visible in the game
    SDL_SetColorKey(tmp,SDL_TRUE,SDL_MapRGB(tmp->format,0xff,0xff,0xff));

    SDL_Texture* tmp2=SDL_CreateTextureFromSurface(Game_Renderer,
                      tmp);
    SDL_FreeSurface(tmp);
    return tmp2;
}

//--------- handleEvents: Poll events to a "stack" and give -----------
//                       the proper response to an event
void Game::handleEvents()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            //If window is closed
        case SDL_QUIT:
            running=false;
            SDL_DestroyWindow(Game_Window);
            break;

            //Key pressed down
        case SDL_KEYDOWN:
            switch(event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                running=false;
                SDL_DestroyWindow(Game_Window);
                break;
            case SDLK_LEFT:
                direction[0]=1;
                player1->setMoving(true);
                break;
            case SDLK_RIGHT:
                direction[1]=1;
                player1->setMoving(true);
                break;
            case SDLK_UP:
                player1->startJump();
                break;
	case SDLK_ENTER:
		stay = false;
		break;
            }
            break;

            //Key release
        case SDL_KEYUP:
            switch(event.key.keysym.sym)
            {
            case SDLK_LEFT:
                direction[0]=0;
                player1->setMoving(false);
                break;
            case SDLK_RIGHT:
                direction[1]=0;
                player1->setMoving(false);
                break;
            }
            break;
        };
    }
}


//---------showmap: Render current level on screen ----------
//                  (modify for up and down motion)
void Game::showmap()
{
    //Renders only the visible blocks
    int startcoord=(baseclass::coord.x-
                    (baseclass::coord.x%baseclass::TILE_SIZE))/
                   baseclass::TILE_SIZE;
    int endcoord=(baseclass::coord.x+baseclass::coord.w+
                  (baseclass::coord.x-
                   (baseclass::coord.x+baseclass::coord.w)%baseclass::TILE_SIZE)/
                  baseclass::TILE_SIZE);

    //prevent comparison int with unsigned
    int end0 = levelMatrix[0].size();
    int end = levelMatrix.size();

    // no negative values for startcoord and endcoord no bigger than level size
    if(startcoord<0)
        startcoord=0;
    if(endcoord>end0)
        endcoord=end0;

    // Step trough the matrix of block information and render them in right
    // posistion.
    for(int i=0; i<end; i++)
        for(int j=startcoord; j<endcoord; j++)
            if(levelMatrix[i][j]!=0)
            {
                // Position in .bmp file
                SDL_Rect blockrect= {(levelMatrix[i][j]-1)*baseclass::TILE_SIZE,
                                     0,baseclass::TILE_SIZE,baseclass::TILE_SIZE
                                    };
                // Position for render on screen
                SDL_Rect destrect = {j*baseclass::TILE_SIZE-baseclass::coord.x,
                                     i*baseclass::TILE_SIZE-baseclass::coord.y,
                                     baseclass::TILE_SIZE,
                                     baseclass::TILE_SIZE
                                    };

                SDL_RenderCopy(Game_Renderer,block,&blockrect,&destrect);
            }


}


//---------run: The gameloop, fps controll, do the player movement----------
//              as well as camera movement.
void Game::run()
{
    Uint32 start;

    // Set current level
    curLevel = level1;

    //Load block matrix
    levelMatrix=curLevel->get_level();

    //load cuLevel background and block image
    background=load_texture(curLevel->get_background());
    block=load_texture(curLevel->get_block_text());

    //Prolog to Game
	bool stay; //Used for keyevent in story between levels
	do{    
	SDL_Rect tmp = {0,0,800,480};
	SDL_RenderCopy(Game_Renderer, prolog1,NULL,&tmp);
	SDL_RenderPresent(Game_Renderer);
	handleEvents()
	}while(stay) 

    // Game loop
    while(running)
    {
        // Timer for fps handler
        start=SDL_GetTicks();

        // Keypresses and so on
        handleEvents();

//---------Camera movement X-axis------------------------------------
        if(direction[0])
        {
            //Collision with left side of level -> Do not move
            if(player1->getRect().x ==0)
                player1->setXvel(0);
            else if(player1->getRect().x > SCREEN_WIDTH/3 or baseclass::coord.x == 0)
                player1->setXvel(-player1->getSpeed());
            else
            {
                //Move the camera left, player follows
                player1->setXvel(0);
                baseclass::coord.x-=player1->getSpeed();
            }
        }
        else if(direction[1])
        {
            //Collision with right side of level -> Do not move
            if(player1->getRect().x+player1->getRect().w >= SCREEN_WIDTH)
                player1->setXvel(0);
            else if(player1->getRect().x + player1->getRect().w < SCREEN_WIDTH*2/3
                    or baseclass::coord.x + SCREEN_WIDTH == curLevel->get_width())
                player1->setXvel(player1->getSpeed());
            else
            {
                //move the camera right
                player1->setXvel(0);
                baseclass::coord.x+=player1->getSpeed();
            }
        }
        else //We're not moving
            player1->setXvel(0);


//-------------Camera movement Y-axis-----------------------------------

        //Die if hit the bottom border
        if(player1->getRect().y  >= SCREEN_HEIGHT)
            player1->die();
        else if(player1->getRect().y + player1->getRect().h > SCREEN_HEIGHT-baseclass::TILE_SIZE
                and baseclass::coord.y + baseclass::coord.h < curLevel->get_height())
        {
            // Move window when falling
            baseclass::coord.y+=player1->getYvel();
            player1->setYvel(0);
        }
        else if(player1->getRect().y < baseclass::TILE_SIZE and baseclass::coord.y > 0)
        {
            if(player1->getYvel() < 0)
            {
                // Move the window up in the level when jumping or using elevator
                baseclass::coord.y+=2*player1->getYvel();
            }
        }



//--------------- Move everything, Collision with object in level-------------------------

        //Move object in level
        curLevel->update_level();

        //Collision enemy?
        if(curLevel->enemy_collission(player1->getRect()))
            player1->die();

        //Player Colliding with anything?
        if(direction[1])
            player1->move(levelMatrix,'r',curLevel->get_mblocks());
        else
            player1->move(levelMatrix,'l',curLevel->get_mblocks());


        //If you die
        if(player1->getDead())
        {
            //Stop Game-loop
            running=false;
            //Render Text delay for 5 sek
            SDL_RenderCopy(Game_Renderer, die_text,NULL,NULL);
            SDL_RenderPresent(Game_Renderer);
            SDL_Delay(5000);

            //Close Game-window
            SDL_DestroyWindow(Game_Window);
            SDL_DestroyRenderer(Game_Renderer);
        }

        //If you win, either proceed to next level or winning screen
        if(player1->getWinner())
        {
            //Render Winner Text delay for 4 sek
            SDL_RenderCopy(Game_Renderer, win_text,NULL,NULL);
            SDL_RenderPresent(Game_Renderer);
            SDL_Delay(4000);

            if(curLevel==level2)
            {
                //Epilog show untill ENTER-key pressed
		do{
                SDL_Rect tmp = {0,0,800,480};
                SDL_RenderCopy(Game_Renderer, epi,NULL,&tmp);
                SDL_RenderPresent(Game_Renderer);
                handleEvents();
		}while(stay)

                //Close the Game-window
                running = false;
                SDL_DestroyWindow(Game_Window);
                SDL_DestroyRenderer(Game_Renderer);
            }
            else
            {
                //Story level2 show untill ENTER-key pressed
		do{                
		SDL_Rect tmp = {0,0,800,480};
                SDL_RenderCopy(Game_Renderer, kap1,NULL,&tmp);
                SDL_RenderPresent(Game_Renderer);
		handleEvents();
                }while(stay)

                // Change level
                curLevel=level2;
                levelMatrix=curLevel->get_level();
                //load level background and block image
                background=load_texture(curLevel->get_background());
                block=load_texture(curLevel->get_block_text());

                // Reset level position and powerups
                player1 = playerInit;
                // Reset window position
                baseclass::coord.x=startpos.x;
                baseclass::coord.y=startpos.y;
            }
        }


//-----------Show everything on screen------------------
        SDL_RenderClear(Game_Renderer);
        SDL_RenderCopy(Game_Renderer, background,&baseclass::coord,NULL);
        showmap();
        curLevel->show(Game_Renderer);
        player1->show(Game_Renderer);

        SDL_RenderPresent(Game_Renderer);


        // Keep the same fps
        if(1000/30>(SDL_GetTicks()-start))
            SDL_Delay(1000/30-(SDL_GetTicks()-start));
    }
}
