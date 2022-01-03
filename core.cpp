#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <iostream>
#include <math.h>
#include <libgen.h>

#define PY_SSIZE_T_CLEAN
#include <python3.9/Python.h>

#include <stdio.h>
#include "libretro.h"

#define VIDEO_WIDTH 1000
#define VIDEO_HEIGHT 1000
#define VIDEO_PIXELS VIDEO_WIDTH * VIDEO_HEIGHT

// Colors
#define CF_BLACK 0
#define CF_WHITE 65535
#define COLOR_BYTEOFFSET 1

static uint8_t *frame_buf;
static struct retro_log_callback logging;
static retro_log_printf_t log_cb;
static bool use_audio_cb;
static float last_aspect;
static float last_sample_rate;
char retro_base_directory[4096];
char retro_game_path[4096];

PyObject *API_Fun_Test;
PyObject *API_Fun_Run;
PyObject *API_Fun_Begin;

PyObject *pyFile;

#include "graphics.hpp" // All the graphics functions

#include "autogenerated.hpp" // Automatically generated data.

void clear(unsigned int color){
    memset(frame_buf, color, VIDEO_PIXELS * sizeof(uint32_t));
}

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
   (void)level;
   va_list va;
   va_start(va, fmt);
   vfprintf(stderr, fmt, va);
   va_end(va);
}

char** getFileNameAndPath(char *path){
    char *buffer = (char*)malloc(strlen(path)); // Buffer is large at the start, we trim it later
    char **returner = (char**)malloc(sizeof(char*) * 2); // List of two strings.
    unsigned int length = 0;
    uint8_t mode = 0; // 0 = reading extension, 1 = reading filename, 2 = reading path.
    for (unsigned int i = strlen(path); i > 0; i --){
        if (mode == 0){
            if (path[i] == '.'){
                mode = 1;
            }
        }
        else if (mode == 1){
            if (path[i] == '/'){
                char* fNameBuffer = (char*)malloc(length + 1);
                for (unsigned int x = 0; x < length; x ++){
                    fNameBuffer[x] = buffer[length - x - 1];
                }
                fNameBuffer[length] = 0; // End string with char 0
                returner[0] = fNameBuffer;
                mode = 2;
                length = 0;
            }
            else{
                buffer[length] = path[i];
                length ++;
            }
        }
        else if (mode == 2){
            buffer[length] = path[i];
            length ++;
        }
    }
    // Finished, swap buffers.
    char* fPathBuffer = (char*)malloc(length + 2); // It appears that length is 1 less than it should be. See "sloppy bugfix" below for further context.
    for (unsigned int x = 0; x <= length; x ++){
        fPathBuffer[x] = buffer[length - x]; // -1 to shift it down, this is the conversion to array-begins-at-0
    }
    fPathBuffer[length + 1] = 0; // End string with char
    fPathBuffer[0] = '/'; // Sloppy bugfix
    returner[1] = fPathBuffer;
    return returner;
}

PyObject *filename;

PyObject *RunROMFunction(char *fname){
    PyObject *func = PyObject_GetAttrString(pyFile, fname);
    if (func){
        PyObject *retval = PyObject_CallObject(func, NULL);
        if (retval != NULL){
            return retval;
        }
        else{
            printf("Error executing embedded Python function ");
            printf(fname);
            printf(".\n");
            PyErr_Print();
        }
    }
    return NULL;
}

void begin_python(char *path){
    if (PyImport_AppendInittab("lrpython", &begin_lrpython_module)){ // It returns 0 if it is successful, -1 if it fails. -1 is truthy, 0 is falsey.
        std::cout << "CANNOT ADD MODULE TO INIT TAB!" << std::endl;
    }
    Py_Initialize();
    char **fileData = getFileNameAndPath(path);
    std::cout << fileData[0] << " " << fileData[1] << std::endl;
    std::string simpleCode = "import sys\nsys.path.append('"; // Eww.
    simpleCode.append(fileData[1]); // Ugh yucky
    simpleCode.append("')"); // Retch gurgle
    PyRun_SimpleString(simpleCode.c_str()); // EWWW
    filename = PyUnicode_FromString(fileData[0]);
    free(fileData[0]);
    free(fileData[1]);
    free(fileData);
    pyFile = PyImport_Import(filename); // The function frees `filename`, this is thus memory safe
    Py_DECREF(filename); // Clear the reference. This prevents memory leaks.
}

static retro_environment_t environ_cb;

void retro_init(void)
{
   frame_buf = (uint8_t*)malloc(VIDEO_PIXELS * sizeof(uint32_t));
   const char *dir = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
   {
      snprintf(retro_base_directory, sizeof(retro_base_directory), "%s", dir);
  }
}

void retro_deinit(void)
{
   free(frame_buf);
   frame_buf = NULL;
   Py_DECREF(pyFile); // Prevent memory leaks!
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
   log_cb(RETRO_LOG_INFO, "Plugging device %u into port %u.\n", device, port);
}

void retro_get_system_info(struct retro_system_info *info)
{
   memset(info, 0, sizeof(*info));
   info->library_name     = "LibRetro Python";
   info->library_version  = "1.0"; // feature.bugfix format.
   info->need_fullpath    = true;
   info->valid_extensions = "py";
}

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   float aspect                = 0.0f;
   float sampling_rate         = 30000.0f;


   info->geometry.base_width   = VIDEO_WIDTH;
   info->geometry.base_height  = VIDEO_HEIGHT;
   info->geometry.max_width    = VIDEO_WIDTH;
   info->geometry.max_height   = VIDEO_HEIGHT;
   info->geometry.aspect_ratio = aspect;

   last_aspect                 = aspect;
   last_sample_rate            = sampling_rate;
}

static struct retro_rumble_interface rumble;

void retro_set_environment(retro_environment_t cb)
{
   environ_cb = cb;

   if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
      log_cb = logging.log;
   else
      log_cb = fallback_log;

   static const struct retro_controller_description controllers[] = {
      { "Nintendo DS", RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 0) },
   };

   static const struct retro_controller_info ports[] = {
      { controllers, 1 },
      { NULL, 0 },
   };

   cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports);
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
   audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
   audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
   input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
   input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}

static unsigned x_coord;
static unsigned y_coord;
static unsigned phase;
static int mouse_rel_x;
static int mouse_rel_y;

void retro_reset(void)
{
    printf("Resat");
   x_coord = 0;
   y_coord = 0;
}

static void update_input(void)
{

}


static void check_variables(void)
{

}

static void audio_callback(void)
{
   for (unsigned i = 0; i < 30000 / 60; i++, phase++)
   {
      int16_t val = 0x800 * sinf(2.0f * M_PI * phase * 300.0f / 30000.0f);
      audio_cb(val, val);
   }

   phase %= 100;
}

static void audio_set_state(bool enable)
{
   (void)enable;
}

bool keyA = false;
bool keyB = false;
bool keyUp = false;
bool keyDown = false;
bool keyLeft = false;
bool keyRight = false;

void retro_run(void)
{
    clear(CF_WHITE);
    RunROMFunction("run");
    unsigned i;
    update_input();

    if ((int)input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A)){
        RunROMFunction("keydown_A");
        keyA = true;
    }
    else if (keyA){
        keyA = false;
        RunROMFunction("keyup_A");
    }

    if ((int)input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B)){
        RunROMFunction("keydown_B");
        keyB = true;
    }
    else if (keyB){
        keyB = false;
        RunROMFunction("keyup_B");
    }

    if ((int)input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP)){
        RunROMFunction("keydown_Up");
        keyUp = true;
    }
    else if (keyUp){
        keyUp = false;
        RunROMFunction("keyup_Up");
    }

    if ((int)input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN)){
        RunROMFunction("keydown_Down");
        keyDown = true;
    }
    else if (keyUp){
        keyDown = false;
        RunROMFunction("keyup_Down");
    }

    if ((int)input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT)){
        RunROMFunction("keydown_Left");
        keyLeft = true;
    }
    else if (keyUp){
        keyLeft = false;
        RunROMFunction("keyup_Left");
    }

    if ((int)input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT)){
        RunROMFunction("keydown_Right");
        keyRight = true;
    }
    else if (keyUp){
        keyRight = false;
        RunROMFunction("keyup_Right");
    }

    video_cb(frame_buf, VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_WIDTH * sizeof(uint32_t));

    bool updated = false;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated){
       check_variables();
    }
}

bool retro_load_game(const struct retro_game_info *info)
{
   struct retro_input_descriptor desc[] = {
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "Left" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "Up" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "Down" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
      { 0 },
   };

   environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      log_cb(RETRO_LOG_INFO, "XRGB8888 is not supported.\n");
      return false;
   }

   struct retro_audio_callback audio_cb = { audio_callback, audio_set_state };
   use_audio_cb = environ_cb(RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK, &audio_cb);

   check_variables();
   // Python stuff
   begin_python((char*)info -> path); // Load the Python API
   if (API_Fun_Begin){
       PyObject_CallObject(API_Fun_Begin, NULL);
   }

   (void)info;
   return true;
}

void retro_unload_game(void)
{

}

unsigned retro_get_region(void)
{
   return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
   return false;
}

size_t retro_serialize_size(void)
{
   return 0;
}

bool retro_serialize(void *data_, size_t size)
{
   return false;
}

bool retro_unserialize(const void *data_, size_t size)
{
   return false;
}

void *retro_get_memory_data(unsigned id)
{
   (void)id;
   return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
   (void)id;
   return 0;
}

void retro_cheat_reset(void)
{}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
   (void)index;
   (void)enabled;
   (void)code;
}
