#include "deform.h"
#include <cmath>
#include <stdio.h>

void weigh_nodes(Mesh* mesh)
{
  //@TODO: More advanced weighing algo.
}

v3 lap_on_mesh(int index, v3 pos, Mesh* mesh)
{
	v3 lapl_pos;
	
	for(int i = 0; i < mesh->vert_buf[index].nbs.size(); i++)
	{
		lapl_pos = lapl_pos + mesh->vert_buf[mesh->vert_buf[index].nbs[i]].pos * 
			 mesh->vert_buf[mesh->vert_buf[index].nbs[i]].weight;
	}
	lapl_pos = pos - lapl_pos;

	return lapl_pos;	
}

v3 lap_on_graph(int index, v3 pos, Mesh* mesh)
{
	v3 lapl_pos;

	for(int i = 0; i < mesh->vert_buf[index].vol_con.size(); i++)
	{
		auto in = mesh->vert_buf[index].vol_con[i];
		if(in.vol == 1)
			lapl_pos = lapl_pos + mesh->v_1[in.ind.x][in.ind.y][in.ind.z]->pos *
				 mesh->v_1[in.ind.x][in.ind.y][in.ind.z]->weight;
		else
			lapl_pos = lapl_pos + mesh->v_2[in.ind.x][in.ind.y][in.ind.z]->pos *
				 mesh->v_2[in.ind.x][in.ind.y][in.ind.z]->weight;

	}
	lapl_pos = pos - lapl_pos;
	
	return lapl_pos;	
}

void move_vert(int index, v3 transform, float str, v3 origin, std::vector<int>& visited, float avgLen, Mesh* mesh, int it)
{

	mesh->vert_buf[index].pos = mesh->vert_buf[index].pos + transform*str;
	visited.push_back(index);

	for(int i =  0; i < mesh->vert_buf[index].nbs.size(); i++)
	{
		bool vis = false;

		for(int j = 0; j < visited.size(); j++)
		{
			if(visited[j] == mesh->vert_buf[index].nbs[i])
			{
				vis = true;
				break;
			}
		}

		if(!vis)
		{
		
			v3 dl = mesh->vert_buf[mesh->vert_buf[index].nbs[i]].pos - origin;
			float len = sqrt(dl[0]*dl[0] + dl[1]*dl[1] + dl[2]*dl[2]);

			str = avgLen/len > 1.f ? 1.f : avgLen/len;

			//printf("Move vert\n");
			move_vert(mesh->vert_buf[index].nbs[i], transform, str, origin, visited, avgLen, mesh, it++);
		}
	}

	
	mesh->vert_buf[index].norm = (mesh->vert_buf[index].pos - mesh->vert_buf[mesh->vert_buf[index].nbs[0]].pos) * (mesh->vert_buf[index].pos - mesh->vert_buf[mesh->vert_buf[index].nbs[1]].pos);
printf("%d %d\n", mesh->vert_buf[index].nbs[0], mesh->vert_buf[index].nbs[1]);
}

void deform(Mesh* mesh, std::vector<int>& edges, std::vector<v3> new_pos)
{
	float alpha, beta;

	alpha = 0.2f;
	beta = (float)mesh->vert_buf.size() / (float)mesh->volume_nodes;



	for(int i = 0; i < edges.size(); i++)
	{	
		float lapP;
		
		v3 p = new_pos[i];
		v3 q = new_pos[i];

		v3 eps = lap_on_mesh(edges[i], mesh->vert_buf[edges[i]].pos, mesh);
		v3 delta = lap_on_graph(edges[i], mesh->vert_buf[edges[i]].pos, mesh);

		v3 a = lap_on_mesh(edges[i], p, mesh) - eps; 
		v3 b = p - q;
		v3 c = lap_on_graph(edges[i], p, mesh)  - delta; 

		//edges[i]->vert->pos = edges[i]->vert->pos + (a + b*alpha + c*beta); 
	  	std::vector<int> visited;
		move_vert(edges[i], (a*(1.f - alpha - beta) + b*alpha + c*beta), 1.f, mesh->vert_buf[edges[i]].pos, visited, mesh->avg_vert_len, mesh, 1);
	}
}

void select_edge(Mesh* mesh, v3 pos, v3 dir, std::vector<int>& selected)
{
	printf("Pos: [%f, %f, %f]\n", pos[0], pos[1], pos[2]);
	printf("Dir: [%f, %f, %f]\n", dir[0], dir[1], dir[2]);

	float min_dist = 999999.f;
	int index = -1;
	
	v3 hit;

	for(int i = 0; i < mesh->vert_buf.size(); i++)
	{
		v3 temp = pos + dir;
		temp = mesh->vert_buf[i].pos - temp;
		float len = sqrt(temp[0]*temp[0] + temp[1]*temp[1]);
		if(len < min_dist && len < 0.05f)
		{
			min_dist = len;
			index = i;
		}
	}
	if(index != -1)
		selected.push_back(index);
	
/*	
	for(int i = 0; i < mesh->ind_buf.size(); i+=3)
	{
		v3 normal;
		v3 ab = mesh->vert_buf[mesh->ind_buf[i + 1]].pos - mesh->vert_buf[mesh->ind_buf[i]].pos; 
		v3 ac = mesh->vert_buf[mesh->ind_buf[i + 2]].pos - mesh->vert_buf[mesh->ind_buf[i]].pos; 

		normal = ac*ab;

		float nDotA = normal.dot(pos);
		float nDotBA = normal.dot(dir);
		float t = (normal.dot(mesh->vert_buf[mesh->ind_buf[i]].pos) - nDotA/nDotBA);

		hit = pos + (dir*t);
		v3 a, b, c;
		float ang;
		float d1, d2, d3;
		float al, bl, cl;

		//printf("HIT: [%f, %f, %f]\n", hit[0], hit[1], hit[2]);

		if(t > 0.f)
		{
			a = mesh->vert_buf[mesh->ind_buf[i]].pos - hit;
			b = mesh->vert_buf[mesh->ind_buf[i + 1]].pos - hit;
			c = mesh->vert_buf[mesh->ind_buf[i + 2]].pos - hit;
		
			al = sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
			bl = sqrt(b.x*b.x + b.y*b.y + b.z*b.z);
			cl = sqrt(c.x*c.x + c.y*c.y + c.z*c.z);

			a.x /= al;
			a.y /= al;
			a.z /= al;
			b.x /= bl;
			b.y /= bl;
			b.z /= bl;
			c.x /= cl;
			c.y /= cl;
			c.z /= cl;

			d1 = a.dot(b);
			d2 = b.dot(c);
			d3 = c.dot(a);

			ang = acos(d1) + acos(d2) + acos(d3);

			if(ang > 1.999999*M_PI && ang < 2.000001*M_PI)
			{
				index = i;
				min_dist = t;
				printf("Ello?\n");
			}
		}

	}

	v3 vert1, vert2, vert3, s1, s2, s3;

	vert1 = mesh->vert_buf[mesh->ind_buf[index]].pos;
	vert2 = mesh->vert_buf[mesh->ind_buf[index+1]].pos;
	vert3 = mesh->vert_buf[mesh->ind_buf[index+2]].pos;

	s1 = vert1 - hit;
	s2 = vert2 - hit;
	s3 = vert3 - hit;

	float l1, l2, l3;

	l1 = sqrt(vert1[0]*vert1[0] + vert1[1]*vert1[1] + vert1[2]*vert1[2]);
	l2 = sqrt(vert2[0]*vert2[0] + vert2[1]*vert2[1] + vert2[2]*vert2[2]);
	l3 = sqrt(vert3[0]*vert3[0] + vert3[1]*vert3[1] + vert3[2]*vert3[2]);

	auto res = mesh->ind_buf[index];

	selected.push_back(res);
*/
}

void clear_edges(std::vector<int>& selected)
{
	selected.clear();
}
