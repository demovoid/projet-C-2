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
	for (i = 0; i < p_game->m_graph->m_sizeY; i++)
	{
		for (j = 0; j < p_game->m_graph->m_sizeX; j++)
		{
			nodeSDL* n = GetNodeSDL(p_game->m_graph->m_data[index]);
			DrawSprite(p_window, n->m_sprite);
			index++;
		}
	}
	index = 0;

	// TODO :	Affichage des cases semi-transparentes pour indiquer la possibilit� de marcher
	SDL_Rect cases;
	SDL_Surface* Rouge = SDL_LoadBMP("./data/fireLayer.bmp");
	
	

	unit* sUnit = GetSelectedUnit(p_game);
	sprite* sprit = NULL;
	cases.w = 64;
	cases.h = 64;
	SDL_SetAlpha(Rouge, SDL_SRCALPHA, 128);
	if (sUnit)
	{
		if(sUnit->m_selected == 1)
			for (j = 0; j < p_game->m_graph->m_sizeY; j++)
			{
				for (i = 0; i < p_game->m_graph->m_sizeX; i++)
				{
					if (sUnit->m_walkGraph[j * p_game->m_graph->m_sizeX + i]->m_distance <= sUnit->m_pm) { 
						cases.x = i * 64;
						cases.y = j * 64;
						SDL_BlitSurface(p_game->m_surfaceWalk, NULL, p_window, &cases);
					}
				}
			}
		else {
			int dist;
			node *src, *dest;
			for (int i = 0; i < p_game->m_graph->m_sizeY; i++)
				for (int j = 0; j < p_game->m_graph->m_sizeX; j++) {
					src = GetNodeFromPosition(p_game->m_graph,sUnit->m_posX,sUnit->m_posY);
					dest = GetNodeFromPosition(p_game->m_graph, j, i);
					dist = GetManhattanDistance(src, dest);
					if (sUnit->m_type->m_type == ROCKET_LAUNCHER){
						if (dist <= 5 && dist >= 2) {
							cases.x = j * 64;
							cases.y = i * 64;
							SDL_BlitSurface(Rouge, NULL, p_window, &cases); //afficher case rouge olala
						}
					}
					else {
						if (dist == 1) {
							cases.x = j * 64;
							cases.y = i * 64;
							SDL_BlitSurface(Rouge, NULL, p_window, &cases); //afficher case rouge olala
						}
					}
				}
		}
	}
	
	char HP[6];
	SDL_Surface* HPD;
	SDL_Rect text;
	SDL_Rect textf;
	SDL_Surface* Fire;
	char canfire;
	HP[1] = ' ';
	HP[2] = ' ';
	HP[3] = 'H';
	HP[4] = 'P';
	HP[5] = '\0';
	for (k = 0; k < 2; k++){
		for (l = 0; l < p_game->m_players[k]->m_nbUnit; l++){
			canfire = p_game->m_players[k]->m_units[l]->m_canFire;
			if (p_game->m_players[k]->m_units[l]->m_hp == 10)
			{
				HP[0] = '1';
				HP[1] = '0';
			}
			else
			{
				HP[0] = '0' + p_game->m_players[k]->m_units[l]->m_hp;
				HP[1] = ' ';
			}
			if (!k) HPD = TTF_RenderText_Blended(p_game->m_fontHP, HP, (SDL_Color) { 255, 0, 0 });
			else HPD = TTF_RenderText_Blended(p_game->m_fontHP, HP, (SDL_Color) { 0, 0, 255 }); 
			Fire = TTF_RenderText_Blended(p_game->m_fontHP, "FIRE", (SDL_Color) { 255 * (!k) * canfire, 0, 255 * k * canfire});


			text.x = p_game->m_players[k]->m_units[l]->m_posX * 64;
			text.y = (p_game->m_players[k]->m_units[l]->m_posY+1) * 64 - 8;
			textf.x = text.x;
			textf.y = text.y + 16;
			SDL_BlitSurface(HPD, NULL, p_window, &text);
			SDL_BlitSurface(Fire, NULL, p_window, &textf);
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
	SDL_FreeSurface(Rouge);
	SDL_FreeSurface(HPD);
	SDL_FreeSurface(Fire);
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
	if (!p_game)
		return NULL;

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < p_game->m_players[i]->m_nbUnit; j++)
		{
			if (p_game->m_players[i]->m_units[j]->m_hp > 0)
			{
				if (p_game->m_players[i]->m_units[j]->m_posX * 64 <= p_posX && p_posX <= (p_game->m_players[i]->m_units[j]->m_posX + 1) * 64)
				{
					if (p_game->m_players[i]->m_units[j]->m_posY * 64 <= p_posY && p_posY <= (p_game->m_players[i]->m_units[j]->m_posY + 1) * 64)
					{
						*p_playerID = i;
						return p_game->m_players[i]->m_units[j];
					}
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
	double D = 0;
	int i = 0;
	int damage = s_damageChart[p_defender->m_type->m_type][p_attacker->m_type->m_type];
	int attaquant_hp = p_attacker->m_hp;
	int groundDefense = s_defenseGround[GetNodeFromPosition(p_game->m_graph, p_defender->m_posX, p_defender->m_posY)->m_layerID];
	int defense_hp = p_defender->m_hp;
	D = (damage * attaquant_hp * 0.1 * (1 - groundDefense * (0.1 - 0.01 * (10 - defense_hp)))) / 10;
	p_defender->m_hp -= (int)D;
	p_attacker->m_canFire = 0;
	//v�rification de mort
	if (p_defender->m_hp <= 0) {
		player* j = p_game->m_players[1-p_game->m_playerTurn];

		for (i = 0; i < j->m_nbUnit && j->m_units[i] != p_defender; i++);

		if (i != j->m_nbUnit) {
			FreeDijkstra(j->m_units[i]->m_walkGraph, p_game->m_graph->m_sizeX* p_game->m_graph->m_sizeY);
			free(j->m_units[i]);
			j->m_nbUnit--;
			j->m_units[i] = j->m_units[j->m_nbUnit];
			j->m_units[j->m_nbUnit + 1] = NULL;
		}
	}

}
