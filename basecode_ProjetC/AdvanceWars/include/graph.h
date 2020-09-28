#pragma once

#ifdef _LIB_C_
	#define	LIB_API	__declspec(dllexport)
#else
	#define	LIB_API	__declspec(dllimport)
#endif // _LIB_C_


#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define INFINITY_DIST		0xFFFF

typedef struct sDijkstraNode dijkstraNode;
typedef struct sGraph graph;
typedef struct sNode node;

enum eNeighbourPosition
{
	NP_UP,
	NP_DOWN,
	NP_LEFT,
	NP_RIGHT,
	NB_NEIGHBORS
};

enum eLayer
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

enum eLayerID
{
	GRASS_ID,
	FOREST_ID,
	MOUNTAIN_ID,
	CITY_ID,
	ROAD_ID,
	WATER_ID,
	NB_GROUND
};

static const char s_layerChar[NB_GROUND] = {
	'G',
	'F',
	'M',
	'C',
	'R',
	'W'
};

struct sGraph
{
	unsigned char	m_sizeX;
	unsigned char	m_sizeY;

	node **			m_data;
};

struct sNode
{
	unsigned int	m_id;
	unsigned char	m_posX;
	unsigned char	m_posY;

	unsigned char	m_layer;
	unsigned int	m_layerID;

	node *			m_neighbors[NB_NEIGHBORS];
	
	void *			m_data;
};

struct sDijkstraNode
{
	node *			m_node;
	int				m_distance;
	unsigned char	m_flag;

	dijkstraNode *	m_prev;
};

LIB_API graph* LoadGraphFromFile(char* p_path);
LIB_API void FreeGraph(graph* p_graph);

LIB_API int GetLayerIDFromChar(char p_layerChar);

LIB_API dijkstraNode** Dijkstra(graph * p_graph, node* p_begin, unsigned char p_layerMask);
LIB_API void FreeDijkstra(dijkstraNode ** p_dijkstra, int p_size);

LIB_API node* GetNodeFromPosition(graph * p_g, unsigned char p_posX, unsigned char p_posY);

LIB_API int GetManhattanDistance(node* p_begin, node* p_end);

LIB_API void SetNodeData(node* p_node, void* p_data);
LIB_API void* GetNodeData(node* p_node);
