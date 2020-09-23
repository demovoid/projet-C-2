#pragma once

#ifdef _LIB_C_
#define LIB_API __declspec(dllexport)
#else
#define LIB_API __declspec(dllimport)
#endif


/*! \file graph.h
\brief Fichier contenant les fonctions utilisees pour le programme
Fonction d'affichage du graphe, d'allocation memoire du fichier contenant le graphe, de liberation , algorithme de Dijkstra, de recuperation des positions des Noeuds, de determination des noeuds adjacents, d'affectation des donnees des noeuds et de recuperation de ces donnees
*/
#include <stdlib.h>
#include <stdio.h>

#define INFINITY_DIST		0xFFFF

typedef unsigned char uchar;
typedef struct sDijkstraNode dijkstraNode;
typedef struct sGraph graph;
typedef struct sNode node;

enum eGroundMask
{
	NOTHING		= 0b00000000,

	GRASS		= 0b00000001,
	FOREST		= 0b00000010,
	MOUNTAIN	= 0b00000100,
	CITY		= 0b00001000,
	ROAD		= 0b00010000,
	WATER		= 0b00100000,

	EVERYTHING	= 0b11111111
};

enum eGroundID
{
	GRASS_ID,
	FOREST_ID,
	MOUNTAIN_ID,
	CITY_ID,
	ROAD_ID,
	WATER_ID,
	NB_GROUND
};

static const char s_groundChar[NB_GROUND] = {
	'G',
	'F',
	'M',
	'C',
	'R',
	'W'
};

struct sGraph
{
	// TODO

	int m_sizeX;
	int m_sizeY;
	node** m_data;

};

struct sNode
{
	// TODO

	int m_id;
	int m_posX;
	int m_posY;
	char m_layer;
	int m_layerID;
	node* m_neighbors[4];
	void* m_data;
};

struct sDijkstraNode
{
	// TODO
	node* m_node;
	int m_distance;
	int m_flag;
	struct sDijkstraNode* m_prev;

};

// TODO

									 
LIB_API graph* LoadGraphFromFile(char* path);/*!
									 \brief Lit la carte dans le dossier data et recupere les valeurs à l'interieur
									 \param[in] Chemin du dossier
									 \return Renvoie le Graphe initialise avec tous les noeuds dans m_data
									 */
LIB_API void FreeGraph(graph* Graph);/*!
									 \brief Libere la memoire du Graph
									 \param[in] Graphique qu'il faut liberer
									 */
LIB_API int GetLayerIDFromChar(char value);/*! 
									 \brief Donne l'ID du type de terrain 
									 \param[in] Caractere alphabetique que tu veux changer en ID
									 */
LIB_API dijkstraNode** Dijkstra(graph* Graph, node* finalNode, uchar masque);/*! 
									 \brief Fait l'algo de Dijkstra et renvoie un tableau contenant les chemins les plus courts pour aller à Node
									 \param[in] Graph dans lequel tu veux chercher les plus courts chemins.
									 \param[in] Noeud final, fin du chemin
									 \param[in] Masque permettant de choisir quel type de chemin on prend(herbe, route, eau,etc...)
									 \return Retourne une liste avec toutes les distances de tout les chemins les plus courts
									 */
LIB_API node* GetNodeFromPosition(graph* Graph, uchar posX, uchar posY);/*! 
									 \brief Trouve un noeud avec la position sur la carte
									 \param[in] Graph
									 \param[in] position en X
									 \param[in] position en Y
									 \return noeud correspondant
									 */
LIB_API int GetManhattanDistance(node* n1, node* n2);/*!
									 \brief Calcul la distance de manhattan entre deux noeuds
									 \param[in] Premier noeud
									 \param[in] Deuxieme noeud
									 \return Distance de Manhatta entre les deux noeuds
									 */
LIB_API char IsNeighbour(node* n1, node* n2);/*!
									 \brief Renvoie 1 si les deux noeuds sont voisins sinon 0
									 \param[in] Premier noeud
									 \param[in] Deuxieme noeud
									 \return 1 ou 0
									 */
LIB_API void SetNodeData(node* n, void* data);/*!
									 \brief Met une adresse dans le noeuds
									 \param[in] Noeud dans lequel tu veux mettre la donnee
									 \param[in] Adresse/Donnee
									 */
LIB_API void* getNodeData(node* n);/*! 
									 \brief Recupere la valeur dans le Noeuds
									 \param[in] Noeud
									 \return Renvoie l'adresse
									 */