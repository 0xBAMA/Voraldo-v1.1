#include "voraldo1_1.h"
// This contains the lower level code


//used in load/save operation to check extension
bool hasEnding(std::string fullString, std::string ending)
{
    if (fullString.length() >= ending.length())
    {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    }
    else
    {
        return false;
    }
}

bool hasPNG(std::string filename)
{
    return hasEnding(filename, std::string(".png"));
}




//got this from http://www.martinbroadhurst.com/list-the-files-in-a-directory-in-c.html
//the intention is to read all the files in the current directory and put them in the array

#define LISTBOX_SIZE 256
std::vector<std::string> directory_strings;

struct path_leaf_string
{
    std::string operator()(const std::filesystem::directory_entry& entry) const
    {
        return entry.path().string();
    }
};

void update_listbox_items()
{
    directory_strings.clear();

    std::filesystem::path p("saves");
    std::filesystem::directory_iterator start(p);
    std::filesystem::directory_iterator end;

    std::transform(start, end, std::back_inserter(directory_strings), path_leaf_string());

    //sort these alphabetically
    std::sort(directory_strings.begin(), directory_strings.end());
}



void Voraldo::create_window()
{

    cout << "   :::     :::   ::::::::   :::::::::       :::      :::         :::::::::    :::::::: " << endl;
    cout << "  :+:     :+:  :+:    :+:  :+:    :+:    :+: :+:    :+:         :+:    :+:  :+:    :+:" << endl;
    cout << " +:+     +:+  +:+    +:+  +:+    +:+   +:+   +:+   +:+         +:+    +:+  +:+    +:+" << endl;
    cout << "+#+     +:+  +#+    +:+  +#++:++#:   +#++:++#++:  +#+         +#+    +:+  +#+    +:+" << endl;
    cout << "+#+   +#+   +#+    +#+  +#+    +#+  +#+     +#+  +#+         +#+    +#+  +#+    +#+" << endl;
    cout << "#+#+#+#    #+#    #+#  #+#    #+#  #+#     #+#  #+#         #+#    #+#  #+#    #+#" << endl;
    cout << " ###       ########   ###    ###  ###     ###  ##########  #########    ########  v1.1" << endl;
    cout << endl;

    if(SDL_Init( SDL_INIT_EVERYTHING ) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
    }

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 8);

    // this is how you query the screen resolution
    SDL_DisplayMode dm;
    SDL_GetDesktopDisplayMode(0, &dm);

    // pulling these out so I can span the whole screen with the window,
    // in a way that's flexible on different resolution screens

#ifdef TRIPLE_MONITOR
    total_screen_width = dm.w*3;
#else
    total_screen_width = dm.w;
#endif

    total_screen_height = dm.h;

    window = SDL_CreateWindow( "OpenGL Window", 0, 0, total_screen_width, total_screen_height, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS );
    // window = SDL_CreateWindow( "OpenGL Window", 0, 0, total_screen_width, total_screen_height, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE );

    // OpenGL 4.3 + GLSL version 430
    const char* glsl_version = "#version 430";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
    GLcontext = SDL_GL_CreateContext( window );

    SDL_GL_MakeCurrent(window, GLcontext);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    // SDL_GL_SetSwapInterval(0); // explicitly disable vsync


    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_TEXTURE_3D);

    glPointSize(3.0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io; // void cast prevents unused variable warning

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, GLcontext);
    ImGui_ImplOpenGL3_Init(glsl_version);

    clear_color = ImVec4(10.0f/255.0f, 10.0f/255.0f, 10.0f/255.0f, 1.0f); // initial value for clear color

    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear( GL_COLOR_BUFFER_BIT );
    SDL_GL_SwapWindow( window );


    update_listbox_items();
    
    #define FPS_HISTORY_SIZE 95
    fps_history.resize(FPS_HISTORY_SIZE);   //initialize the array of fps values

    ImVec4* colors = ImGui::GetStyle().Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.67f, 0.50f, 0.16f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.33f, 0.27f, 0.16f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.05f, 0.00f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.23f, 0.17f, 0.02f, 0.05f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.12f, 0.07f, 0.01f, 0.94f);
    colors[ImGuiCol_Border]                 = ImVec4(0.25f, 0.18f, 0.09f, 0.33f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.33f, 0.15f, 0.02f, 0.17f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.33f, 0.15f, 0.02f, 0.17f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.19f, 0.09f, 0.02f, 0.17f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.25f, 0.12f, 0.01f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.25f, 0.12f, 0.01f, 1.00f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.07f, 0.02f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.13f, 0.10f, 0.08f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.69f, 0.45f, 0.11f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.28f, 0.18f, 0.06f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.36f, 0.22f, 0.06f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
    colors[ImGuiCol_Header]                 = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
    colors[ImGuiCol_Separator]              = ImVec4(0.28f, 0.18f, 0.06f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.42f, 0.18f, 0.06f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.34f, 0.14f, 0.01f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.42f, 0.18f, 0.06f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.06f, 0.03f, 0.01f, 0.78f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.64f, 0.42f, 0.09f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.64f, 0.42f, 0.09f, 0.90f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    ImGuiStyle& style = ImGui::GetStyle();

    style.FrameRounding = 2;
    style.WindowRounding = 3;

    // querying the things that might cause issues with the drivers - so far this has not given me any real idea of what's going on

    // if (glewIsSupported("GL_EXT_shader_image_load_store"))
    //     cout << "GL_EXT_shader_image_load_store is supported" << endl;
    // else
    //     cout << "GL_EXT_shader_image_load_store is not supported" << endl;

    // GLint textureCount;
    // glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &textureCount);
    // cout << endl << "GL_MAX_TEXTURE_IMAGE_UNITS returned:" << textureCount << endl << endl;

    // glGetIntegerv(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, &textureCount);
    // cout << endl << "GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS returned:" << textureCount << endl << endl;

    // glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &textureCount);
    // cout << endl << "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS returned:" << textureCount << endl << endl;

    // glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &textureCount);
    // cout << endl << "GL_MAX_3D_TEXTURE_SIZE returned:" << textureCount << endl << endl;

    GLint data[3];

    glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_COUNT,0, &data[0]);
    glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_COUNT,1, &data[1]);
    glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_COUNT,2, &data[2]);
    cout << endl << "GL_MAX_COMPUTE_WORK_GROUP_COUNT returned x:" << data[0] << " y:" << data[1] << " z:" << data[2] << endl;

    glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_SIZE,0, &data[0]);
    glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_SIZE,1, &data[1]);
    glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_SIZE,2, &data[2]);
    cout << endl << "GL_MAX_COMPUTE_WORK_GROUP_SIZE returned x:" << data[0] << " y:" << data[1] << " z:" << data[2] << endl;

    GLint max;
    glGetIntegerv(  GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max);
    cout << endl << "GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS returned: " << max << endl << endl;

    // glGetIntegerv(  GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &max);
    // cout << endl << "GL_MAX_COMPUTE_SHARED_MEMORY_SIZE returned: " << max << " bytes" << endl << endl;
  
}

void Voraldo::gl_setup()
{
    // some info on your current platform
    const GLubyte *renderer = glGetString( GL_RENDERER ); // get renderer string
    const GLubyte *version = glGetString( GL_VERSION );  // version as a string
    printf( "Renderer: %s\n", renderer );
    printf( "OpenGL version supported %s\n\n\n", version );

    GPU_Data.screen_width = total_screen_width;
    GPU_Data.screen_height = total_screen_height;

    GPU_Data.clear_color = glm::vec4(clear_color.x, clear_color.y, clear_color.z, clear_color.w);

    GPU_Data.init(); // wrapper for all the GPU-side setup

    SDL_ShowWindow(window); // setup completed, show the window and start rendering
}


// these are some utility functions for the imgui stuff

// basically just a tooltip, useful for including a note that won't shit up a significant chunk of the window
void Voraldo::HelpMarker(const char* indicator, const char* desc)
{
    ImGui::TextDisabled("%s", indicator);
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void Voraldo::WrappedText(const char* string, float wrap)
{
    // ImGui::PushTextWrapPos(ImGui::GetFontSize() * wrap);
    ImGui::PushTextWrapPos(wrap);
    ImGui::TextUnformatted(string);
    ImGui::PopTextWrapPos();
}


// small overlay to show the FPS counter, FPS graph
void Voraldo::FPSOverlay(bool* p_open)
{
    if(*p_open)
    {
    const float DISTANCE = 2.0f;
    static int corner = 3;
    ImGuiIO& io = ImGui::GetIO();
    if (corner != -1)
    {
        ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
        ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    }
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("Example: Simple overlay", p_open, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
        // ImGui::Text("Simple overlay\n" "in the corner of the screen.\n" "(right-click to change position)");
        // ImGui::Separator();
        // if (ImGui::IsMousePosValid())
        //     ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
        // else
        //     ImGui::Text("Mouse Position: <invalid>");


        // fps graph
        static float values[FPS_HISTORY_SIZE] = {};
        float average = 0;

        for(int n = 0; n < FPS_HISTORY_SIZE; n++)
        {
            values[n] = fps_history[n];
            average += fps_history[n];
        }

        average /= FPS_HISTORY_SIZE;
        char overlay[32];
        sprintf(overlay, "avg %.2f fps (%.2f ms)", average, 1000.0f/average);
        ImGui::PlotLines("", values, IM_ARRAYSIZE(values), 0, overlay, 0.0f, 100.0f, ImVec2(240,60));


        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Custom",       NULL, corner == -1)) corner = -1;
            if (ImGui::MenuItem("Top-left",     NULL, corner == 0)) corner = 0;
            if (ImGui::MenuItem("Top-right",    NULL, corner == 1)) corner = 1;
            if (ImGui::MenuItem("Bottom-left",  NULL, corner == 2)) corner = 2;
            if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
            if (p_open && ImGui::MenuItem("Close")) *p_open = false;
            ImGui::EndPopup();
        }
    }
    ImGui::End();
    }
}


// populates a menu dropdown, this will change significantly as I figure out what functionality I'm going to include
void Voraldo::ShowExampleMenuFile()
{
    ImGui::MenuItem("(dummy menu)", NULL, false, false);
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
    if (ImGui::BeginMenu("Open Recent"))
    {
        ImGui::MenuItem("fish_hat.c");
        ImGui::MenuItem("fish_hat.inl");
        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("More.."))
        {
            ImGui::MenuItem("Hello");
            ImGui::MenuItem("Sailor");
            if (ImGui::BeginMenu("Recurse.."))
            {
                ShowExampleMenuFile();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
    if (ImGui::MenuItem("Save As..")) {}

    ImGui::Separator();
    if (ImGui::BeginMenu("Options"))
    {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, 60), true);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Colors"))
    {
        float sz = ImGui::GetTextLineHeight();
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x+sz, p.y+sz), ImGui::GetColorU32((ImGuiCol)i));
            ImGui::Dummy(ImVec2(sz, sz));
            ImGui::SameLine();
            ImGui::MenuItem(name);
        }
        ImGui::EndMenu();
    }

    // Here we demonstrate appending again to the "Options" menu (which we already created above)
    // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
    // In a real code-base using it would make senses to use this feature from very different code locations.
    if (ImGui::BeginMenu("Options")) // <-- Append!
    {
        static bool b = true;
        ImGui::Checkbox("SomeOption", &b);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Disabled", false)) // Disabled
    {
        IM_ASSERT(0);
    }
    if (ImGui::MenuItem("Checked", NULL, true)) {}
    if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}


// this is my top bar menu
void Voraldo::AppMainMenuBar(bool *open)
{
    if(*open)
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ShowExampleMenuFile();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                if (ImGui::MenuItem("Paste", "CTRL+V")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Display"))
            {
                if (ImGui::MenuItem("Show FPS Overlay", NULL, show_fpsoverlay)) {show_fpsoverlay = !show_fpsoverlay;}
                if (ImGui::MenuItem("Show Orientation", NULL, GPU_Data.show_widget)) {GPU_Data.show_widget = !GPU_Data.show_widget;}
                if (ImGui::MenuItem("Show Controls", NULL, show_controls)) {show_controls = !show_controls;}
                if (ImGui::MenuItem("Show Demo Window", NULL, show_demo_window)) {show_demo_window = !show_demo_window;}
                if (ImGui::BeginMenu("Show on Display"))
                {
                    if (ImGui::MenuItem("Display 0")) {SDL_SetWindowPosition(window, 0, 0);}
                    if (ImGui::MenuItem("Display 1")) {SDL_SetWindowPosition(window, total_screen_width, 0);}
                    if (ImGui::MenuItem("Display 2")) {SDL_SetWindowPosition(window, 2*total_screen_width, 0);}
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
}


void Voraldo::QuitConfirm(bool *open)
{
    if(*open)
    {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration;
        
        // create centered window
        ImGui::SetNextWindowPos(ImVec2(total_screen_width/2 - 120, total_screen_height/2 - 25));
        ImGui::SetNextWindowSize(ImVec2(240, 50));
        ImGui::Begin("quit", open, flags);

        ImGui::Text("Are you sure you want to quit?");

        ImGui::Text("  ");
        ImGui::SameLine();
        
        // button to cancel -> set this window's bool to false
        if(ImGui::Button(" Cancel "))
            *open = false;

        ImGui::SameLine();
        ImGui::Text("      ");
        ImGui::SameLine();

        // button to quit -> set pquit to true
        if(ImGui::Button(" Quit "))
            pquit = true;
        
        ImGui::End();
    }
}


void Voraldo::ControlWindow(bool *open)
{
    if(*open)
    {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar;
        // ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration;

        ImGui::SetNextWindowPos(ImVec2(0,show_menu?19:0));
        ImGui::SetNextWindowSize(ImVec2(320,385), ImGuiCond_FirstUseEver);
        ImGui::Begin("Controls", open, flags);

        ImVec2 windowsize = ImGui::GetWindowSize();
        (void)windowsize; // this is just to remove the compiler warning 
        
        // tabbed layout
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None | ImGuiTabBarFlags_FittingPolicyScroll;

        if (ImGui::BeginTabBar("TopTabBar", tab_bar_flags))
        {
            if (ImGui::BeginTabItem(" Shapes "))
            {
                ImGui::BeginTabBar("s", tab_bar_flags);

                if(ImGui::BeginTabItem(" AABB "))
                {
                    static glm::vec3 max, min;
                    static ImVec4 aabb_draw_color;
                    static bool aabb_draw = true, aabb_mask = false;

                    ImGui::SliderFloat(" x max", &max.x, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat(" x min", &min.x, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::SliderFloat(" y max", &max.y, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat(" y min", &min.y, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::SliderFloat(" z max", &max.z, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat(" z min", &min.z, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::Checkbox("  Draw ", &aabb_draw);
                    ImGui::SameLine();
                    ImGui::Checkbox("  Mask ", &aabb_mask);

                    ImGui::ColorEdit4("  Color", (float*)&aabb_draw_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

                    ImGui::Text(" ");
                    ImGui::SetCursorPosX(16);

                    if (ImGui::Button("Draw", ImVec2(100, 22)))
                    {
                        //draw the sphere with the selected values
                        GPU_Data.draw_aabb(min, max, glm::vec4(aabb_draw_color.x, aabb_draw_color.y, aabb_draw_color.z, aabb_draw_color.w), aabb_draw, aabb_mask);
                    }

                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem(" Cuboid "))
                {
                    //kind of start with some defaults because this is not super easy to use
                    static glm::vec3 a = glm::vec3( 20,230,230);   // a : -x, +y, +z
                    static glm::vec3 b = glm::vec3( 20, 20,230);   // b : -x, -y, +z
                    static glm::vec3 c = glm::vec3(230,230,230);   // c : +x, +y, +z
                    static glm::vec3 d = glm::vec3(230, 20,230);   // d : +x, -y, +z
                    static glm::vec3 e = glm::vec3( 20,230, 20);   // e : -x, +y, -z
                    static glm::vec3 f = glm::vec3( 20, 20, 20);   // f : -x, -y, -z
                    static glm::vec3 g = glm::vec3(230,230, 20);   // g : +x, +y, -z
                    static glm::vec3 h = glm::vec3(230, 20, 20);   // h : +x, -y, -z

                    static bool cuboid_draw = true, cuboid_mask = false;
                    static ImVec4 cuboid_draw_color;

                    ImGui::Text("This is hard to use.");
                    ImGui::Text("     e-------g    +y     ");
                    ImGui::Text("    /|      /|     |     ");
                    ImGui::Text("   / |     / |     |___+x");
                    ImGui::Text("  a-------c  |    /      ");
                    ImGui::Text("  |  f----|--h   +z      ");
                    ImGui::Text("  | /     | /            ");
                    ImGui::Text("  |/      |/             ");
                    ImGui::Text("  b-------d              ");

                    ImGui::Separator();

                    ImGui::SliderFloat("a x", &a.x, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("a y", &a.y, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("a z", &a.z, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::SliderFloat("b x", &b.x, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("b y", &b.y, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("b z", &b.z, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::SliderFloat("c x", &c.x, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("c y", &c.y, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("c z", &c.z, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::SliderFloat("d x", &d.x, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("d y", &d.y, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("d z", &d.z, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::SliderFloat("e x", &e.x, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("e y", &e.y, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("e z", &e.z, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::SliderFloat("f x", &f.x, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("f y", &f.y, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("f z", &f.z, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::SliderFloat("g x", &g.x, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("g y", &g.y, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("g z", &g.z, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::SliderFloat("h x", &h.x, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("h y", &h.y, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("h z", &h.z, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::Checkbox("  Draw ", &cuboid_draw);
                    ImGui::SameLine();
                    ImGui::Checkbox("  Mask ", &cuboid_mask);

                    ImGui::ColorEdit4("  Color", (float*)&cuboid_draw_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

                    ImGui::Text(" ");
                    ImGui::SetCursorPosX(16);

                    if (ImGui::Button("Draw", ImVec2(100, 22)))
                    {
                        //draw the cylinder with the selected values
                        GPU_Data.draw_cuboid(a, b, c, d, e, f, g, h, glm::vec4(cuboid_draw_color.x, cuboid_draw_color.y, cuboid_draw_color.z, cuboid_draw_color.w), cuboid_draw, cuboid_mask);
                    }

                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem(" Cylinder "))
                {
                    static glm::vec3 cylinder_bvec, cylinder_tvec;
                    static bool cylinder_draw = true, cylinder_mask = false;
                    static ImVec4 cylinder_draw_color;
                    static float cylinder_radius;

                    WrappedText("Cylinder is defined by two points. tvec is in the center of the top and bvec is in the center of the bottom. Thickness determines the radius of the cylinder. ", windowsize.x);
                    ImGui::Text(" ");

                    ImGui::SliderFloat(" radius", &cylinder_radius, 0.0f, 300.0f, "%.3f");

                    ImGui::Separator();

                    ImGui::SliderFloat("bvec x", &cylinder_bvec.x, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("bvec y", &cylinder_bvec.y, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("bvec z", &cylinder_bvec.z, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::SliderFloat("tvec x", &cylinder_tvec.x, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("tvec y", &cylinder_tvec.y, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("tvec z", &cylinder_tvec.z, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::Checkbox("  Draw ", &cylinder_draw);
                    ImGui::SameLine();
                    ImGui::Checkbox("  Mask ", &cylinder_mask);

                    ImGui::ColorEdit4("  Color", (float*)&cylinder_draw_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

                    ImGui::Text(" ");
                    ImGui::SetCursorPosX(16);

                    if (ImGui::Button("Draw", ImVec2(100, 22)))
                    {
                        //draw the cylinder with the selected values
                        GPU_Data.draw_cylinder(cylinder_bvec, cylinder_tvec, cylinder_radius, glm::vec4(cylinder_draw_color.x, cylinder_draw_color.y, cylinder_draw_color.z, cylinder_draw_color.w), cylinder_draw, cylinder_mask);
                    }

                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem(" Ellipsoid "))
                {
                    static glm::vec3 radius, center, rotation;
                    static bool ellipsoid_draw = true, ellipsoid_mask = false;
                    static ImVec4 ellipsoid_draw_color;

                    WrappedText("Ellipsoid is similar to the sphere but has three different radii. In addition to this it can be rotated to give the desired orientation. ", windowsize.x);
                    ImGui::Text(" ");

                    ImGui::SliderFloat("x location", &center.x, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("y location", &center.y, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("z location", &center.z, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::SliderFloat("x radius", &radius.x, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("y radius", &radius.y, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("z radius", &radius.z, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::SliderFloat("x rotation", &rotation.x, 0.0f, 6.28f, "%.3f");
                    ImGui::SliderFloat("y rotation", &rotation.y, 0.0f, 6.28f, "%.3f");
                    ImGui::SliderFloat("z rotation", &rotation.z, 0.0f, 6.28f, "%.3f");

                    ImGui::Separator();

                    ImGui::Checkbox("  Draw ", &ellipsoid_draw);
                    ImGui::SameLine();
                    ImGui::Checkbox("  Mask ", &ellipsoid_mask);

                    ImGui::ColorEdit4("  Color", (float*)&ellipsoid_draw_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

                    ImGui::Text(" ");
                    ImGui::SetCursorPosX(16);

                    if (ImGui::Button("Draw", ImVec2(100, 22)))
                    {
                        //draw the ellipsoid with the selected values
                        GPU_Data.draw_ellipsoid(center, radius, rotation, glm::vec4(ellipsoid_draw_color.x, ellipsoid_draw_color.y, ellipsoid_draw_color.z, ellipsoid_draw_color.w), ellipsoid_draw, ellipsoid_mask);
                    }

                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem(" Grid "))
                {
                    static int xoff, yoff, zoff;
                    static int xspacing, yspacing, zspacing;
                    static int xwid, ywid, zwid;
                    static ImVec4 grid_draw_color;
                    static bool grid_draw = true;
                    static bool grid_mask = false;

                    WrappedText("Use the spacing control to set the distance between grid lines. Width sets the width of these grid lines, and offset allows the whole grid to be moved. ", windowsize.x);
                    ImGui::Text(" ");
                    ImGui::Text("spacing");
                    ImGui::SliderInt(" xs", &xspacing, 0, 15);
                    ImGui::SliderInt(" ys", &yspacing, 0, 15);
                    ImGui::SliderInt(" zs", &zspacing, 0, 15);

                    ImGui::Text("width");
                    ImGui::SliderInt(" xw", &xwid, 0, 15);
                    ImGui::SliderInt(" yw", &ywid, 0, 15);
                    ImGui::SliderInt(" zw", &zwid, 0, 15);

                    ImGui::Text("offset");
                    ImGui::SliderInt(" xo", &xoff, 0, 15);
                    ImGui::SliderInt(" yo", &yoff, 0, 15);
                    ImGui::SliderInt(" zo", &zoff, 0, 15);

                    ImGui::Checkbox("  Draw ", &grid_draw);
                    ImGui::SameLine();
                    ImGui::Checkbox("  Mask ", &grid_mask);

                    ImGui::ColorEdit4("  Color", (float*)&grid_draw_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

                    ImGui::Text(" ");

                    ImGui::SetCursorPosX(16);
                    if (ImGui::Button("Draw", ImVec2(100, 22)))
                    {
                        //draw with the selected values
                        GPU_Data.draw_grid(glm::ivec3(xspacing, yspacing, zspacing), glm::ivec3(xwid, ywid, zwid), glm::ivec3(xoff, yoff, zoff), glm::vec4(grid_draw_color.x, grid_draw_color.y, grid_draw_color.z, grid_draw_color.w), grid_draw, grid_mask);
                    }

                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem(" Heightmap "))
                {
                    static float heightmap_vertical_scale = 1.0;
                    static bool heightmap_draw = true, heightmap_mask = false;
                    static ImVec4 heightmap_draw_color;

                    // ImGui::Text("This is a tool to draw heightmaps.");
                    // ImGui::Text("Use the options to generate new");
                    // ImGui::Text("ones, and use the vertical scale");
                    // ImGui::Text("to set the height scaling. ");
                    // ImGui::Text(" ");
                    // ImGui::Text("There are three types of maps you");
                    // ImGui::Text("can generate. ");
                    // ImGui::Text(" ");

                    //show off the currently held texture - for some reason it is the number of the texture unit + 1
                    ImGui::Image((void*)(intptr_t) 13   /*not a good way to do this, but it works right now*/ , ImVec2(240,256));

                    if (ImGui::Button("perlin"))
                    {
                        GPU_Data.generate_heightmap_perlin();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("diamond-square"))
                    {
                        GPU_Data.generate_heightmap_diamond_square();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("XOR"))
                    {
                        GPU_Data.generate_heightmap_XOR();
                    }


                    ImGui::Separator();
                    ImGui::SliderFloat(" Scale", &heightmap_vertical_scale, 0.0f, 5.0f, "%.3f");
                    ImGui::Separator();

                    ImGui::Checkbox("  Draw ", &heightmap_draw);
                    ImGui::SameLine();
                    ImGui::Checkbox("  Mask ", &heightmap_mask);

                    ImGui::ColorEdit4("  Color", (float*)&heightmap_draw_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

                    ImGui::Text(" ");
                    ImGui::SetCursorPosX(16);

                    if (ImGui::Button("Draw", ImVec2(100, 22)))
                    {
                        //draw the heightmap with the selected values
                        GPU_Data.draw_heightmap(heightmap_vertical_scale, true, glm::vec4(heightmap_draw_color.x, heightmap_draw_color.y, heightmap_draw_color.z, heightmap_draw_color.w), heightmap_mask, heightmap_draw);
                    }
                   
                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem(" Icosahedron "))
                {
                    static ImVec4 vertex_color;
                    static float vertex_radius = 0.;
                    
                    static ImVec4 edge_color;
                    static float edge_radius = 0.;
                    
                    static ImVec4 face_color;
                    static float face_thickness = 0.;

                    static glm::vec3 center_point = glm::vec3(0);
                    static glm::vec3 rotations = glm::vec3(0);
                    static bool draw = true;
                    static bool mask = false;
                    static float scale = 20.0;

                    ImGui::Text(" ");
                    ImGui::SliderFloat("  scale", &scale, 0.0f, DIM, "%.3f");

                    ImGui::Text(" ");
                    ImGui::SliderFloat("  xpos", &center_point.x, 0.0f, DIM, "%.3f");
                    ImGui::SliderFloat("  ypos", &center_point.y, 0.0f, DIM, "%.3f");
                    ImGui::SliderFloat("  zpos", &center_point.z, 0.0f, DIM, "%.3f");
                    ImGui::Text(" ");
                    ImGui::Text(" ");
                    ImGui::SliderFloat("  xrot", &rotations.x, -2.*pi, 2*pi, "%.3f");
                    ImGui::SliderFloat("  yrot", &rotations.y, -2.*pi, 2*pi, "%.3f");
                    ImGui::SliderFloat("  zrot", &rotations.z, -2.*pi, 2*pi, "%.3f");
                    ImGui::Text(" ");
                    ImGui::Text(" ");
                    ImGui::SliderFloat("  vertex radius", &vertex_radius, 0., 20., "%.3f");
                    ImGui::SliderFloat("  edge radius", &edge_radius, 0., 20., "%.3f");
                    ImGui::SliderFloat("  face thickness", &face_thickness, 0., 20., "%.3f");
                    ImGui::Text(" ");
                    
                    ImGui::ColorEdit4("  Vertex", (float*)&vertex_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
                    ImGui::Text(" ");
                    ImGui::ColorEdit4("  Edge", (float*)&edge_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
                    ImGui::Text(" ");
                    ImGui::ColorEdit4("  Face", (float*)&face_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

                    ImGui::Checkbox("  Draw ", &draw);
                    ImGui::SameLine();
                    ImGui::Checkbox("  Mask ", &mask);
                    ImGui::Text(" ");
                    ImGui::SetCursorPosX(16);
                    
                    if(ImGui::Button("Draw", ImVec2(100, 22)))
                    {
                        GPU_Data.draw_regular_icosahedron(rotations.x, rotations.y, rotations.z, scale, center_point, glm::vec4(vertex_color.x, vertex_color.y, vertex_color.z, vertex_color.w), vertex_radius, glm::vec4(edge_color.x, edge_color.y, edge_color.z, edge_color.w), edge_radius, glm::vec4(face_color.x, face_color.y, face_color.z, face_color.w), face_thickness, draw, mask);
                    }
                    
                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem(" Perlin "))
                {
                    static float perlin_scale_x = 0.014;
                    static float perlin_scale_y = 0.014;
                    static float perlin_scale_z = 0.014;
                    static float perlin_threshold_lo = 0.0f;
                    static float perlin_threshold_hi = 0.0f;
                    static ImVec4 perlin_draw_color;
                    static bool perlin_draw = true;
                    static bool perlin_mask = false;
                    static bool perlin_smooth = false;

                    WrappedText("Larger numbers are smaller lobes. Click generate to send your new scalings to the GPU to draw with.", windowsize.x);
                    ImGui::Text(" ");
                    ImGui::SliderFloat("  xscale", &perlin_scale_x, 0.01f, 0.5f, "%.3f");
                    ImGui::SliderFloat("  yscale", &perlin_scale_y, 0.01f, 0.5f, "%.3f");
                    ImGui::SliderFloat("  zscale", &perlin_scale_z, 0.01f, 0.5f, "%.3f");
                    ImGui::Text(" ");

                    if(ImGui::Button("generate"))
                    {
                        GPU_Data.generate_perlin_noise(perlin_scale_x, perlin_scale_y, perlin_scale_z);
                    }

                    ImGui::Separator();

                    WrappedText("Perlin noise ranges from 0 to 1. Use hithresh and lowthresh to tell how much of this perlin texture to color in.", windowsize.x);


                    ImGui::SliderFloat(" hithresh", &perlin_threshold_hi, 0.0f, 1.0f, "%.3f");
                    ImGui::SliderFloat(" lothresh", &perlin_threshold_lo, 0.0f, 1.0f, "%.3f");

                    ImGui::Checkbox(" Smooth Color ", &perlin_smooth);

                    ImGui::Separator();


                    ImGui::Checkbox("  Draw ", &perlin_draw);
                    ImGui::SameLine();
                    ImGui::Checkbox("  Mask ", &perlin_mask);

                    ImGui::ColorEdit4("  Color", (float*)&perlin_draw_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

                    ImGui::Text(" ");

                    ImGui::SetCursorPosX(16);
                    if (ImGui::Button("Draw", ImVec2(100, 22)))
                    {
                        //draw with the selected values
                        GPU_Data.draw_perlin_noise(perlin_threshold_lo, perlin_threshold_hi, perlin_smooth, glm::vec4(perlin_draw_color.x, perlin_draw_color.y, perlin_draw_color.z, perlin_draw_color.w), perlin_draw, perlin_mask);
                    }
                   
                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem(" Sphere "))
                {
                    static bool sphere_draw = true, sphere_mask = false;
                    static float sphere_radius = 0.0;
                    static ImVec4 sphere_draw_color;
                    static glm::vec3 sphere_location;

                    WrappedText("Use the sliders to set the radius and the x, y, z components of the center's position.", windowsize.x);
                    ImGui::Text(" ");

                    ImGui::SliderFloat("  radius", &sphere_radius, 0.0f, 500.0f, "%.3f");

                    ImGui::Separator();

                    ImGui::SliderFloat("  x pos", &sphere_location.x, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("  y pos", &sphere_location.y, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("  z pos", &sphere_location.z, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::Checkbox("  Draw ", &sphere_draw);
                    ImGui::SameLine();
                    ImGui::Checkbox("  Mask ", &sphere_mask);

                    ImGui::ColorEdit4("  Color", (float*)&sphere_draw_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

                    // ImGui::SetCursorPosY(200);
                    ImGui::Text(" ");
                    ImGui::SetCursorPosX(16);

                    if (ImGui::Button("Draw", ImVec2(100, 22)))
                    {
                        //draw the sphere with the selected values
                        GPU_Data.draw_sphere(sphere_location, sphere_radius, glm::vec4(sphere_draw_color.x, sphere_draw_color.y, sphere_draw_color.z, sphere_draw_color.w), sphere_draw, sphere_mask);
                    }

                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem(" Tube "))
                {
                    static glm::vec3 tube_bvec, tube_tvec;
                    static bool tube_draw = true, tube_mask = false;
                    static ImVec4 tube_draw_color;
                    static float tube_inner_radius, tube_outer_radius;

                    WrappedText("Tube is a cylinder with a cylinder cut out from the center. Outer is the outer radius, and inner is the radius of the cutout.", windowsize.x);
                    ImGui::Text(" ");

                    ImGui::Text("Radii");

                    ImGui::SliderFloat("inner", &tube_inner_radius, 0.0f, 300.0f, "%.3f");
                    ImGui::SliderFloat("outer", &tube_outer_radius, 0.0f, 300.0f, "%.3f");

                    ImGui::Separator();

                    ImGui::SliderFloat("bvec x", &tube_bvec.x, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("bvec y", &tube_bvec.y, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("bvec z", &tube_bvec.z, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::SliderFloat("tvec x", &tube_tvec.x, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("tvec y", &tube_tvec.y, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("tvec z", &tube_tvec.z, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::Checkbox("  Draw ", &tube_draw);
                    ImGui::SameLine();
                    ImGui::Checkbox("  Mask ", &tube_mask);

                    ImGui::ColorEdit4("  Color", (float*)&tube_draw_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

                    ImGui::Text(" ");
                    ImGui::SetCursorPosX(16);

                    if (ImGui::Button("Draw", ImVec2(100, 22)))
                    {
                        //draw the cylinder with the selected values
                        GPU_Data.draw_tube(tube_bvec, tube_tvec, tube_inner_radius, tube_outer_radius, glm::vec4(tube_draw_color.x, tube_draw_color.y, tube_draw_color.z, tube_draw_color.w), tube_draw, tube_mask);
                    }

                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem(" Triangle "))
                {
                    static float thickness;
                    static glm::vec3 point1, point2, point3;
                    static ImVec4 triangle_draw_color;
                    static bool triangle_draw = true;
                    static bool triangle_mask = false;

                    WrappedText("Triangles consist of three points, use the sliders below to set each x, y and z value. Thickness will set the thickness of the triangle.", windowsize.x);
                    ImGui::Text(" ");

                    ImGui::SliderFloat(" thickness", &thickness, 0.0f, 300.0f, "%.3f");

                    ImGui::Separator();

                    ImGui::SliderFloat("  x1 ", &point1.x, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("  y1 ", &point1.y, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("  z1 ", &point1.z, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::SliderFloat("  x2 ", &point2.x, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("  y2 ", &point2.y, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("  z2 ", &point2.z, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::SliderFloat("  x3 ", &point3.x, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("  y3 ", &point3.y, 0.0f, float(DIM), "%.3f");
                    ImGui::SliderFloat("  z3 ", &point3.z, 0.0f, float(DIM), "%.3f");

                    ImGui::Separator();

                    ImGui::Checkbox("  Draw ", &triangle_draw);
                    ImGui::SameLine();
                    ImGui::Checkbox("  Mask ", &triangle_mask);

                    ImGui::ColorEdit4("  Color", (float*)&triangle_draw_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

                    ImGui::Text(" ");
                    ImGui::SetCursorPosX(16);

                    if (ImGui::Button("Draw", ImVec2(100, 22)))
                    {
                        //draw the triangle with the selected values
                        GPU_Data.draw_triangle(point1, point2, point3, thickness, glm::vec4(triangle_draw_color.x, triangle_draw_color.y, triangle_draw_color.z, triangle_draw_color.w), triangle_draw, triangle_mask);
                    }

                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem(" VAT "))
                {
                    // https://softologyblog.wordpress.com/2017/05/27/voxel-automata-terrain/
                    // https://bitbucket.org/BWerness/voxel-automata-terrain/src/master/

                    static ImVec4 color0 = ImVec4(165.0/255.0, 118.0/255.0,  64.0/255.0,  10.0/255.0); // neutral volume color
                    static ImVec4 color1 = ImVec4(210.0/255.0, 180.0/255.0, 140.0/255.0, 105.0/255.0); // Wikipedia Tan
                    static ImVec4 color2 = ImVec4(143.0/255.0, 151.0/255.0, 121.0/255.0,  95.0/255.0); // Wikipedia Artichoke Green

                    static float lambda = 0.35;
                    static float beta = 0.5;
                    static float mag = 0.0;

                    static bool respect_mask = false;

                    static int initmode;
                    static float flip;

                    static char str0[256] = "";
                    
                    static bool plusx, plusy, plusz;
                    static bool minusx, minusy, minusz;
                    
                    //config options for this operation
                    WrappedText("This is an interesting way to generate shapes, developed by Brent Werness - enter a rule, r for Random or i for IsingRandom", windowsize.x); // may want to parameterize this further, with beta, lambda, mag - just want to get it working first
                    ImGui::Text(" ");

                    // string entry, letting the user input a rule
                    ImGui::Text("Enter base62 encoded rule, r or i");
                    ImGui::InputTextWithHint("", "", str0, IM_ARRAYSIZE(str0));
                    
                    if (ImGui::Button("Compute", ImVec2(100, 22)))
                    {
                        // invoke the constructor, etc - return a string from the OpenGL_container::vat(...), and put it in str0
                        glm::vec4 col0, col1, col2;
                        col0 = glm::vec4(color0.x, color0.y, color0.z, color0.w);
                        col1 = glm::vec4(color1.x, color1.y, color1.z, color1.w);
                        col2 = glm::vec4(color2.x, color2.y, color2.z, color2.w);

                        std::string temp = GPU_Data.vat(flip, std::string(str0), initmode, col0, col1, col2, lambda, beta, mag, respect_mask, glm::bvec3(minusx, minusy, minusz), glm::bvec3(plusx, plusy, plusz));  //assign with the function call

                        strcpy(str0, temp.c_str()); // you get to see how the random rule you generated, or retain the rule you entered
                    }
                    
                    // flip slider (float)
                    ImGui::Text("Make nonzero for stochastic result");
                    ImGui::SliderFloat(" flip", &flip, 0.0f, 1.0f, "%.3f");
                    ImGui::Text(" ");

                    ImGui::Text("Lambda is a parameter for Random");
                    ImGui::SliderFloat(" lambda", &lambda, 0.0f, 1.0f, "%.3f");
                    ImGui::Text(" ");
                    
                    if (ImGui::Button("Compute Random", ImVec2(100, 22)))
                    {
                        // invoke the constructor, etc - return a string from the OpenGL_container::vat(...), and put it in str0
                        glm::vec4 col0, col1, col2;
                        col0 = glm::vec4(color0.x, color0.y, color0.z, color0.w);
                        col1 = glm::vec4(color1.x, color1.y, color1.z, color1.w);
                        col2 = glm::vec4(color2.x, color2.y, color2.z, color2.w);

                        std::string temp = GPU_Data.vat(flip, std::string("r"), initmode, col0, col1, col2, lambda, beta, mag, respect_mask, glm::bvec3(minusx, minusy, minusz), glm::bvec3(plusx, plusy, plusz));  //assign with the function call

                        strcpy(str0, temp.c_str()); // you get to see how the random rule you generated, or retain the rule you entered
                    }

                    
                    ImGui::Text(" ");
                    ImGui::Text("Beta and Mag are parameters");
                    ImGui::Text("for IRandom");
                    ImGui::SliderFloat(" beta", &beta, 0.0f, 1.0f, "%.3f");
                    ImGui::SliderFloat(" mag", &mag, 0.0f, 1.0f, "%.3f");
                    ImGui::Text(" ");
                    
                    if (ImGui::Button("Compute IRandom", ImVec2(100, 22)))
                    {
                        // invoke the constructor, etc - return a string from the OpenGL_container::vat(...), and put it in str0
                        glm::vec4 col0, col1, col2;
                        col0 = glm::vec4(color0.x, color0.y, color0.z, color0.w);
                        col1 = glm::vec4(color1.x, color1.y, color1.z, color1.w);
                        col2 = glm::vec4(color2.x, color2.y, color2.z, color2.w);

                        std::string temp = GPU_Data.vat(flip, std::string("i"), initmode, col0, col1, col2, lambda, beta, mag, respect_mask, glm::bvec3(minusx, minusy, minusz), glm::bvec3(plusx, plusy, plusz));  //assign with the function call

                        strcpy(str0, temp.c_str()); // you get to see how the random rule you generated, or retain the rule you entered
                    }

                    ImGui::Text(" ");
                    // mode slider (int)
                    ImGui::Text("0 - fill side with 0");
                    ImGui::Text("1 - fill side with 1");
                    ImGui::Text("2 - fill side with 2");
                    ImGui::Text("3 - fill side with random values");
                    ImGui::SliderInt(" mode", &initmode, 0, 3);
                    ImGui::Text(" ");

                    ImGui::Checkbox(" fill -x", &minusx);
						  ImGui::Checkbox(" fill +x", &plusx);

                    ImGui::Checkbox(" fill -y", &minusy);
						  ImGui::Checkbox(" fill +y", &plusy);

                    ImGui::Checkbox(" fill -z", &minusz);
						  ImGui::Checkbox(" fill +z", &plusz);
						  
						  
                    // three colors
                    ImGui::ColorEdit4(" State 0", (float*)&color0, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
                    ImGui::ColorEdit4(" State 1", (float*)&color1, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
                    ImGui::ColorEdit4(" State 2", (float*)&color2, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

                    ImGui::Text(" ");
                    ImGui::Checkbox(" respect mask ", &respect_mask);

                    ImGui::Text(" ");
                    ImGui::SetCursorPosX(16);



                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(" Utilities "))
            {
                ImGui::BeginTabBar("u", tab_bar_flags);

                if(ImGui::BeginTabItem(" Clear "))
                {
                    static bool respect_mask = false;

                    WrappedText("This will clear the block, with an option to respect the mask. If you check that, masked cells won't be cleared.", windowsize.x);
                    ImGui::Text(" ");

                    ImGui::Checkbox("  Respect mask ", &respect_mask);

                    ImGui::Text(" ");
                    ImGui::SetCursorPosX(16);

                    if (ImGui::Button("Clear", ImVec2(100, 22)))
                    {
                        //do the clear all operation - note that this respects the mask values
                        GPU_Data.clear_all(respect_mask);
                    }

                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem(" Masking "))
                {
                    WrappedText("This will clear the mask value for all cells. Equivalently, set mask to false for all voxels. ", windowsize.x);
                    ImGui::Text(" ");

                    ImGui::SetCursorPosX(16);

                    if (ImGui::Button("Unmask All", ImVec2(100, 22)))
                    {
                        //unmask all cells
                        GPU_Data.unmask_all();
                    }

                    ImGui::Text(" ");
                    ImGui::Text(" ");

                    WrappedText("This will toggle the value of mask for all voxels. Masked cells will become unmasked, and unmasked will become masked.", windowsize.x);
                    ImGui::Text(" ");

                    ImGui::SetCursorPosX(16);

                    if (ImGui::Button("Invert", ImVec2(100, 22)))
                    {
                        //do the toggle operation
                        GPU_Data.invert_mask();
                    }

                    ImGui::Text(" ");
                    ImGui::Text(" ");

                    static bool use_r;
                    static bool use_g;
                    static bool use_b;
                    static bool use_a;
                    static bool use_l;

                    static ImVec4 select_color;
                    static float light_val=0.0001;

                    static float r_variance=0.0;
                    static float g_variance=0.0;
                    static float b_variance=0.0;
                    static float a_variance=0.0;
                    static float l_variance=0.0;


                    WrappedText("Use the HSV picker or the RGB fields to enter a color. Once you do that, use the check boxes and sliders to express how you want to use each channel. ", windowsize.x);
                    ImGui::Text(" ");
                    WrappedText("For example, if I pick 255 in the red channel, check the red check box, and set the slider to a non zero value, you will be masking the parts of the image that have a high value in the red channel. ", windowsize.x);
                    ImGui::Text(" ");
                    WrappedText("The slider sets how broadly this operation will be applied. ", windowsize.x);
                    ImGui::Text(" ");
                    WrappedText("This can be applied to the RGBA color channels as well as the value in the lighting buffer, to mask only light or dark areas. ", windowsize.x);
                    ImGui::Text(" ");


                    ImGui::ColorEdit4("  Color", (float*)&select_color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

                    //sliders

                    ImGui::Checkbox("use r", &use_r);
                    ImGui::SameLine();
                    ImGui::SliderFloat("r variance", &r_variance, 0.0f, 1.0f, "%.3f");

                    ImGui::Checkbox("use g", &use_g);
                    ImGui::SameLine();
                    ImGui::SliderFloat("g variance", &g_variance, 0.0f, 1.0f, "%.3f");

                    ImGui::Checkbox("use b", &use_b);
                    ImGui::SameLine();
                    ImGui::SliderFloat("b variance", &b_variance, 0.0f, 1.0f, "%.3f");

                    ImGui::Checkbox("use a", &use_a);
                    ImGui::SameLine();
                    ImGui::SliderFloat("a variance", &a_variance, 0.0f, 1.0f, "%.3f");

                    ImGui::Separator();

                    ImGui::Checkbox("use l", &use_l);
                    ImGui::SameLine();
                    ImGui::SliderFloat("l value", &light_val, 0.0f, 1.0f, "%.3f");
                    ImGui::SliderFloat("l variance", &l_variance, 0.0f, 1.0f, "%.3f");

                    if (ImGui::Button("Mask", ImVec2(100, 22)))
                    {
                        GPU_Data.mask_by_color(use_r, use_g, use_b, use_a, use_l, glm::vec4(select_color.x, select_color.y, select_color.z, select_color.w), light_val, r_variance, g_variance, b_variance, a_variance, l_variance);
                    }

                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem(" Box Blur "))
                {
                    static int blur_radius = 0;
                    static bool touch_alpha = true;
                    static bool respect_mask = false;

                    WrappedText("This is a simple box blur. It will consider the size neighborhood you select, and average the colors to give smoother transitions beteen neighboring cells.", windowsize.x);
                    ImGui::Text(" ");

                    ImGui::SliderInt(" Radius", &blur_radius, 0, 5);

                    ImGui::Separator();

                    ImGui::Checkbox("  Touch alpha ", &touch_alpha);
                    ImGui::Checkbox("  Respect mask ", &respect_mask);

                    ImGui::Text(" ");
                    ImGui::SetCursorPosX(16);

                    if (ImGui::Button("Blur", ImVec2(100, 22)))
                    {
                        //do the blur operation with the selected values
                        GPU_Data.box_blur(blur_radius, touch_alpha, respect_mask);
                    }
                    ImGui::EndTabItem();
                }


                if(ImGui::BeginTabItem(" Gaussian Blur "))
                {
                    static int blur_radius = 0;
                    static bool touch_alpha = true;
                    static bool respect_mask = false;

                    WrappedText("This is a gaussian blur. It will consider the size neighborhood you select, and average the colors to give smoother transitions beteen neighboring cells.", windowsize.x);
                    ImGui::Text(" ");

                    ImGui::SliderInt(" Radius", &blur_radius, 0, 5);

                    ImGui::Separator();

                    ImGui::Checkbox("  Touch alpha ", &touch_alpha);
                    ImGui::Checkbox("  Respect mask ", &respect_mask);

                    ImGui::Text(" ");
                    ImGui::SetCursorPosX(16);

                    if (ImGui::Button("Blur", ImVec2(100, 22)))
                    {
                        //do the blur operation with the selected values
                        GPU_Data.gaussian_blur(blur_radius, touch_alpha, respect_mask);
                    }

                    ImGui::EndTabItem();
                }


                if(ImGui::BeginTabItem(" Limiter "))
                {
                    if (ImGui::Button("Limit", ImVec2(100, 22)))
                    {
                        GPU_Data.limiter();
                    }
                   
                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem(" Shift "))
                {
                    static int xmove;
                    static int ymove;
                    static int zmove;
                    static bool loop;
                    static int shift_mode = 1;

                    WrappedText("This allows you to shift the voxel data, by some amount along the x, y and z axes. Modes are described below. ", windowsize.x);
                    ImGui::Text(" ");
                    WrappedText("If you turn on looping, data that goes off one side will appear on the opposite edge, torus-style.", windowsize.x);


                    ImGui::Text(" ");
                    ImGui::SetCursorPosX(16);
                    ImGui::SliderInt(" x", &xmove, -DIM, DIM);
                    ImGui::SetCursorPosX(16);
                    ImGui::SliderInt(" y", &ymove, -DIM, DIM);
                    ImGui::SetCursorPosX(16);
                    ImGui::SliderInt(" z", &zmove, -DIM, DIM);
                    ImGui::Text(" ");

                    ImGui::SetCursorPosX(16);
                    ImGui::SliderInt(" Mode", &shift_mode, 1, 3);
                    ImGui::Text(" ");

                    ImGui::SetCursorPosX(16);

                    switch(shift_mode)
                    {
                        case 1:
                            ImGui::Text("Mode 1: Ignore mask buffer, \nmove color data only");
                            break;
                        case 2:
                            ImGui::Text("Mode 2: Respect mask buffer, \ncells retain color if masked");
                            break;
                        case 3:
                            ImGui::Text("Mode 3: Carry mask buffer, \nmask and color move together");
                            break;
                        default:
                            ImGui::Text("Pick a valid mode");
                            break;
                    }

                    ImGui::Text(" ");
                    ImGui::SetCursorPosX(16);
                    ImGui::Checkbox(" loop", &loop);

                    ImGui::Text(" ");
                    ImGui::SetCursorPosX(16);

                    if (ImGui::Button("Shift", ImVec2(90,22)))
                        GPU_Data.shift(glm::ivec3(xmove,ymove,zmove), loop, shift_mode);

                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem(" Copy/Paste "))
                {

                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem(" Load/Save "))
                {
                    static char str0[256] = "";

                    const char* listbox_items[LISTBOX_SIZE];

                    //count up the number of non-null c-strings
                    unsigned int i;
                    for(i = 0; i < LISTBOX_SIZE && i < directory_strings.size(); ++i)
                    {
                        listbox_items[i] = directory_strings[i].c_str();
                    }

                    static bool respect_mask_on_load = false;

                    WrappedText("This function lets you load or save of blocks using the PNG image format. ", windowsize.x);
                    ImGui::Text(" ");


                    ImGui::Text("Files in saves folder:");
                    static int listbox_select_index = 1;
                    ImGui::ListBox(" ", &listbox_select_index, listbox_items, i, 10);



                    ImGui::Text("Enter filename to save:");
                    ImGui::InputTextWithHint(".png", "", str0, IM_ARRAYSIZE(str0));
                    ImGui::SameLine();
                    HelpMarker("(?)","USER:\nHold SHIFT or use mouse to select text.\n" "CTRL+Left/Right to word jump.\n" "CTRL+A or double-click to select all.\n" "CTRL+X,CTRL+C,CTRL+V clipboard.\n" "CTRL+Z,CTRL+Y undo/redo.\n" "ESCAPE to revert.");


                    ImGui::Text(" ");
                    ImGui::Checkbox("  Respect mask on load", &respect_mask_on_load);

                    ImGui::SetCursorPosX(16);

                    if (ImGui::Button("Load", ImVec2(60, 22)))
                    {
                        //load that image
                        GPU_Data.load(directory_strings[listbox_select_index], respect_mask_on_load);
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("Save", ImVec2(60, 22)))
                    {
                        if(hasPNG(std::string(str0)))
                        {
                            GPU_Data.save(std::string(str0));
                        }
                        else
                        {
                            GPU_Data.save(std::string(str0)+std::string(".png"));
                        }

                        update_listbox_items();
                    }
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(" Lighting "))
            {
                ImGui::BeginTabBar("l", tab_bar_flags);

                static float clear_level;
                static bool use_cache;

                static float directional_theta;
                static float directional_phi;
                static float directional_intensity;
                static float decay_power;

                static int AO_radius;

                static float GI_scale_factor = 0.028;
                static float GI_alpha_thresh = 0.010;
                static float GI_sky_intensity = 0.16;


                static glm::vec3 point_light_position = glm::vec3(0,0,0);
                static float point_intensity = 0;
                static float point_decay_power = 0;
                static float point_distance_power = 0;


                static glm::vec3 cone_light_position = glm::vec3(0,0,0);
                static float cone_theta = 0;
                static float cone_phi = 0;
                static float cone_angle = 0;
                static float cone_intensity = 0;
                static float cone_decay_power = 0;
                static float cone_distance_power = 0;
                    
                if(ImGui::BeginTabItem(" Clear "))
                {

                    ImGui::Text("Clear Level - 0.25 is neutral");
                    ImGui::SliderFloat("level", &clear_level, 0.0f, 1.0f, "%.3f");

                    ImGui::Checkbox(" use cached levels ", &use_cache);

                    if (ImGui::Button("Clear", ImVec2(120, 22))) // Buttons return true when clicked (most widgets return true when edited/activated)
                        GPU_Data.lighting_clear(use_cache, clear_level);

                    ImGui::Separator();
                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem(" Point "))
                {

                    ImGui::Text("Point Light");
                    ImGui::SliderFloat("loc x", &point_light_position.x, -100, DIM+100, "%.3f");
                    ImGui::SliderFloat("loc y", &point_light_position.y, -100, DIM+100, "%.3f");
                    ImGui::SliderFloat("loc z", &point_light_position.z, -100, DIM+100, "%.3f");
                    ImGui::Text(" ");
                    ImGui::SliderFloat("value", &point_intensity, 0, 1.0, "%.3f");
                    ImGui::SliderFloat("decay", &point_decay_power, 0, 3.0, "%.3f");
                    ImGui::SliderFloat("dist power", &point_distance_power, 0, 3.0f, "%.3f");
                    
                    if (ImGui::Button("Point Light", ImVec2(120, 22))) // Buttons return true when clicked (most widgets return true when edited/activated)
                        GPU_Data.compute_point_lighting(point_light_position, point_intensity, point_decay_power, point_distance_power);

                    
                    ImGui::Separator();
                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem(" Cone "))
                {
                    ImGui::Text("Location of the tip of the cone");
                    ImGui::SliderFloat("loc x", &cone_light_position.x, -100, DIM+100, "%.3f");
                    ImGui::SliderFloat("loc y", &cone_light_position.y, -100, DIM+100, "%.3f");
                    ImGui::SliderFloat("loc z", &cone_light_position.z, -100, DIM+100, "%.3f");
                    ImGui::Text(" ");
                    ImGui::Text("Rotation of the cone");
                    ImGui::SliderFloat("theta", &cone_theta, -3.14f, 3.14f, "%.3f");
                    ImGui::SliderFloat("phi", &cone_phi, -3.14f, 3.14f, "%.3f");
                    ImGui::Text(" ");
                    ImGui::Text("Defines the width of the cone");
                    ImGui::SliderFloat("cone angle", &cone_angle, -3.14f, 3.14f, "%.3f");
                    ImGui::Text(" ");
                    ImGui::Text("Defines the initial intensity of this light source");
                    ImGui::SliderFloat("value", &cone_intensity, 0, 1.0, "%.3f");
                    ImGui::Text(" ");
                    ImGui::Text("Defines the falloff - decay is interaction with alpha");
                    ImGui::SliderFloat("decay", &cone_decay_power, 0, 3.0, "%.3f");
                    ImGui::SliderFloat("dist power", &cone_distance_power, 0, 3.0f, "%.3f");

                    ImGui::Separator();
                    ImGui::EndTabItem();
                }
                                
                if(ImGui::BeginTabItem(" Directional "))
                {

                    ImGui::Text("Directional");
                    ImGui::SliderFloat("theta", &directional_theta, -3.14f, 3.14f, "%.3f");
                    ImGui::SliderFloat("phi", &directional_phi, -3.14f, 3.14f, "%.3f");
                    ImGui::Text(" ");
                    ImGui::SliderFloat("value", &directional_intensity, 0.0f, 1.0f, "%.3f");
                    ImGui::SliderFloat("decay", &decay_power, 0.0f, 3.0f, "%.3f");

                    if (ImGui::Button("New Directional", ImVec2(120, 22))) // Buttons return true when clicked (most widgets return true when edited/activated)
                        GPU_Data.compute_new_directional_lighting(directional_theta, directional_phi, directional_intensity, decay_power);

                    ImGui::Separator();
                    ImGui::EndTabItem();
                }
  
                if(ImGui::BeginTabItem(" Fake GI "))
                {
                    WrappedText("Fake GI is computed by tracing rays upwards from each cell. If they escape the volume, they get the sky_intensity added. Otherwise they take a portion of the light of the cell they hit, set by sfactor.", windowsize.x);

                    ImGui::SliderFloat("sfactor", &GI_scale_factor, 0.0f, 1.0f);
                    ImGui::SliderFloat("alpha threshold", &GI_alpha_thresh, 0.0f, 1.0f);
                    ImGui::SliderFloat("sky intensity", &GI_sky_intensity, 0.0f, 1.0f);

                    if(ImGui::Button("Apply GI", ImVec2(120, 22)))
                    {
                        GPU_Data.compute_fake_GI(GI_scale_factor, GI_sky_intensity, GI_alpha_thresh);
                    }

                    ImGui::Separator();
                    ImGui::EndTabItem();
                }

                if(ImGui::BeginTabItem(" Ambient Occlusion "))
                {
                    WrappedText("Ambient occlusion is based on a weighted average of the alpha values in the specified size neighborhood.", windowsize.x);
                    ImGui::Text(" ");
                    ImGui::SliderInt("radius", &AO_radius, 0, 5);

                    if (ImGui::Button("Apply AO", ImVec2(120, 22)))
                    {
                        GPU_Data.compute_ambient_occlusion(AO_radius);
                    }


                    ImGui::Separator();
                    ImGui::EndTabItem();
                }


                if(ImGui::BeginTabItem(" Mash "))
                {
                    WrappedText("Mash combines the lighting buffer and the color buffer, so that the block can be saved with the lighting applied.", windowsize.x);

                    if (ImGui::Button("Mash", ImVec2(120, 22)))
                    {
                        GPU_Data.mash();
                    }

                    ImGui::Separator();
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
                ImGui::EndTabItem();
            }

        }

        if (ImGui::BeginTabItem(" Render Settings "))
        {
            // adjust postition of orientation widget, move it in x, y and z
            static glm::vec3 offset = glm::vec3(0.9, -0.741, 0.0);

            ImGui::Text("Move the orientation widget.");

            ImGui::SliderFloat("offset x", &offset.x, -1, 1);
            ImGui::SliderFloat("offset y", &offset.y, -1, 1);

            GPU_Data.orientation_widget_offset = offset;

            ImGui::Text(" ");
            ImGui::Text(" ");

            ImGui::Separator();

            ImGui::Text(" ");
            ImGui::Text(" ");

            // adjust power on alpha correction - make sure to set GPU_Data.redraw_flag if it changes (handled internal to GPU_Data)
            WrappedText("This is the correction factor for the alpha channel. A value of 1.0 uses the unmanipulated value of the alpha channel - I find 2.0 is a good amount for most situations, and it makes much better use of the range than 1.0", windowsize.x);

            ImGui::SliderFloat("alpha correction power", &GPU_Data.alpha_correction_power, 0.5, 4.0);

            ImGui::Text(" ");
            ImGui::Text(" ");

            ImGui::Separator();

            ImGui::Text(" ");
            ImGui::Text(" ");

            ImGui::Text("0 - none");
            ImGui::Text("1 - cheap");
            ImGui::Text("2 - full");
            ImGui::SliderInt("tonemapping mode", &GPU_Data.tonemap_mode, 0, 2);
            
            ImGui::Text(" ");
            ImGui::Text(" ");

            ImGui::Separator();

            ImGui::Text(" ");
            ImGui::Text(" ");

            // slider control over phi, theta, scale
            ImGui::Text("Control over display parameters.");

            ImGui::SliderFloat("theta", &GPU_Data.theta, -3.14f, 3.14f, "%.3f");
            ImGui::SliderFloat("phi", &GPU_Data.phi, -3.14f, 3.14f, "%.3f");
            ImGui::Text(" ");
            ImGui::SliderFloat("scale", &GPU_Data.scale, 0.0f, 10.0f, "%.3f");

            ImGui::Text(" ");
            ImGui::Text(" ");

            ImGui::Separator();

            ImGui::Text(" ");
            ImGui::Text(" ");

            // adjusting the clear color
            ImGui::Text("Set up OpenGL clear color.");

            ImGui::ColorEdit3("", (float*)&clear_color); // Edit 3 floats representing a color
            GPU_Data.clear_color = glm::vec4(clear_color.x, clear_color.y, clear_color.z, 1.0);



            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(" Instructions "))
        {
            // need to figure out what basic info I want to relay to the user
            // cout << "size is " << windowsize.x << " " << windowsize.y << endl;
            // need to set up wrapping based on windowsize.x
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
        ImGui::End();
    }
}

void Voraldo::draw_everything()
{
    ImGuiIO& io = ImGui::GetIO(); // void cast prevents unused variable warning

  //maintaining history of fps values
  //push back - put in the new value
    fps_history.push_back(io.Framerate);

  //pop front - take out the oldest value
    fps_history.pop_front();

    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);   // from hsv picker
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                     // clear the screen




    // draw the stuff on the GPU (block and orientation widget)
    GPU_Data.display();




    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    // draw the menu bar
    AppMainMenuBar(&show_menu);

    // draw the FPS overlay
    FPSOverlay(&show_fpsoverlay);

    // do my own window
    ControlWindow(&show_controls);

    // show quit confirm window
    QuitConfirm(&quit_confirm);

    // show the demo window
    if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

    

    // get the data go the GPU
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());   // put imgui data into the framebuffer

    SDL_GL_SwapWindow(window);  // swap the double buffers

    // handle events

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);

        if (event.type == SDL_QUIT)
            pquit = true;

        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
            pquit = true;

        // if ((event.type == SDL_KEYUP  && event.key.keysym.sym == SDLK_ESCAPE) || (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_X1)) //x1 is browser back on the mouse
        if (event.type == SDL_KEYUP  && event.key.keysym.sym == SDLK_ESCAPE)
            quit_confirm = !quit_confirm;

        if(event.type == SDL_KEYDOWN  && event.key.keysym.sym == SDLK_UP)
            GPU_Data.phi  += 0.03f;     //increment phi
        if(event.type == SDL_KEYDOWN  && event.key.keysym.sym == SDLK_DOWN)
            GPU_Data.phi  -= 0.03f;     //decrement phi
        if(event.type == SDL_KEYDOWN  && event.key.keysym.sym == SDLK_LEFT)
            GPU_Data.theta += 0.03f;    //increment theta
        if(event.type == SDL_KEYDOWN  && event.key.keysym.sym == SDLK_RIGHT)
            GPU_Data.theta -= 0.03f;    //decrement theta
        if(event.type == SDL_KEYDOWN  && event.key.keysym.sym == SDLK_MINUS)
            GPU_Data.scale += 0.1f;     //make scale smaller (offsets are larger)
        if(event.type == SDL_KEYDOWN  && event.key.keysym.sym == SDLK_EQUALS) //SDLK_PLUS requires that you hit the shift
            GPU_Data.scale -= 0.1f;     //make scale larger  (offsets are smaller)

        if(event.type == SDL_KEYDOWN  && event.key.keysym.sym == SDLK_m)
            show_menu = !show_menu;
        if(event.type == SDL_KEYDOWN  && event.key.keysym.sym == SDLK_c)
            show_controls = !show_controls;


        // till I come up with a good way to maintain state for the mouse click and drag, this is how that offset is controlled
        if(event.type == SDL_KEYDOWN  && event.key.keysym.sym == SDLK_h)
            GPU_Data.clickndragx += SDL_GetModState() & KMOD_SHIFT ? 50 : 5;
        if(event.type == SDL_KEYDOWN  && event.key.keysym.sym == SDLK_l)
            GPU_Data.clickndragx -= SDL_GetModState() & KMOD_SHIFT ? 50 : 5;

        if(event.type == SDL_KEYDOWN  && event.key.keysym.sym == SDLK_k)
            GPU_Data.clickndragy += SDL_GetModState() & KMOD_SHIFT ? 50 : 5;
        if(event.type == SDL_KEYDOWN  && event.key.keysym.sym == SDLK_j)
            GPU_Data.clickndragy -= SDL_GetModState() & KMOD_SHIFT ? 50 : 5;

        
        // specific directions
        if(event.type == SDL_KEYDOWN  && event.key.keysym.sym == SDLK_F1)
            GPU_Data.theta = 0.0;
        if(event.type == SDL_KEYDOWN  && event.key.keysym.sym == SDLK_F2)
            GPU_Data.theta = pi/2.0;
        if(event.type == SDL_KEYDOWN  && event.key.keysym.sym == SDLK_F3)
            GPU_Data.theta = pi;
        if(event.type == SDL_KEYDOWN  && event.key.keysym.sym == SDLK_F4)
            GPU_Data.theta = 3.0*(pi/2.0);


        // if(event.type == SDL_MOUSEMOTION && (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))) // mouse moved, left button down
        // {
        //     int x,y;
        //     static glm::ivec2 prev_mouseloc = glm::ivec2(0,0);

        //     // get the location of the mouse
        //     SDL_GetMouseState(&x, &y);
                
        //     // how much has the mouse moved since the last time this if statement evaluated true
        //     GPU_Data.clickndragx += prev_mouseloc.x-x;
        //     GPU_Data.clickndragy -= prev_mouseloc.y-y;

        //     cout << "clickndragx " << GPU_Data.clickndragx << " clickndragy " << GPU_Data.clickndragy << endl;
            
        //     prev_mouseloc = glm::ivec2(x, y);
        // }

        
        if(event.type == SDL_MOUSEWHEEL)  //allow scroll to do the same thing as +/-
        {
            if(event.wheel.y > 0) // scroll up
            {
                GPU_Data.scale -= 0.1f;
            }
            else if(event.wheel.y < 0) // scroll down
            {
                GPU_Data.scale += 0.1f;
            }
        }
    }
}


void Voraldo::quit()
{
  // delete textures
  GPU_Data.delete_textures();
    
  // shutdown everything
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  // destroy window
  SDL_GL_DeleteContext(GLcontext);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
