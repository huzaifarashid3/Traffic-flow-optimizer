#define OLC_PGE_APPLICATION
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include "olcPixelGameEngine.h"
#include <vector>
using namespace std;

class TrafficLight
{

};



class Game :public olc::PixelGameEngine
{
private:
	string map[20];
	string elements[20];
	int width = 20; 
	int height = 20; 
	olc::vi2d size = { 18,18}; 
	olc::vi2d border = { 1,1 }; 
	olc::vi2d padding = { 0,0 }; 
	olc::vi2d selected;
	struct node
	{
		bool visited = false;
		float global = INFINITY;
		float local = INFINITY;
		olc::vf2d pos = { 0,0 };
		vector<node*> neighbours;
		node* parent = NULL;
	};
	class Car
	{
	public:

		int ID;
		int radius;
		olc::vf2d pos;
		olc::vf2d sensor;
		olc::vf2d pos1;
		olc::vf2d pos2;
		olc::vf2d pos3;
		olc::vf2d size;
		olc::vf2d sensor_size;

		olc::vf2d vel;
		node* start;
		node* end;
		list<node*> path;
		list<node*>::iterator goal;

		Car(int ID = 0, olc::vf2d pos = { 0,0 }, olc::vf2d vel = { 1,1 }, int radius = 4, node* start = NULL, node* end = NULL):
			ID(ID), pos(pos), vel(vel), radius(radius), start(start), end(end), size({ 4,4 }), sensor_size({7,7}) {}
		
	};
	node** nodes = NULL;
	vector<Car> car;
	node* destination;
	node* spawnHere;
	vector<TrafficLight> tLight;

	bool collision = false;

public:
	Game()
	{
		sAppName = "Moving Cars";
	}


	bool OnUserCreate() override
	{

		for (int i = 0; i < height; i++)
			elements[i] = "                    ";
	
		//           01234567890123456789  
		map[0]   += "         du         ";
		map[1]   += "         du         ";
		map[2]   += "         du         ";
		map[3]   += "llllllllloolllllllll";
		map[4]   += "rrrrrrrrroorrrrrrrrr";
		map[5]   += "         du    du   ";
		map[6]   += "         du    du   ";
		map[7]   += "         du    du   ";
		map[8]   += "         du    du   ";
		map[9]   += "         du    dolll";
		map[10]  += "         du    dorrr";
		map[11]  += "         du    du   ";
		map[12]  += "         du    du   ";
		map[13]  += "         du    du   ";
		map[14]  += "         du    du   ";
		map[15]  += "llllllllloolllloolll";
		map[16]  += "rrrrrrrrroorrrrrrrrr";
		map[17]  += "         du         ";
		map[18]  += "         du         ";
		map[19]  += "         du         ";


		nodes = new node*[height];
		for (int i = 0; i < height; i++)
			nodes[i] = new node[width];
		
		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++)
				nodes[y][x].pos = { float(x),float(y) };
	

		for (int i = 0; i < 8; i++)
		{
			car.push_back(Car(i));
		}
		
		BuildNeighbours();


		car[0].start = &nodes[3][19];
		car[0].end = &nodes[19][9];
		car[0].pos = car[0].start->pos;

		car[1].start = &nodes[4][0];
		car[1].end = &nodes[16][19];
		car[1].pos = car[1].start->pos;
		
		car[2].start = &nodes[19][10];
		car[2].end = &nodes[4][19];
		car[2].pos = car[2].start->pos;

		car[3].start = &nodes[0][9];
		car[3].end = &nodes[15][0];
		car[3].pos = car[3].start->pos;

		car[4].start = &nodes[16][0];
		car[4].end = &nodes[0][10];
		car[4].pos = car[4].start->pos;

		car[5].start = &nodes[15][19];
		car[5].end = &nodes[3][0];
		car[5].pos = car[5].start->pos;
		
		car[6].start = &nodes[9][19];
		car[6].end = &nodes[0][10];
		car[6].pos = car[6].start->pos;
		
		car[7].start = &nodes[17][10];
		car[7].end = &nodes[10][15];
		car[7].pos = car[7].start->pos;

		for (auto& n : car)
		{
			GeneratePath(n);
		}


		return true;
	}

	bool OnUserUpdate(float ftime) override
	{
		Input();	
		Update(ftime);	
		Render();
		
		if (GetKey(olc::Key::X).bReleased)
			return false;
		return true;
	}

	
	
	
	void Input()
	{
		selected = { (GetMousePos() + border) / size };
		SelectStartEnd(car[0], olc::Key::A);

	}

	void Update(float ftime)
	{

		ModifyMap();

		ModifyElements();


		//generate a path;
		if (GetKey(olc::Key::SPACE).bReleased)
		{
			for (auto& n : car)
			{
				GeneratePath(n);
			}
		}


		for (auto& n : car)
		{
			MoveCar(n, ftime);
		}
	}

	void Render()
	{

		Clear(olc::BLACK);
		
		DrawMap();

		DrawDirections();

		DrawElements();


		for (auto& n : car)
		{
			DrawStartEnd(n);
		}	
	

		
		for (auto& n : car)
		{
			DrawCar(n);		
		}

		//DrawTest();

	}



	
	
	//INPUT FUNCTIONS
	void ModifyMap()
	{

		if (selected.x < width && selected.y < height)
		{
			if (GetKey(olc::Key::K).bHeld)
			{
				map[selected.y][selected.x] = 'd';
				BuildNeighbours();
			}
			else if (GetKey(olc::Key::I).bHeld)
			{
				map[selected.y][selected.x] = 'u';
				BuildNeighbours();
			}
			else if (GetKey(olc::Key::J).bHeld)
			{
				map[selected.y][selected.x] = 'l';
				BuildNeighbours();
			}
			else if (GetKey(olc::Key::L).bHeld)
			{
				map[selected.y][selected.x] = 'r';
				BuildNeighbours();
			}
			else if (GetKey(olc::Key::O).bHeld)
			{
				map[selected.y][selected.x] = 'o';
				BuildNeighbours();
			}
			else if (GetKey(olc::Key::Q).bHeld)
			{
				map[selected.y][selected.x] = 'q';
				BuildNeighbours();
			}
			else if (GetKey(olc::Key::W).bHeld)
			{
				map[selected.y][selected.x] = 'w';
				BuildNeighbours();
			}
			else if (GetKey(olc::Key::E).bHeld)
			{
				map[selected.y][selected.x] = 'e';
				BuildNeighbours();
			}

			else if (GetKey(olc::Key::T).bHeld)
			{
				map[selected.y][selected.x] = 't';
				BuildNeighbours();
			}

		}

		if (GetMouse(0).bHeld)
		{
			if (selected.x < width && selected.y < height)
			{
				map[selected.y][selected.x] = ' ';
				BuildNeighbours();
			}
		}
	}

	void ModifyElements()
	{
		if (selected.x < width && selected.y < height)
		{
			if (GetKey(olc::Key::Z).bHeld)
			{
				elements[selected.y][selected.x] = 'z';
			}

			if (GetKey(olc::Key::C).bHeld)
			{
				elements[selected.y][selected.x] = 'c';
			}
		}
	}

	void SelectStartEnd(Car& car , olc::Key key)
	{
		if (GetKey(olc::Key::SHIFT).bHeld && GetKey(key).bHeld)
		{
			car.start = &nodes[selected.y][selected.x];
		}
		else if (GetKey(olc::Key::CTRL).bHeld && GetKey(key).bHeld)
		{
			car.end = &nodes[selected.y][selected.x];
		}

	}


	
	
	
	//UPDATE FUNCTIONS
	void GeneratePath(Car& car)
	{
		
		if (car.start == NULL || car.end == NULL)
		{
			return;
		}
		car.pos = car.start->pos;
		car.path.clear();
		
		SolveAstar(car);
		if (car.end != NULL)
		{
			node* p = car.end;
			while (p->parent != NULL)
			{
				car.path.push_front(p);
				p = p->parent;
			}
		}
		cout << endl;
		for (auto const& n : car.path)
		{
			cout << n->pos.str() << endl;
		}
		car.goal = car.path.begin();

	}

	void BuildNeighbours()
	{

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				nodes[y][x].neighbours.clear();

				if (map[y][x] == ' ')
					continue;
				//right
				if (map[y][x] == 'r')
				{		
					if (x < width - 1)
						if (map[y][x + 1] != ' ' && map[y][x + 1] != 'l')
							nodes[y][x].neighbours.push_back(&nodes[y][x + 1]);

					//right to down
					if (y < height - 1)
						if (map[y + 1][x] == 'd')
							nodes[y][x].neighbours.push_back(&nodes[y + 1][x]);

					//right to up
					if (0 < y)
					if (map[y - 1][x] == 'u')
						nodes[y][x].neighbours.push_back(&nodes[y - 1][x]);
				}

				//left
				if (map[y][x] == 'l')
				{
					if (0 < x)
						if (map[y][x - 1] != ' ' && map[y][x - 1] != 'r')
							nodes[y][x].neighbours.push_back(&nodes[y][x - 1]);

					//left to down
					if (y < height - 1)
						if (map[y + 1][x] == 'd')
							nodes[y][x].neighbours.push_back(&nodes[y + 1][x]);

					//left to up
					if (y > 0)
						if (map[y - 1][x] == 'u')
							nodes[y][x].neighbours.push_back(&nodes[y - 1][x]);

				}

				//down
				if (map[y][x] == 'd')
				{
					if (y < height - 1)
						if (map[y + 1][x] != ' ' && map[y + 1][x] != 'u')
							nodes[y][x].neighbours.push_back(&nodes[y + 1][x]);

					//down right
					if (x < width - 1)
						if (map[y][x + 1] == 'r')
							nodes[y][x].neighbours.push_back(&nodes[y][x + 1]);

					//down to left
					if (0 < x)
						if (map[y][x - 1] == 'l')
							nodes[y][x].neighbours.push_back(&nodes[y][x - 1]);
				}

				

				//right
				if (map[y][x] == 'u')
				{
					if (0 < y)
						if (map[y - 1][x] != ' ' && map[y - 1][x] != 'd')
							nodes[y][x].neighbours.push_back(&nodes[y - 1][x]);

					//up to right
					if (x < width - 1)
						if (map[y][x + 1] == 'r')
							nodes[y][x].neighbours.push_back(&nodes[y][x + 1]);

					//down to left
					if (0 < x)
						if (map[y][x - 1] == 'l')
							nodes[y][x].neighbours.push_back(&nodes[y][x - 1]);
				}

				//intersection
				if (map[y][x] == 'o')
				{
					//act as a right
					if (x>0 && x<width-1)
						if (map[y][x - 1] == 'r' || map[y][x + 1] == 'r')
							nodes[y][x].neighbours.push_back(&nodes[y][x + 1]);
					
					//act as a left
					if (x > 0 && x < width - 1)
						if (map[y][x + 1] == 'l' || map[y][x - 1] == 'l')
							nodes[y][x].neighbours.push_back(&nodes[y][x - 1]);
					
					//act as a down
					if (y > 0 && y < height - 1)
						if (map[y - 1][x] == 'd' || map[y + 1][x] == 'd')
							nodes[y][x].neighbours.push_back(&nodes[y + 1][x]);
					
					//act as a up
					if (y > 0 && y < height - 1)
						if (map[y+1][x] == 'u' || map[y - 1][x] == 'u')
							nodes[y][x].neighbours.push_back(&nodes[y-1][x]);


				}


				if (map[y][x] == 'q') //down left
				{
					nodes[y][x].neighbours.push_back(&nodes[y][x - 1]);
					nodes[y][x].neighbours.push_back(&nodes[y+1][x]);

				}
				if (map[y][x] == 'w') //up left
				{
					nodes[y][x].neighbours.push_back(&nodes[y][x - 1]);
					nodes[y][x].neighbours.push_back(&nodes[y-1][x]);

				}
				if (map[y][x] == 'e') //right down
				{
					nodes[y][x].neighbours.push_back(&nodes[y][x + 1]);
					nodes[y][x].neighbours.push_back(&nodes[y+1][x]);

				}
				if (map[y][x] == 't') //right up
				{
					nodes[y][x].neighbours.push_back(&nodes[y][x + 1]);
					nodes[y][x].neighbours.push_back(&nodes[y-1][x]);

				}




			}
		}
	}

	void MoveCar(Car& car, float ftime)
	{

		if (!car.path.empty())
		{


			if (((*car.goal)->pos - car.pos).mag2() < 0.1)
			{
				if (next(car.goal, 1) != car.path.end())
					car.goal++;
			}
			else
			{		
				collision = false;

				olc::vf2d direction = ((*car.goal)->pos - car.pos).norm();
				float angle = atan2(direction.y, direction.x);

				car.pos1 = { cosf(angle + 0.0f), sinf(angle + 0.0f) };
				car.pos2 = { cosf(angle - 2 * M_PI / 3), sinf(angle - 2 * M_PI / 3) };
				car.pos3 = { cosf(angle + 2 * M_PI / 3), sinf(angle + 2 * M_PI / 3) };

				car.pos1 = car.pos1 * car.size / size;
				car.pos2 = car.pos2 * car.size / size;
				car.pos3 = car.pos3 * car.size / size;

				car.sensor = { cosf(angle + 0.0f), sinf(angle + 0.0f) };
				car.sensor = car.sensor * car.sensor_size / size;

				collision = checkCollision(car);
				if (!collision)
					car.pos += direction * car.vel * ftime;	
			}
		}
	}

	void SolveAstar(Car &car)
	{
		if (car.start == NULL || car.end == NULL)
		{
			return;
		}
		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++)
			{
				nodes[y][x].local = INFINITY;
				nodes[y][x].global = INFINITY;
				nodes[y][x].parent = NULL;
				nodes[y][x].visited = false;
			}

		auto distance = [](node* a, node* b)
		{
			return	(a->pos - b->pos).mag();
		};

		auto heuristic = [distance](node* a, node* b)
		{
			return distance(a, b);
			//return 1;
		};

		node* current = car.start;
		car.start->local = 0.0f;
		car.start->global = heuristic(car.start,car.end);

		list<node*> testNodes;
		testNodes.push_back(car.start);

		while (!testNodes.empty())
		{
			testNodes.sort([](const node* lhs, const node* rhs) {return lhs->global < rhs->global; });

			while (!testNodes.empty() && testNodes.front()->visited)
				testNodes.pop_front();

			if (testNodes.empty())
				break;

			current = testNodes.front();
			current->visited = true;

			for (auto n : current->neighbours)
			{
				if (!n->visited && !map[int(n->pos.y)][int(n->pos.x)] != ' ')
					testNodes.push_back(n);

				float lowerGoal = current->local + distance(current, n);

				if (lowerGoal < n->local)
				{
					n->parent = current;
					n->local = lowerGoal;
					n->global = n->local + heuristic(n, car.end);
				}
		
			}
		}
		return;
	}

	bool checkCollision(Car &car)
	{

		auto CheckOverlap = [](olc::vf2d point, olc::vf2d center, float radius)
		{
			if ((point - center).mag2() < radius * radius)
				return true;
			return false;
		};


		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++)
			{

				olc::vi2d testPoint(car.pos.x  + car.sensor.x + 0.5, car.pos.y + car.sensor.y + 0.5);

				if (elements[testPoint.y][testPoint.x] == 'z')
					return true;

			}

		for (auto& n : this->car)
		{
			if (car.ID == n.ID)
				continue;
			if (CheckOverlap(car.sensor + car.pos + olc::vf2d(0.5 , 0.5), n.pos + olc::vf2d(0.5, 0.5), float(n.radius) / size.x))
			{
				return true;
			}

		}
		return false;
	}
	
	
	
	//RENDER FUNCTIONS
	void DrawMap()
	{
		//draw lines between a node and its neighbours
		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++)
				for (auto n : nodes[y][x].neighbours)
					DrawLine(nodes[y][x].pos * size + size / 2, n->pos * size + size / 2, olc::DARK_YELLOW);


		//draw nodes
		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++)
			{
				if (map[y][x] == ' ')
					FillRect(nodes[y][x].pos * size + border, size - 2 * border, olc::DARK_GREY);
				else
					FillRect(nodes[y][x].pos * size + border, size - 2 * border, olc::DARK_BLUE);
				
				//Draw(olc::vf2d(x,y) * size, olc::YELLOW);
			}
	}

	void DrawElements()
	{
		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++)
			{
				if (elements[y][x] == 'z')
					FillRect(nodes[y][x].pos * size + border, size - 2 * border, olc::DARK_YELLOW);
			}
	}
	
	void DrawStartEnd(Car& car)
	{
		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++)
			{
				if (&nodes[y][x] == car.start)
					FillRect(nodes[y][x].pos * size + border, size - 2 * border, olc::DARK_GREEN);
				if (&nodes[y][x] == car.end)
					FillRect(nodes[y][x].pos * size + border, size - 2 * border, olc::DARK_RED);
			}

	}
	
	void DrawPath(Car& car)
	{
		node* p = car.end;
		if (car.end != NULL)
		{
			while (p->parent != NULL)
			{
				DrawLine(p->pos * size + size / 2, p->parent->pos * size + size / 2, olc::YELLOW);
				p = p->parent;
			}
		}
	}

	void DrawCar(Car& car)
	{
		FillTriangle((car.pos + car.pos1) * size + size/2, (car.pos + car.pos2) * size + size / 2, (car.pos + car.pos3) * size + size / 2,olc::MAGENTA);
		//DrawLine((car.sensor + car.pos)*size + size/2, car.pos * size + size / 2);
		Draw((car.sensor + car.pos) * size + size / 2,olc::CYAN);
		//Draw((car.pos) * size + size / 2,olc::CYAN);
		//((car.sensor + car.pos)*size + size/2);
	}

	void DrawDirections()
	{
		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++)
				DrawString(nodes[y][x].pos*size + size/2, string(1,map[y][x]));
	}	

	void DrawMousePos()
	{
		SetPixelMode(olc::Pixel::ALPHA);
		FillRect(selected * size + border, size - 2 * border,olc::GREY);
		SetPixelMode(olc::Pixel::NORMAL);

	}	
	
	void DrawTest()
	{

		olc::vi2d  abc(car[0].pos.x, car[0].sensor.y + car[0].pos.y);
		olc::vf2d abcd(car[0].pos.x, car[0].sensor.y + car[0].pos.y);

		
		
		DrawString(olc::vi2d(1, 1) * size + border, abc.str());
		DrawString(olc::vi2d(1, 2) * size + border, abcd.str());
		
		
		


		/*DrawString(olc::vi2d(1, 2) * size + border, to_string(int(car[0].pos.x)));
		DrawString(olc::vi2d(1, 3) * size + border, to_string((car[0].pos.x)));

		DrawString(olc::vi2d(1, 5) * size + border, to_string(int(car[0].pos.y)));
		DrawString(olc::vi2d(1, 6) * size + border, to_string((car[0].pos.y)));*/
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
	if (game.Construct(360, 360, 2, 2))
		game.Start();
	return 0;
}



// i want my car to have sensor in the direction of the travel
// if they collide with a light, they dont move