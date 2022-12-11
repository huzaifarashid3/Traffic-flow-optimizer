#pragma once
struct node
{
	bool visited = false;
	float global = INFINITY;
	float local = INFINITY;
	olc::vf2d pos = { 0,0 };
	vector<node*> neighbours;
	node* parent = NULL;
};
