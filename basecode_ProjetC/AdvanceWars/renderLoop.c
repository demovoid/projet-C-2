#include "renderLoop.h"
#include "include/graph.h"
#include "unit.h"

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

static void verifDijkstra(game* p_game, unit* clickt, int ind, int prev) {
	if (ind < 0 || ind >= p_game->m_graph->m_sizeX* p_game->m_graph->m_sizeY || clickt->m_walkGraph[ind]->m_distance == 1 || clickt->m_walkGraph[ind]->m_distance == INFINITY_DIST)
		return;
	int dist = INFINITY_DIST;
	if (ind % p_game->m_graph->m_sizeX + 1 < p_game->m_graph->m_sizeX && clickt->m_walkGraph[ind + 1]->m_distance < dist)
		dist = clickt->m_walkGraph[ind + 1]->m_distance;
	if (ind % p_game->m_graph->m_sizeX - 1 >= 0 && clickt->m_walkGraph[ind - 1]->m_distance < dist)
		dist = clickt->m_walkGraph[ind - 1]->m_distance;
	if (ind + p_game->m_graph->m_sizeX < p_game->m_graph->m_sizeX * p_game->m_graph->m_sizeY && clickt->m_walkGraph[ind + p_game->m_graph->m_sizeX]->m_distance < dist)
		dist = clickt->m_walkGraph[ind + p_game->m_graph->m_sizeX]->m_distance;
	if (ind - p_game->m_graph->m_sizeX >= 0 && clickt->m_walkGraph[ind - p_game->m_graph->m_sizeX]->m_distance < dist)
		dist = clickt->m_walkGraph[ind - p_game->m_graph->m_sizeX]->m_distance;

	int d = clickt->m_walkGraph[ind]->m_distance;

	if (dist + 1 == clickt->m_walkGraph[ind]->m_distance)
		return;

	clickt->m_walkGraph[ind]->m_distance = dist + 1;
	if(ind + 1 != prev)
		verifDijkstra(p_game, clickt, ind + 1, ind);
	if(ind - 1 != prev)
		verifDijkstra(p_game, clickt, ind - 1, ind);
	if(ind + p_game->m_graph->m_sizeX != prev)
		verifDijkstra(p_game, clickt, ind + p_game->m_graph->m_sizeX, ind);
	if(ind - p_game->m_graph->m_sizeX != prev)
		verifDijkstra(p_game, clickt, ind - p_game->m_graph->m_sizeX, ind);
}

int update(game* p_game)
{
	if (p_game->m_lclic) //Sélectionner l'unité de son camp
	{
		int pID;
		unit* selec = GetSelectedUnit(p_game);
		unit* clickt = GetUnitFromPos(p_game, p_game->m_mousePosX, p_game->m_mousePosY, &pID);

		if (selec) { //On a une unité selectionnée
			if(!clickt && selec->m_selected == 1) { //J'ai une unité selectionnée et la case choisie est vide et que l'unité est selectionnée en déplacement
				int dist = selec->m_walkGraph[(p_game->m_mousePosX / 64) + (p_game->m_mousePosY / 64) * p_game->m_graph->m_sizeX]->m_distance;
				if (dist <= selec->m_pm) {
					selec->m_posX = p_game->m_mousePosX / 64;
					selec->m_posY = p_game->m_mousePosY / 64;
					selec->m_pm -= dist;
				}
			}
			selec->m_selected = 0;
		}
		else { //Aucune unité selectionnée
			if (clickt && pID == p_game->m_playerTurn) {
				clickt->m_selected = 1;
				CalculateMovement(p_game->m_graph, clickt);

				//Enlève toutes les cases ou se trouvent une unité
				unit* currentUnit;
				for (int c = 0; c < 2; c++)
					for (int d = 0; d < p_game->m_players[c]->m_nbUnit; d++) {
						currentUnit = p_game->m_players[c]->m_units[d];
						clickt->m_walkGraph[currentUnit->m_posY * p_game->m_graph->m_sizeX + currentUnit->m_posX]->m_distance = INFINITY_DIST;

						//Enlève toutes les cases qui n'ont pas une case accessible à coté
					}

				for (int c = 0; c < 2; c++)
					for (int d = 0; d < p_game->m_players[c]->m_nbUnit; d++) {
						currentUnit = p_game->m_players[c]->m_units[d];
						int ind = currentUnit->m_posY * p_game->m_graph->m_sizeX + currentUnit->m_posX;
						clickt->m_walkGraph[ind]->m_distance = INFINITY_DIST;

						int indVois[4] = { ind + 1, ind - 1, ind + p_game->m_graph->m_sizeX, ind - p_game->m_graph->m_sizeX };

						//Enlève toutes les cases qui n'ont pas une case accessible à coté
						for (int a = 0; a < 4; a++)
							verifDijkstra(p_game, clickt, indVois[a], ind);
					}
				
				int err;
				do {
					err = 0;
					for (int ind = 0; ind < p_game->m_graph->m_sizeX * p_game->m_graph->m_sizeY; ind++) {
						if (clickt->m_walkGraph[ind]->m_distance != 1 && clickt->m_walkGraph[ind]->m_distance != INFINITY_DIST) {

							int dist = INFINITY_DIST;
							if (ind % p_game->m_graph->m_sizeX + 1 < p_game->m_graph->m_sizeX && clickt->m_walkGraph[ind + 1]->m_distance < dist)
								dist = clickt->m_walkGraph[ind + 1]->m_distance;
							if (ind % p_game->m_graph->m_sizeX - 1 >= 0 && clickt->m_walkGraph[ind - 1]->m_distance < dist)
								dist = clickt->m_walkGraph[ind - 1]->m_distance;
							if (ind + p_game->m_graph->m_sizeX < p_game->m_graph->m_sizeX * p_game->m_graph->m_sizeY && clickt->m_walkGraph[ind + p_game->m_graph->m_sizeX]->m_distance < dist)
								dist = clickt->m_walkGraph[ind + p_game->m_graph->m_sizeX]->m_distance;
							if (ind - p_game->m_graph->m_sizeX >= 0 && clickt->m_walkGraph[ind - p_game->m_graph->m_sizeX]->m_distance < dist)
								dist = clickt->m_walkGraph[ind - p_game->m_graph->m_sizeX]->m_distance;

							if (clickt->m_walkGraph[ind]->m_distance <= dist) {
								if (clickt->m_walkGraph[ind]->m_distance <= clickt->m_pm) {
									clickt->m_walkGraph[ind]->m_distance = INFINITY_DIST;
									err++;
								}
							}
						}
					}
				} while (err);
			}
		}


		/*int playerattaquantID=-1;
		//TROUVER LA CASE DU JOUEUR PRESENT SELECTIONNEE
		unit* attaquant = GetUnitFromPos(p_game, p_game->m_mousePosX, p_game->m_mousePosY, &playerattaquantID);
		unit* pastattaquant = GetSelectedUnit(p_game);
		if (attaquant) 
		{
			if (!attaquant->m_selected)
				attaquant->m_selected = 1;
			else 
				attaquant->m_selected = 0;	
		}
		if (pastattaquant)
		{
			pastattaquant->m_selected = 0;
		}*/
		
		
	}

	if (p_game->m_rclic)
	{

		int pID, dist;
		unit* selec = GetSelectedUnit(p_game);
		unit* clickt = GetUnitFromPos(p_game, p_game->m_mousePosX, p_game->m_mousePosY, &pID);

		if (selec) { //On a une unité selectionnée
			if (clickt && selec->m_selected == 2) { //J'ai une unité selectionnée et la case choisie est vide et que l'unité est selectionnée en attaque
				//omg j'attaque !
				if (selec->m_canFire) {
					node* src = GetNodeFromPosition(p_game->m_graph, selec->m_posX, selec->m_posY);
					node* dest = GetNodeFromPosition(p_game->m_graph, clickt->m_posX, clickt->m_posY);
					dist = GetManhattanDistance(src, dest);

					if (selec->m_type->m_type == ROCKET_LAUNCHER) {
						if (dist >= 2 && dist <= 5) {
							Atttack(p_game, selec, clickt);
							selec->m_canFire = 0;
						}
					}
					else
						if (dist == 1) {
							Atttack(p_game, selec, clickt);
							selec->m_canFire = 0;
						}
				}
			}
			selec->m_selected = 0;
		}
		else { //Aucune unité selectionnée
			if (clickt && pID == p_game->m_playerTurn) 
				clickt->m_selected = 2;
		}

		/*int playerdefenseID = -1;
		unit* defense = GetUnitFromPos(p_game, p_game->m_mousePosX, p_game->m_mousePosY, &playerdefenseID);
		unit* attaquant = GetSelectedUnit(p_game);
		int dtmp = 0;
		node* cdefense = NULL;
		node* cattaque = NULL;
		node* current = NULL;
		if(defense) cdefense = GetNodeFromPosition(p_game->m_graph, defense->m_posX, defense->m_posY);
		if(attaquant) cattaque = GetNodeFromPosition(p_game->m_graph, attaquant->m_posX, attaquant->m_posY);
		current = GetNodeFromPosition(p_game->m_graph, p_game->m_mousePosX/64, p_game->m_mousePosY/64);
		if (attaquant) {
			if (defense &&  playerdefenseID != p_game->m_playerTurn) //Attaque
			{
				//ARTILLERIE (ROCKET LAUNCHER) 2 a 5 cases
				//RESTE CASE ADJACENTE
				if (attaquant->m_canFire) {
					if (attaquant->m_type == ROCKET_LAUNCHER)
					{
						dtmp = GetManhattanDistance(cdefense, cattaque);
						if (dtmp <= 5 && dtmp >= 2)
						{
							Atttack(p_game, attaquant, defense);
							attaquant->m_canFire = 0;
						}
					}
					else
					{
						dtmp = GetManhattanDistance(cdefense, cattaque);
						if (dtmp == 1)
						{
							Atttack(p_game, attaquant, defense);
							attaquant->m_canFire = 0;
						}
					}
				}

			}
			else //Deplacement 
			{
				/*afficherGraphByID(p_game->m_graph);
				afficherGraphByTID(p_game->m_graph);*/
			/*	defense = GetUnitFromPos(p_game, p_game->m_mousePosX, p_game->m_mousePosY, &playerdefenseID);
				if (!defense)
				{
					dtmp = attaquant->m_walkGraph[(p_game->m_mousePosX / 64) + (p_game->m_mousePosY / 64) * p_game->m_graph->m_sizeX]->m_distance;

					if (dtmp <= attaquant->m_pm)
					{
						attaquant->m_posX = p_game->m_mousePosX / 64;
						attaquant->m_posY = p_game->m_mousePosY / 64;
						attaquant->m_pm -= dtmp;
						CalculateMovement(p_game->m_graph, attaquant);
						attaquant->m_selected = 0;
					}
				}
				else
					printf("Deplacement impossible");
			}
		}*/
		
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
