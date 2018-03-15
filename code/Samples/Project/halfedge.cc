#include "halfedge.h"
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
    deleteHalfEdge();
}

void Mesh::generateHalfEdge(char* path)
{
    std::ifstream infile(path);
    
    if(!infile.is_open()){
        printf("ERROR: File not open\n");
        return;	
    }
    init = true;
    std::string line;

    std::vector<v3> tempV;
    std::vector<v2> tempUV;
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
            
                while(std::getline(sst, split, ' '))
                {
                    splits.push_back(split);
                }

                v2 p(std::stof(splits[1]), std::stof(splits[2]));

                tempUV.push_back(p);
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

    //Allocating Mesh
    
    int edgeCount = (int)tempF.size();
    int faceCount = edgeCount/step;  

    int vNm = (int)tempV.size();
    vertPool.pre_allocate_elements(vNm);

    for(int i = 0; i < vNm; i++)
    {
        vertPool.new_element()->pos = tempV[i];
    }

    facePool.pre_allocate_elements(faceCount);

    for(int i = 0; i < faceCount; i++)
        facePool.new_element();
    
    edgePool.pre_allocate_elements(edgeCount);
 
    for(int i = 0; i < edgeCount; i++)
        edgePool.new_element();

    for(int i = 0; i < faceCount*step; i+=step)
    {
        for(int j = 0; j < step - 1; j++)
        {
            edgePool[i + j]->next = edgePool[i + j + 1];
        }
        edgePool[i + step - 1]->next = edgePool[i];
    }

    //Inner shell
    m2_shell = new Mesh;

    float avgLen = 0;
    int count = faceCount < 100 ? faceCount : 100;
    
    for(int i = 0; i < faceCount && i < 100; i++)
    {
        int *vIndex = new int[step];
        int *nIndex = new int[step];
        
        for(int j = 0; j < step; j++)
        {
            std::stringstream f(tempF[i*step+j]);
            std::string split;
	  
            std::vector<std::string> splits;

            while(std::getline(f, split, '/'))
            {
                splits.push_back(split);
            }
            nIndex[j] = std::stoi(splits[2]) - 1;
            vIndex[j] = std::stoi(splits[0]) - 1;
        }

        v3 ve = vertPool[vIndex[0]]->pos - vertPool[vIndex[1]]->pos;
        avgLen += sqrt(ve[0]*ve[0] + ve[1]*ve[1] + ve[2]*ve[2]);
        
        delete[] vIndex;
        delete[] nIndex;
    }

    avgLen /= count;

    printf("Average length between vertices: %f\n", avgLen);

    //Allocating M2
    
    m2_shell->vertPool.pre_allocate_elements(vNm);

    for(int i = 0; i < vNm; i++)
    {
        m2_shell->vertPool.new_element()->pos = tempV[i];
    }

    m2_shell->facePool.pre_allocate_elements(faceCount);

    for(int i = 0; i < faceCount; i++)
        m2_shell->facePool.new_element();
    
    m2_shell->edgePool.pre_allocate_elements(edgeCount);
 
    for(int i = 0; i < edgeCount; i++)
        m2_shell->edgePool.new_element();

    for(int i = 0; i < faceCount*step; i+=step)
    {
        for(int j = 0; j < step - 1; j++)
        {
           m2_shell->edgePool[i + j]->next = m2_shell->edgePool[i + j + 1];
        }
        m2_shell->edgePool[i + step - 1]->next = m2_shell->edgePool[i];
    }
    
    //printf("Build faces.\n");
    
    for(int i = 0; i < faceCount; i++)
    {
        bool uv = false;
        bool n = false;

        int *vIndex = new int[step];
        int *uvIndex = new int[step];
        int *nIndex = new int[step];

        for(int j = 0; j < step; j++)
        {
            uv = false;
            n = false;
            std::stringstream f(tempF[i*step+j]);
            std::string split;

	  
            std::vector<std::string> splits;

            while(std::getline(f, split, '/'))
            {
                splits.push_back(split);
            }

            switch(splits.size())
            {
            case(3):
                if(splits[2].size() > 0)
                {
                    nIndex[j] = std::stoi(splits[2]) - 1;
                    n = true;
                }
            case(2):
                if(splits[1].size() > 0)
                {
                    uvIndex[j] = std::stoi(splits[1]) - 1;
                    uv = true;
                }
            case(1):
                vIndex[j] = std::stoi(splits[0]) - 1;
                break;
            default:
                break;
            }
            
        }

        facePool[i]->edge = edgePool[step*i];
        m2_shell->facePool[i]->edge = m2_shell->edgePool[step*i];
        
        if(uv)
        {
            facePool[i]->edge->uv = tempUV[uvIndex[0]];
            m2_shell->facePool[i]->edge->uv = tempUV[uvIndex[0]];
        }
	
        if(n)
        {
            facePool[i]->edge->norm = tempN[nIndex[0]];
            m2_shell->facePool[i]->edge->norm = tempN[nIndex[0]];
        }
            
        HEEdge* temp = facePool[i]->edge;
        temp->vert = vertPool[vIndex[0]];

        HEEdge* temp2 = m2_shell->facePool[i]->edge;
        temp2->vert = m2_shell->vertPool[vIndex[0]];
        //temp2->vert->pos = temp2->vert->pos - temp2->norm * avgLen * 0.01;
        
        for(int j = 1; j < step; j++)
        {
            temp = temp->next;
            temp2 = temp2->next;
            if(uv)
            {
                temp->uv = tempUV[uvIndex[j]];
                temp2->uv = tempUV[uvIndex[j]];
            }
            if(n)
            {
                temp->norm = tempN[nIndex[j]];
                temp2->norm = tempN[nIndex[j]];
            }            
            
            temp->vert = vertPool[vIndex[j]];
            temp2->vert = m2_shell->vertPool[vIndex[j]];
            //temp2->vert->pos = temp2->vert->pos - temp2->norm * avgLen * 0.01;
        }

        delete[] vIndex;
        delete[] uvIndex;
        delete[] nIndex;
    }

    //printf("Link faces.\n");
    
    for(int i = 0; i < faceCount*step; i++)
    {

        //printf("step: %i of %i\n", i, faceCount*step);
        edgePool[i]->vert->edge = edgePool[i];
        m2_shell->edgePool[i]->vert->edge = m2_shell->edgePool[i];

        for(int j = 0; j < faceCount*step; j++)
        {
            if(edgePool[i]->vert == edgePool[j]->next->vert && edgePool[j]->vert == edgePool[i]->next->vert)
            {
                edgePool[i]->pair = edgePool[j];
                m2_shell->edgePool[i]->pair = m2_shell->edgePool[j];
            }
        }
    }

    //printf("Amount of faces: %i\n", faceCount);
    
    for(int i = 0; i < faceCount; i++)
    {
        HEEdge *temp = facePool[i]->edge->next;
        HEEdge *temp2 = m2_shell->facePool[i]->edge->next;

        //printf("Face: %i\n", i);
        while(true)
        {
            temp->face = facePool[i];
            temp2->face = m2_shell->facePool[i];
            
            if(temp == facePool[i]->edge)
                break;
            temp = temp->next;
            temp2 = temp2->next;
        }
    }
    
    fCount = faceCount;
    eStep = step;
    vCount = vNm;
    avgVertLen = avgLen;
    
    m2_shell->fCount = faceCount;
    m2_shell->eStep = step;
    m2_shell->vCount = vNm;

    for(int i = 0; i < vCount; i++)
    {
        v3 meshNorm  = vertPool[i]->edge->norm;
        
        float dist = 0.01f;
        v3 newV = vertPool[i]->pos - (meshNorm * avgLen * dist);

        while(true)
        {
            int intersects = 0;

            v3 direction = meshNorm;
                
            float dirLen = sqrt(direction[0]*direction[0] + direction[1]*direction[1] + direction[2]*direction[2]);
            direction.x /= dirLen;
            direction.y /= dirLen;
            direction.z /= dirLen;
                
            v3 start = vertPool[i]->pos - (meshNorm * avgLen * dist*2);

            for(int j = 0; j < fCount; j++)
            {
          
                float nDotA = facePool[j]->edge->norm.dot(start);
                float nDotBA = facePool[j]->edge->norm.dot(direction);

                float t = (facePool[j]->edge->norm.dot(facePool[j]->edge->vert->pos) - nDotA)/nDotBA;
                    
                if(t >= 0 && t < avgLen * dist * 2)
                {

                    v3 hit = start + (direction * t);
                    
                    v3 a, b, c;
                    float ang;
                    float d1, d2, d3;
                    float al, bl, cl;
                    
                    a = facePool[j]->edge->vert->pos - hit;
                    b = facePool[j]->edge->next->vert->pos - hit;
                    c = facePool[j]->edge->next->next->vert->pos - hit;

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

                    if(ang > 1.99999*M_PI && ang < 2.00001*M_PI)
                    {
                        intersects++;
                    }
                }
            }
            
            if(intersects > 0 || dist > 1.0f)
            {
                dist /= 2.f;
                newV = vertPool[i]->pos - (meshNorm * avgLen * 0.0f);//dist);
                
                break;
            }
            else
                dist *= 1.3f;
            
            newV = vertPool[i]->pos - (meshNorm * avgLen * dist);
            //printf("Dist: %f\n", dist);
        }
        m2_shell->vertPool[i]->pos = newV;
    }

	for(int i = 0; i < faceCount*step; i++)
	{
		m2_shell->edgePool[i]->v_pair = edgePool[i];	
		edgePool[i]->v_pair = m2_shell->edgePool[i];	
	}
}



void Mesh::generateVolume()
{
    //Calculate average length of an edge in the mesh
    float avgLen = avgVertLen;

    printf("%f\n", avgLen);
    
    float max_X = -HUGE_VALF;
    float min_X = HUGE_VALF;
    float max_Y = -HUGE_VALF;
    float min_Y = HUGE_VALF;
    float max_Z = -HUGE_VALF;
    float min_Z = HUGE_VALF;

    //Calculate max and min values for volume
    for(int i = 0; i < m2_shell->vCount; i++)
    {
        
        //Determine max and min possible x, y, z for the volume structure
        if (m2_shell->vertPool[i]->pos[0] > max_X)
            max_X = m2_shell->vertPool[i]->pos[0];
        else if (m2_shell->vertPool[i]->pos[0] < min_X)
            min_X = m2_shell->vertPool[i]->pos[0];

        if (m2_shell->vertPool[i]->pos[1] > max_Y)
            max_Y = m2_shell->vertPool[i]->pos[1];
        else if (m2_shell->vertPool[i]->pos[1] < min_Y)
            min_Y = m2_shell->vertPool[i]->pos[1];

        if (m2_shell->vertPool[i]->pos[2] > max_Z)
            max_Z = m2_shell->vertPool[i]->pos[2];
        else if (m2_shell->vertPool[i]->pos[2] < min_Z)
            min_Z = m2_shell->vertPool[i]->pos[2];

    }

    printf("max_X: %f\nmin_X: %f\nmax_Y: %f\nmin_Y: %f\nmax_Z: %f\nmin_Z: %f\n",
           max_X, min_X, max_Y, min_Y, max_Z, min_Z);

    //Build crystal structure in mesh
	
	int* arr = new int[vCount];

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
                //printf("Add new point\n");
                
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
                
                for(int j = 0; j < m2_shell->fCount; j++)
                {
          
                    float nDotA = facePool[j]->edge->norm.dot(start1);
                    float nDotBA = facePool[j]->edge->norm.dot(direction);

                    float t = (facePool[j]->edge->norm.dot(facePool[j]->edge->vert->pos) - nDotA)/nDotBA;
                    
                    v3 hit = start1 + (direction * t);
                  
                    v3 a, b, c;
                    float ang;
                    float d1, d2, d3;
                    float al, bl, cl;
            
                    if(t >= 0)
                    {
                        a = facePool[j]->edge->vert->pos - hit;
                        b = facePool[j]->edge->next->vert->pos - hit;
                        c = facePool[j]->edge->next->next->vert->pos - hit;

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
                  
                    nDotA = facePool[j]->edge->norm.dot(newV2);
                    nDotBA = facePool[j]->edge->norm.dot(direction);              

                    t = (facePool[j]->edge->norm.dot(facePool[j]->edge->vert->pos) - nDotA)/nDotBA;
                    
                    hit = start2 + (direction * t);

                    if(t >= 0)
                    {
                        a = facePool[j]->edge->vert->pos - hit;
                        b = facePool[j]->edge->next->vert->pos - hit;
                        c = facePool[j]->edge->next->next->vert->pos - hit;
                  
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

    //Connect nodes

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

		    //Connect to m2_shell

		    for(int l = 0; l < m2_shell->vCount; l++)
		      {
			auto edg = m2_shell->vertPool[l]->edge;

			v3 vvert = edg->vert->pos - v_1[i][j][k]->pos;
			float vlen = sqrt(vvert[0]*vvert[0] + vvert[1]*vvert[1] + vvert[2]*vvert[2]);
			
			if(vlen < avgLen)
			  {
			    v_1[i][j][k]->shell_connections.push_back( m2_shell->vertPool[l]); 
			    n_count++;
				arr[l]++;
			    m2_shell->vertPool[l]->neighbors.push_back(aid(i, j, k, 1));
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

		    //Connect to m2_shell


		    for(int l = 0; l < m2_shell->vCount; l++)
		      {
			auto edg = m2_shell->vertPool[l]->edge;

			v3 vvert = edg->vert->pos - v_2[i][j][k]->pos;
			float vlen = sqrt(vvert[0]*vvert[0] + vvert[1]*vvert[1] + vvert[2]*vvert[2]);
			
			if(vlen < avgLen)
			  {
			    v_2[i][j][k]->shell_connections.push_back( m2_shell->vertPool[l]); 
			    n_count++;
			    arr[l]++;
			    m2_shell->vertPool[l]->neighbors.push_back(aid(i, j, k, 2));
			  }
		      }

		    v_2[i][j][k]->weight = 1.f/n_count;
		    
		  }
	      }
	  }
      }

    for(int i = 0; i < m2_shell->vCount; i++)
    {

	    if(arr[i] < 1)
	    	m2_shell->vertPool[i]->weight = 1;
	    else	
	    	m2_shell->vertPool[i]->weight = 1.f/arr[i];
    }
}

v3* Mesh::getVB()
{
    v3* VB = new v3[fCount*eStep];
    for(int i = 0; i < fCount; i++)
    {
        HEEdge *temp = facePool[i]->edge;
        for(int j = 0; j < eStep; j++)
        {
            VB[i*eStep + j] = temp->vert->pos;

            temp = temp->next;
        }
    }
    return VB;
}

v2* Mesh::getUV()
{
    v2* UV = new v2[fCount*eStep];
    for(int i = 0; i < fCount; i++)
    {
        HEEdge *temp = facePool[i]->edge;
        for(int j = 0; j < eStep; j++)
        {
            UV[i*eStep + j] = temp->uv;

            temp = temp->next;
        }
    }
    return UV;
}

v3* Mesh::getNormals()
{
    v3* N = new v3[fCount*eStep];
    for(int i = 0; i < fCount; i++)
    {
        HEEdge *temp = facePool[i]->edge;
        for(int j = 0; j < eStep; j++)
        {
            N[i*eStep + j] = temp->norm;

            temp = temp->next;
        }
    }
    return N;
}
