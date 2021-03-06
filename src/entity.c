#include "internal.h"
#include "shader.h"

struct _Entity {
    Model *model;

    vec3 color;
    vec3 emissive;

    GLuint shader;

    mat4x4 transform;
    vec3 position;
    quat rotation;
    float scale;
};

Entity *create_entity(Model *model) {
    Entity *entity = malloc(sizeof(Entity));

    entity->model = model;

    entity->color[0] = 1.0f;
    entity->color[1] = 1.0f;
    entity->color[2] = 1.0f;

    entity->emissive[0] = 0.0f;
    entity->emissive[1] = 0.0f;
    entity->emissive[2] = 0.0f;

    entity->shader = load_shader("assets/static.vs", "assets/static.fs");

    mat4x4_identity(entity->transform);
    entity->position[0] = 0.0f;
    entity->position[1] = 0.0f;
    entity->position[2] = 0.0f;

    quat_identity(entity->rotation);

    entity->scale = 1;

    return entity;
}

void destroy_entity(Entity *entity) {
    free(entity);
}

void render_entity(Entity *entity, Scene *scene) {
    glUseProgram(entity->shader);

    GLint color_loc = glGetUniformLocation(entity->shader, "color");
    GLint emissive_loc = glGetUniformLocation(entity->shader, "emissive");

    glUniform3fv(color_loc, 1, (float *)entity->color);
    glUniform3fv(emissive_loc, 1, (float *)entity->emissive);

    GLint transformation_matrix_loc = glGetUniformLocation(entity->shader, "transformationMatrix");
    GLint normal_matrix_loc = glGetUniformLocation(entity->shader, "normalMatrix");
    GLint projection_matrix_loc = glGetUniformLocation(entity->shader, "projectionMatrix");
    GLint view_matrix_loc = glGetUniformLocation(entity->shader, "viewMatrix");

    mat4x4 translation, rotation, scale;

    mat4x4_translate(translation, entity->position[0], entity->position[1], entity->position[2]);
    mat4x4_from_quat(rotation, entity->rotation);

    mat4x4_identity(scale);
    mat4x4_scale_aniso(scale, scale, entity->scale, entity->scale, entity->scale);

    mat4x4_identity(entity->transform);
    mat4x4_mul(entity->transform, entity->transform, translation);
    mat4x4_mul(entity->transform, entity->transform, rotation);
    mat4x4_mul(entity->transform, entity->transform, scale);

    mat4x4 normal_matrix, temp_matrix;
    mat4x4_transpose(temp_matrix, entity->transform);
    mat4x4_invert(normal_matrix, temp_matrix);

    glUniformMatrix4fv(transformation_matrix_loc, 1, GL_FALSE, (float *)entity->transform);
    glUniformMatrix4fv(normal_matrix_loc, 1, GL_FALSE, (float *)normal_matrix);
    glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, (float *)scene->projection);

    vec3 forward, up = {0, 1, 0};
    Camera *camera = scene->camera;

    vec3_add(forward, camera->position, camera->direction);
    mat4x4_look_at(camera->view, camera->position, forward, up);

    glUniformMatrix4fv(view_matrix_loc, 1, GL_FALSE, (float *)camera->view);

    GLint view_position_loc = glGetUniformLocation(entity->shader, "viewPos");
    glUniform3fv(view_position_loc, 1, (float *)camera->position);

    // clang-format off
    GLint light_position_loc  = glGetUniformLocation(entity->shader, "light.position");
    GLint light_ambient_loc   = glGetUniformLocation(entity->shader, "light.ambient");
    GLint light_diffuse_loc   = glGetUniformLocation(entity->shader, "light.diffuse");
    GLint light_specular_loc  = glGetUniformLocation(entity->shader, "light.specular");
    GLint light_constant_loc  = glGetUniformLocation(entity->shader, "light.constant");
    GLint light_linear_loc    = glGetUniformLocation(entity->shader, "light.linear");
    GLint light_quadratic_loc = glGetUniformLocation(entity->shader, "light.quadratic");
    // clang-format on

    Light *light = scene->lights[0];

    glUniform4fv(light_position_loc, 1, (float *)light->position);
    glUniform3fv(light_ambient_loc, 1, (float *)light->ambient_color);
    glUniform3fv(light_diffuse_loc, 1, (float *)light->diffuse_color);
    glUniform3fv(light_specular_loc, 1, (float *)light->specular_color);
    glUniform1f(light_constant_loc, light->constant);
    glUniform1f(light_linear_loc, light->linear);
    glUniform1f(light_quadratic_loc, light->quadratic);

    GLint has_diffuse_texture_loc = glGetUniformLocation(entity->shader, "hasDiffuseTexture");
    glUniform1ui(has_diffuse_texture_loc, entity->model->has_texture);

    render_model(entity->model, entity->shader);
}

void set_model(Entity *entity, Model *model) {
    entity->model = model;
}

void set_color(Entity *entity, float r, float g, float b) {
    entity->color[0] = r;
    entity->color[1] = g;
    entity->color[2] = b;
}

void set_emissive(Entity *entity, float r, float g, float b) {
    entity->emissive[0] = r;
    entity->emissive[1] = g;
    entity->emissive[2] = b;
}

void set_position(Entity *entity, float x, float y, float z) {
    entity->position[0] = x;
    entity->position[1] = y;
    entity->position[2] = z;
}

void set_rotation(Entity *entity, float x, float y, float z, float angle) {
    quat_identity(entity->rotation);

    vec3 axis = {x, y, z};
    quat_rotate(entity->rotation, angle, axis);
}

void set_scale(Entity *entity, float scale) {
    entity->scale = scale;
}
