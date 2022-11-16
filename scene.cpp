#include "include/scene.hpp"

#include "Generated_Code/VertexShader.h"
#include "Generated_Code/PixelShader.h"
#include "Generated_Code/TexVertexShader.h"
#include "Generated_Code/TexPixelShader.h"



//void Mesh::loadObj(const std::string& path)
//{
//    std::ifstream file(path);
//    std::string line;
//    while (std::getline(file, line))
//    {
//        std::istringstream ss(line);
//        char t;
//        if (line.starts_with('v'))
//        {
//            DirectX::XMFLOAT3 v;
//            ss >> t >> v.x >> v.y >> v.z;
//            vertex.emplace_back(v);
//        }
//        if (strncmp(line.c_str(), "vt", 2))
//        {
//            printf("not doing anything for now");
//        }
//        if (strncmp(line.c_str(), "vn", 2))
//        {
//            printf("not doing anything for now");
//        }
//        if (line.starts_with('f'))
//        {
//            int d1, d2, d3;
//            //int ta;
//            // f 5/5/1 3/3/1 1/1/1
//            //ss >> t >> d1 >> t >> ta >> t >> ta
//            //    >> d2 >> t >> ta >> t >> ta
//            //    >> d3 >> t >> ta >> t >> ta;
//            ss >> t >> d1 >> d2 >> d3;
//            indices.emplace_back(d1 - 1);
//            indices.emplace_back(d2 - 1);
//            indices.emplace_back(d3 - 1);
//        }
//    }
//}