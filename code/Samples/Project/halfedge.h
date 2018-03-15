struct HEVert;
struct HEFace;
#include <vector>
#include "memorypool.h"
#include <string.h>

struct v2
{
  float x;
  float y;

  inline v2(){};

  inline v2(float p1, float p2)
  {
    x = p1;
    y = p2;
  }

       
  inline v2(const v2& a)
  {
    memcpy(&x, &(a.x), sizeof(float)*2);
  }
    
  inline bool operator==(const v2& one) const
  {
    if(x == one.x && y == one.y)
      return true;
    return false;
  }

  inline v2 operator+(const v2& one) const
  {
    return v2(this->x + one.x, this->y + one.y);
  }
  inline v2 operator*(const float& mult) const
  {
    return v2(this->x * mult, this->y * mult);
  }
};

struct v3
{
  float x;
  float y;
  float z;

  inline v3(){
  	x = 0;
	y = 0;
	z = 0;
  };

  inline v3(float p1, float p2, float p3)
  {
    x = p1;
    y = p2;
    z = p3;
  }
    
  inline v3(const v3& a)
  {
    memcpy(&x, &(a.x), sizeof(float)*3);
  }

  friend v3 operator+(const v3&, const v3&);

  friend v3 operator-(const v3&, const v3&);
    
  /*inline v3 operator-(const v3& one) const
    {
    return v3(this->x - one.x, this->y - one.y, this->z - one.z);
    }*/
    
  inline v3 operator*(const float& mult) const
  {
    return v3(this->x * mult, this->y * mult, this->z * mult);
  }
    
  inline v3 operator*(const v3& cross) const
  {
    v3 r(
	 this->y*cross.z - cross.y*this->z,
	 this->z*cross.x - cross.z*this->x,
	 this->x*cross.y - cross.x*this->y
	 );
    return r;
  }

  inline float dot(const v3& dot) const
  {
    return this->x*dot.x + this->y*dot.y + this->z*dot.z;
  }
    
  inline v3 operator/(const float& div) const
  {
    float divm = 1/div;
    return v3(this->x *divm, this->y *divm, this->z *divm);
  }

  inline float operator[](const int& i) const
  {
    if(i == 0)
      return x;
    if(i == 1)
      return y;
    return z;
  }
  
};

inline v3 operator+(const v3& one, const v3& two)
{
  return v3(two.x + one.x, two.y + one.y, two.z + one.z);
}

inline v3 operator-(const v3& one, const v3& two)
{
  return v3(one.x - two.x, one.y - two.y, one.z - two.z);
}


struct HEEdge
{
  HEVert* vert; //Vertex at the end of the half-edge
  HEEdge* pair; //Oppositely oriented adjacent half-edge
  HEFace* face; //Face the half-edge borders
  HEEdge* next; //next half-edge around the face
  HEEdge* v_pair;

  v2 uv;
  v3 norm;

  char padding[4];

  HEEdge()
  {
    vert = nullptr;
    pair = nullptr;
    face = nullptr;
    next = nullptr;
  };
};

struct index_3
{
  int x;
  int y;
  int z;

  index_3()
  {
    x = -1;
    y = -1;
    z = -1;
  };
  index_3(int xx, int yy, int zz)
  {
    x = xx;
    y = yy;
    z = zz;
  };
};

struct aid
{
	index_3 ind;
	int vol;

	aid()
	{
		ind = index_3();
		vol = -1;
	}
	aid(int xx, int yy, int zz, int v)
	{
		ind = index_3(xx, yy, zz);
		vol = v;
	}
};

struct HEVert
{
  v3 pos;
  HEEdge* edge; //One of the half-edges emantating from the vertex

  float weight;

  char* padding[1];
  std::vector<aid> neighbors;

  HEVert()
  {
    edge = nullptr;
    weight = 0;
  };
  HEVert(v3 v)
  {
    pos = v;
    edge = nullptr;
  };
};

struct HEFace
{
  HEEdge* edge; //One of the half-edges bordering the face
    
  HEFace()
  {
    edge = nullptr;
  };
};

struct volumeNode
{
  index_3 diag[8];

  index_3 side[6];    
    
  std::vector<HEVert*> shell_connections;

  v3 pos;
  float weight;
};

class Mesh
{
 public:
  Mesh* m2_shell;

  Memorypool<HEEdge> edgePool;
  Memorypool<HEVert> vertPool;
  Memorypool<HEFace> facePool;
  int fCount;
  int eStep;
  int vCount;
  float avgVertLen;

  std::vector< std::vector< std::vector<volumeNode*> > > v_1;
  std::vector< std::vector< std::vector<volumeNode*> > > v_2;

  int volume_nodes;

  bool init = false;

  Mesh();
  ~Mesh();
  void generateHalfEdge(char*);
  void generateVolume();
  v3* getVB();
  v2* getUV();
  v3* getNormals();
  int VBLen(){ return fCount*eStep; };
  int ePerF(){ return eStep; };

  void deleteHalfEdge() {};
};
