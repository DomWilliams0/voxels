#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <cglm/cglm.h>

#include "renderer.h"
#include "chunk_mesh.h"
#include "world.h"
#include "log.h"
#include "camera.h"

static int window_width, window_height;

/** returns 0 if invalid **/
static int load_shader(const char *filename, int type);

static ERR load_world_program(int *prog_out, const char *vertex_path, const char *fragment_path);

void GLAPIENTRY on_debug_message(GLenum source,
                                 GLenum type,
                                 GLuint id,
                                 GLenum severity,
                                 GLsizei length,
                                 const GLchar *message,
                                 const void *userParam) {
    LOG_INFO("GL: %s type = 0x%x, severity = 0x%x, message = %s",
             (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
             type, severity, message);
}


ERR renderer_init(struct renderer *renderer, int width, int height) {
    window_width = width;
    window_height = height;
    renderer->world = NULL;
    renderer->is_wireframe = 0;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(on_debug_message, 0);


    glClearColor(0.05, 0.05, 0.08, 1.0);

    ERR result = load_world_program(&renderer->world_program, "res/shaders/world.glslv", "res/shaders/world.glslf");

    return result;
}

static void set_projection_matrix(int program) {
    mat4 proj;
    glm_perspective(
            glm_rad(45),
            ((float) window_width) / window_height,
            0.1,
            50,
            proj);
    int loc = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(loc, 1, GL_FALSE, proj);
}

static void set_chunk_view(int program, mat4 camera_transform, struct chunk *chunk) {
    // offset view by chunk coords
    mat4 view;
    glm_mat4_copy(camera_transform, view);
    vec3 translation;
    chunk_world_space_pos(chunk, translation);
    glm_translate(view, translation);

    int loc = glGetUniformLocation(program, "view");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &view);
}

static void enable_terrain_attributes() {
    size_t word_size = sizeof(float);
    size_t stride = CHUNK_MESH_WORDS_PER_INSTANCE * word_size;

    // pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, 0);

    // colour
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, stride, 3L * word_size);

    // ao
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, false, stride, (3L + 1L) * word_size);
}

static void render_terrain(struct renderer *renderer, struct camera_state *camera_state) {
    // update chunks
    struct chunk_iterator it;
    world_chunks_first(renderer->world, &it);
    while (it.current) {
        struct chunk_render_objs *render_objs = chunk_render_objs(it.current);
        struct chunk_mesh_meta *meta = chunk_mesh_meta(it.current);

        // new vao and vbo for chunk
        if (chunk_has_flag(it.current, CHUNK_FLAG_NEW)) {
            glGenBuffers(1, &render_objs->vbo);
            glGenVertexArrays(1, &render_objs->vao);
            chunk_init_lighting(renderer->world, it.current);
        }

        // recalculate chunk terrain mesh
        if (chunk_has_flag(it.current, CHUNK_FLAG_DIRTY)) {
            glBindVertexArray(render_objs->vao);
            glBindBuffer(GL_ARRAY_BUFFER, render_objs->vbo);
            enable_terrain_attributes();
            int *mesh = chunk_mesh_gen(it.current, meta);
            glBufferData(GL_ARRAY_BUFFER, meta->vertex_count * sizeof(int), mesh, GL_STATIC_DRAW);
        }

        // render visible chunk
        if (chunk_has_flag(it.current, CHUNK_FLAG_VISIBLE)) {
            glBindVertexArray(render_objs->vao);
            set_chunk_view(renderer->world_program, camera_state->transform, it.current);
            glDrawArrays(GL_TRIANGLES, 0, meta->vertex_count);
        }

        world_chunks_next(renderer->world, &it);
    }
    world_chunks_clear_dirty(renderer->world);
}

void renderer_render(struct renderer *renderer, struct camera_state *camera_state, float interpolation) {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, window_width, window_height);
    glUseProgram(renderer->world_program);

    set_projection_matrix(renderer->world_program);

    // chunks
    render_terrain(renderer, camera_state);

    // TODO dynamic entities
}

void resize_callback(int width, int height) {
    window_width = width;
    window_height = height;
}

static char *read_shader_source(const char *filename) {
    FILE *f = fopen(filename, "r");

    if (!f) {
        LOG_ERROR("failed to open shader '%s'", filename);
        return NULL;
    }

    // find file length
    fseek(f, 0, SEEK_END);
    size_t
            len = (size_t) ftell(f);
    rewind(f);

    char *buf = NULL;

    // alloc buffer
    buf = calloc(len + 1, sizeof(char));
    if (!buf) {
        LOG_ERROR("failed to allocate %ld bytes for shader", len);
        goto cleanup;
    }

    // read
    size_t
            read = fread(buf, sizeof(char), len, f);
    if (read != len) {
        LOG_ERROR("failed to read shader: read %ld/%ld", read, len);
        free(buf);
        buf = NULL;
    }

    cleanup:
    fclose(f);
    return buf;
}

static int load_shader(const char *filename, int type) {
    char *src = read_shader_source(filename);
    int shader = 0;
    if (src) {
        shader = glCreateShader(type);
        size_t len = strlen(src);
        glShaderSource(shader, 1, &src, &len);
        glCompileShader(shader);

        int compile_status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
        if (compile_status == GL_FALSE) {
            size_t log_len = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);

            char *log_str = calloc(sizeof(char), log_len + 1);
            if (!log_str) {
                LOG_ERROR("failed to alloc log buffer while failing to load shader, oh dear");
            } else {
                glGetShaderInfoLog(shader, log_len, NULL, log_str);
                LOG_ERROR("failed to compile shader: %s", log_str);
                free(log_str);

            }
            glDeleteShader(shader);
            shader = 0;
        }

        free(src);
    }

    return shader;
}

static ERR load_world_program(int *prog_out, const char *vertex_path, const char *fragment_path) {
    int vert = load_shader(vertex_path, GL_VERTEX_SHADER);
    int frag = load_shader(fragment_path, GL_FRAGMENT_SHADER);

    if (vert == 0 || frag == 0) {
        LOG_ERROR("failed to load shaders");
        return ERR_FAIL;
    }

    int prog = *prog_out = glCreateProgram();

    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);

    int link_status;
    glGetProgramiv(prog, GL_LINK_STATUS, &link_status);
    if (link_status == GL_FALSE) {
        LOG_ERROR("failed to link program");
        return ERR_FAIL;
    }

    glProgramParameteri(prog, GL_PROGRAM_SEPARABLE, GL_TRUE);

    glDetachShader(prog, vert);
    glDetachShader(prog, frag);
    glDeleteShader(vert);
    glDeleteShader(frag);

    return ERR_SUCC;

}

void renderer_toggle_wireframe(struct renderer *renderer) {
    glPolygonMode(GL_FRONT_AND_BACK, (renderer->is_wireframe = !renderer->is_wireframe) ? GL_LINE : GL_FILL);
}
