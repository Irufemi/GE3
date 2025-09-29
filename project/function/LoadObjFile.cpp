#include "Function.h"

#include "../math/Vector4.h"
#include "../math/Vector3.h"
#include "../math/Vector2.h"
#include "../math/Matrix4x4.h"
#include "../function/Math.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <cassert>
#include <map>

ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename) {
    // 1. 中で必要となる変数の宣言
    // 2. ファイルを開く
    // 3. 実際にファイルを読み、ModelDataを構築していく
    // 4. ModelDataを返す

    /// 1.2.必要な変数の宣言とファイルを開く

    ModelData modelData; //構築するModelData
    std::vector<Vector4> positions; //位置
    std::vector<Vector3> normals; //法線
    std::vector<Vector2> texcoords; //テクスチャ座標
    std::string line; //ファイルから読んだ1行を格納するもの

    std::ifstream file(directoryPath + "/" + filename); //ファイルを開く
    assert(file.is_open()); //とりあえず開けなかったら止める

    ///3.ファイルを読み、ModelDataを構築
    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier; //先頭の識別子を読む


        //identifierに応じた処理

        ///頂点情報を読む
        if (identifier == "v") {
            Vector4 position;
            s >> position.x >> position.y >> position.z;
            position.w = 1.0f;
            positions.push_back(position);
        } 
        else if (identifier == "vt") {
            Vector2 texcoord;
            s >> texcoord.x >> texcoord.y;
            texcoords.push_back(texcoord);
        } 
        else if (identifier == "vn") {
            Vector3 normal;
            s >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }

        ///三角形を作る

        else if (identifier == "f") {
            VertexData triangle[3];
            //面は三角形限定。その他は未対応
            for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
                std::string vertexDefinition;
                s >> vertexDefinition;
                //頂点の要素のIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
                std::istringstream v(vertexDefinition);
                uint32_t elementIndices[3];
                for (int32_t element = 0; element < 3; ++element) {
                    std::string index;
                    std::getline(v, index, '/'); //区切りでインデックスを読んでいく
                    elementIndices[element] = std::stoi(index);
                }
                //要素へのIndexから、実際の要素の値を取得して、頂点を構築する
                Vector4 position = positions[elementIndices[0] - 1];
                Vector2 texcoord = texcoords[elementIndices[1] - 1];
                Vector3 normal = normals[elementIndices[2] - 1];
                //VertexData vertex = { position,texcoord,normal };
                //modelData.vertices.push_back(vertex);

                ///右手系から左手系へ

                position.x *= -1.0f;
                normal.x *= -1.0f;

                ///Texture座標の原点

                texcoord.y = 1.0f - texcoord.y;

                ///右手系から左手系へ

                triangle[faceVertex] = { position,texcoord,normal };

            }

            //頂点を逆順で登録することで、回り順を逆にする
            modelData.vertices.push_back(triangle[2]);
            modelData.vertices.push_back(triangle[1]);
            modelData.vertices.push_back(triangle[0]);
        }

        ///obj読み込みにmaterial読み込みを追加

        else if (identifier == "mtllib") {
            //materialTempalateLibraryファイルの名前を取得する
            std::string materialFilename;
            s >> materialFilename;
            //基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイルを渡す
            modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
        }
    }

    return modelData;
}


ObjModel LoadObjFileM(const std::string& directoryPath, const std::string& filename) {
    ObjModel objModel;
    std::vector<Vector4> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> texcoords;
    std::map<std::string, ObjMaterial> materialMap;

    std::ifstream file(directoryPath + "/" + filename);
    assert(file.is_open());

    std::string line;
    ObjMesh currentMesh;

    while (std::getline(file, line)) {
        std::istringstream s(line);
        std::string id;
        s >> id;

        if (id == "v") {
            Vector4 pos;
            s >> pos.x >> pos.y >> pos.z;
            pos.w = 1.0f;
            // 左手系変換はここだけ
            pos.x *= -1.0f;
            positions.push_back(pos);
        } else if (id == "vt") {
            Vector2 uv;
            s >> uv.x >> uv.y;
            // y反転のみここで
            uv.y = 1.0f - uv.y;
            texcoords.push_back(uv);
        } else if (id == "vn") {
            Vector3 n;
            s >> n.x >> n.y >> n.z;
            // 左手系変換はここだけ
            n.x *= -1.0f;
            normals.push_back(n);
        } else if (id == "f") {
            VertexData tri[3];
            for (int i = 0; i < 3; ++i) {
                std::string def;
                s >> def;
                int pIdx = -1, tIdx = -1, nIdx = -1;
                ParseObjFaceToken(def, pIdx, tIdx, nIdx);

                Vector4 position = (pIdx > 0) ? positions[pIdx - 1] : Vector4{};
                Vector2 texcoord = (tIdx > 0) ? texcoords[tIdx - 1] : Vector2{ 0.5f, 0.5f };
                Vector3 normal = (nIdx > 0) ? normals[nIdx - 1] : Vector3{};

                tri[i] = { position, texcoord, normal };
            }
            // 三角形の回り順は逆にしている（必要な場合のみ）
            currentMesh.vertices.push_back(tri[2]);
            currentMesh.vertices.push_back(tri[1]);
            currentMesh.vertices.push_back(tri[0]);
        } else if (id == "usemtl") {
            if (!currentMesh.vertices.empty()) {
                objModel.meshes.push_back(currentMesh);
                currentMesh = ObjMesh();
            }
            std::string matName;
            s >> matName;
            if (materialMap.count(matName)) {
                currentMesh.material = materialMap[matName];
            } else {
                currentMesh.material = ObjMaterial(); // デフォルト値
            }
        } else if (id == "mtllib") {
            std::string mtlFilename;
            s >> mtlFilename;
            std::ifstream mtlFile(directoryPath + "/" + mtlFilename);
            assert(mtlFile.is_open());

            std::string mtlLine, currentName;
            while (std::getline(mtlFile, mtlLine)) {
                std::istringstream ms(mtlLine);
                std::string mtlId;
                ms >> mtlId;

                if (mtlId == "newmtl") {
                    ms >> currentName;
                    materialMap[currentName] = ObjMaterial();
                } else if (mtlId == "Kd") {
                    ms >> materialMap[currentName].color.x
                        >> materialMap[currentName].color.y
                        >> materialMap[currentName].color.z;
                    materialMap[currentName].color.w = 1.0f;
                } else if (mtlId == "Ka") {
                    ms >> materialMap[currentName].ambient.x
                        >> materialMap[currentName].ambient.y
                        >> materialMap[currentName].ambient.z;
                } else if (mtlId == "Ks") {
                    ms >> materialMap[currentName].specular.x
                        >> materialMap[currentName].specular.y
                        >> materialMap[currentName].specular.z;
                } else if (mtlId == "Ns") {
                    ms >> materialMap[currentName].shininess;
                } else if (mtlId == "d" || mtlId == "Tr") {
                    ms >> materialMap[currentName].alpha;
                } else if (mtlId == "map_Kd") {
                    std::string token;
                    bool hasTransform = false;
                    // テクスチャオプション対応
                    while (ms >> token) {
                        if (token == "-o") {
                            ms >> materialMap[currentName].uvTransform.m[3][0]
                                >> materialMap[currentName].uvTransform.m[3][1];
                            hasTransform = true;
                        } else if (token == "-s") {
                            ms >> materialMap[currentName].uvTransform.m[0][0]
                                >> materialMap[currentName].uvTransform.m[1][1];
                            hasTransform = true;
                        } else {
                            materialMap[currentName].textureFilePath = directoryPath + "/" + token;
                            break;
                        }
                    }
                    // デフォルト値セット
                    if (!hasTransform) {
                        materialMap[currentName].uvTransform = Math::MakeAffineMatrix(
                            { 1.0f, 1.0f, 1.0f }, { 0,0,0 }, { 0,0,0 });
                    }
                }
            }
        }
    }

    if (!currentMesh.vertices.empty()) {
        objModel.meshes.push_back(currentMesh);
    }

    return objModel;
}

// f行の頂点データを安全にパースする関数例
bool ParseObjFaceToken(const std::string& token, int& posIdx, int& uvIdx, int& normIdx) {
    posIdx = uvIdx = normIdx = -1; // デフォルト値（0開始なら0に）

    size_t firstSlash = token.find('/');
    size_t secondSlash = token.find('/', firstSlash + 1);

    // 位置インデックス
    if (firstSlash == std::string::npos) {
        // 例: "1"
        if (!token.empty()) posIdx = std::stoi(token);
    } else {
        // 例: "1/2/3", "1//3", "1/2"
        if (firstSlash > 0) posIdx = std::stoi(token.substr(0, firstSlash));
        // UVインデックス
        if (secondSlash != std::string::npos) {
            // "1/2/3"
            if (secondSlash > firstSlash + 1) uvIdx = std::stoi(token.substr(firstSlash + 1, secondSlash - firstSlash - 1));
            // 法線インデックス
            if (token.size() > secondSlash + 1) normIdx = std::stoi(token.substr(secondSlash + 1));
        } else {
            // "1/2"
            if (token.size() > firstSlash + 1) uvIdx = std::stoi(token.substr(firstSlash + 1));
        }
    }
    return true;
}