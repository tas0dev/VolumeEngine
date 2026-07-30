/*
 * Copyright (c) 2026, tas0dev.
 * This software is provided under the zlib License.
 *
 * Created by tas0dev
 */

#include "volume.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum brush_dialog_action {
	BRUSH_DIALOG_NONE,
	BRUSH_DIALOG_OPEN,
	BRUSH_DIALOG_SAVE,
} brush_dialog_action_t;

typedef struct brush_state {
	map_document_t *document;
	asset_manager_t *assets;
	renderer_font_t *font;
	ui_context_t *ui;
	camera_t camera;
	vec3_t camera_target;
	material_t fallback_material;
	float camera_yaw;
	float camera_pitch;
	float camera_distance;
	char *path;
	const char *initial_path;
	SDL_Mutex *dialog_mutex;
	char *dialog_path;
	brush_dialog_action_t dialog_action;
	bool dialog_open;
	bool dialog_finished;
	bool dialog_failed;
	bool dragging_entity;
	bool drag_transaction;
	size_t selected_entity;
	bool has_selection;
	char status[512];
} brush_state_t;

static bool initialize(engine_t *engine, void *user_data);
static void update(engine_t *engine, float delta_time, void *user_data);
static void render(engine_t *engine, void *user_data);
static void shutdown(engine_t *engine, void *user_data);
static void begin_dialog(brush_state_t *brush, brush_dialog_action_t action);
static void process_dialog(brush_state_t *brush);
static void create_map(brush_state_t *brush);
static void save_map(brush_state_t *brush);
static void draw_toolbar(brush_state_t *brush, int width);
static void draw_hierarchy(brush_state_t *brush, ui_rect_t area);
static void draw_viewport(brush_state_t *brush,
			  renderer_t *renderer,
			  const render_view_t *view,
			  ui_rect_t area);
static void draw_inspector(brush_state_t *brush, ui_rect_t area);
static void add_entity(brush_state_t *brush, const char *classname);
static void remove_selected_entity(brush_state_t *brush);
static void validate_selection(brush_state_t *brush);
static void adjust_vec3_property(brush_state_t *brush,
				 const char *key,
				 size_t component,
				 float amount,
				 vec3_t default_value);
static void adjust_uniform_scale(brush_state_t *brush, float amount);
static void
update_camera(brush_state_t *brush, engine_t *engine, input_t *input);
static render_view_t
create_render_view(brush_state_t *brush, int width, int height);
static void draw_scene(brush_state_t *brush,
		       renderer_t *renderer,
		       const render_view_t *view);
static bool project_point(const brush_state_t *brush,
			  vec3_t point,
			  int width,
			  int height,
			  float *screen_x,
			  float *screen_y);
static void update_viewport_interaction(brush_state_t *brush,
					input_t *input,
					int width,
					int height);
static void focus_camera(brush_state_t *brush);
static void SDLCALL dialog_callback(void *user_data,
				    const char *const *files,
				    int filter);
static char *duplicate_string(const char *text);
static const char *display_name(const char *path);

static brush_state_t state;

static const game_t application = {
	.initialize = initialize,
	.update = update,
	.render = render,
	.shutdown = shutdown,
	.user_data = &state,
};

static const SDL_DialogFileFilter map_filter = {
	"VolumeEngine Map",
	"volmap",
};

int main(const int argc, char **argv) {
	engine_config_t config = {0};
	engine_t *engine;
	bool result;

	if (argc > 2) {
		fprintf(stderr, "usage: brush [map.volmap]\n");
		return EXIT_FAILURE;
	}
	state.initial_path = argc == 2 ? argv[1] : NULL;
	config.application_name = "Brush";
	config.window_width = 1440;
	config.window_height = 850;
	config.capture_mouse = false;
	config.fixed_delta_time = 1.0f / 60.0f;
	config.game = &application;
	engine = engine_create(&config);
	if (engine == NULL) { return EXIT_FAILURE; }
	result = engine_run(engine);
	engine_destroy(engine);
	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}

static bool initialize(engine_t *engine, void *user_data) {
	brush_state_t *brush;
	char *asset_root;
	char *font_path;
	char error[512];

	brush = user_data;
	brush->dialog_mutex = SDL_CreateMutex();
	asset_root = path_from_executable("assets/game");
	font_path =
		path_from_executable("assets/game/fonts/RobotoCondensed.ttf");
	if (brush->dialog_mutex == NULL || asset_root == NULL ||
	    font_path == NULL) {
		free(asset_root);
		free(font_path);
		return false;
	}
	brush->assets = asset_manager_create_at(asset_root);
	free(asset_root);
	brush->font =
		renderer_font_create(engine_get_renderer(engine), font_path);
	free(font_path);
	if (brush->assets == NULL || brush->font == NULL) {
		log_error("Brush UI font could not be loaded");
		return false;
	}
	brush->ui = ui_context_create(engine_get_renderer(engine),
				      engine_get_input(engine), brush->font);
	if (brush->ui == NULL) { return false; }
	brush->camera_target = vec3_create(0.0f, 0.0f, 0.0f);
	brush->camera_yaw = -0.75f;
	brush->camera_pitch = -0.45f;
	brush->camera_distance = 14.0f;
	brush->camera = camera_create(vec3_create(8.0f, 7.0f, 8.0f));
	brush->fallback_material =
		material_create(vec3_create(0.65f, 0.68f, 0.72f));
	if (brush->initial_path != NULL) {
		brush->document = map_document_load(brush->initial_path, error,
						    sizeof(error));
		brush->path = duplicate_string(brush->initial_path);
		if (brush->document == NULL || brush->path == NULL) {
			map_document_destroy(brush->document);
			brush->document = NULL;
			free(brush->path);
			brush->path = NULL;
			snprintf(brush->status, sizeof(brush->status),
				 "Open failed: %.460s", error);
			return true;
		}
		snprintf(brush->status, sizeof(brush->status), "Opened %s",
			 display_name(brush->path));
		focus_camera(brush);
	} else {
		snprintf(brush->status, sizeof(brush->status),
			 "Create a new map or open an existing .volmap file.");
	}
	return true;
}

static void update(engine_t *engine, const float delta_time, void *user_data) {
	brush_state_t *brush;
	input_t *input;
	renderer_t *renderer;
	int width;
	int height;

	(void)delta_time;
	brush = user_data;
	input = engine_get_input(engine);
	renderer = engine_get_renderer(engine);
	process_dialog(brush);
	renderer_get_size(renderer, &width, &height);
	update_camera(brush, engine, input);
	update_viewport_interaction(brush, input, width, height);
}

static void render(engine_t *engine, void *user_data) {
	const renderer_color_t status_color = {0.11f, 0.12f, 0.14f, 1.0f};
	brush_state_t *brush;
	renderer_t *renderer;
	render_view_t view;
	ui_rect_t hierarchy;
	ui_rect_t viewport;
	ui_rect_t inspector;
	int width;
	int height;

	brush = user_data;
	renderer = engine_get_renderer(engine);
	renderer_get_size(renderer, &width, &height);
	validate_selection(brush);
	view = create_render_view(brush, width, height);
	draw_scene(brush, renderer, &view);
	draw_toolbar(brush, width);
	hierarchy = (ui_rect_t){8.0f, 58.0f, 250.0f, (float)height - 92.0f};
	inspector = (ui_rect_t){(float)width - 350.0f, 58.0f, 342.0f,
				(float)height - 92.0f};
	viewport = (ui_rect_t){266.0f, 58.0f, (float)width - 624.0f,
			       (float)height - 92.0f};
	draw_hierarchy(brush, hierarchy);
	draw_viewport(brush, renderer, &view, viewport);
	draw_inspector(brush, inspector);
	renderer_draw_rectangle(renderer, 0.0f, (float)height - 27.0f,
				(float)width, 27.0f, status_color);
	ui_label(brush->ui, 10.0f, (float)height - 23.0f, 15.0f,
		 (renderer_color_t){0.72f, 0.75f, 0.80f, 1.0f},
		 brush->dialog_open ? "Waiting for file selection..."
				    : brush->status);
}

static void draw_toolbar(brush_state_t *brush, const int width) {
	const renderer_color_t title_color = {0.91f, 0.93f, 0.96f, 1.0f};
	float x;
	bool available;

	ui_panel(brush->ui, (ui_rect_t){0.0f, 0.0f, (float)width, 50.0f});
	ui_label(brush->ui, 12.0f, 11.0f, 26.0f, title_color, "Brush");
	x = 118.0f;
	available = !brush->dialog_open;
	if (ui_button(brush->ui, (ui_rect_t){x, 8.0f, 74.0f, 34.0f}, "New",
		      available)) {
		create_map(brush);
	}
	x += 80.0f;
	if (ui_button(brush->ui, (ui_rect_t){x, 8.0f, 74.0f, 34.0f}, "Open",
		      available)) {
		begin_dialog(brush, BRUSH_DIALOG_OPEN);
	}
	x += 80.0f;
	if (ui_button(brush->ui, (ui_rect_t){x, 8.0f, 74.0f, 34.0f}, "Save",
		      available && brush->document != NULL)) {
		save_map(brush);
	}
	x += 90.0f;
	if (ui_button(brush->ui, (ui_rect_t){x, 8.0f, 72.0f, 34.0f}, "Undo",
		      available && map_document_can_undo(brush->document))) {
		map_document_undo(brush->document);
		validate_selection(brush);
	}
	x += 78.0f;
	if (ui_button(brush->ui, (ui_rect_t){x, 8.0f, 72.0f, 34.0f}, "Redo",
		      available && map_document_can_redo(brush->document))) {
		map_document_redo(brush->document);
		validate_selection(brush);
	}
	x += 92.0f;
	if (ui_button(brush->ui, (ui_rect_t){x, 8.0f, 96.0f, 34.0f}, "Add Prop",
		      available && brush->document != NULL)) {
		add_entity(brush, "prop_static");
	}
	x += 102.0f;
	if (ui_button(brush->ui, (ui_rect_t){x, 8.0f, 100.0f, 34.0f},
		      "Add Light", available && brush->document != NULL)) {
		add_entity(brush, "light_environment");
	}
	x += 106.0f;
	if (ui_button(brush->ui, (ui_rect_t){x, 8.0f, 106.0f, 34.0f},
		      "Add Spawn", available && brush->document != NULL)) {
		add_entity(brush, "info_player_start");
	}
	x += 122.0f;
	if (ui_button(brush->ui, (ui_rect_t){x, 8.0f, 82.0f, 34.0f}, "Delete",
		      available && brush->has_selection)) {
		remove_selected_entity(brush);
	}
	x += 88.0f;
	if (ui_button(brush->ui, (ui_rect_t){x, 8.0f, 78.0f, 34.0f}, "Focus",
		      available && brush->document != NULL)) {
		focus_camera(brush);
	}
}

static void draw_hierarchy(brush_state_t *brush, const ui_rect_t area) {
	const renderer_color_t heading = {0.84f, 0.87f, 0.91f, 1.0f};
	const map_t *map;
	const map_entity_t *entity;
	const char *classname;
	const char *targetname;
	char label[128];
	float y;
	size_t count;
	size_t index;

	ui_panel(brush->ui, area);
	ui_label(brush->ui, area.x + 12.0f, area.y + 10.0f, 20.0f, heading,
		 "Hierarchy");
	if (brush->document == NULL) { return; }
	map = map_document_get_map(brush->document);
	count = map_get_entity_count(map);
	y = area.y + 42.0f;
	for (index = 0; index < count && y + 28.0f < area.y + area.height;
	     index++) {
		entity = map_get_entity(map, index);
		classname = map_entity_get_property(entity, "classname");
		targetname = map_entity_get_property(entity, "targetname");
		snprintf(label, sizeof(label), "%s%zu  %s%s%s",
			 brush->has_selection && brush->selected_entity == index
				 ? "> "
				 : "",
			 index, classname == NULL ? "entity" : classname,
			 targetname == NULL ? "" : " / ",
			 targetname == NULL ? "" : targetname);
		if (ui_button(brush->ui,
			      (ui_rect_t){area.x + 8.0f, y, area.width - 16.0f,
					  27.0f},
			      label, true)) {
			brush->selected_entity = index;
			brush->has_selection = true;
		}
		y += 30.0f;
	}
}

static void draw_viewport(brush_state_t *brush,
			  renderer_t *renderer,
			  const render_view_t *view,
			  const ui_rect_t area) {
	const renderer_color_t overlay = {0.055f, 0.063f, 0.075f, 0.88f};
	const renderer_color_t heading = {0.90f, 0.92f, 0.96f, 1.0f};
	const renderer_color_t hint = {0.66f, 0.70f, 0.76f, 1.0f};
	char camera_text[160];

	(void)view;
	renderer_draw_rectangle(renderer, area.x + 8.0f, area.y + 8.0f,
				area.width - 16.0f, 34.0f, overlay);
	ui_label(brush->ui, area.x + 18.0f, area.y + 14.0f, 18.0f, heading,
		 "3D Perspective");
	snprintf(camera_text, sizeof(camera_text),
		 "RMB orbit   MMB pan   Wheel zoom   LMB select/drag   %.1f "
		 "units",
		 brush->camera_distance);
	ui_label(brush->ui, area.x + 155.0f, area.y + 15.0f, 15.0f, hint,
		 camera_text);
}

static void draw_inspector(brush_state_t *brush, const ui_rect_t area) {
	const renderer_color_t heading = {0.84f, 0.87f, 0.91f, 1.0f};
	const renderer_color_t text = {0.68f, 0.72f, 0.77f, 1.0f};
	const map_entity_t *entity;
	const char *classname;
	const char *key;
	const char *value;
	vec3_t origin;
	vec3_t angles;
	vec3_t scale;
	char line[192];
	float y;
	size_t property_count;
	size_t index;

	ui_panel(brush->ui, area);
	ui_label(brush->ui, area.x + 12.0f, area.y + 10.0f, 20.0f, heading,
		 "Properties");
	if (!brush->has_selection || brush->document == NULL) {
		ui_label(brush->ui, area.x + 12.0f, area.y + 50.0f, 17.0f, text,
			 "Select an entity to edit it.");
		return;
	}
	entity = map_get_entity(map_document_get_map(brush->document),
				brush->selected_entity);
	classname = map_entity_get_property(entity, "classname");
	snprintf(line, sizeof(line), "%zu  %s", brush->selected_entity,
		 classname == NULL ? "entity" : classname);
	ui_label(brush->ui, area.x + 12.0f, area.y + 46.0f, 22.0f, heading,
		 line);
	origin = (vec3_t){0.0f, 0.0f, 0.0f};
	angles = (vec3_t){0.0f, 0.0f, 0.0f};
	scale = (vec3_t){1.0f, 1.0f, 1.0f};
	map_entity_get_vec3(entity, "origin", &origin);
	map_entity_get_vec3(entity, "angles", &angles);
	map_entity_get_vec3(entity, "scale", &scale);
	snprintf(line, sizeof(line), "Origin   %.2f   %.2f   %.2f", origin.x,
		 origin.y, origin.z);
	ui_label(brush->ui, area.x + 12.0f, area.y + 86.0f, 17.0f, text, line);
	y = area.y + 112.0f;
	for (index = 0; index < 3; index++) {
		static const char *minus_labels[] = {"X-", "Y-", "Z-"};
		static const char *plus_labels[] = {"X+", "Y+", "Z+"};
		float x = area.x + 12.0f + (float)index * 106.0f;
		if (ui_button(brush->ui, (ui_rect_t){x, y, 48.0f, 30.0f},
			      minus_labels[index], true)) {
			adjust_vec3_property(brush, "origin", index, -0.25f,
					     (vec3_t){0.0f, 0.0f, 0.0f});
		}
		if (ui_button(brush->ui,
			      (ui_rect_t){x + 52.0f, y, 48.0f, 30.0f},
			      plus_labels[index], true)) {
			adjust_vec3_property(brush, "origin", index, 0.25f,
					     (vec3_t){0.0f, 0.0f, 0.0f});
		}
	}
	snprintf(line, sizeof(line), "Angles   %.1f   %.1f   %.1f", angles.x,
		 angles.y, angles.z);
	ui_label(brush->ui, area.x + 12.0f, area.y + 156.0f, 17.0f, text, line);
	if (ui_button(
		    brush->ui,
		    (ui_rect_t){area.x + 12.0f, area.y + 182.0f, 75.0f, 30.0f},
		    "Yaw -", true)) {
		adjust_vec3_property(brush, "angles", 1, -15.0f,
				     (vec3_t){0.0f, 0.0f, 0.0f});
	}
	if (ui_button(
		    brush->ui,
		    (ui_rect_t){area.x + 93.0f, area.y + 182.0f, 75.0f, 30.0f},
		    "Yaw +", true)) {
		adjust_vec3_property(brush, "angles", 1, 15.0f,
				     (vec3_t){0.0f, 0.0f, 0.0f});
	}
	snprintf(line, sizeof(line), "Scale    %.2f   %.2f   %.2f", scale.x,
		 scale.y, scale.z);
	ui_label(brush->ui, area.x + 12.0f, area.y + 226.0f, 17.0f, text, line);
	if (ui_button(
		    brush->ui,
		    (ui_rect_t){area.x + 12.0f, area.y + 252.0f, 75.0f, 30.0f},
		    "Scale -", true)) {
		adjust_uniform_scale(brush, -0.1f);
	}
	if (ui_button(
		    brush->ui,
		    (ui_rect_t){area.x + 93.0f, area.y + 252.0f, 75.0f, 30.0f},
		    "Scale +", true)) {
		adjust_uniform_scale(brush, 0.1f);
	}
	ui_label(brush->ui, area.x + 12.0f, area.y + 306.0f, 18.0f, heading,
		 "Entity Properties");
	entity = map_get_entity(map_document_get_map(brush->document),
				brush->selected_entity);
	property_count = map_entity_get_property_count(entity);
	y = area.y + 338.0f;
	for (index = 0;
	     index < property_count && y + 18.0f < area.y + area.height;
	     index++) {
		if (map_entity_get_property_at(entity, index, &key, &value)) {
			snprintf(line, sizeof(line), "%s = %.100s", key, value);
			ui_label(brush->ui, area.x + 12.0f, y, 15.0f, text,
				 line);
			y += 20.0f;
		}
	}
}

static void add_entity(brush_state_t *brush, const char *classname) {
	char targetname[64];
	size_t index;

	if (!map_document_begin_transaction(brush->document) ||
	    !map_document_add_entity(brush->document, classname, &index)) {
		map_document_cancel_transaction(brush->document);
		snprintf(brush->status, sizeof(brush->status),
			 "Could not add %s.", classname);
		return;
	}
	snprintf(targetname, sizeof(targetname), "%s_%zu", classname, index);
	if (!map_document_set_entity_property(brush->document, index,
					      "targetname", targetname) ||
	    !map_document_set_entity_property(brush->document, index, "origin",
					      "0 0 0")) {
		map_document_cancel_transaction(brush->document);
		return;
	}
	if (strcmp(classname, "prop_static") == 0) {
		if (!map_document_set_entity_property(brush->document, index,
						      "model",
						      "models/test_cube.obj") ||
		    !map_document_set_entity_property(
			    brush->document, index, "material",
			    "materials/test_cube.volmat") ||
		    !map_document_set_entity_property(brush->document, index,
						      "scale", "1 1 1") ||
		    !map_document_set_entity_property(brush->document, index,
						      "casts_shadow", "1") ||
		    !map_document_set_entity_property(brush->document, index,
						      "collision", "box") ||
		    !map_document_set_entity_property(brush->document, index,
						      "collision_size",
						      "1 1 1")) {
			map_document_cancel_transaction(brush->document);
			return;
		}
	}
	if (!map_document_end_transaction(brush->document)) {
		snprintf(brush->status, sizeof(brush->status),
			 "Could not commit entity creation.");
		return;
	}
	brush->selected_entity = index;
	brush->has_selection = true;
	snprintf(brush->status, sizeof(brush->status), "Added %s.", classname);
}

static void remove_selected_entity(brush_state_t *brush) {
	if (!brush->has_selection ||
	    !map_document_remove_entity(brush->document,
					brush->selected_entity)) {
		return;
	}
	snprintf(brush->status, sizeof(brush->status), "Deleted entity.");
	validate_selection(brush);
}

static void validate_selection(brush_state_t *brush) {
	size_t count;

	if (brush->document == NULL) {
		brush->has_selection = false;
		return;
	}
	count = map_get_entity_count(map_document_get_map(brush->document));
	if (count == 0) {
		brush->has_selection = false;
	} else if (brush->has_selection && brush->selected_entity >= count) {
		brush->selected_entity = count - 1;
	}
}

static void adjust_vec3_property(brush_state_t *brush,
				 const char *key,
				 const size_t component,
				 const float amount,
				 const vec3_t default_value) {
	const map_entity_t *entity;
	vec3_t value;
	char text[96];

	entity = map_get_entity(map_document_get_map(brush->document),
				brush->selected_entity);
	value = default_value;
	map_entity_get_vec3(entity, key, &value);
	if (component == 0) { value.x += amount; }
	if (component == 1) { value.y += amount; }
	if (component == 2) { value.z += amount; }
	if (strcmp(key, "scale") == 0) {
		if (value.x < 0.1f) { value.x = 0.1f; }
		if (value.y < 0.1f) { value.y = 0.1f; }
		if (value.z < 0.1f) { value.z = 0.1f; }
	}
	snprintf(text, sizeof(text), "%.3g %.3g %.3g", value.x, value.y,
		 value.z);
	map_document_set_entity_property(brush->document,
					 brush->selected_entity, key, text);
}

static void adjust_uniform_scale(brush_state_t *brush, const float amount) {
	const map_entity_t *entity;
	vec3_t value;
	char text[96];

	entity = map_get_entity(map_document_get_map(brush->document),
				brush->selected_entity);
	value = (vec3_t){1.0f, 1.0f, 1.0f};
	map_entity_get_vec3(entity, "scale", &value);
	value.x += amount;
	value.y += amount;
	value.z += amount;
	if (value.x < 0.1f) { value.x = 0.1f; }
	if (value.y < 0.1f) { value.y = 0.1f; }
	if (value.z < 0.1f) { value.z = 0.1f; }
	snprintf(text, sizeof(text), "%.3g %.3g %.3g", value.x, value.y,
		 value.z);
	map_document_set_entity_property(brush->document,
					 brush->selected_entity, "scale", text);
}

static void
update_camera(brush_state_t *brush, engine_t *engine, input_t *input) {
	const vec3_t world_up = {0.0f, 1.0f, 0.0f};
	renderer_t *renderer;
	vec3_t right;
	vec3_t up;
	float mouse_x;
	float mouse_y;
	float delta_x;
	float delta_y;
	float wheel_y;
	float cosine_pitch;
	int width;
	int height;
	bool inside;

	renderer = engine_get_renderer(engine);
	renderer_get_size(renderer, &width, &height);
	input_get_mouse_position(input, &mouse_x, &mouse_y);
	input_get_mouse_delta(input, &delta_x, &delta_y);
	input_get_mouse_wheel(input, NULL, &wheel_y);
	inside = mouse_x >= 266.0f && mouse_x <= (float)width - 350.0f &&
		 mouse_y >= 58.0f && mouse_y <= (float)height - 34.0f;
	if (inside &&
	    input_mouse_button_down(input, INPUT_MOUSE_BUTTON_RIGHT)) {
		brush->camera_yaw -= delta_x * 0.008f;
		brush->camera_pitch -= delta_y * 0.008f;
		if (brush->camera_pitch < -1.45f) {
			brush->camera_pitch = -1.45f;
		}
		if (brush->camera_pitch > 1.45f) {
			brush->camera_pitch = 1.45f;
		}
	}
	cosine_pitch = cosf(brush->camera_pitch);
	brush->camera.forward = vec3_normalize(
		vec3_create(cosine_pitch * sinf(brush->camera_yaw),
			    sinf(brush->camera_pitch),
			    -cosine_pitch * cosf(brush->camera_yaw)));
	right = vec3_normalize(vec3_cross(brush->camera.forward, world_up));
	up = vec3_normalize(vec3_cross(right, brush->camera.forward));
	if (inside &&
	    input_mouse_button_down(input, INPUT_MOUSE_BUTTON_MIDDLE)) {
		brush->camera_target = vec3_add(
			brush->camera_target,
			vec3_scale(right,
				   -delta_x * brush->camera_distance * 0.002f));
		brush->camera_target = vec3_add(
			brush->camera_target,
			vec3_scale(up,
				   delta_y * brush->camera_distance * 0.002f));
	}
	if (inside && wheel_y != 0.0f) {
		brush->camera_distance *= expf(-wheel_y * 0.12f);
		if (brush->camera_distance < 1.0f) {
			brush->camera_distance = 1.0f;
		}
		if (brush->camera_distance > 100.0f) {
			brush->camera_distance = 100.0f;
		}
	}
	brush->camera.position = vec3_subtract(
		brush->camera_target,
		vec3_scale(brush->camera.forward, brush->camera_distance));
	brush->camera.up = world_up;
}

static render_view_t
create_render_view(brush_state_t *brush, const int width, const int height) {
	render_view_t view;
	float aspect;

	aspect = width > 0 && height > 0 ? (float)width / (float)height : 1.0f;
	view.view = camera_get_view(&brush->camera);
	view.projection = camera_get_projection(&brush->camera, aspect);
	view.light_view_projection = mat4_identity();
	view.light_direction = vec3_normalize(vec3_create(-0.4f, -1.0f, -0.3f));
	view.light_color = vec3_create(1.0f, 0.96f, 0.90f);
	view.light_intensity = 1.15f;
	return view;
}

static void draw_scene(brush_state_t *brush,
		       renderer_t *renderer,
		       const render_view_t *view) {
	const renderer_color_t grid = {0.19f, 0.21f, 0.24f, 1.0f};
	const renderer_color_t axis = {0.40f, 0.42f, 0.46f, 1.0f};
	const renderer_color_t point = {0.25f, 0.62f, 0.95f, 1.0f};
	const map_t *map;
	const map_entity_t *entity;
	const char *model_path;
	const char *material_path;
	mesh_t *mesh;
	material_t *material;
	transform_t transform;
	mat4_t model;
	vec3_t angles;
	vec3_t start;
	vec3_t end;
	char error[512];
	int line;
	size_t count;
	size_t index;

	if (brush->document == NULL) { return; }
	map = map_document_get_map(brush->document);
	count = map_get_entity_count(map);
	for (index = 0; index < count; index++) {
		entity = map_get_entity(map, index);
		model_path = map_entity_get_property(entity, "model");
		if (model_path == NULL) { continue; }
		mesh = asset_manager_load_mesh(brush->assets, model_path, error,
					       sizeof(error));
		if (mesh == NULL) { continue; }
		material_path = map_entity_get_property(entity, "material");
		material = material_path == NULL
				   ? NULL
				   : asset_manager_load_material(
					     brush->assets, material_path,
					     error, sizeof(error));
		if (material == NULL) { material = &brush->fallback_material; }
		transform = transform_create();
		map_entity_get_vec3(entity, "origin", &transform.position);
		angles = vec3_create(0.0f, 0.0f, 0.0f);
		map_entity_get_vec3(entity, "angles", &angles);
		transform.rotation = vec3_scale(angles, PI / 180.0f);
		map_entity_get_vec3(entity, "scale", &transform.scale);
		model = transform_get_matrix(&transform);
		renderer_draw_mesh_unshadowed(renderer, mesh, material, &model,
					      view);
	}
	renderer_begin_debug_lines(renderer);
	for (line = -20; line <= 20; line++) {
		start = vec3_create((float)line, 0.0f, -20.0f);
		end = vec3_create((float)line, 0.0f, 20.0f);
		renderer_add_debug_line(renderer, start, end,
					line == 0 ? axis : grid);
		start = vec3_create(-20.0f, 0.0f, (float)line);
		end = vec3_create(20.0f, 0.0f, (float)line);
		renderer_add_debug_line(renderer, start, end,
					line == 0 ? axis : grid);
	}
	for (index = 0; index < count; index++) {
		entity = map_get_entity(map, index);
		start = vec3_create(0.0f, 0.0f, 0.0f);
		map_entity_get_vec3(entity, "origin", &start);
		end = vec3_add(start, vec3_create(0.0f, 0.35f, 0.0f));
		renderer_add_debug_line(renderer, start, end, point);
		renderer_add_debug_line(
			renderer,
			vec3_add(start, vec3_create(-0.18f, 0.0f, 0.0f)),
			vec3_add(start, vec3_create(0.18f, 0.0f, 0.0f)), point);
		renderer_add_debug_line(
			renderer,
			vec3_add(start, vec3_create(0.0f, 0.0f, -0.18f)),
			vec3_add(start, vec3_create(0.0f, 0.0f, 0.18f)), point);
	}
	if (brush->has_selection) {
		entity = map_get_entity(map, brush->selected_entity);
		start = vec3_create(0.0f, 0.0f, 0.0f);
		map_entity_get_vec3(entity, "origin", &start);
		renderer_add_debug_line(
			renderer, start,
			vec3_add(start, vec3_create(2.0f, 0.0f, 0.0f)),
			(renderer_color_t){1.0f, 0.18f, 0.12f, 1.0f});
		renderer_add_debug_line(
			renderer, start,
			vec3_add(start, vec3_create(0.0f, 2.0f, 0.0f)),
			(renderer_color_t){0.20f, 1.0f, 0.25f, 1.0f});
		renderer_add_debug_line(
			renderer, start,
			vec3_add(start, vec3_create(0.0f, 0.0f, 2.0f)),
			(renderer_color_t){0.18f, 0.42f, 1.0f, 1.0f});
	}
	renderer_flush_debug_lines(renderer, view);
}

static bool project_point(const brush_state_t *brush,
			  const vec3_t point,
			  const int width,
			  const int height,
			  float *screen_x,
			  float *screen_y) {
	const vec3_t world_up = {0.0f, 1.0f, 0.0f};
	vec3_t relative;
	vec3_t right;
	vec3_t up;
	float depth;
	float tangent;
	float aspect;
	float ndc_x;
	float ndc_y;

	if (width <= 0 || height <= 0) { return false; }
	relative = vec3_subtract(point, brush->camera.position);
	depth = vec3_dot(relative, brush->camera.forward);
	if (depth <= brush->camera.near_plane) { return false; }
	right = vec3_normalize(vec3_cross(brush->camera.forward, world_up));
	up = vec3_normalize(vec3_cross(right, brush->camera.forward));
	tangent = tanf(brush->camera.field_of_view * PI / 360.0f);
	aspect = (float)width / (float)height;
	ndc_x = vec3_dot(relative, right) / (depth * tangent * aspect);
	ndc_y = vec3_dot(relative, up) / (depth * tangent);
	if (ndc_x < -1.0f || ndc_x > 1.0f || ndc_y < -1.0f || ndc_y > 1.0f) {
		return false;
	}
	if (screen_x != NULL) { *screen_x = (ndc_x + 1.0f) * 0.5f * width; }
	if (screen_y != NULL) { *screen_y = (1.0f - ndc_y) * 0.5f * height; }
	return true;
}

static void update_viewport_interaction(brush_state_t *brush,
					input_t *input,
					const int width,
					const int height) {
	const vec3_t world_up = {0.0f, 1.0f, 0.0f};
	const map_t *map;
	const map_entity_t *entity;
	vec3_t origin;
	vec3_t right;
	vec3_t up;
	vec3_t ray;
	vec3_t position;
	float mouse_x;
	float mouse_y;
	float screen_x;
	float screen_y;
	float nearest;
	float distance;
	float ndc_x;
	float ndc_y;
	float tangent;
	float t;
	char value[96];
	size_t picked;
	size_t count;
	size_t index;
	bool found;
	bool inside;

	if (brush->document == NULL || brush->dialog_open || width <= 0 ||
	    height <= 0) {
		return;
	}
	input_get_mouse_position(input, &mouse_x, &mouse_y);
	inside = mouse_x >= 266.0f && mouse_x <= (float)width - 350.0f &&
		 mouse_y >= 100.0f && mouse_y <= (float)height - 34.0f;
	map = map_document_get_map(brush->document);
	count = map_get_entity_count(map);
	if (inside &&
	    input_mouse_button_pressed(input, INPUT_MOUSE_BUTTON_LEFT)) {
		found = false;
		nearest = 18.0f;
		picked = 0;
		for (index = 0; index < count; index++) {
			entity = map_get_entity(map, index);
			origin = vec3_create(0.0f, 0.0f, 0.0f);
			map_entity_get_vec3(entity, "origin", &origin);
			if (!project_point(brush, origin, width, height,
					   &screen_x, &screen_y)) {
				continue;
			}
			distance = sqrtf(
				(mouse_x - screen_x) * (mouse_x - screen_x) +
				(mouse_y - screen_y) * (mouse_y - screen_y));
			if (distance < nearest) {
				nearest = distance;
				picked = index;
				found = true;
			}
		}
		if (found) {
			brush->selected_entity = picked;
			brush->has_selection = true;
			brush->dragging_entity = true;
			brush->drag_transaction =
				map_document_begin_transaction(brush->document);
		} else {
			brush->has_selection = false;
		}
	}
	if (brush->dragging_entity &&
	    input_mouse_button_down(input, INPUT_MOUSE_BUTTON_LEFT) &&
	    brush->has_selection) {
		entity = map_get_entity(map_document_get_map(brush->document),
					brush->selected_entity);
		origin = vec3_create(0.0f, 0.0f, 0.0f);
		map_entity_get_vec3(entity, "origin", &origin);
		right = vec3_normalize(
			vec3_cross(brush->camera.forward, world_up));
		up = vec3_normalize(vec3_cross(right, brush->camera.forward));
		tangent = tanf(brush->camera.field_of_view * PI / 360.0f);
		ndc_x = mouse_x / (float)width * 2.0f - 1.0f;
		ndc_y = 1.0f - mouse_y / (float)height * 2.0f;
		ray = vec3_add(
			brush->camera.forward,
			vec3_add(vec3_scale(right, ndc_x * tangent *
							   (float)width /
							   (float)height),
				 vec3_scale(up, ndc_y * tangent)));
		ray = vec3_normalize(ray);
		if (fabsf(ray.y) > 0.0001f) {
			t = (origin.y - brush->camera.position.y) / ray.y;
			if (t > 0.0f) {
				position = vec3_add(brush->camera.position,
						    vec3_scale(ray, t));
				position.x = roundf(position.x * 4.0f) / 4.0f;
				position.z = roundf(position.z * 4.0f) / 4.0f;
				snprintf(value, sizeof(value), "%.3g %.3g %.3g",
					 position.x, origin.y, position.z);
				map_document_set_entity_property(
					brush->document, brush->selected_entity,
					"origin", value);
			}
		}
	}
	if (brush->dragging_entity &&
	    input_mouse_button_released(input, INPUT_MOUSE_BUTTON_LEFT)) {
		brush->dragging_entity = false;
		if (brush->drag_transaction) {
			map_document_end_transaction(brush->document);
			brush->drag_transaction = false;
		}
	}
}

static void focus_camera(brush_state_t *brush) {
	const map_t *map;
	const map_entity_t *entity;
	vec3_t origin;
	vec3_t total;
	size_t count;
	size_t index;

	if (brush->document == NULL) { return; }
	map = map_document_get_map(brush->document);
	if (brush->has_selection) {
		entity = map_get_entity(map, brush->selected_entity);
		origin = vec3_create(0.0f, 0.0f, 0.0f);
		map_entity_get_vec3(entity, "origin", &origin);
		brush->camera_target = origin;
		brush->camera_distance = 8.0f;
		return;
	}
	count = map_get_entity_count(map);
	if (count == 0) { return; }
	total = vec3_create(0.0f, 0.0f, 0.0f);
	for (index = 0; index < count; index++) {
		entity = map_get_entity(map, index);
		origin = vec3_create(0.0f, 0.0f, 0.0f);
		map_entity_get_vec3(entity, "origin", &origin);
		total = vec3_add(total, origin);
	}
	brush->camera_target = vec3_scale(total, 1.0f / (float)count);
	brush->camera_distance = 16.0f;
}

static void shutdown(engine_t *engine, void *user_data) {
	brush_state_t *brush;

	(void)engine;
	brush = user_data;
	ui_context_destroy(brush->ui);
	renderer_font_destroy(brush->font);
	asset_manager_destroy(brush->assets);
	map_document_destroy(brush->document);
	free(brush->path);
	free(brush->dialog_path);
	SDL_DestroyMutex(brush->dialog_mutex);
}

static void begin_dialog(brush_state_t *brush,
			 const brush_dialog_action_t action) {
	brush->dialog_open = true;
	brush->dialog_action = action;
	if (action == BRUSH_DIALOG_OPEN) {
		SDL_ShowOpenFileDialog(dialog_callback, brush,
				       SDL_GetKeyboardFocus(), &map_filter, 1,
				       NULL, false);
	} else {
		SDL_ShowSaveFileDialog(dialog_callback, brush,
				       SDL_GetKeyboardFocus(), &map_filter, 1,
				       "untitled.volmap");
	}
}

static void process_dialog(brush_state_t *brush) {
	brush_dialog_action_t action;
	map_document_t *document;
	char *path;
	char error[512];
	bool failed;

	SDL_LockMutex(brush->dialog_mutex);
	if (!brush->dialog_finished) {
		SDL_UnlockMutex(brush->dialog_mutex);
		return;
	}
	action = brush->dialog_action;
	path = brush->dialog_path;
	failed = brush->dialog_failed;
	brush->dialog_path = NULL;
	brush->dialog_finished = false;
	brush->dialog_open = false;
	brush->dialog_action = BRUSH_DIALOG_NONE;
	SDL_UnlockMutex(brush->dialog_mutex);
	if (failed) {
		snprintf(brush->status, sizeof(brush->status),
			 "File dialog failed.");
		return;
	}
	if (path == NULL) {
		snprintf(brush->status, sizeof(brush->status),
			 "File selection cancelled.");
		return;
	}
	if (action == BRUSH_DIALOG_OPEN) {
		document = map_document_load(path, error, sizeof(error));
		if (document == NULL) {
			snprintf(brush->status, sizeof(brush->status),
				 "Open failed: %.460s", error);
			free(path);
			return;
		}
		map_document_destroy(brush->document);
		brush->document = document;
		brush->has_selection = false;
		focus_camera(brush);
	} else if (!map_document_save(brush->document, path, error,
				      sizeof(error))) {
		snprintf(brush->status, sizeof(brush->status),
			 "Save failed: %.460s", error);
		free(path);
		return;
	}
	free(brush->path);
	brush->path = path;
	snprintf(brush->status, sizeof(brush->status), "%s %s",
		 action == BRUSH_DIALOG_OPEN ? "Opened" : "Saved",
		 display_name(path));
}

static void create_map(brush_state_t *brush) {
	map_document_t *document;

	document = map_document_create();
	if (document == NULL) {
		snprintf(brush->status, sizeof(brush->status),
			 "Could not create a new map.");
		return;
	}
	map_document_destroy(brush->document);
	brush->document = document;
	brush->has_selection = false;
	free(brush->path);
	brush->path = NULL;
	snprintf(brush->status, sizeof(brush->status),
		 "Created an untitled map.");
}

static void save_map(brush_state_t *brush) {
	char error[512];

	if (brush->path == NULL) {
		begin_dialog(brush, BRUSH_DIALOG_SAVE);
		return;
	}
	if (map_document_save(brush->document, NULL, error, sizeof(error))) {
		snprintf(brush->status, sizeof(brush->status), "Saved %s",
			 display_name(brush->path));
	} else {
		snprintf(brush->status, sizeof(brush->status),
			 "Save failed: %.460s", error);
	}
}

static void SDLCALL dialog_callback(void *user_data,
				    const char *const *files,
				    const int filter) {
	brush_state_t *brush;

	(void)filter;
	brush = user_data;
	SDL_LockMutex(brush->dialog_mutex);
	brush->dialog_failed = files == NULL;
	if (files != NULL && files[0] != NULL) {
		brush->dialog_path = duplicate_string(files[0]);
		if (brush->dialog_path == NULL) { brush->dialog_failed = true; }
	}
	brush->dialog_finished = true;
	SDL_UnlockMutex(brush->dialog_mutex);
}

static char *duplicate_string(const char *text) {
	char *copy;
	size_t length;

	if (text == NULL) { return NULL; }
	length = strlen(text);
	copy = malloc(length + 1);
	if (copy != NULL) { memcpy(copy, text, length + 1); }
	return copy;
}

static const char *display_name(const char *path) {
	const char *slash;
	const char *backslash;

	if (path == NULL) { return "Untitled Map"; }
	slash = strrchr(path, '/');
	backslash = strrchr(path, '\\');
	if (slash == NULL || (backslash != NULL && backslash > slash)) {
		slash = backslash;
	}
	return slash == NULL ? path : slash + 1;
}
