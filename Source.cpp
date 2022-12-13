#define OLC_PGE_APPLICATION
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include "olcPixelGameEngine.h"
#include <vector>
using namespace std;

class TrafficLight
{
public:
	olc::vf2d pos;
	float timer;
	int status; // 0 = green, 1 = red

};

class TrafficLightSystem
{
public:
	TrafficLight arr[4];
	olc::vf2d pos;
	int currentGreen = 0;
	float gTimer = 0.0f;
	float maxTime = 3.0f;

	TrafficLightSystem(olc::vf2d p = {8,4}, float t = 3.0f)
	{

		maxTime = t;
		pos = p;
		//right
		arr[0].pos = pos;
		arr[0].status = 0;
		//up
		arr[1].pos = pos + olc::vf2d(1, -2);
		arr[1].status = 1;
		//left
		arr[2].pos = pos + olc::vf2d(3, -1);
		arr[2].status = 1;
		//down
		arr[3].pos = pos + olc::vf2d(2, 1);
		arr[3].status = 1;
	}

};

class Game :public olc::PixelGameEngine
{
private:
	int IDtracker = 0;
	string map[20];
	int elements[20][20] = { 0 };
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

		Car(int ID = 0, node* start = NULL, node* end = NULL, olc::vf2d pos = { 0,0 }, olc::vf2d vel = { 6,6 }, int radius = 4):
			ID(ID), pos(pos), vel(vel), radius(radius), start(start), end(end), size({ 4,4 }), sensor_size({7,7}) {}
		
	};
	node** nodes = NULL;
	vector<Car> car;
	node* StartArr[7];
	node* EndArr[7];
	float gTimer = 0.0f;


	vector<TrafficLightSystem> tLight;

public:
	Game()
	{
		sAppName = "Moving Cars";
	}


	bool OnUserCreate() override
	{

		for (int i = 0; i < height; i++)
			for (int j = 0; j < width; j++)
				elements[i][j] = 0;
	
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
	

		StartArr[0] = &nodes[3][19];
		EndArr[0] = &nodes[19][9];
		StartArr[1] = &nodes[4][0];
		EndArr[1] = &nodes[16][19];
		StartArr[2] = &nodes[19][10];
		EndArr[2] = &nodes[4][19];
		StartArr[3] = &nodes[0][9];
		EndArr[3] = &nodes[15][0];
		StartArr[4] = &nodes[16][0];
		EndArr[4] = &nodes[0][10];
		StartArr[5] = &nodes[15][19];
		EndArr[5] = &nodes[3][0];
		StartArr[6] = &nodes[9][19];
		EndArr[6] = &nodes[0][10];

		BuildNeighbours();

		tLight.push_back(TrafficLightSystem({8,4},1));
		tLight.push_back(TrafficLightSystem({8,16},2));

		for (auto& t : tLight)
		{

			for (int i = 0; i < 4; i++)
			{
				olc::vi2d temp = t.arr[i].pos;
				elements[temp.y][temp.x] = 1;
			}
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
		//SelectStartEnd(car[0], olc::Key::A);

	}

	void Update(float ftime)
	{
		
		gTimer += ftime;
		if (gTimer > 0.5f)
		{
			SpawnCar();
			gTimer = 0.0f;		
		}

		ModifyMap();

		ModifyElements();

		int i = 0;
		for (auto& n : car)
		{
			MoveCar(n, ftime, i);
			i++;
		}

		for (auto& t : tLight)
		{
			SwitchLights(t, ftime);
		}
	}

	void Render()
	{

		Clear(olc::BLACK);
		
		DrawMap();

		DrawDirections();






		for (auto& n : car)
		{
			DrawStartEnd(n);
			//DrawPath(n);
			
		}

	
		for (auto& t : tLight)
		{
			DrawTLight(t);
		}


	//	DrawElements();

		
		for (auto& n : car)
		{
			DrawCar(n);		
		}

		DrawTest();


		DrawString(olc::vi2d(1, 1) * size, to_string(gTimer));

		CurrentNoCars();

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
		for (auto& t : tLight)
		{
			for (int i = 0; i < 4; i++)
			{
				olc::vi2d temp = t.arr[i].pos;
				elements[temp.y][temp.x] = t.arr[i].status;
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
		//car.goal = car.path.begin();

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

	void MoveCar(Car& car, float ftime, int counter)
	{	
		if (!car.path.empty())
		{		
			if ((car.path.front()->pos - car.pos).mag2() < 0.1)
			{
				car.path.pop_front();
			}
			else
			{
				olc::vf2d direction = (car.path.front()->pos - car.pos).norm();
				float angle = atan2(direction.y, direction.x);

				car.pos1 = { cosf(angle + 0.0f), sinf(angle + 0.0f) };
				car.pos2 = { cosf(angle - 2 * M_PI / 3), sinf(angle - 2 * M_PI / 3) };
				car.pos3 = { cosf(angle + 2 * M_PI / 3), sinf(angle + 2 * M_PI / 3) };

				car.pos1 = car.pos1 * car.size / size;
				car.pos2 = car.pos2 * car.size / size;
				car.pos3 = car.pos3 * car.size / size;

				car.sensor = { cosf(angle + 0.0f), sinf(angle + 0.0f) };
				car.sensor = car.sensor * car.sensor_size / size;


				if (!checkCollision(car))
					car.pos += direction * car.vel * ftime;

			}
		}
		else if (this->car.begin() + counter < this->car.end())
		{
			this->car.erase(this->car.begin() + counter);
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
		   // return 1;
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

				if (elements[testPoint.y][testPoint.x] == 1)
				{
					return true;			
				}

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
	
	void SwitchLights(TrafficLightSystem& tLight, float ftime)
	{

		tLight.gTimer += ftime;
		if (tLight.gTimer > tLight.maxTime)
		{
			tLight.arr[tLight.currentGreen].status = 1;
			tLight.currentGreen = (tLight.currentGreen + 1) % 4;
			tLight.arr[tLight.currentGreen].status = 0;
			tLight.gTimer = 0.0f;
		}

	
	}

	void SpawnCar()
	{
		Car c(IDtracker++, StartArr[rand() % 7], EndArr[rand() % 7]);
		car.push_back(c);
		GeneratePath(car.back());
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

	}
	
	void DrawTLight(TrafficLightSystem& tLight)
	{
		for (int i = 0; i < 4; i++)
		{
			FillRect(tLight.arr[i].pos * size, size, tLight.arr[i].status == 1 ? olc::RED: olc::GREEN);
		}
	}

	void CurrentNoCars()
	{
		DrawString(olc::vi2d(2, 2) * size, to_string(car.size()));
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

