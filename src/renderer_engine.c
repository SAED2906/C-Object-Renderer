#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define MAX_VERTICES 1000
#define MAX_OBJECTS 100

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    float x, y;
} Vec2;

typedef struct {
    float x, y, z;
    float pitch, yaw;
} Camera;

typedef struct {
    Vec3 vertices[8];
} Object3D;

typedef struct {
    Vec2 position;
    SDL_Color color;
} Vertex2D;

Object3D objects[MAX_OBJECTS];
int object_count = 0;
SDL_Vertex verts[MAX_VERTICES];
int vert_count = 0;

Vec2 project(Vec3 point, float fov, float aspect, float near, float far) {
    Vec2 screen_point;
    if (point.z == 0) point.z = 0.001f; // Prevent division by zero
    float f = 1.0f / tan(fov / 2.0f);
    screen_point.x = (point.x * f) / (aspect * point.z);
    screen_point.y = (point.y * f) / point.z;
    return screen_point;
}

Vec3 subtract(Vec3 a, Vec3 b) {
    Vec3 result = {a.x - b.x, a.y - b.y, a.z - b.z};
    return result;
}

Vec3 crossProduct(Vec3 a, Vec3 b) {
    Vec3 result = {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
    return result;
}

float dotProduct(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

void handle_camera_movement(Camera *camera, float deltaTime) {
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    float move_speed = 5.0f * deltaTime;

    float cos_pitch = cos(camera->pitch);
    float sin_pitch = sin(camera->pitch);
    float cos_yaw = cos(camera->yaw);
    float sin_yaw = sin(camera->yaw);

    Vec3 forward = {
        cos_pitch * sin_yaw,
        sin_pitch,
        cos_pitch * cos_yaw
    };

    Vec3 horizontal = {
        cos_yaw,
        0,
        -sin_yaw
    };

    if (state[SDL_SCANCODE_W]) {
        camera->x += forward.x * move_speed;
        camera->y += forward.y * move_speed;
        camera->z += forward.z * move_speed;
    }
    if (state[SDL_SCANCODE_S]) {
        camera->x -= forward.x * move_speed;
        camera->y -= forward.y * move_speed;
        camera->z -= forward.z * move_speed;
    }
    if (state[SDL_SCANCODE_A]) {
        camera->x -= horizontal.x * move_speed;
        camera->z -= horizontal.z * move_speed;
    }
    if (state[SDL_SCANCODE_D]) {
        camera->x += horizontal.x * move_speed;
        camera->z += horizontal.z * move_speed;
    }
}

void handle_mouse_movement(Camera *camera, int relx_mouse, int rely_mouse) {
    float sensitivity = 0.001f;
    camera->yaw += relx_mouse * sensitivity;
    camera->pitch -= rely_mouse * sensitivity;

    if (camera->pitch > 1.5f) camera->pitch = 1.5f;
    if (camera->pitch < -1.5f) camera->pitch = -1.5f;
}

void add_object(Object3D object) {
    if (object_count < MAX_OBJECTS) {
        objects[object_count++] = object;
    }
}

void render_objects(Camera camera, float fov, float aspect, float near, float far, SDL_Renderer *renderer) {
    vert_count = 0;
    float cos_pitch = cos(camera.pitch);
    float sin_pitch = sin(camera.pitch);
    float cos_yaw = cos(camera.yaw);
    float sin_yaw = sin(camera.yaw);

    for (int i = 0; i < object_count; i++) {
        for (int j = 0; j < 8; j++) {
            Vec3 transformed_vertex = {
                objects[i].vertices[j].x - camera.x,
                objects[i].vertices[j].y - camera.y,
                objects[i].vertices[j].z - camera.z
            };

            float x = transformed_vertex.x * cos_yaw - transformed_vertex.z * sin_yaw;
            float z = transformed_vertex.x * sin_yaw + transformed_vertex.z * cos_yaw;
            transformed_vertex.x = x;
            transformed_vertex.z = z;

            float y = transformed_vertex.y * cos_pitch - transformed_vertex.z * sin_pitch;
            transformed_vertex.z = transformed_vertex.y * sin_pitch + transformed_vertex.z * cos_pitch;
            transformed_vertex.y = y;

            if (transformed_vertex.z < near || transformed_vertex.z > far) continue;

            Vec2 screen_coord = project(transformed_vertex, fov, aspect, near, far);
            verts[vert_count].position.x = (screen_coord.x + 1.0f) * 0.5f * SCREEN_WIDTH;
            verts[vert_count].position.y = (1.0f - screen_coord.y) * 0.5f * SCREEN_HEIGHT;
            verts[vert_count].color.r = 128;
            verts[vert_count].color.g = 128;
            verts[vert_count].color.b = 128;
            verts[vert_count].color.a = 255;

            if (vert_count == 3) {
                verts[vert_count].color.r = 0;
                verts[vert_count].color.g = 255;
                verts[vert_count].color.b = 255;
                verts[vert_count].color.a = 255;
            }

            if (vert_count == 7) {
                verts[vert_count].color.r = 255;
                verts[vert_count].color.g = 0;
                verts[vert_count].color.b = 255;
                verts[vert_count].color.a = 255;
            }

            vert_count++;
        }
    }

    int indices[] = {
        0, 1, 2, 1, 2, 3, // front face
        4, 5, 6, 5, 6, 7, // back face
        0, 1, 4, 1, 4, 5, // top face
        2, 3, 6, 3, 6, 7, // bottom face
        0, 2, 4, 2, 4, 6, // left face
        1, 3, 5, 3, 5, 7  // right face
    };

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_RenderGeometry(renderer, NULL, verts, 8, indices, 36);
    // Backface culling
    for (int i = 0; i < sizeof(indices) / sizeof(indices[0]); i += 3) {
        Vec3 v0 = objects[0].vertices[indices[i]];
        Vec3 v1 = objects[0].vertices[indices[i + 1]];
        Vec3 v2 = objects[0].vertices[indices[i + 2]];

        Vec3 edge1 = subtract(v1, v0);
        Vec3 edge2 = subtract(v2, v0);
        Vec3 normal = crossProduct(edge1, edge2);

        Vec3 camera_direction = {camera.x - v0.x, camera.y - v0.y, camera.z - v0.z};
        
        if (dotProduct(normal, camera_direction) < 0) {
            SDL_RenderGeometry(renderer, NULL, &verts[indices[i]], 3, NULL, 0);
        }
    }

    SDL_RenderPresent(renderer);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("3D to 2D Projection", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_MOUSE_CAPTURE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Camera camera = {0.0f, 0.0f, 5.0f, 0.0f, 0.0f};

    Object3D cube = {{
        {1.0f, 1.0f, -1.0f},
        {-1.0f, 1.0f, -1.0f},
        {1.0f, -1.0f, -1.0f},
        {-1.0f, -1.0f, -1.0f},
        {1.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f, 1.0f},
        {1.0f, -1.0f, 1.0f},
        {-1.0f, -1.0f, 1.0f}
    }};

    add_object(cube);

    // Object3D cube2 = {{
    //     {1.0f+2.0f, 1.0f+2.0f, -1.0f+2.0f},
    //     {-1.0f+2.0f, 1.0f+2.0f, -1.0f+2.0f},
    //     {1.0f+2, -1.0f+2, -1.0f+2},
    //     {-1.0f+2, -1.0f+2, -1.0f+2},
    //     {1.0f+2, 1.0f+2, 1.0f+2},
    //     {-1.0f+2, 1.0f+2, 1.0f+2},
    //     {1.0f+2, -1.0f+2, 1.0f+2},
    //     {-1.0f+2, -1.0f+2, 1.0f+2}
    // }};

    // addObject(cube2);

    float fov = 90.0f * (3.14159f / 180.0f);
    float aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
    float near = 0.1f;
    float far = 100.0f;

    int running = 1;
    Uint32 last_time = SDL_GetTicks();
    SDL_SetRelativeMouseMode(SDL_TRUE);

    while (running) {
        Uint32 current_time = SDL_GetTicks();
        float delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                running = 0;
            }
            if (event.type == SDL_MOUSEMOTION) {
                handle_mouse_movement(&camera, event.motion.xrel, event.motion.yrel);
            }
        }

        handle_camera_movement(&camera, delta_time);

        render_objects(camera, fov, aspect, near, far, renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
