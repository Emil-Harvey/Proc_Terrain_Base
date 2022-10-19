#include "QuadTreeMesh.h"

QuadTreeMesh::~QuadTreeMesh()
{
	Root->~QuadtreeNode();
	delete Root;
	Root = 0;
}
