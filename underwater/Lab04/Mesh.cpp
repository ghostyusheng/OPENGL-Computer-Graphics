#include "Mesh.h"

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Shader.h"

// ���� OBJ ��ʽ������
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

// ����������ʽ������
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

            // ������������ɫ
            if (scene->mMaterials[mesh->mMaterialIndex]) {
                aiColor4D diffuse;
                if (AI_SUCCESS == aiGetMaterialColor(scene->mMaterials[mesh->mMaterialIndex], AI_MATKEY_COLOR_DIFFUSE, &diffuse)) {
                    modelData.diffuseColor = vec3(diffuse.r, diffuse.g, diffuse.b);
                    modelData.hasColor = true; // ���ñ�־��ָʾ��ɫ�Ĵ���
                }
            }
        }
    }

    printf("=> count : %d \n", modelData.mPointCount);
    aiReleaseImport(scene);
    return modelData;
}

// ����ģ��
Model Mesh::load_model(const char* file_name, vec3 position, float rotationY, const char* textureFile, int scale) {
    printf("loading model: %s \n", file_name);
    Model model;
    model.name = file_name;

    // ����ļ���չ��
    std::string fileStr(file_name);
    if (fileStr.substr(fileStr.find_last_of(".") + 1) == "obj") {
        model.data = load_obj_mesh(file_name);
    }
    else {
        model.data = load_mesh(file_name); // ���е� DAE ����
    }

    model.position = position;
    model.rotationY = rotationY;
    model.hasTexture = false;

    if (textureFile != nullptr && strlen(textureFile) > 0) {
        model.textureID = loadTexture(textureFile);
        model.hasTexture = true;
    }

    // ���ɺͰ� VAO
    glGenVertexArrays(1, &model.vao);
    glBindVertexArray(model.vao);

    // ���� VBOs
    GLuint vp_vbo, vn_vbo;
    glGenBuffers(1, &vp_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
    glBufferData(GL_ARRAY_BUFFER, model.data.mPointCount * sizeof(vec3), &model.data.mVertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &vn_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
    glBufferData(GL_ARRAY_BUFFER, model.data.mPointCount * sizeof(vec3), &model.data.mNormals[0], GL_STATIC_DRAW);

    // ���Ӷ���λ��
    glEnableVertexAttribArray(glGetAttribLocation(Shader::shaders["model"], "vertex_position"));
    glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
    glVertexAttribPointer(glGetAttribLocation(Shader::shaders["model"], "vertex_position"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // ���Ӷ��㷨��
    glEnableVertexAttribArray(glGetAttribLocation(Shader::shaders["model"], "vertex_normal"));
    glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
    glVertexAttribPointer(glGetAttribLocation(Shader::shaders["model"], "vertex_normal"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    if (!model.data.mTextureCoords.empty()) {
        // ��������������ʵ�������ظ�
        for (auto& texCoord : model.data.mTextureCoords) {
            texCoord.v[0] *= scale;  // ����ֵ��Ϊ����Ҫ���ظ�����
            texCoord.v[1] *= scale;
        }

        GLuint vt_vbo;
        glGenBuffers(1, &vt_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
        glBufferData(GL_ARRAY_BUFFER, model.data.mPointCount * sizeof(vec2), &model.data.mTextureCoords[0], GL_STATIC_DRAW);
        GLint loc3 = glGetAttribLocation(Shader::shaders["model"], "vertex_texcoord");
        glEnableVertexAttribArray(loc3);
        glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    }

    glBindVertexArray(0); // ������ú��� VAO
    return model;
}

// ��������ĺ���ʵ�֣�ʾ����
GLuint Mesh::loadTexture(const char* filePath) {
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // ���������Ʒ�ʽ�͹��˷�ʽ
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // ������������
        int width, height, channels;
        unsigned char* data = stbi_load(filePath, &width, &height, &channels, 0);
        if (data) {
            GLenum format = (channels == 3) ? GL_RGB : GL_RGBA;

            // �����������ϴ��� OpenGL
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
