#include "renderLoop.h"
#include "./include/graph.h"
#include "unit.h"

void afficherGraphByTID(graph* g) {
	if (!g || !g->m_data)
		return;

	char couleur[20];

	for (int i = 0; i < g->m_sizeY; i++) {
		for (int j = 0; j < g->m_sizeX; j++) {
			switch (g->m_data[i * g->m_sizeX + j]->m_layerID) {
			case GRASS_ID:
				strcpy(couleur, "\x1b[38;2;0;255;127m");
				break;
			case FOREST_ID:
				strcpy(couleur, "\033[0;32;40m");
				break;
			case MOUNTAIN_ID:
				strcpy(couleur, "\033[0;31;40m");
				break;
			case CITY_ID:
				strcpy(couleur, "\033[0;33;40m");
				break;
			case ROAD_ID:
				strcpy(couleur, "\x1b[38;2;255;215;0m");
				break;
			case WATER_ID:
				strcpy(couleur, "\033[0;36;40m");
				break;
			default:
				strcpy(couleur, "");
			}
			printf("%s%c\x1b[0m ", couleur, s_defenseGround[g->m_data[i * g->m_sizeX + j]->m_layerID]);
			//TODO A REMODIFIER
		}
		printf("\n");
	}
}

void afficherGraphByID(graph* g) {
	if (!g || !g->m_data)
		return;

	for (int i = 0; i < g->m_sizeY; i++) {
		for (int j = 0; j < g->m_sizeX; j++) {
			printf("%03d ", g->m_data[i * g->m_sizeX + j]->m_id);
		}
		printf("\n");
	}
}


SDL_Surface* init(char* p_windowName, int p_resX, int p_resY)
{
	SDL_Surface* window;
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("[-] ERROR - %s\n", SDL_GetError());
		return NULL;
	}

	if (!IMG_Init(IMG_INIT_PNG))
	{
		printf("[-] ERROR - Failed to initialise SDL_Image (%s)\n", SDL_GetError());
		SDL_Quit();
		return NULL;
	}

	if (TTF_Init() == -1)
	{
		printf("[-] ERROR - Erreur d'initialisation de TTF_Init : %s\n", TTF_GetError());
		SDL_Quit();
		return NULL;
	}

	window = SDL_SetVideoMode(p_resX, p_resY, 32, SDL_DOUBLEBUF | SDL_HWSURFACE);
	if (!window)
	{
		printf("[-] ERROR - %s\n", SDL_GetError());
		SDL_Quit();
		return NULL;
	}

	SDL_WM_SetCaption(p_windowName, NULL);

	return window;
}

int interaction(SDL_Event* p_e, game* p_game)
{
	int quit = 0;

	SDL_GetMouseState(&p_game->m_mousePosX, &p_game->m_mousePosY);
	p_game->m_lclic = 0;
	p_game->m_rclic = 0;
	printf("X : %4d, Y: %4d\r", p_game->m_mousePosX, p_game->m_mousePosY);
	while (SDL_PollEvent(p_e))
	{
		switch (p_e->type)
		{
			// Appuie sur la croix rouge de la fenêtre
		case SDL_QUIT:
			quit = 1;
			break;

			// Appuie sur une touche du clavier
		case SDL_KEYDOWN:
			switch (p_e->key.keysym.sym)
			{
			case SDLK_ESCAPE:
				quit = 1;
				break;

			case SDLK_SPACE:
				p_game->m_playerTurn = (p_game->m_playerTurn + 1) % 2;
				ResetPlayers(p_game);

			default:
				break;
			}
			break;

			// Relachement d'une touche du clavier
		case SDL_KEYUP:
			switch (p_e->key.keysym.sym)
			{
			default:
				break;
			}
			break;

			// Appuie sur un bouton de la souris
		case SDL_MOUSEBUTTONDOWN:
			switch (p_e->button.button)
			{
			case SDL_BUTTON_LEFT:
				p_game->m_lclic = 1;
				break;

			case SDL_BUTTON_RIGHT:
				p_game->m_rclic = 1;
				break;

			case SDL_BUTTON_MIDDLE:
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
	}

	return quit;
}

int update(game* p_game)
{
	if (p_game->m_lclic) //Sélectionner l'unité de son camp
	{
		int playerattaquantID=-1;
		//TROUVER LA CASE DU JOUEUR PRESENT SELECTIONNEE
		unit* attaquant = GetUnitFromPos(p_game, p_game->m_mousePosX, p_game->m_mousePosY, &playerattaquantID);
		unit* pastattaquant = GetSelectedUnit(p_game);
		if (attaquant) 
		{
			if (!attaquant->m_selected)
				attaquant->m_selected = 1;
			else attaquant->m_selected = 0;	
		}
		if (pastattaquant)
		{
			pastattaquant->m_selected = 0;
		}
	}

	if (p_game->m_rclic)
	{
		int playerdefenseID = -1;
		unit* defense = GetUnitFromPos(p_game, p_game->m_mousePosX, p_game->m_mousePosY, &playerdefenseID);
		unit* attaquant = GetSelectedUnit(p_game);
		int dtmp = 0;
		node* cdefense = NULL;
		node* cattaque = NULL;
		node* current = NULL;
		if(defense) cdefense = GetNodeFromPosition(p_game->m_graph, defense->m_posX, defense->m_posY);
		if(attaquant) cattaque = GetNodeFromPosition(p_game->m_graph, attaquant->m_posX, attaquant->m_posY);
		current = GetNodeFromPosition(p_game->m_graph, p_game->m_mousePosX/64, p_game->m_mousePosY/64);
		if (defense && defense->m_hp > 0) //Attaque
		{
			//ARTILLERIE (ROCKET LAUNCHER) 2 a 5 cases
			//RESTE CASE ADJACENTE
			
			if (attaquant->m_type == ROCKET_LAUNCHER)
			{
				dtmp = GetManhattanDistance(cdefense,cattaque);
				if (dtmp <= 5 && dtmp >= 2)
				{
					Atttack(p_game, attaquant, defense);
					p_game->m_playerTurn = (p_game->m_playerTurn + 1) % 2;
					ResetPlayers(p_game);
				}
			}
			else
			{
				dtmp = GetManhattanDistance(cdefense, cattaque);
				if (dtmp == 1)
				{
					Atttack(p_game, attaquant, defense);
					p_game->m_playerTurn = (p_game->m_playerTurn + 1) % 2;
					ResetPlayers(p_game);
				}
			}

		}
		else //Deplacement 
		{
			/*afficherGraphByID(p_game->m_graph);
			afficherGraphByTID(p_game->m_graph);*/
			if (!GetUnitFromPos(p_game, p_game->m_mousePosX, p_game->m_mousePosY,&playerdefenseID))
			{
				dtmp = GetManhattanDistance(cattaque, current);

				if (dtmp <= attaquant->m_pm)
				{
					attaquant->m_posX = p_game->m_mousePosX/64;
					attaquant->m_posY = p_game->m_mousePosY/64;
					p_game->m_playerTurn = (p_game->m_playerTurn + 1) % 2;
					ResetPlayers(p_game);
				}
			}
			else
				printf("Deplacement impossible");
		}
		
	}

	return 0;
}

int draw(SDL_Surface* p_window, game* p_game)
{
	SDL_FillRect(p_window, NULL, SDL_MapRGBA(p_window->format, 0, 0, 0, 0));

	DrawGame(p_window, p_game);

	SDL_Flip(p_window);

	return 0;
}
