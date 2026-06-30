#pragma once

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_rtc.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/popup.h>
#include <input/input.h>
#include <notification/notification_messages.h>
#include <storage/storage.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "epaper.h"

#define TAG             "MultiTimer"
#define MAX_TIMERS      20
#define TIMER_DATA_FILE APP_DATA_PATH("timers.dat")
#define SETTINGS_FILE   APP_DATA_PATH("settings.dat")
#define DEFAULT_EPAPER_REFRESH_SECONDS 30
#define DEFAULT_ALARM_DURATION_SECONDS 10
#define SETTINGS_TIME_STEP_SECONDS 10
#define SETTINGS_EPAPER_REFRESH_MAX_SECONDS 300
#define SETTINGS_ALARM_DURATION_MAX_SECONDS 120

#define TIMER_STORAGE_MAGIC   0x54494D52
#define TIMER_STORAGE_LEGACY_VERSION 1
#define TIMER_STORAGE_V2             2
#define TIMER_STORAGE_V3             3
#define TIMER_STORAGE_V4             4
#define TIMER_STORAGE_V5             5
#define TIMER_STORAGE_V6             6
#define TIMER_STORAGE_V7             7
#define TIMER_STORAGE_VERSION        8
#define TIMER_STORAGE_LEGACY_MAX_TIMERS 10

#define SETTINGS_MAGIC   0x53455453
#define SETTINGS_VERSION 1

typedef enum {
    MultiTimerCustomEventEpaperRefresh = 100,
    MultiTimerCustomEventEpaperSplash,
    MultiTimerCustomEventEpaperClear,
} MultiTimerCustomEvent;

typedef enum {
    EinkExitClean = 0,
    EinkExitLogo,
    EinkExitNothing,
    EinkExitCount,
} EinkExitAction;

typedef enum {
    MultiTimerViewSubmenu,
    MultiTimerViewClock,
    MultiTimerViewTimerSetup,
    MultiTimerViewTimerRunning,
    MultiTimerViewTimerList,
    MultiTimerViewTimerFinished,
    MultiTimerViewWelcomePopup,
    MultiTimerViewSettings,
    MultiTimerViewEinkSettings,
    MultiTimerViewDeleteTimer,
} MultiTimerView;

typedef enum {
    MultiTimerMenuClock = 1,
    MultiTimerMenuSavedTimerBase = 100,
    MultiTimerMenuAddTimer = 1000,
    MultiTimerMenuActiveTimers,
    MultiTimerMenuSettings,
} MultiTimerMenuIndex;

typedef enum {
    TimerStateSetup,
    TimerStateRunning,
    TimerStatePaused,
    TimerStateFinished,
} TimerState;

typedef struct {
    bool active;
    char name[32];
    uint32_t start_timestamp;
    uint32_t duration_seconds;
    TimerState state;
} TimerData;

typedef struct {
    bool used;
    char name[32];
    uint32_t duration_seconds;
} SavedTimerData;

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t count;
    TimerData timers[MAX_TIMERS];
    uint32_t setup_hours;
    uint32_t setup_minutes;
    uint32_t setup_seconds;
    uint8_t setup_selection;
    bool backlight_on;
    bool show_on_eink;
    bool epaper_partial_mode;
    bool epaper_invert_colors;
    uint32_t epaper_refresh_seconds;
    uint32_t alarm_duration_seconds;
    uint32_t saved_count;
    SavedTimerData saved_timers[MAX_TIMERS];
    uint8_t epaper_model;
} TimerStorage;

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t count;
    TimerData timers[MAX_TIMERS];
    uint32_t setup_hours;
    uint32_t setup_minutes;
    uint32_t setup_seconds;
    uint8_t setup_selection;
    bool backlight_on;
    bool show_on_eink;
    bool epaper_partial_mode;
    bool epaper_invert_colors;
    uint32_t epaper_refresh_seconds;
    uint8_t epaper_model_v7_broken;
    uint32_t alarm_duration_seconds;
    uint32_t saved_count;
    SavedTimerData saved_timers[MAX_TIMERS];
} TimerStorageV7Broken;

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint8_t show_on_eink;
    uint8_t backlight_on;
    uint8_t epaper_partial_mode;
    uint8_t epaper_invert_colors;
    uint8_t epaper_model;
    uint8_t rotate_screen;
    uint8_t clear_timers_on_exit;
    uint8_t epaper_on_exit;
    uint32_t epaper_refresh_seconds;
    uint32_t alarm_duration_seconds;
} AppSettingsFile;

typedef struct MultiTimerApp MultiTimerApp;

struct MultiTimerApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    View* clock_view;
    View* timer_setup_view;
    View* timer_running_view;
    View* timer_list_view;
    View* timer_finished_view;
    View* timer_delete_view;
    Popup* welcome_popup;
    VariableItemList* settings_list;
    VariableItemList* eink_settings_list;
    NotificationApp* notifications;
    Storage* storage;

    TimerStorage timer_storage;

    TimerState state;
    uint32_t setup_hours;
    uint32_t setup_minutes;
    uint32_t setup_seconds;
    uint8_t setup_selection;
    char timer_name[32];

    int selected_timer_index;
    int selected_saved_timer_index;
    int delete_saved_timer_index;
    int timer_list_selected_index;
    int timer_list_scroll;

    bool epaper_ready;
    uint32_t last_epaper_update_timestamp;
    uint32_t epaper_refresh_seconds;
    bool backlight_on;
    bool show_on_eink;
    bool epaper_partial_mode;
    bool epaper_invert_colors;
    bool rotate_screen;
    bool clear_timers_on_exit;
    uint8_t epaper_on_exit;
    char epaper_on_exit_text[16];
    char epaper_refresh_text[16];
    uint8_t epaper_model;

    uint32_t alarm_duration_seconds;
    uint32_t alarm_until_timestamp;
    uint32_t last_alarm_timestamp;
    char alarm_duration_text[16];

    bool clock_screen_active;
    bool setup_screen_active;
    bool timer_running;
};

extern MultiTimerApp* multitimer_active_app;
extern bool eink_settings_rebuilding;

void save_timer_data(MultiTimerApp* app);
void load_timer_data(MultiTimerApp* app);
void multitimer_load_settings(MultiTimerApp* app);
void multitimer_save_settings(MultiTimerApp* app);

uint32_t get_remaining_seconds(TimerData* timer);
void check_expired_timers(MultiTimerApp* app);
void multitimer_update_alarm(MultiTimerApp* app);
int count_active_timers(MultiTimerApp* app);
int get_active_timer_slot_by_row(MultiTimerApp* app, int row);
int get_active_timer_row_for_slot(MultiTimerApp* app, int slot);
void deactivate_timer(MultiTimerApp* app, int slot);
void multitimer_clear_active_timers(MultiTimerApp* app);
void navigate_after_timer_removed(MultiTimerApp* app);
void timer_list_prepare_selection(MultiTimerApp* app, int prefer_slot);
bool multitimer_has_finished_timers(MultiTimerApp* app);
int multitimer_count_finished_timers(MultiTimerApp* app);
void finished_popup_callback(void* context);
bool start_timer(MultiTimerApp* app, uint32_t duration_seconds, const char* name);
int save_new_timer_preset(MultiTimerApp* app, uint32_t duration_seconds);

void multitimer_epaper_normalize_model(MultiTimerApp* app);
void multitimer_epaper_refresh_now(MultiTimerApp* app);
void multitimer_request_epaper_refresh(MultiTimerApp* app);
void multitimer_epaper_show_stopped(MultiTimerApp* app, uint32_t duration_seconds);
void multitimer_epaper_show_setup(MultiTimerApp* app);
void multitimer_epaper_show_setup_throttled(MultiTimerApp* app);
void multitimer_epaper_show_splash(MultiTimerApp* app);
void multitimer_epaper_show_clear(MultiTimerApp* app);
void multitimer_epaper_apply_exit_action(MultiTimerApp* app);
void multitimer_request_epaper_splash(MultiTimerApp* app);
void multitimer_request_epaper_clear(MultiTimerApp* app);
void multitimer_epaper_show_clock(MultiTimerApp* app);
void multitimer_epaper_update(MultiTimerApp* app, bool force);

void format_time(uint32_t seconds, char* buffer, size_t buffer_size);
MultiTimerApp* multitimer_view_get_app(void* model);
void timer_setup_refresh(MultiTimerApp* app);

void multitimer_apply_backlight(MultiTimerApp* app);
void multitimer_apply_screen_rotation(MultiTimerApp* app);
void settings_refresh(MultiTimerApp* app);
void eink_settings_refresh(MultiTimerApp* app);
uint32_t settings_previous_callback(void* context);
uint32_t eink_settings_previous_callback(void* context);
void settings_enter_callback(void* context, uint32_t index);

void submenu_rebuild(MultiTimerApp* app);
void submenu_callback(void* context, uint32_t index);
void welcome_popup_callback(void* context);
void goodbye_popup_callback(void* context);
void multitimer_show_splash_popup(
    MultiTimerApp* app,
    PopupCallback callback,
    uint32_t timeout_ms,
    bool show_epaper_splash);

void clock_draw_callback(Canvas* canvas, void* model);
bool clock_input_callback(InputEvent* event, void* context);
void timer_setup_draw_callback(Canvas* canvas, void* model);
bool timer_setup_input_callback(InputEvent* event, void* context);
void timer_running_draw_callback(Canvas* canvas, void* model);
bool timer_running_input_callback(InputEvent* event, void* context);
void timer_delete_draw_callback(Canvas* canvas, void* model);
bool timer_delete_input_callback(InputEvent* event, void* context);
void timer_list_draw_callback(Canvas* canvas, void* model);
bool timer_list_input_callback(InputEvent* event, void* context);

MultiTimerApp* multitimer_app_alloc(void);
void multitimer_app_free(MultiTimerApp* app);
void timer_tick_callback(void* context);
bool multitimer_navigation_event_callback(void* context);
bool multitimer_custom_event_callback(void* context, uint32_t event);
