#define OLC_PGE_APPLICATION
#include <iostream>
#include "olcPixelGameEngine.h"
using namespace std;

class Game :public olc::PixelGameEngine
{


private:
	int width = 16; //world space
	int height = 16; //world space
	olc::vf2d size = { 16,16 }; //transformation from world space to screen space
	olc::vf2d border = { 2,2 }; //screen space
	olc::vf2d padding = { 12,12 }; //screen space
	olc::vi2d selected;
	struct node
	{
		bool visited = false;
		bool obstacle = false;
		float global = INFINITY;
		float local = INFINITY;
		olc::vf2d pos = { 0,0 };
		vector<node*> neighbours;
		node* parent = NULL;
	};
	node** nodes = NULL;
	node* start = NULL;
	node* end = NULL;


public:
	bool OnUserCreate() override
	{
		nodes = new node*[height];
		for (int i = 0; i < height; i++)
			nodes[i] = new node[width];
		
		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++)
				nodes[y][x].pos = { float(x),float(y) };



		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				//southern neighbours
				if (y != height - 1)
					nodes[y][x].neighbours.push_back(&nodes[y + 1][x]);

				//northern neigbours
				if (y != 0)
					nodes[y][x].neighbours.push_back(&nodes[y - 1][x]);

				//eastern neighbours
				if (x != width - 1)
					nodes[y][x].neighbours.push_back(&nodes[y][x + 1]);

				//western neighbours
				if (x != 0)
					nodes[y][x].neighbours.push_back(&nodes[y][x - 1]);

			}
		}

		start = &nodes[12][2];
		end = &nodes[12][12];
				
		return true;
	}


	bool OnUserUpdate(float ftime) override
	{

		//get mouse coordinates in the node space
		selected = { ( GetMousePos() + border) / size };
		if (GetMouse(0).bReleased)
		{
			if (selected.x < width && selected.y < height)
			{
				if (GetKey(olc::Key::SHIFT).bHeld)
				{
					start = &nodes[selected.y][selected.x];
				}
				else if (GetKey(olc::Key::CTRL).bHeld)
				{
					end = &nodes[selected.y][selected.x];
				}
				else
					nodes[selected.y][selected.x].obstacle = !nodes[selected.y][selected.x].obstacle;
			}
		}
		start->obstacle = false;
		end->obstacle = false;





		Clear(olc::BLACK);

		//draw lines between a node and its neighbours
		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++)
				for (auto n : nodes[y][x].neighbours)
					DrawLine(nodes[y][x].pos * size + size / 2, n->pos * size + size / 2, olc::DARK_BLUE);


		//draw nodes
		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++)
			{
				if (&nodes[y][x] == start)
					FillRect(nodes[y][x].pos * size + border, size - 2*border,  olc::DARK_GREEN);
				else if (&nodes[y][x] == end)
					FillRect(nodes[y][x].pos * size + border, size - 2*border, olc::DARK_RED);
				else
					FillRect(nodes[y][x].pos * size + border, size - 2*border, nodes[y][x].obstacle ? olc::WHITE : olc::DARK_BLUE);
			}		
		return true;
	}


	void SolveAstar()
	{
		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++)
			{
				nodes[y][x].local = INFINITY;
				nodes[y][x].global = INFINITY;
				nodes[y][x].parent = NULL;
				nodes[y][x].visited = NULL;
			}

		auto distance = [](node* a, node* b)
		{
			return	(a->pos - b->pos).mag();
		};

		auto heuristic = [distance](node* a, node* b)
		{
			return distance(a, b);;
		};

		node* current = start;
		start->local = 0.0f;
		start->global = heuristic(start,end);

		list<node*> testNodes;
		testNodes.push_back(start);

		while (!testNodes.empty())
		{
			testNodes.sort([](const node* lhs, const node* rhs) {return lhs->global < rhs->global; });
		}



		return;
	}


	bool OnUserDestroy() override
	{
		for (int i = 0; i < height; i++)
			delete[] nodes[i];
		delete[] nodes;
		return true;
	}
};

int main(void)
{
	Game game;
	if (game.Construct(270, 270, 2, 2))
		game.Start();
	return 0;
}