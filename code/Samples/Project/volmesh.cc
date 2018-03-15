#include "volmesh.h"
#include <string>
#include <iostream>
#include <vector>
#include <cstdlib>
#define M_PI 3.14159265358979323846264338327950288
#include <cmath>
#include <cstdio>
#include <sstream>
#include <fstream>
#include <stdio.h>


Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::load_obj(char* path)
{
	std::ifstream infile(path);
    
    if(!infile.is_open()){
        printf("ERROR: File not open\n");
        return;	
    }
    init = true;
    std::string line;

    std::vector<v3> tempV;
    std::vector<v3> tempN;
    std::vector<std::string> tempF;

    int step = 0;
    
    while(std::getline(infile, line))
    {

        if(line.size() < 1);
        else
        {
            std::stringstream sst(line.c_str());
            std::string split;
            std::vector<std::string> splits;
                    
            if(line.at(0) == 'v' && line.at(1) == ' ')
            {
            
                while(std::getline(sst, split, ' '))
                {
                    splits.push_back(split);
                }

                v3 p(std::stof(splits[1]), std::stof(splits[2]), std::stof(splits[3]));

                tempV.push_back(p);
            }
            if(line.at(0) == 'v' && line.at(1) == 't')
            {
            }
            if(line.at(0) == 'v' && line.at(1) == 'n')
            {
            
                while(std::getline(sst, split, ' '))
                {
                    splits.push_back(split);
                }

                v3 p(std::stof(splits[1]), std::stof(splits[2]), std::stof(splits[3]));

                tempN.push_back(p);
            }
            else if(line.at(0) == 'f')
            {

                step = 0;
                while(std::getline(sst, split, ' '))
                {
                    if(split.at(0) != 'f')
                    {
                        tempF.push_back(split);

                        step++;
                    }
                }    
            }
        }
    }

    	for(int i = 0; i < tempV.size(); i++)
	{
		vert v;
		v.pos = tempV[i];
		vert_buf.push_back(v);
	}

	for(int i = 0; i < tempF.size(); i++)
	{
		std::stringstream f(tempF[i]);
            	std::string split;
	  
            	std::vector<std::string> splits;

            	while(std::getline(f, split, '/'))
            	{
                	splits.push_back(split);
            	}


            	vert_buf[(std::stoi(splits[0]) - 1)].norm = tempN[(std::stoi(splits[2]) - 1)];
            	ind_buf.push_back(std::stoi(splits[0]) - 1); 
	}

	avg_vert_len = 0;
	
	for(int i = 0; i < ind_buf.size(); i+=3)
	{
		v3 ve = vert_buf[ind_buf[i]].pos - vert_buf[ind_buf[i + 1]].pos;
		avg_vert_len += sqrt(ve[0]*ve[0] + ve[1]*ve[1] + ve[2] * ve[2]);


		bool oex = false;
		bool tex = false;

		for(int j = 0; j < vert_buf[ind_buf[i]].nbs.size(); j++)
		{
			if(ind_buf[i + 1] == vert_buf[ind_buf[i]].nbs[j])
				oex = true;
			
			if(ind_buf[i + 2] == vert_buf[ind_buf[i]].nbs[j])
				tex = true;
		}

		if(!oex)
			vert_buf[ind_buf[i]].nbs.push_back(ind_buf[i + 1]);

		if(!tex)
			vert_buf[ind_buf[i]].nbs.push_back(ind_buf[i + 2]);

		oex = false;
		tex = false;

		for(int j = 0; j < vert_buf[ind_buf[i + 1]].nbs.size(); j++)
		{
			if(ind_buf[i] == vert_buf[ind_buf[i + 1]].nbs[j])
				oex = true;
			
			if(ind_buf[i + 2] == vert_buf[ind_buf[i + 1]].nbs[j])
				tex = true;
		}

		if(!oex)
			vert_buf[ind_buf[i + 1]].nbs.push_back(ind_buf[i]);

		if(!tex)
			vert_buf[ind_buf[i + 1]].nbs.push_back(ind_buf[i + 2]);

		oex = false;
		tex = false;

		for(int j = 0; j < vert_buf[ind_buf[i + 2]].nbs.size(); j++)
		{
			if(ind_buf[i] == vert_buf[ind_buf[i + 2]].nbs[j])
				oex = true;
			
			if(ind_buf[i + 1] == vert_buf[ind_buf[i + 2]].nbs[j])
				tex = true;
		}

		if(!oex)
			vert_buf[ind_buf[i + 2]].nbs.push_back(ind_buf[i]);

		if(!tex)
			vert_buf[ind_buf[i + 2]].nbs.push_back(ind_buf[i + 1]);

	}

	avg_vert_len /= (ind_buf.size()/3);

	printf("Number of vertices: %d\n", vert_buf.size());
	printf("Length: %f\n", avg_vert_len);
}

void Mesh::generate_volume()
{
 
    float max_X = -HUGE_VALF;
    float min_X = HUGE_VALF;
    float max_Y = -HUGE_VALF;
    float min_Y = HUGE_VALF;
    float max_Z = -HUGE_VALF;
    float min_Z = HUGE_VALF;

    //Calculate max and min values for volume
    for(int i = 0; i < vert_buf.size(); i++)
    {
        //Determine max and min possible x, y, z for the volume structure
        if (vert_buf[i].pos[0] > max_X)
            max_X = vert_buf[i].pos[0]; 
        else if (vert_buf[i].pos[0]  < min_X)
            min_X = vert_buf[i].pos[0];

        if (vert_buf[i].pos[1] > max_Y)
            max_Y = vert_buf[i].pos[1];
        else if (vert_buf[i].pos[1]  < min_Y)
            min_Y = vert_buf[i].pos[1];

        if (vert_buf[i].pos[2] > max_Z)
            max_Z = vert_buf[i].pos[2];
        else if (vert_buf[i].pos[2] < min_Z)
            min_Z = vert_buf[i].pos[2];
    }

    printf("max_X: %f\nmin_X: %f\nmax_Y: %f\nmin_Y: %f\nmax_Z: %f\nmin_Z: %f\n",
           max_X, min_X, max_Y, min_Y, max_Z, min_Z);

    //Build crystal structure in mesh

    	float avgLen = avg_vert_len;

	for(float x = min_X; x < max_X; x+=avgLen)
    	{
        	std::vector< std::vector<volumeNode*> > temp_y1;
        	std::vector< std::vector<volumeNode*> > temp_y2;
        
        	for(float y = min_Y; y < max_Y; y+=avgLen)
        	{
            		std::vector<volumeNode*> temp_z1;
            		std::vector<volumeNode*> temp_z2;
            
            		for(float z = min_Z; z < max_Z; z+=avgLen)
            		{
             			int inter = 0;
                		int inter2 = 0;

				v3 newV(x, y, z);
				v3 newV2(x - (avgLen/2), y - (avgLen/2), z - (avgLen/2));

				//Check that newV and newV2 is inside the mesh
				
				//v3 direction(rand()%10 - 5, rand()%10 - 5, rand()%10 - 5);
				v3 direction = newV;
				
				float dirLen = sqrt(direction[0]*direction[0] + direction[1]*direction[1] + direction[2]*direction[2]);
				direction.x /= dirLen;
				direction.y /= dirLen;
				direction.z /= dirLen;
				
				v3 start1 = newV;
				v3 start2 = newV2;
               			
				for(int i = 0; i < ind_buf.size(); i+=3)
				{
					v3 normal;
					v3 ab = vert_buf[ind_buf[i + 1]].pos - vert_buf[ind_buf[i]].pos;
					v3 ac = vert_buf[ind_buf[i + 2]].pos - vert_buf[ind_buf[i]].pos;

					normal = ac*ab;

					//float nDotA = vert_buf[ind_buf[i]].norm.dot(start1);
				    	//float nDotBA = vert_buf[ind_buf[i]].norm.dot(direction);

					float nDotA = normal.dot(start1);
				    	float nDotBA = normal.dot(direction);
				    	
					//float t = (vert_buf[ind_buf[i]].norm.dot(vert_buf[ind_buf[i]].pos) - nDotA)/nDotBA;
				    
				    	float t = (normal.dot(vert_buf[ind_buf[i]].pos) - nDotA)/nDotBA;
				    	
					v3 hit = start1 + (direction * t);
				  
				   	v3 a, b, c;
				    	float ang;
				    	float d1, d2, d3;
				    	float al, bl, cl;

				    	if(t >= 0)
				    	{
						a = vert_buf[ind_buf[i]].pos - hit;
						b = vert_buf[ind_buf[i + 1]].pos - hit;
						c = vert_buf[ind_buf[i + 2]].pos - hit;

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
						    inter++;
						}
				    	}
				    	//second
				  
					nDotA = normal.dot(start2);
				    	nDotBA = normal.dot(direction);
				    
				    	t = (normal.dot(vert_buf[ind_buf[i]].pos) - nDotA)/nDotBA;
				    	
				    	//nDotA = vert_buf[ind_buf[i]].norm.dot(start2);
				    	//nDotBA = vert_buf[ind_buf[i]].norm.dot(direction);              

				    	//t = (vert_buf[ind_buf[i]].norm.dot(vert_buf[ind_buf[i]].pos) - nDotA)/nDotBA;
				    
				    	hit = start2 + (direction * t);

				    	if(t >= 0)
				   	{
						a = vert_buf[ind_buf[i]].pos - hit;
						b = vert_buf[ind_buf[i + 1]].pos - hit;
						c = vert_buf[ind_buf[i + 2]].pos - hit;
					  
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
							inter2++;
					  	}
                    			}
				}
				if(inter%2 == 1)
		  		{
                    			volumeNode *n1 = new volumeNode;
                    			n1->pos = newV;
                    			temp_z1.push_back(n1);
		    			volume_nodes++;
		  		}
                		else
		  		{
                    			temp_z1.push_back(nullptr);
		  		}
                		if(inter2%2 == 1)
		  		{
                    			volumeNode *n2 = new volumeNode;
                    			n2->pos = newV2;
                    			temp_z2.push_back(n2);
		    			volume_nodes++;
		  		}
                		else
		  		{
                    			temp_z2.push_back(nullptr);
		  		}
			}

            		temp_y1.push_back(temp_z1);
            		temp_y2.push_back(temp_z2);
        	}

        	v_1.push_back(temp_y1);
        	v_2.push_back(temp_y2);
    	}

    int n_count = 0;

    //V1

    for(uint i = 0; i < v_1.size(); i++)
      {
        for(uint j = 0; j < v_1[0].size(); j++)
	  {
            for(uint k = 0; k < v_1[0][0].size(); k++)
	      {
		n_count = 0;
                auto a_n = v_1[i][j][k];

                if(a_n != nullptr)
		  {
		    //Do things

		    //Sides
		    if(k > 0) { if(v_1[i][j][k - 1] != nullptr)
			{
			  a_n->side[0] = index_3(i, j, k - 1); n_count++;
			} }           
		    if(k < v_1[i][j].size() - 1) { if(v_1[i][j][k + 1] != nullptr)
			{
			  a_n->side[1] = index_3(i, j, k + 1); n_count++;
			} }

		    if(j > 0) { if(v_1[i][j - 1][k] != nullptr)
			{
			  a_n->side[2] = index_3(i, j - 1, k); n_count++;
			} }           
		    if(j < v_1[i].size() - 1) { if(v_1[i][j + 1][k] != nullptr)
			{
			  a_n->side[3] = index_3(i, j + 1, k); n_count++;
			} }

		    if(i > 0) { if(v_1[i - 1][j][k] != nullptr)
			{
			  a_n->side[4] = index_3(i - 1, j, k); n_count++;
			} }           
		    if(i < v_1.size() - 1) { if(v_1[i + 1][j][k] != nullptr)
			{
			  a_n->side[5] = index_3(i + 1, j, k); n_count++;
			} }

		    //Diags
		    
		    if(v_2[i][j][k] != nullptr)
		      {
			a_n->diag[0] = index_3(i, j, k); n_count++;
		      }
		    if(k > 0) { if(v_2[i][j][k - 1] != nullptr) 
			{
			  a_n->diag[1] = index_3(i, j, k - 1); n_count++;
			} }
		    if(j > 0) { if(v_2[i][j - 1][k] != nullptr) 
			{
			  a_n->diag[2] = index_3(i, j - 1, k); n_count++;
			} }
		    if(i > 0) { if(v_2[i - 1][j][k] != nullptr) 
			{
			  a_n->diag[3] = index_3(i - 1, j, k); n_count++;
			} }

		    if(k > 0 && j > 0) { if(v_2[i][j - 1][k - 1] != nullptr) 
			{
			  a_n->diag[4] = index_3(i, j - 1, k - 1); n_count++;
			} }
		    if(i > 0 && k > 0) { if(v_2[i - 1][j][k - 1] != nullptr) 
			{
			  a_n->diag[5] = index_3(i - 1, j, k - 1); n_count++;
			} }
		    if(i > 0 && j > 0) { if(v_2[i - 1][j - 1][k] != nullptr) 
			{
			  a_n->diag[6] = index_3(i - 1, j - 1, k); n_count++;
			} }
		    if(i > 0 && j > 0 && k > 0) { if(v_2[i - 1][j - 1][k - 1] != nullptr) 
			{
			  a_n->diag[7] = index_3(i - 1, j - 1, k - 1); n_count++;
			} }

		    //Connect to shell

		    for(int l = 0; l < vert_buf.size(); l++)
		      {
			v3 vvert = vert_buf[l].pos - v_1[i][j][k]->pos;
			float vlen = sqrt(vvert[0]*vvert[0] + vvert[1]*vvert[1] + vvert[2]*vvert[2]);
			
			if(vlen < avgLen)
			  {
			    v_1[i][j][k]->shell_connections.push_back(l); 
			    n_count++;
			    vert_buf[l].vol_con.push_back(aid(i, j, k, 1));
			  }
		      }

		    v_1[i][j][k]->weight = 1.f/n_count;

		  }
	      }
	  }
      }

    //V2    

    for(uint i = 0; i < v_2.size(); i++)
      {
        for(uint j = 0; j < v_2[0].size(); j++)
	  {
            for(uint k = 0; k < v_2[0][0].size(); k++)
	      {
		n_count = 0;
		
                auto a_n = v_2[i][j][k];

                if(a_n != nullptr)
		  {
		    //Do things

		    //Sides
		    if(k > 0) { if(v_2[i][j][k - 1] != nullptr)
			{
			  a_n->side[0] = index_3(i, j, k - 1); n_count++;
			} }           
		    if(k < v_2[i][j].size() - 1) { if(v_2[i][j][k + 1] != nullptr)
			{
			  a_n->side[1] = index_3(i, j, k + 1); n_count++;
			} }

		    if(j > 0) { if(v_2[i][j - 1][k] != nullptr)
			{
			  a_n->side[2] = index_3(i, j - 1, k); n_count++;
			} }           
		    if(j < v_2[i].size() - 1) { if(v_2[i][j + 1][k] != nullptr)
			{
			  a_n->side[3] = index_3(i, j + 1, k); n_count++;
			} }

		    if(i > 0) { if(v_2[i - 1][j][k] != nullptr)
			{
			  a_n->side[4] = index_3(i - 1, j, k); n_count++;
			} }           
		    if(i < v_2.size() - 1) { if(v_2[i][j][k] != nullptr)
			{
			  a_n->side[5] = index_3(i + 1, j, k); n_count++;
			} }

		    //Diags
		    
		    if(v_1[i][j][k] != nullptr)
		      {
			a_n->diag[0] = index_3(i, j, k); n_count++;
		      }
		    if(k < v_1[i][j].size() - 1) { if(v_1[i][j][k + 1] != nullptr) 
			{
			  a_n->diag[1] = index_3(i, j, k + 1); n_count++;
			} }
		    if(j < v_1[i].size() - 1) { if(v_1[i][j + 1][k] != nullptr) 
			{
			  a_n->diag[2] = index_3(i, j + 1, k); n_count++;
			} }
		    if(i < v_1.size() - 1) { if(v_1[i + 1][j][k] != nullptr) 
			{
			  a_n->diag[3] = index_3(i + 1, j, k); n_count++;
			} }

		    if(k < v_1[i][j].size() - 1 && j < v_1[i].size() - 1) { if(v_1[i][j + 1][k + 1] != nullptr) 
			{
			  a_n->diag[4] = index_3(i, j + 1, k + 1); n_count++;
			} }
		    if(i < v_1.size() - 1 && k < v_1[i][j].size() - 1) { if(v_1[i + 1][j][k + 1] != nullptr) 
			{
			  a_n->diag[5] = index_3(i + 1, j, k + 1); n_count++;
			} }
		    if(i < v_1.size() - 1 && j < v_1[i].size() - 1) { if(v_1[i + 1][j + 1][k] != nullptr) 
			{
			  a_n->diag[6] = index_3(i + 1, j + 1, k); n_count++;
			} }
		    if(i < v_1.size() - 1 && j < v_1[i].size() - 1 && k < v_1[i][j].size() - 1) { if(v_1[i + 1][j + 1][k + 1] != nullptr) 
			{
			  a_n->diag[7] = index_3(i + 1, j + 1, k + 1); n_count++;
			} }

		    //Connect to shell

		    for(int l = 0; l < vert_buf.size(); l++)
		      {
			v3 vvert = vert_buf[l].pos - v_2[i][j][k]->pos;
			float vlen = sqrt(vvert[0]*vvert[0] + vvert[1]*vvert[1] + vvert[2]*vvert[2]);
			
			if(vlen < avgLen)
			  {
			    v_2[i][j][k]->shell_connections.push_back(l); 
			    n_count++;
			    vert_buf[l].vol_con.push_back(aid(i, j, k, 2));
			  }
		      }

		    v_2[i][j][k]->weight = 1.f/n_count;
		    
		  }
	      }
	  }
      }

    for(int i = 0; i < vert_buf.size(); i++)
    {
	    vert_buf[i].weight = 1.f/(vert_buf[i].nbs.size() + vert_buf[i].vol_con.size());
    }

}

std::vector<vert> Mesh::get_vb()
{
	std::vector<vert> res;

	for(int i = 0; i < ind_buf.size(); i++)
		res.push_back(vert_buf[ind_buf[i]]);

	return res;
}
