#include "Mesh.h"

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Shader.h"
#include "Log.h"
#include "global.h"

// 加载 OBJ 格式的网格
ModelData Mesh::load_obj_mesh(const char* file_name) {
    ModelData modelData;
    const aiScene* scene = aiImportFile(file_name, aiProcess_Triangulate | aiProcess_PreTransformVertices);

    if (!scene) {
        fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
        return modelData;
    }

    for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
        const aiMesh* mesh = scene->mMeshes[m_i];
        modelData.mPointCount += mesh->mNumVertices;

        for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
            if (mesh->HasPositions()) {
                const aiVector3D* vp = &(mesh->mVertices[v_i]);
                modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
            }
            if (mesh->HasNormals()) {
                const aiVector3D* vn = &(mesh->mNormals[v_i]);
                modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
            }
            if (mesh->HasTextureCoords(0)) {
                const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
                modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
            }
        }
    }

    aiReleaseImport(scene);
    return modelData;
}

void loadMeshRecursive(const aiNode* node, const aiScene* scene, ModelData& modelData) {
    // 遍历当前节点的所有网格
    for (unsigned int m_i = 0; m_i < node->mNumMeshes; m_i++) {
        Log::log(Log::Level::INFO, Str(" -> loading mesh {}", m_i)); // 保留您的日志
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[m_i]];
        SubMesh subMesh; // 创建一个子网格

        for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
            if (mesh->HasPositions()) {
                const aiVector3D* vp = &(mesh->mVertices[v_i]);
                subMesh.vertices.push_back(vec3(vp->x, vp->y, vp->z));
            }
            if (mesh->HasNormals()) {
                const aiVector3D* vn = &(mesh->mNormals[v_i]);
                subMesh.normals.push_back(vec3(vn->x, vn->y, vn->z));
            }
            if (mesh->HasTextureCoords(0)) {
                const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
                subMesh.textureCoords.push_back(vec2(vt->x, vt->y));
            }
        }

        // 加载漫反射颜色
        if (scene->mMaterials[mesh->mMaterialIndex]) {
            aiColor4D diffuse;
            if (AI_SUCCESS == aiGetMaterialColor(scene->mMaterials[mesh->mMaterialIndex], AI_MATKEY_COLOR_DIFFUSE, &diffuse)) {
                subMesh.diffuseColor = vec3(diffuse.r, diffuse.g, diffuse.b);
                subMesh.hasColor = true; // 设置标志以指示颜色的存在
            }
        }

        modelData.subMeshes.push_back(subMesh); // 添加子网格
        modelData.mPointCount += mesh->mNumVertices; // 更新总点数
    }

    // 递归加载子节点
    for (unsigned int n_i = 0; n_i < node->mNumChildren; n_i++) {
        loadMeshRecursive(node->mChildren[n_i], scene, modelData);
    }
}


// 加载其他格式的网格
ModelData Mesh::load_mesh(const char* file_name) {
    ModelData modelData;
    const aiScene* scene = aiImportFile(file_name, aiProcess_Triangulate | aiProcess_PreTransformVertices);

    if (!scene) {
        fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
        return modelData;
    }
    else {
        printf("=> loaded: %s \n", file_name);
    }

    // 从根节点开始加载
    loadMeshRecursive(scene->mRootNode, scene, modelData);

    printf("=> count : %d \n", modelData.mPointCount);
    printf("=> sub mesh count: %d \n", modelData.subMeshes.size()); // 打印子网格数量
    aiReleaseImport(scene);
    return modelData;
}



// 加载模型
Model Mesh::load_model(const char* file_name, vec3 position, float rotationY, const char* textureFile, int scale) {
    printf("loading model: %s \n", file_name);
    Model model;
    model.name = file_name;

    // 检查文件扩展名
    std::string fileStr(file_name);
    if (fileStr.substr(fileStr.find_last_of(".") + 1) == "obj") {
        model.data = load_obj_mesh(file_name);
    }
    else {
        model.data = load_mesh(file_name); // 现有的 DAE 加载
    }

    model.position = position;
    model.rotationY = rotationY;
    model.hasTexture = false;

    if (textureFile != nullptr && strlen(textureFile) > 0) {
        model.textureID = loadTexture(textureFile);
        model.hasTexture = true;
    }

    // 生成和绑定 VAO
    glGenVertexArrays(1, &model.vao);
    glBindVertexArray(model.vao);

    // 合并所有子网格的顶点和法线
    std::vector<vec3> allVertices;
    std::vector<vec3> allNormals;
    std::vector<vec2> allTextureCoords;

    for (auto& subMesh : model.data.subMeshes) {
        // 计算并设置 vertexCount
        subMesh.vertexCount = subMesh.vertices.size(); // 假设 subMesh 有 vertices 属性
        allVertices.insert(allVertices.end(), subMesh.vertices.begin(), subMesh.vertices.end());
        allNormals.insert(allNormals.end(), subMesh.normals.begin(), subMesh.normals.end());
        allTextureCoords.insert(allTextureCoords.end(), subMesh.textureCoords.begin(), subMesh.textureCoords.end());
    }

    // 生成 VBOs
    GLuint vp_vbo, vn_vbo;
    glGenBuffers(1, &vp_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
    glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(vec3), allVertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &vn_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
    glBufferData(GL_ARRAY_BUFFER, allNormals.size() * sizeof(vec3), allNormals.data(), GL_STATIC_DRAW);

    // 链接顶点位置
    glEnableVertexAttribArray(glGetAttribLocation(Shader::shaders["model"], "vertex_position"));
    glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
    glVertexAttribPointer(glGetAttribLocation(Shader::shaders["model"], "vertex_position"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // 链接顶点法线
    glEnableVertexAttribArray(glGetAttribLocation(Shader::shaders["model"], "vertex_normal"));
    glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
    glVertexAttribPointer(glGetAttribLocation(Shader::shaders["model"], "vertex_normal"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    if (!allTextureCoords.empty()) {
        // 缩放纹理坐标以实现纹理重复
        for (auto& texCoord : allTextureCoords) {
            texCoord.v[0] *= scale;  // 将此值设为你想要的重复倍数
            texCoord.v[1] *= scale;
        }

        GLuint vt_vbo;
        glGenBuffers(1, &vt_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
        glBufferData(GL_ARRAY_BUFFER, allTextureCoords.size() * sizeof(vec2), allTextureCoords.data(), GL_STATIC_DRAW);
        GLint loc3 = glGetAttribLocation(Shader::shaders["model"], "vertex_texcoord");
        glEnableVertexAttribArray(loc3);
        glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    }

    glBindVertexArray(0); // 完成设置后解绑 VAO
    return model;
}


// 加载纹理的函数实现（示例）
GLuint Mesh::loadTexture(const char* filePath) {
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // 设置纹理环绕方式和过滤方式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // 加载纹理数据
        int width, height, channels;
        unsigned char* data = stbi_load(filePath, &width, &height, &channels, 0);
        if (data) {
            GLenum format = (channels == 3) ? GL_RGB : GL_RGBA;

            // 将纹理数据上传到 OpenGL
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            std::cout << "Texture loaded: " << filePath << " (" << width << "x" << height << ")" << std::endl;
            stbi_image_free(data);
        }
        else {
            std::cerr << "Failed to load texture: " << filePath << std::endl;
            stbi_image_free(data);
            return 0;
        }

        return textureID;
}
