#include "voraldo1_1.h"
// This contains the lower level code

void Voraldo::create_window()
{
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

	// pulling these out because I'm going to try to span the whole screen with
	// the window, in a way that's flexible on different resolution screens
	int total_screen_width = dm.w;
	int total_screen_height = dm.h;

	cout << "creating window...";

	// window = SDL_CreateWindow( "OpenGL Window", 0, 0, total_screen_width, total_screen_height, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS );
	window = SDL_CreateWindow( "OpenGL Window", 0, 0, total_screen_width, total_screen_height, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE );
  	SDL_ShowWindow(window);

	cout << "done." << endl;


	cout << "setting up OpenGL context...";
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

    // CONTINUUM REPRESENTATION POINTS









	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
	}

	glEnable(GL_DEPTH_TEST);
    glEnable(GL_POINT_SMOOTH);

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

	clear_color = ImVec4(75.0f/255.0f, 75.0f/255.0f, 75.0f/255.0f, 0.5f); // initial value for clear color

	// really excited by the fact imgui has an hsv picker to set this
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear( GL_COLOR_BUFFER_BIT );
	SDL_GL_SwapWindow( window );

	cout << "done." << endl;

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

}

void Voraldo::gl_setup()
{
	// some info on your current platform
	const GLubyte *renderer = glGetString( GL_RENDERER ); // get renderer string
	const GLubyte *version = glGetString( GL_VERSION );		// version as a string
	printf( "Renderer: %s\n", renderer );
	printf( "OpenGL version supported %s\n\n\n", version );



    // create the shader for the triangles to cover the screen
    display_shader = Shader("resources/code/shaders/blit.vs.glsl", "resources/code/shaders/blit.fs.glsl").Program;

    // set up the points for the display
    //  A---------------B
    //  |          .    |
    //  |       .       |
    //  |    .          |
    //  |               |
    //  C---------------D

    // diagonal runs from C to B
    //  A is -1, 1
    //  B is  1, 1
    //  C is -1,-1
    //  D is  1,-1
    std::vector<glm::vec3> points;

    points.clear();
    points.push_back(glm::vec3(-1, 1, 0.5));  //A
    points.push_back(glm::vec3(-1,-1, 0.5));  //C
    points.push_back(glm::vec3( 1, 1, 0.5));  //B

    points.push_back(glm::vec3( 1, 1, 0.5));  //B
    points.push_back(glm::vec3(-1,-1, 0.5));  //C
    points.push_back(glm::vec3( 1,-1, 0.5));  //D

    // vao, vbo
    cout << "  setting up vao, vbo for display geometry...........";
    glGenVertexArrays( 1, &display_vao );
    glBindVertexArray( display_vao );

    glGenBuffers( 1, &display_vbo );
    glBindBuffer( GL_ARRAY_BUFFER, display_vbo );
    cout << "done." << endl;

    // buffer the data
    cout << "  buffering vertex data..............................";
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * points.size(), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * points.size(), &points[0]);
    cout << "done." << endl;

    // set up attributes
    cout << "  setting up attributes in display shader............";
    GLuint points_attrib = glGetAttribLocation(display_shader, "vPosition");
    glEnableVertexAttribArray(points_attrib);
    glVertexAttribPointer(points_attrib, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) (static_cast<const char*>(0) + (0)));
    cout << "done." << endl;


    // create the image textures

    // compile the compute shader to do the raycasting

    // ...



}


// these are some utility functions for the imgui stuff

// basically just a tooltip, useful for including a note that won't shit up a significant chunk of the window
void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}


// small overlay to show the FPS counter, FPS graph
void FPSOverlay(bool* p_open)
{
    const float DISTANCE = 10.0f;
    static int corner = 0;
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
        ImGui::Text("Simple overlay\n" "in the corner of the screen.\n" "(right-click to change position)");
        ImGui::Separator();
        if (ImGui::IsMousePosValid())
            ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
        else
            ImGui::Text("Mouse Position: <invalid>");
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


// populates a menu dropdown, this will change significantly as I figure out what functionality I'm going to include
void ShowExampleMenuFile()
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
void AppMainMenuBar()
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
            if (ImGui::MenuItem("Copy", "CTRL+C")) {cout << "copy" << endl;}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}


void ControlWindow()
{
    ImGuiWindowFlags flags = 0;

    ImGui::SetNextWindowPos(ImVec2(0,19));
    ImGui::SetNextWindowSize(ImVec2(320,385), ImGuiCond_FirstUseEver);
    ImGui::Begin("Controls", NULL, flags);


  // tabbed layout
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None | ImGuiTabBarFlags_FittingPolicyScroll;

    if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Simulation"))
        {
            ImGui::Separator();
            ImGui::Text(" ");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Rendering"))
        {
            ImGui::Separator();
            ImGui::Text(" ");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Instructions"))
        {
            ImGui::Separator();
            ImGui::Text(" ");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Instructions1"))
        {
            ImGui::Separator();
            ImGui::Text(" ");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Instructions2"))
        {
            ImGui::Separator();
            ImGui::Text(" ");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Instructions3"))
        {
            ImGui::Separator();
            ImGui::Text(" ");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Instructions4"))
        {
            ImGui::Separator();
            ImGui::Text(" ");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    //do the other widgets
   HelpMarker("shut up, compiler");


   ImGui::End();
}

void Voraldo::draw_everything()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io; // void cast prevents unused variable warning
    //get the screen dimensions and pass in as uniforms


	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);   // from hsv picker
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                     // clear the background




    // draw the stuff on the GPU




    // texture display
    glUseProgram(display_shader);
    glBindVertexArray( display_vao );
    glBindBuffer( GL_ARRAY_BUFFER, display_vbo );

    glDrawArrays( GL_TRIANGLES, 0, 6 );


	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();


    // draw the menu bar
    AppMainMenuBar();


	// show the demo window
	static bool show_demo_window = true;
	if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);


	// do my own window
    ControlWindow();


    // get the data go the GPU
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

		if ((event.type == SDL_KEYUP  && event.key.keysym.sym == SDLK_ESCAPE) || (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_X1)) //x1 is browser back on the mouse
			pquit = true;
	}
}


void Voraldo::quit()
{
  //shutdown everything
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  //destroy window
  SDL_GL_DeleteContext(GLcontext);
  SDL_DestroyWindow(window);
  SDL_Quit();

  cout << "goodbye." << endl;
}
