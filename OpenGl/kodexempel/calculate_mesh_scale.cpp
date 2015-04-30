/****************************************************
 **********File name: MeshLoader03.cpp****************
 ******************************************************/

 /*************Includes*****************************/
 #include "stdafx.h"
 #include "MeshLoader03.h"

 /******************Class**************************/
 bool MeshLoader03::LoadModel(const std::string& a_file, float a_scale,
     std::vector<Vertex::Basic28>& vertices,
     std::vector<USHORT>& indices,
     std::vector<MeshGeometry::Subset>& subsets,
     std::vector<ModelMaterial>& mats)
 {

     //check if file exists
     std::ifstream fin(a_file.c_str());
     if(!fin.fail())
     {
         fin.close();
     }
     else
     {
         MessageBox(NULL,  TEXT("Couldn't open file: ") , TEXT("ERROR"), MB_OK | MB_ICONEXCLAMATION);
         return false;
     }

     //Assign scale
     m_scale = a_scale;

     //Assign mesh properties
     numVertices = 0;
     numSubsets = 0;
     numMaterials = 0;
     numTriangles = 0;

     //initialize success variable
     bool Ret = false;

     //Load mesh
     Ret = loadMesh(a_file,a_scale, vertices,indices,subsets,mats);

     //Return
     return Ret;
 }




 //Load mesh with Assimp
 bool MeshLoader03::loadMesh(const std::string& a_file, float a_scale,
     std::vector<Vertex::Basic28>& vertices,
     std::vector<USHORT>& indices,
     std::vector<MeshGeometry::Subset>& subsets,
     std::vector<ModelMaterial>& mats)
 {
     //Assimp importer we are using
     Assimp::Importer importer;
     const aiScene* loadedScene = importer.ReadFile( a_file,
         aiProcessPreset_TargetRealtime_Quality |
         aiProcess_ConvertToLeftHanded
         );
     //we have NO assimp object
     if(!loadedScene)
     {
         OutputDebugString(L"Model failed to load \n");
         //OutputDebugString( (importer.GetErrorString()).c_str() );
         return false;
     }
     else
     {   //we have an assimp object with meshes
         if(loadedScene->HasMeshes())
         {
         //Get mesh properties
         GetMeshProperties(loadedScene);
         //Extract Faces,Vertices,Subsets,Materials
         InitMaterials(a_file,loadedScene, numMaterials,  mats);
         InitSubsetTable(a_file,loadedScene,numSubsets,subsets);
         InitVertices(a_file,loadedScene, numVertices,  vertices);
         InitTriangles(a_file,loadedScene, numTriangles,  indices);
         ScaleAsset(loadedScene);
         return true;
         }
         else //an assimp object with NO meshes
             return false;
     }

 }


 //Get mesh properties
 void MeshLoader03::GetMeshProperties(const aiScene* loadedScene)
 {
     //Get rootNode
     aiNode* rootNode = loadedScene->mRootNode;

     //Attributes
     numSubsets = loadedScene->mNumMeshes;
     numMaterials = loadedScene->mNumMaterials;

     //If only one node
     if(rootNode->mNumChildren == 0)
     {
         numVertices = loadedScene->mMeshes[0]->mNumVertices;
         numTriangles = loadedScene->mMeshes[0]->mNumFaces;
     }
     else
     {
         //For all the children
         for (size_t i =0 ;i<rootNode->mNumChildren; ++i)
         {

             //Get child node
             aiNode* childNode = rootNode->mChildren[i];

             // Initialize the meshes in the scene one by one
             for (size_t n=0; n < childNode->mNumMeshes; ++n)
             {
                 //get the mesh
                 const aiMesh* paiMesh = loadedScene->mMeshes[childNode->mMeshes[n]];

                 //Update total vertices and faces
                 numVertices+= paiMesh->mNumVertices;
                 numTriangles+= paiMesh->mNumFaces;
             }
         }
     }

 }




 //Extract subsets
 void MeshLoader03::InitSubsetTable(const std::string& a_file,const aiScene* pScene, UINT numSubsets, std::vector<MeshGeometry::Subset>& subsets)
 {
     subsets.resize(numSubsets);

     //Get rootNode
     aiNode* rootNode = pScene->mRootNode;


     //Counter for vertices and indices
     unsigned int vertexCounter, faceCounter;

     //If only one node
     if(rootNode->mNumChildren == 0)
     {
         subsets[0].Id = pScene->mMeshes[0]->mMaterialIndex;
         subsets[0].VertexStart = 0;
         subsets[0].VertexCount = pScene->mMeshes[0]->mNumVertices;
         subsets[0].FaceStart = 0;
         subsets[0].FaceCount = pScene->mMeshes[0]->mNumFaces;
     }
     else
     {
         //Counter
         unsigned int subsetCounter = 0;
         vertexCounter = 0; faceCounter = 0;
         //For all the children
         for (size_t i =0 ;i<rootNode->mNumChildren; ++i)
         {

             //Get child node
             aiNode* childNode = rootNode->mChildren[i];

             // Initialize the meshes in the scene one by one
             for (size_t n=0; n < childNode->mNumMeshes; ++n)
             {
                 //get the mesh
                 const aiMesh* paiMesh = pScene->mMeshes[childNode->mMeshes[n]];

                 subsets[subsetCounter].Id = pScene->mMeshes[childNode->mMeshes[n]]->mMaterialIndex;
                 subsets[subsetCounter].VertexStart = vertexCounter;
                 subsets[subsetCounter].VertexCount = paiMesh->mNumVertices;
                 subsets[subsetCounter].FaceStart = faceCounter;
                 subsets[subsetCounter].FaceCount = paiMesh->mNumFaces;
                 //Update total vertices and faces
                 vertexCounter+= paiMesh->mNumVertices;
                 faceCounter+= paiMesh->mNumFaces;
                 //Update counter
                 subsetCounter++;
             }
         }
     }


 }



 //Extract Vertices
 void MeshLoader03::InitVertices(const std::string& a_file,const aiScene* pScene, UINT numVertices, std::vector<Vertex::Basic28>& vertices)
 {
     vertices.resize(numVertices);
     //Get rootNode
     aiNode* rootNode = pScene->mRootNode;


     //Counter for vertices and indices
     unsigned int vertexCounter;

     //If only one node
     if(rootNode->mNumChildren == 0)
     {
         const aiMesh* mesh = pScene->mMeshes[0];
         for(size_t b = 0;b<mesh->mNumVertices;b++)
         {
             vertices[b].Pos.x = mesh->mVertices[b].x;
             vertices[b].Pos.y = mesh->mVertices[b].y;
             vertices[b].Pos.z = mesh->mVertices[b].z;

             if(mesh->HasVertexColors(0))
             {
                 const aiColor4D pColr = mesh->mColors[0][b];
                 vertices[b].Color.x = pColr.r;
                 vertices[b].Color.y = pColr.g;
                 vertices[b].Color.z = pColr.b;
                 vertices[b].Color.w = pColr.a;
             }
             else
                 vertices[b].Color = XMFLOAT4(1.0f,1.0f,0.0f,0.0f);
         }

     }
     else
     {
         //Counter
         vertexCounter = 0;

         //For all the children
         for (size_t i =0 ;i<rootNode->mNumChildren; ++i)
         {

             //Get child node
             aiNode* childNode = rootNode->mChildren[i];

             // Initialize the meshes in the scene one by one
             for (size_t n=0; n < childNode->mNumMeshes; ++n)
             {
                 //get the mesh
                 const aiMesh* paiMesh = pScene->mMeshes[childNode->mMeshes[n]];

                 for(size_t b = 0;b<paiMesh->mNumVertices;b++)
                 {
                     vertices[vertexCounter].Pos.x = paiMesh->mVertices[b].x;
                     vertices[vertexCounter].Pos.y = paiMesh->mVertices[b].y;
                     vertices[vertexCounter].Pos.z = paiMesh->mVertices[b].z;

                     if(paiMesh->HasVertexColors(0))
                     {
                         const aiColor4D pColr = paiMesh->mColors[0][i];
                         vertices[vertexCounter].Color.x = pColr.r;
                         vertices[vertexCounter].Color.y = pColr.g;
                         vertices[vertexCounter].Color.z = pColr.b;
                         vertices[vertexCounter].Color.w = pColr.a;
                     }
                     else
                         vertices[vertexCounter].Color = XMFLOAT4(1.0f,1.0f,0.0f,0.0f);

                     vertexCounter++;
                 }
             }
         }
     }


 }

 //Extract Triangles
 void MeshLoader03::InitTriangles(const std::string& a_file,const aiScene* pScene, UINT numTriangles, std::vector<USHORT>& indices)
 {
     //indices.resize(numTriangles*3);

     //Get rootNode
     aiNode* rootNode = pScene->mRootNode;


     //Counter for vertices and indices
     unsigned int indexCounter, indexStart;
     indexCounter = 0;
     indexStart = indexCounter;
     //If only one node
     if(rootNode->mNumChildren == 0)
     {
         const aiMesh* mesh = pScene->mMeshes[0];
         for(size_t b = 0;b<mesh->mNumFaces;b++)
         {
             indices[indexCounter++] = mesh->mFaces[b].mIndices[0] + indexStart;
             indices[indexCounter++] = mesh->mFaces[b].mIndices[1] + indexStart;
             indices[indexCounter++] = mesh->mFaces[b].mIndices[2] + indexStart;
         }

     }
     else
     {
         //Counter
         indexStart = 0;
         indexCounter = 0;

         //For all the children
         for (size_t i =0 ;i<rootNode->mNumChildren; ++i)
         {

             //Get child node
             aiNode* childNode = rootNode->mChildren[i];

             // Initialize the meshes in the scene one by one
             for (size_t n=0; n < childNode->mNumMeshes; ++n)
             {
                 //get the mesh
                 const aiMesh* mesh = pScene->mMeshes[childNode->mMeshes[n]];

                 for(size_t b = 0;b<mesh->mNumFaces;b++)
                 {
                     //indices[indexCounter++] = mesh->mFaces[b].mIndices[0] + indexStart;
                     //indices[indexCounter++] = mesh->mFaces[b].mIndices[1] + indexStart;
                     //indices[indexCounter++] = mesh->mFaces[b].mIndices[2] + indexStart;

                     indices.push_back(mesh->mFaces[b].mIndices[0] + indexStart);
                     indices.push_back(mesh->mFaces[b].mIndices[1] + indexStart);
                     indices.push_back(mesh->mFaces[b].mIndices[2] + indexStart);
                 }
                 indexStart += mesh->mNumVertices;
             }
         }
     }


 }

 /*********************Conversion functions**********************************/
 //AiVector3 to XMFLOAT3
 XMFLOAT3 MeshLoader03::aiVec3ToXMFloat3(const aiVector3D* vector)
 {
     //Ooutput
     XMFLOAT3 output;

     //Assignmetns
     output.x = vector->x;
     output.y = vector->y;
     output.z = vector->z;

     //Send back result
     return output;
 }

 //AiColor to XMFLOAT4
     XMFLOAT4 MeshLoader03::aiColorToXMFLOAT4(const aiColor4D* color)
     {
     //Ooutput
     XMFLOAT4 output;

     //Assignmetns
     output.x = color->r;
     output.y = color->g;
     output.z = color->b;
     output.w = color->a;

     //Send back result
     return output;

     }

 //AiMatrix to XMFLOAT4X4
 XMFLOAT4X4 MeshLoader03::aiMatrixToXMFloat4x4(const aiMatrix4x4* aiMe)
 {
     XMFLOAT4X4 output;
     output._11 = aiMe->a1;
     output._12 = aiMe->a2;
     output._13 = aiMe->a3;
     output._14 = aiMe->a4;

     output._21 = aiMe->b1;
     output._22 = aiMe->b2;
     output._23 = aiMe->b3;
     output._24 = aiMe->b4;

     output._31 = aiMe->c1;
     output._32 = aiMe->c2;
     output._33 = aiMe->c3;
     output._34 = aiMe->c4;

     output._41 = aiMe->d1;
     output._42 = aiMe->d2;
     output._43 = aiMe->d3;
     output._44 = aiMe->d4;

     return output;
 }



 /****************Scaling Functions: Taken from Assimp**********************/
 //-------------------------------------------------------------------------------
 // Calculate the boundaries of a given node and all of its children
 // The boundaries are in Worldspace (AABB)
 // piNode Input node
 // p_avOut Receives the min/max boundaries. Must point to 2 vec3s
 // piMatrix Transformation matrix of the graph at this position
 //-------------------------------------------------------------------------------
 int MeshLoader03::CalculateBounds(aiNode* piNode, aiVector3D* p_avOut,
     const aiMatrix4x4& piMatrix, const aiScene* pcScene)
 {
     assert(NULL != piNode);
     assert(NULL != p_avOut);

     aiMatrix4x4 mTemp = piNode->mTransformation;
     mTemp.Transpose();
     const aiMatrix4x4 aiMe = mTemp * piMatrix;

     for (unsigned int i = 0; i < piNode->mNumMeshes;++i)
     {
         for( unsigned int a = 0; a < pcScene->mMeshes[piNode->mMeshes[i]]->mNumVertices;++a)
         {

             const aiVector3D pc = pcScene->mMeshes[piNode->mMeshes[i]]->mVertices[a];
             XMFLOAT3 pc11 = aiVec3ToXMFloat3(&pc);
             XMFLOAT4X4 aiMe1 = aiMatrixToXMFloat4x4(&aiMe);

             XMVECTOR pc1 = XMVector3TransformCoord(XMLoadFloat3(&pc11),XMLoadFloat4x4(&aiMe1));

             p_avOut[0].x = min( p_avOut[0].x, pc1.m128_f32[0]);
             p_avOut[0].y = min( p_avOut[0].y, pc1.m128_f32[1]);
             p_avOut[0].z = min( p_avOut[0].z, pc1.m128_f32[2]);
             p_avOut[1].x = max( p_avOut[1].x, pc1.m128_f32[0]);
             p_avOut[1].y = max( p_avOut[1].y, pc1.m128_f32[1]);
             p_avOut[1].z = max( p_avOut[1].z, pc1.m128_f32[2]);
         }
     }
     for (unsigned int i = 0; i < piNode->mNumChildren;++i)
     {
         CalculateBounds( piNode->mChildren[i], p_avOut, aiMe, pcScene);
     }
     return 1;
 }


 //-------------------------------------------------------------------------------
 // Scale the asset that it fits perfectly into the viewer window
 // The function calculates the boundaries of the mesh and modifies the
 // global world transformation matrix according to the aset AABB
 //-------------------------------------------------------------------------------
 int MeshLoader03::ScaleAsset(const aiScene* pcScene)
 {
     aiVector3D aiVecs[2] = {
         aiVector3D( 1e10f, 1e10f, 1e10f),
         aiVector3D( -1e10f, -1e10f, -1e10f) };

     if (pcScene->mRootNode)
     {
         aiMatrix4x4 m;
         CalculateBounds(pcScene->mRootNode,aiVecs,m, pcScene);
     }

     aiVector3D vDelta = aiVecs[1] -  aiVecs[0];
     aiVector3D vHalf =  aiVecs[0] + (vDelta / 2.0f);
     float fScale = 10.0f / vDelta.Length();

     aiMatrix4x4 g_mWorld =  aiMatrix4x4(
                             1.0f,0.0f,0.0f,0.0f,
                             0.0f,1.0f,0.0f,0.0f,
                             0.0f,0.0f,1.0f,0.0f,
                             -vHalf.x,-vHalf.y,-vHalf.z,1.0f) *
         aiMatrix4x4(
                     fScale*m_scale,0.0f,0.0f,0.0f,
                     0.0f,fScale*m_scale,0.0f,0.0f,
                     0.0f,0.0f,fScale*m_scale,0.0f,
                     0.0f,0.0f,0.0f,1.0f);

     m_World = aiMatrixToXMFloat4x4(&g_mWorld);
     return 1;
 }
