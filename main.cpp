// dear imgui: standalone example application for SDL2 + OpenGL
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
// (GL3W is a helper library to access OpenGL functions since there is no standard header to access modern OpenGL functions easily. Alternatives are GLEW, Glad, etc.)

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL.h>

// About OpenGL function loaders: modern OpenGL doesn't have a standard header file and requires individual function pointers to be loaded manually.
// Helper libraries are often used for this purpose! Here we are supporting a few common ones: gl3w, glew, glad.
// You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Forward declare to keep main func on top for readability
int SDLInit(SDL_Window** window, SDL_GLContext* gl_context, const char** glsl_version);
ImGuiIO& ImGUIInit(SDL_Window* window, SDL_GLContext* gl_context, const char* glsl_version);
void UILoop(SDL_Window* window, ImGuiIO& io );
void Cleanup(SDL_Window* window, SDL_GLContext* gl_context);

void ShowConfigWindow();

// Main code
int main(int argc, char** argv)
{
    //TODO: Figure out how arguments come into the program
    bool headless = false;
    for (int i = 0; i < argc; ++i) {
        printf( "Arguments %s \n", argv[i]);
        if (strcmp(argv[i], "-h") == 0 )
            headless = true;
    }
    
    printf(" -- HEADLESS: %i\n", headless ? 1 : 0 );
    
    //TODO: Load default config (maybe start with an example one?), create nodes etc.
    //TODO: Skip all of this if headless
    
    SDL_Window* window;
    SDL_GLContext gl_context;
    const char* glsl_version;
    int result = SDLInit(&window, &gl_context, &glsl_version);
    if ( result != 0 ) {
        return result;
    }
    
    printf("VERSION: %s", glsl_version);
    
    ImGuiIO& io = ImGUIInit(window, &gl_context, glsl_version);
    
    //TODO: blocking loop when running headless...
    // Blocking UI loop
    UILoop(window, io);
    
    // Cleanup
    Cleanup(window, &gl_context);

    return 0;
}

int SDLInit( SDL_Window** window, SDL_GLContext* gl_context, const char** glsl_version ) {
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 Core + GLSL 150
    *glsl_version = new char[13] { "#version 150" };
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    *glsl_version = new char[13] { "#version 130" };
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    //TODO: Make this a skippable part of the system...
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    *window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    *gl_context = SDL_GL_CreateContext(*window);
    SDL_GL_MakeCurrent(*window, *gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    
    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }
    
    return 0;
}

ImGuiIO& ImGUIInit(SDL_Window* window, SDL_GLContext* gl_context, const char* glsl_version) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    return io;
}

void UILoop( SDL_Window* window, ImGuiIO& io ) {
    // Our state
    bool show_demo_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);    //should be irrelevant soon...
    
    // Main loop
    bool done = false;
    while (!done)
    {
        static int w, h;
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        //main window
        //  this will be our main workspace
        
        SDL_GetWindowSize(window, &w, &h);
        ImVec2 size = ImVec2(w,h);
        ImGui::SetNextWindowSize(size);
        ImGui::ShowDemoWindow(&show_demo_window);

        // Save/load window
        ShowConfigWindow();
        
        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }
}

void Cleanup( SDL_Window* window, SDL_GLContext* gl_context) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    SDL_GL_DeleteContext(*gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}