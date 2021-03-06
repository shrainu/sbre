#include "common.h"



/* GLFW / Window */

GLFWwindow* _SBRE_main_window;
static uint32_t _SBRE_SCREEN_WIDTH, _SBRE_SCREEN_HEIGHT;



/* Delta Time Varaibles */

static const double _SBRE_delta_time_high = 0.1;
static double _SBRE_last_frame_time;
static double _SBRE_current_frame_time;
double _SBRE_delta_time;



/* FPS Variables */

static double _SBRE_previous_time;
static double _SBRE_current_time;
static double _SBRE_time_diff;
static uint32_t _SBRE_fps_counter;
static uint32_t _SBRE_FPS;
static double _SBRE_frame_time;



/* Keyboard Input */

static SBRE_CharInput _SBRE_char_input;
static SBRE_KeyInput _SBRE_keyboard_input;



/* Mouse Input */

static SBRE_MouseInput _SBRE_mouse_input;



/* Delta Time */

void SBRE_calculate_delta_time(void) {

    _SBRE_current_frame_time = glfwGetTime();

    _SBRE_delta_time = _SBRE_current_frame_time - _SBRE_last_frame_time;
    if (_SBRE_delta_time > _SBRE_delta_time_high)
        _SBRE_delta_time = _SBRE_delta_time_high;

    _SBRE_last_frame_time = _SBRE_current_frame_time;
}



double SBRE_delta_time(void) {

	return _SBRE_delta_time;
}



/* FPS */

void SBRE_calculate_fps(void) {

	_SBRE_current_time = glfwGetTime();
    _SBRE_time_diff = _SBRE_current_time - _SBRE_previous_time;
    _SBRE_fps_counter++;

    if (_SBRE_time_diff >= 1 / 30.0) {
		_SBRE_FPS = (1.0 / _SBRE_time_diff) * _SBRE_fps_counter;
        _SBRE_frame_time = (_SBRE_time_diff / _SBRE_fps_counter) * 1000;
        _SBRE_previous_time = _SBRE_current_time;
		_SBRE_fps_counter = 0;
    }
}



uint32_t SBRE_fps(void) {
	
	return _SBRE_FPS;
}



double SBRE_frame_time(void) {

	return _SBRE_frame_time;
}



/* Input */

Vec2 SBRE_get_mouse_pos(void) {

	double xpos, ypos;
	glfwGetCursorPos(_SBRE_main_window, &xpos, &ypos);

	return (Vec2) { xpos, ypos };
}



bool SBRE_get_mouse_button(int button) {

	if (button >= _SBRE_mouse_input.max_button_count)
		return false;

	return _SBRE_mouse_input.buttons[button].pressed;
}



bool SBRE_get_mouse_button_pressed(int button) {

	if (button >= _SBRE_mouse_input.max_button_count)
		return false;

	return _SBRE_mouse_input.buttons[button].just_pressed;
}



bool SBRE_get_mouse_button_released(int button) {

	if (button >= _SBRE_mouse_input.max_button_count)
		return false;

	return _SBRE_mouse_input.buttons[button].just_released;
}



bool SBRE_get_key_press(int key_code) {

	return glfwGetKey(_SBRE_main_window, key_code) == GLFW_PRESS;
}



bool SBRE_get_key_release(int key_code) {

	return glfwGetKey(_SBRE_main_window, key_code) == GLFW_RELEASE;
}



const SBRE_CharInput* SBRE_get_chars_pressed(void) {

	return &_SBRE_char_input;
}



const SBRE_KeyInput* SBRE_get_keys_pressed(void) {

	return &_SBRE_keyboard_input;
}



void _SBRE_char_callback(GLFWwindow* window, uint32_t code_point) {

	_SBRE_char_input.keys_pressed[_SBRE_char_input.input_count] = code_point;

	_SBRE_char_input.input_count++;
}



void _SBRE_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	_SBRE_keyboard_input.keys_pressed[_SBRE_keyboard_input.input_count].key = key;
	_SBRE_keyboard_input.keys_pressed[_SBRE_keyboard_input.input_count].action_type = action;

	_SBRE_keyboard_input.input_count++;
}



void _SBRE_mouse_callback(GLFWwindow* window, int button, int action, int mods) {
	
	SBRE_MouseAction* mouse_action = &_SBRE_mouse_input.buttons[button];

	if (action == GLFW_PRESS) {

		mouse_action->just_pressed  = true;
		mouse_action->pressed = true;
	}
	else if (action == GLFW_RELEASE) {

		mouse_action->just_released = true;
		mouse_action->pressed = false;
	}
}



void _SBRE_clear_mouse_input_buffer(void) {

	for (int i = 0; i < _SBRE_mouse_input.max_button_count; ++i) {

		SBRE_MouseAction* mouse_action = &_SBRE_mouse_input.buttons[i];

		if (mouse_action->just_pressed)
			mouse_action->just_pressed = false;
		else if (mouse_action->just_released)
			mouse_action->just_released = false;
	}
}



void _SBRE_clear_char_input_buffer(void) {

	memset(_SBRE_char_input.keys_pressed, 0, 256);
	_SBRE_char_input.input_count = 0;
}



void _SBRE_clear_key_input_buffer(void) {

	memset(_SBRE_keyboard_input.keys_pressed, 0, 256 * sizeof(SBRE_KeyAction));
	_SBRE_keyboard_input.input_count = 0;
}



/* Core Initialization */

bool SBRE_init(int SCREEN_WIDTH, int SCREEN_HEIGHT, const char* TITLE, int vsync) {
	
	// Initialize GLFW

    if(!glfwInit()) {
		printf("[GLFW Error][GLFW could not be initialized!]\n");
        return false;
    }

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif


	// Create window

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE, NULL, NULL);
    if (!window) {
		printf("[GLFW Error][Window could now be initialized!]\n");
        return false;
    }


	// Make the window current context

    glfwMakeContextCurrent(window);


	// Set the screen width and height

	_SBRE_SCREEN_WIDTH  = SCREEN_WIDTH;
	_SBRE_SCREEN_HEIGHT = SCREEN_HEIGHT;


    // Enable-Disable V-Sync

    glfwSwapInterval(vsync);


    // Initialize GLEW

    if (glewInit() != GLEW_OK) {
		printf("[GLEW Error][GLEW could not be initialized!]\n");
        glfwTerminate();
        return false;
    }


	// Set the main_window

	_SBRE_main_window = window;


	// Initialize Rendering Systems

	_SBRE_init_render_systems();


	// Set the default Projection Matrix

	mat4 proj;
	glm_mat4_identity(proj);
	glm_ortho(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, -1.0f, 1.0f, proj);	
	
	Mat4 mvp = {
		.r1 = { proj[0][0], proj[0][1], proj[0][2], proj[0][3] },
		.r2 = { proj[1][0], proj[1][1], proj[1][2], proj[1][3] },
		.r3 = { proj[2][0], proj[2][1], proj[2][2], proj[2][3] },
		.r4 = { proj[3][0], proj[3][1], proj[3][2], proj[3][3] },
	};

	_SBRE_set_projection_marix(mvp);


	// Keyboard Input

	glfwSetCharCallback(_SBRE_main_window, _SBRE_char_callback);

	_SBRE_char_input = (SBRE_CharInput) {

		.input_count = 0,
		.max_input_per_frame = 256
	};


	glfwSetKeyCallback(_SBRE_main_window, _SBRE_key_callback);

	_SBRE_keyboard_input = (SBRE_KeyInput) {

		.input_count = 0,
		.max_input_per_frame = 256
	};


	// Mouse Input

	glfwSetMouseButtonCallback(_SBRE_main_window, _SBRE_mouse_callback);

	_SBRE_mouse_input = (SBRE_MouseInput) {

		.max_button_count = 9
	};

	
	return true;
}



void SBRE_terminate(void) {
	
	_SBRE_terminate_render_systems();

	glfwTerminate();
}



/* Core */

bool SBRE_window_should_close(void) {
	
	return glfwWindowShouldClose(_SBRE_main_window);
}



void SBRE_set_clear_color(Color clear_color) {
	
	glClearColor(clear_color.r / 255.0f, clear_color.g / 255.0f, clear_color.b / 255.0f, clear_color.a / 255.0f);
}



void SBRE_clear(void) {
	
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}



void SBRE_display(void) {
	
	glfwSwapBuffers(_SBRE_main_window);
}



void SBRE_poll_events(void) {

	_SBRE_clear_char_input_buffer();
	_SBRE_clear_key_input_buffer();
	_SBRE_clear_mouse_input_buffer();

	glfwPollEvents();
}



/* Texture */

Texture* SBRE_load_texture(const char* filepath, uint32_t filter) {
	
	FILE* fp = NULL;
	fp = fopen(filepath, "r");

	if (!(fp)) {
		printf("[SBRE Error][Image file \"%s\" could not be opened.]\n", filepath);
		fclose(fp);
		return NULL;
	}
	
	fclose(fp);

	uint32_t texture_id;
	int width, height, bpp;
	
	stbi_set_flip_vertically_on_load(1);
	unsigned char* local_buffer = stbi_load(filepath, &width, &height, &bpp, 4);
	
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, local_buffer);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	stbi_image_free(local_buffer);
	
	Texture* t = (Texture*) malloc(sizeof(Texture));
	*t = (Texture){
		
		.texture_id = texture_id,
		.color = SBRE_COLOR(255, 255, 255, 255),
		.initial_width  = width,
		.initial_height = height,
		.width  = width,
		.height = height,
		.bpp = bpp
	};
	
	return t;
}



void SBRE_free_texture(Texture* texture) {

	glDeleteTextures(1, &texture->texture_id);
	free(texture);
	
	texture = NULL;
}



void SBRE_load_texture_to_texture(Texture* texture, const char* filepath, int32_t offset_x, int32_t offset_y) {

	FILE* fp = NULL;
	fp = fopen(filepath, "r");

	if (!(fp)) {
		printf("[SBRE Error][Image file \"%s\" could not be opened.]\n", filepath);
		fclose(fp);
		return;
	}
	
	fclose(fp);

	uint32_t texture_id;
	int width, height, bpp;
	
	stbi_set_flip_vertically_on_load(1);
	unsigned char* local_buffer = stbi_load(filepath, &width, &height, &bpp, 4);

	/* Convert offset_y to opengl coordinates. */
	offset_y = (texture->initial_height - height) - offset_y;
	
	glBindTexture(GL_TEXTURE_2D, texture->texture_id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, offset_x, offset_y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, local_buffer);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(local_buffer);
}