#include "Minimap.h"
#include "../Graphics.h"
#include "../Data/Building.h"
#include "../Data/CityView.h"
#include "../Data/Constants.h"
#include "../Data/Graphics.h"
#include "../Data/Grid.h"
#include "../Data/Mouse.h"
#include "../Data/Scenario.h"
#include "../Data/Settings.h"

#define FOREACH_XY_VIEW(block)\
	int odd = 0;\
	int yAbs = minimapAbsoluteY - 4;\
	int yView = yOffset - 4;\
	for (int yRel = -4; yRel < heightTiles + 4; yRel++, yAbs++, yView++) {\
		int xView;\
		if (odd) {\
			xView = xOffset - 9;\
			odd = 0;\
		} else {\
			xView = xOffset - 8;\
			odd = 1;\
		}\
		int xAbs = minimapAbsoluteX - 4;\
		for (int xRel = -4; xRel < widthTiles; xRel++, xAbs++, xView += 2) {\
			if (xAbs < 0 || xAbs >= VIEW_X_MAX) continue;\
			if (yAbs < 0 || yAbs >= VIEW_Y_MAX) continue;\
			block;\
		}\
	}

static void setBounds(int xOffset, int yOffset, int widthTiles, int heightTiles);
static void drawMinimap(int xOffset, int yOffset, int widthTiles, int heightTiles);
static int drawWalker(int xView, int yView, int gridOffset);
static void drawTile(int xView, int yView, int gridOffset);
static void drawViewportRectangle(int xView, int yView, int widthTiles, int heightTiles);
static int getMouseGridOffset(int xOffset, int yOffset, int widthTiles, int heightTiles);

static int minimapAbsoluteX;
static int minimapAbsoluteY;
static int minimapLeft;
static int minimapTop;
static int minimapRight;
static int minimapBottom;
static Color soldierColor;
static Color enemyColor;

void UI_Minimap_draw(int xOffset, int yOffset, int widthTiles, int heightTiles)
{
	Graphics_setClipRectangle(xOffset, yOffset, 2 * widthTiles, heightTiles);
	
	soldierColor = 0xf000;
	if (Data_Scenario.climate == Climate_Central) {
		enemyColor = 0x7800;
	} else if (Data_Scenario.climate == Climate_Northern) {
		enemyColor = 0x181F;
	} else {
		enemyColor = 0x080F;
	}

	setBounds(xOffset, yOffset, widthTiles, heightTiles);
	drawMinimap(xOffset, yOffset, widthTiles, heightTiles);
	drawViewportRectangle(xOffset, yOffset, widthTiles, heightTiles);

	Graphics_resetClipRectangle();
}

static void setBounds(int xOffset, int yOffset, int widthTiles, int heightTiles)
{
	minimapAbsoluteX = (VIEW_X_MAX - widthTiles) / 2;
	minimapAbsoluteY = (VIEW_Y_MAX - heightTiles) / 2;
	minimapLeft = xOffset;
	minimapTop = yOffset;
	minimapRight = xOffset + 2 * widthTiles;
	minimapBottom = yOffset + heightTiles;

	if ((Data_Scenario.mapSizeX - widthTiles) / 2 > 0) {
		if (Data_CityView.xInTiles < minimapAbsoluteX) {
			minimapAbsoluteX = Data_CityView.xInTiles;
		} else if (Data_CityView.xInTiles > widthTiles + minimapAbsoluteX - Data_CityView.widthInTiles) {
			minimapAbsoluteX = Data_CityView.widthInTiles + Data_CityView.xInTiles - widthTiles;
		}
	}
	if ((2 * Data_Scenario.mapSizeY - heightTiles) / 2 > 0) {
		if (Data_CityView.yInTiles < minimapAbsoluteY) {
			minimapAbsoluteY = Data_CityView.yInTiles;
		} else if (Data_CityView.yInTiles > heightTiles + minimapAbsoluteY - Data_CityView.heightInTiles) {
			minimapAbsoluteY = Data_CityView.heightInTiles + Data_CityView.yInTiles - heightTiles;
		}
	}
	// ensure even height
	minimapAbsoluteY &= ~1;
}

static void drawMinimap(int xOffset, int yOffset, int widthTiles, int heightTiles)
{
	FOREACH_XY_VIEW(
		int gridOffset = ViewToGridOffset(xAbs, yAbs);
		drawTile(xView, yView, gridOffset);
	);
}

static int drawWalker(int xView, int yView, int gridOffset)
{
	Color color = 0;
	int hasWalker = 0;

	int walkerId = Data_Grid_walkerIds[gridOffset];
	//while (walkerId > 0) {
		// TODO walker on tile
		//if (Data_Walkers[walkerId].type 
	//}
	if (hasWalker) {
		Graphics_drawLine(xView, yView, xView+1, yView, color);
		return 1;
	} else {
		return 0;
	}
}

static void drawTile(int xView, int yView, int gridOffset)
{
	if (gridOffset < 0) {
		Graphics_drawImage(GraphicId(ID_Graphic_MinimapBlack), xView, yView);
		return;
	}

	if (drawWalker(xView, yView, gridOffset)) {
		return;
	}
	
	int terrain = Data_Grid_terrain[gridOffset];
	// exception for fort ground: display as empty land
	if (terrain & Terrain_Building) {
		if (Data_Buildings[Data_Grid_buildingIds[gridOffset]].type == Building_FortGround) {
			terrain = 0;
		}
	}

	if (terrain & Terrain_Building) {
		if (Data_Grid_edge[gridOffset] & Edge_leftmostTile) {
			int graphicId;
			int buildingId = Data_Grid_buildingIds[gridOffset];
			if (Data_Buildings[buildingId].houseSize) {
				graphicId = GraphicId(ID_Graphic_MinimapHouse);
			} else if (Data_Buildings[buildingId].type == Building_Reservoir) {
				graphicId = GraphicId(ID_Graphic_MinimapAqueduct) - 1;
			} else {
				graphicId = GraphicId(ID_Graphic_MinimapBuilding);
			}
			switch (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
				case 0:
					Graphics_drawImage(graphicId, xView, yView); break;
				case 1:
					Graphics_drawImage(graphicId + 1, xView, yView - 1); break;
				case 2:
					Graphics_drawImage(graphicId + 2, xView, yView - 2); break;
				case 4:
					Graphics_drawImage(graphicId + 3, xView, yView - 3); break;
				case 8:
					Graphics_drawImage(graphicId + 4, xView, yView - 4); break;
			}
		}
	} else {
		int rand = Data_Grid_random[gridOffset];
		int graphicId;
		if (terrain & Terrain_Water) {
			graphicId = GraphicId(ID_Graphic_MinimapWater) + (rand & 3);
		} else if (terrain & Terrain_Scrub) {
			graphicId = GraphicId(ID_Graphic_MinimapTree) + (rand & 3);
		} else if (terrain & Terrain_Tree) {
			graphicId = GraphicId(ID_Graphic_MinimapTree) + (rand & 3);
		} else if (terrain & Terrain_Rock) {
			graphicId = GraphicId(ID_Graphic_MinimapRock) + (rand & 3);
		} else if (terrain & Terrain_Elevation) {
			graphicId = GraphicId(ID_Graphic_MinimapRock) + (rand & 3);
		} else if (terrain & Terrain_Road) {
			graphicId = GraphicId(ID_Graphic_MinimapRoad);
		} else if (terrain & Terrain_Aqueduct) {
			graphicId = GraphicId(ID_Graphic_MinimapAqueduct);
		} else if (terrain & Terrain_Wall) {
			graphicId = GraphicId(ID_Graphic_MinimapWall);
		} else if (terrain & Terrain_Meadow) {
			graphicId = GraphicId(ID_Graphic_MinimapMeadow) + (rand & 3);
		} else {
			graphicId = GraphicId(ID_Graphic_MinimapEmptyLand) + (rand & 7);
		}
		Graphics_drawImage(graphicId, xView, yView);
	}
}

static void drawViewportRectangle(int xView, int yView, int widthTiles, int heightTiles)
{
	int xOffset = xView + 2 * (Data_CityView.xInTiles - minimapAbsoluteX) - 2;
	if (xOffset < xView) {
		xOffset = xView;
	}
	if (xOffset + 2 * Data_CityView.widthInTiles + 4 > xView + widthTiles) {
		xOffset -= 2;
	}
	int yOffset = yView + Data_CityView.yInTiles - minimapAbsoluteY + 2;
	Graphics_drawRect(xOffset, yOffset,
		Data_CityView.widthInTiles * 2 + 4,
		Data_CityView.heightInTiles - 4,
		Color_Yellow);
}

static int getMouseGridOffset(int xOffset, int yOffset, int widthTiles, int heightTiles)
{
	setBounds(xOffset, yOffset, widthTiles, heightTiles);
	FOREACH_XY_VIEW(
		if (Data_Mouse.y == yView && (Data_Mouse.x == xView || Data_Mouse.x == xView + 1)) {
			int gridOffset = ViewToGridOffset(xAbs, yAbs);
			return gridOffset < 0 ? 0 : gridOffset;
		}
	);
	return 0;
}

void UI_Minimap_handleClick()
{

}
