#include "game.h"

int LoadSprites(game* p_game, const char* p_path)
{
	char pathSprite[255];
	SDL_Rect src, dest;
	FILE* file;

	fopen_s(&file, p_path, "r");
	if (!file)
		return 0;

	dest.x = dest.y = 0;
	dest.w = dest.h = 64;

	/////////////////////
	//	GROUND LOADING //
	/////////////////////

	fscanf_s(file, "%s\n", pathSprite, 255);
	fscanf_s(file, "%hd %hd\n", &src.w, &src.h);

	for (size_t i = 0; i < NB_GROUND; i++)
	{
		char c;
		fscanf_s(file, "%c %hd %hd\n", &c, 1, &src.x, &src.y);

		if (i == 0)
			p_game->m_groundSpritesTab[GetLayerIDFromChar(c)] = LoadSprite(pathSprite, src, dest);
		else
			p_game->m_groundSpritesTab[GetLayerIDFromChar(c)] = LoadSpriteWithImage(pathSprite, p_game->m_groundSpritesTab[0]->m_image, src, dest);
	}

	fclose(file);

	return 1;
}

int LoadUnitType(game* p_game, const char* p_path)
{
	// TODO :	Chargement des types d'unit�s
	if (!p_game)
		return 0;

	FILE* file = NULL;

	fopen_s(&file, p_path, "r");
	if (!file)
		return 0;

	char pathSprite[255];
	SDL_Rect src, dest;
	dest.x = dest.y = 0;
	dest.w = dest.h = 64;

	fscanf_s(file, "%s\n", pathSprite, 255);
	fscanf_s(file, "%hd %hd\n", &src.w, &src.h);

	char c;
	int mask, pm, ind;

	for (int i = 0; i < NB_UNIT_TYPE; i++) {
		fscanf_s(file, "%c %hd %hd", &c, 1, &src.x, &src.y);
		fscanf(file, "%d%d\n", &mask, &pm);

		ind = GetUnitIDFromChar(c);

		p_game->m_unitTab[ind] = malloc(sizeof(unitType));

		//TODO: sprite
		if (i == 0)
			p_game->m_unitTab[ind]->m_sprite[0] = LoadSprite(pathSprite, src, dest);
		else 
			p_game->m_unitTab[ind]->m_sprite[0] = LoadSpriteWithImage(pathSprite, p_game->m_unitTab[0]->m_sprite[0]->m_image, src, dest);

		src.y = src.h;
		p_game->m_unitTab[ind]->m_sprite[1] = LoadSpriteWithImage(pathSprite, p_game->m_unitTab[0]->m_sprite[0]->m_image, src, dest);
		
		p_game->m_unitTab[ind]->m_pmMax = pm;
		p_game->m_unitTab[ind]->m_layerMask = mask;
		p_game->m_unitTab[ind]->m_type = GetUnitIDFromChar(c);
	}

	fclose(file);

	return 1;
}

int LoadPlayer(game* p_game, int p_idPLayer, const char* p_path)
{
	// TODO :	Chargement des joueurs
	if (!p_game)
		return 0;

	FILE* file = NULL;

	fopen_s(&file, p_path, "r");
	if (!file)
		return 0;

	int nbUnit, typeId, posX, posY;
	player* j = malloc(sizeof(player));
	if (!j) {
		fclose(file);
		return 0;
	}

	p_game->m_players[p_idPLayer] = j;

	fscanf(file, "%d", &nbUnit);
	p_game->m_players[p_idPLayer]->m_nbUnit = nbUnit;
	p_game->m_players[p_idPLayer]->m_units = malloc(sizeof(unit*) * nbUnit);

	for (int i = 0; i < nbUnit; i++) {
		fscanf(file, "%d%d%d", &typeId, &posX, &posY);
		j = p_game->m_players[p_idPLayer];
		j->m_units[i] = malloc(sizeof(unit));

		j->m_units[i]->m_posX = posX > 0 ? (posX < p_game->m_graph->m_sizeX ? posX : p_game->m_graph->m_sizeX - 1) : 0;
		j->m_units[i]->m_posY = posY > 0 ? (posY < p_game->m_graph->m_sizeY ? posY : p_game->m_graph->m_sizeY - 1) : 0;
		j->m_units[i]->m_type = p_game->m_unitTab[typeId];

		//init
		j->m_units[i]->m_hp = 10;
		j->m_units[i]->m_canFire = 1;
		j->m_units[i]->m_pm = j->m_units[i]->m_type->m_pmMax;
		j->m_units[i]->m_selected = 0;
		CalculateMovement(p_game->m_graph, j->m_units[i]);
	}

	fclose(file);

	return 1;
}

game* InitGame()
{
	game* result = NULL;
	int i;

	result = (game*)calloc(1, sizeof(game));
	if (!result)
		return NULL;

	result->m_font = TTF_OpenFont("./data/fonts/font_1.ttf", 65);
	result->m_fontHP = TTF_OpenFont("./data/fonts/font_1.ttf", 20);
	result->m_textP1 = TTF_RenderText_Blended(result->m_font, "Joueur 1", (SDL_Color){ 0, 0, 0 });
	result->m_textP2 = TTF_RenderText_Blended(result->m_font, "Joueur 2", (SDL_Color){ 0, 0, 0 });

	LoadSprites(result, "./data/sprites.ini");
	LoadUnitType(result, "./data/unitType.ini");

	result->m_graph = LoadGraphFromFile("./data/1_map.ini");
	if (!result->m_graph)
		return NULL;

	for (i = 0; i < result->m_graph->m_sizeX * result->m_graph->m_sizeY; i++)
	{
		sprite* s = CopySprite(result->m_groundSpritesTab[result->m_graph->m_data[i]->m_layerID]);
		MoveSprite(s, result->m_graph->m_data[i]->m_posX * 64, result->m_graph->m_data[i]->m_posY * 64);
		SetNodeSDL(result->m_graph->m_data[i], CreateNodeSDL(s));
	}

	LoadPlayer(result, 0, "./data/1_player1.ini");
	LoadPlayer(result, 1, "./data/1_player2.ini");

	result->m_surfaceWalk = SDL_LoadBMP("./data/walkLayer.bmp");
	SDL_SetAlpha(result->m_surfaceWalk, SDL_SRCALPHA, 128);

	result->m_playerTurn = 0;

	return result;
}

void DrawGame(SDL_Surface* p_window, game* p_game)
{
	int i, j, k, l;
	int index = 0;
	int tmp = 0;
	dijkstraNode** walk = NULL;
	// Affichage du niveau
	/*for (i = 0; i < p_game->m_graph->m_sizeY; i++)
	{
		for (j = 0; j < p_game->m_graph->m_sizeX; j++)
		{
			nodeSDL* n = GetNodeSDL(p_game->m_graph->m_data[index]);
			DrawSprite(p_window, n->m_sprite);
			index++;
		}
	}
	index = 0;*/

	for(int i = 0; i < p_game->m_graph->m_sizeY*p_game->m_graph->m_sizeX; i++){
		nodeSDL* n = GetNodeSDL(p_game->m_graph->m_data[i]);
		DrawSprite(p_window, n->m_sprite);
	}

	// TODO :	Affichage des cases semi-transparentes pour indiquer la possibilit� de marcher
	/*for (i = 0; i < NB_UNIT_TYPE; i++)
	{
		if (p_game->m_players[p_game->m_playerTurn]->m_units[i]->m_selected == 1)
			k = i;
	}
	walk = p_game->m_players[p_game->m_playerTurn]->m_units[k]->m_walkGraph;
	for (i = 0; i < p_game->m_graph->m_sizeY; i++)
	{
		for (j = 0; j < p_game->m_graph->m_sizeX; j++)
		{
			if (walk[index]->m_distance < p_game->m_players[p_game->m_playerTurn]->m_units[k]->m_type->m_pmMax)
			{

			}
			index++;
		}
	}*/

	unit* sUnit = GetSelectedUnit(p_game);
	sprite* sprit = NULL;

	for(int b = 0; b < p_game->m_graph->m_sizeY*p_game->m_graph->m_sizeX; b++){
		if(sUnit->m_walkGraph[b]->m_distance <= sUnit->m_pm){ //TODO: ajouter la vérification du au blocage par une autre unité (potentiellement dans dijkstra ou calculate movement)
			//afficher via p_game->m_surfaceWalk
		}
	}

	// TODO :	Affichage des unit�s
	for (k = 0; k < 2; k++){
		for (l = 0; l < p_game->m_players[k]->m_nbUnit; l++){
			sprit = p_game->m_players[k]->m_units[l]->m_type->m_sprite[k];
			MoveSprite(sprit, p_game->m_players[k]->m_units[l]->m_posX*64, p_game->m_players[k]->m_units[l]->m_posY*64);
			DrawSprite(p_window, sprit);
		}
	}
	

	// Affichage du texte
	if(p_game->m_playerTurn == 0)
		SDL_BlitSurface(p_game->m_textP1, NULL, p_window, NULL);
	else
	{
		SDL_Rect dest;
		dest.x = p_window->w - p_game->m_textP2->w;
		dest.y = 0;

		SDL_BlitSurface(p_game->m_textP2, NULL, p_window, &dest);
	}
}

unit* GetSelectedUnit(game* p_game)
{
	int index, j;
	index = p_game->m_playerTurn;

	for (size_t j = 0; j < p_game->m_players[index]->m_nbUnit; j++)
	{
		if (p_game->m_players[index]->m_units[j]->m_selected)
			return p_game->m_players[index]->m_units[j];
	}

	return NULL;
}

unit* GetUnitFromPos(game* p_game, int p_posX, int p_posY, int* p_playerID)
{
	// TODO :	Fonction retournant le pointeur vers l'unit� � la position pass�e en param�tre
	//			p_playerID est une variable retourn�e s'il existe une unit� � la position demand�e
	if (!p_game)
		return NULL;

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < NB_UNIT_TYPE; j++)
		{
			if (p_game->m_players[i]->m_units[j]->m_hp > 0)
			{
				if (p_game->m_players[i]->m_units[j]->m_posX == p_posX && p_game->m_players[i]->m_units[j]->m_posY == p_posY)
				{
					*p_playerID = i;
					return p_game->m_players[i]->m_units[j];
				}
			}
		}
	}
	return NULL;
}

void CalculateMovement(graph* p_graph, unit* p_unit)
{
	p_unit->m_walkGraph = Dijkstra(p_graph, GetNodeFromPosition(p_graph, p_unit->m_posX, p_unit->m_posY), p_unit->m_type->m_layerMask);
}

void ResetPlayers(game* p_game)
{
	// TODO :	Initialises les variables des unit� en d�but de tour
	player* j = NULL;
	for (int k = 0; k < 2; k++) {
		j = p_game->m_players[k];
		for (int i = 0; i < j->m_nbUnit; i++) {
			j->m_units[i]->m_canFire = 1;
			j->m_units[i]->m_pm = j->m_units[i]->m_type->m_pmMax;
			j->m_units[i]->m_selected = 0;
			CalculateMovement(p_game->m_graph, j->m_units[i]);
		}
	}

}

void Atttack(game* p_game, unit* p_attacker, unit* p_defender)
{
	float D = 0;
	int R = 0, i;
	D = s_damageChart[p_defender->m_type->m_type][p_attacker->m_type->m_type] * p_attacker->m_hp * 0,1;
	R = s_defenseGround[GetNodeFromPosition(p_game->m_graph, p_defender->m_posX, p_defender->m_posY)->m_layerID];
	D *= (1 - (R * (0, 1 - (0, 01 * (10 - p_defender->m_hp)))));
	D /= 10;
	p_defender->m_hp -= (int)D;

	//v�rification de mort
	if (p_defender->m_hp <= 0) {
		player* j = p_game->m_players[p_game->m_playerTurn];

		for (i = 0; i < j->m_nbUnit && p_game->m_players[p_game->m_playerTurn]->m_units[i] != p_defender; i++);

		if (i != j->m_nbUnit) {
			//FreeDijkstra(j->m_units[i]->m_walkGraph, p_game->m_graph->m_sizeX* p_game->m_graph->m_sizeY);
			free(j->m_units[i]);
			for (; i < j->m_nbUnit - 1; i++)
				j->m_units[i] = j->m_units[i + 1];
			j->m_units[i] = NULL;
		}
	}

}
