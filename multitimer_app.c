#include "multitimer.h"
#include "epaper42_bw.h"
#include <multitimerext_icons.h>

MultiTimerApp* multitimer_active_app = NULL;

void timer_tick_callback(void* context) {
    MultiTimerApp* app = context;
    if(!app) return;

    check_expired_timers(app);
    multitimer_update_alarm(app);

    if(app->clock_screen_active) {
        uint32_t now = furi_hal_rtc_get_timestamp();
        if(now - app->last_epaper_update_timestamp >= app->epaper_refresh_seconds) {
            multitimer_epaper_show_clock(app);
        }
    } else if(app->setup_screen_active) {
        multitimer_epaper_show_setup_throttled(app);
    } else if(app->timer_storage.count > 0) {
        multitimer_epaper_update(app, false);
    }

    if(app->timer_storage.count > 0) {
        if(app->timer_running_view) {
            with_view_model(
                app->timer_running_view, MultiTimerApp** model, { UNUSED(model); }, true);
        }
        if(app->timer_list_view) {
            with_view_model(app->timer_list_view, MultiTimerApp** model, { UNUSED(model); }, true);
        }
    }

    if(app->clock_view) {
        with_view_model(app->clock_view, MultiTimerApp** model, { UNUSED(model); }, true);
    }
}
bool multitimer_navigation_event_callback(void* context) {
    MultiTimerApp* app = context;
    if(!app) return true;

    if(app->clear_timers_on_exit) {
        multitimer_clear_active_timers(app);
    }
    save_timer_data(app);
    multitimer_epaper_apply_exit_action(app);
    multitimer_show_splash_popup(app, goodbye_popup_callback, 3000, false);
    return true;
}

bool multitimer_custom_event_callback(void* context, uint32_t event) {
    MultiTimerApp* app = context;
    if(!app) return false;

    if(event == MultiTimerCustomEventEpaperRefresh) {
        multitimer_epaper_refresh_now(app);
        return true;
    }

    if(event == MultiTimerCustomEventEpaperSplash) {
        multitimer_epaper_show_splash(app);
        return true;
    }

    if(event == MultiTimerCustomEventEpaperClear) {
        multitimer_epaper_show_clear(app);
        return true;
    }

    return false;
}

// App initialization
MultiTimerApp* multitimer_app_alloc() {
    MultiTimerApp* app = malloc(sizeof(MultiTimerApp));
    if(!app) {
        return NULL;
    }

    // Initialize all pointers to NULL first
    memset(app, 0, sizeof(MultiTimerApp));

    multitimer_active_app = app;

    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->storage = furi_record_open(RECORD_STORAGE);

    if(!app->gui || !app->notifications || !app->storage) {
        FURI_LOG_E(TAG, "Failed to open required records");
        multitimer_app_free(app);
        return NULL;
    }

    app->view_dispatcher = view_dispatcher_alloc();
    if(!app->view_dispatcher) {
        FURI_LOG_E(TAG, "Failed to allocate view dispatcher");
        multitimer_app_free(app);
        return NULL;
    }

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, multitimer_navigation_event_callback);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, multitimer_custom_event_callback);
    view_dispatcher_set_tick_event_callback(app->view_dispatcher, timer_tick_callback, 1000);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Initialize submenu
    app->submenu = submenu_alloc();
    if(!app->submenu) {
        FURI_LOG_E(TAG, "Failed to allocate submenu");
        multitimer_app_free(app);
        return NULL;
    }

    view_dispatcher_add_view(
        app->view_dispatcher, MultiTimerViewSubmenu, submenu_get_view(app->submenu));

    app->clock_view = view_alloc();
    if(!app->clock_view) {
        FURI_LOG_E(TAG, "Failed to allocate clock view");
        multitimer_app_free(app);
        return NULL;
    }
    view_set_context(app->clock_view, app);
    view_allocate_model(app->clock_view, ViewModelTypeLocking, sizeof(MultiTimerApp*));
    with_view_model(app->clock_view, MultiTimerApp** model, { *model = app; }, true);
    view_set_draw_callback(app->clock_view, clock_draw_callback);
    view_set_input_callback(app->clock_view, clock_input_callback);
    view_dispatcher_add_view(app->view_dispatcher, MultiTimerViewClock, app->clock_view);

    // Initialize timer setup view
    app->timer_setup_view = view_alloc();
    if(!app->timer_setup_view) {
        FURI_LOG_E(TAG, "Failed to allocate timer setup view");
        multitimer_app_free(app);
        return NULL;
    }
    view_set_context(app->timer_setup_view, app);
    view_allocate_model(app->timer_setup_view, ViewModelTypeLocking, sizeof(MultiTimerApp*));
    with_view_model(
        app->timer_setup_view, MultiTimerApp** model, { *model = app; }, true);
    view_set_draw_callback(app->timer_setup_view, timer_setup_draw_callback);
    view_set_input_callback(app->timer_setup_view, timer_setup_input_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, MultiTimerViewTimerSetup, app->timer_setup_view);

    // Initialize timer running view
    app->timer_running_view = view_alloc();
    if(!app->timer_running_view) {
        FURI_LOG_E(TAG, "Failed to allocate timer running view");
        multitimer_app_free(app);
        return NULL;
    }
    view_set_context(app->timer_running_view, app);
    view_allocate_model(app->timer_running_view, ViewModelTypeLocking, sizeof(MultiTimerApp*));
    with_view_model(
        app->timer_running_view, MultiTimerApp** model, { *model = app; }, true);
    view_set_draw_callback(app->timer_running_view, timer_running_draw_callback);
    view_set_input_callback(app->timer_running_view, timer_running_input_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, MultiTimerViewTimerRunning, app->timer_running_view);

    // Initialize timer list view
    app->timer_list_view = view_alloc();
    if(!app->timer_list_view) {
        FURI_LOG_E(TAG, "Failed to allocate timer list view");
        multitimer_app_free(app);
        return NULL;
    }
    view_set_context(app->timer_list_view, app);
    view_allocate_model(app->timer_list_view, ViewModelTypeLocking, sizeof(MultiTimerApp*));
    with_view_model(
        app->timer_list_view, MultiTimerApp** model, { *model = app; }, true);
    view_set_draw_callback(app->timer_list_view, timer_list_draw_callback);
    view_set_input_callback(app->timer_list_view, timer_list_input_callback);
    view_dispatcher_add_view(app->view_dispatcher, MultiTimerViewTimerList, app->timer_list_view);

    app->timer_delete_view = view_alloc();
    if(!app->timer_delete_view) {
        FURI_LOG_E(TAG, "Failed to allocate timer delete view");
        multitimer_app_free(app);
        return NULL;
    }
    view_set_context(app->timer_delete_view, app);
    view_allocate_model(app->timer_delete_view, ViewModelTypeLocking, sizeof(MultiTimerApp*));
    with_view_model(
        app->timer_delete_view, MultiTimerApp** model, { *model = app; }, true);
    view_set_draw_callback(app->timer_delete_view, timer_delete_draw_callback);
    view_set_input_callback(app->timer_delete_view, timer_delete_input_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, MultiTimerViewDeleteTimer, app->timer_delete_view);

    app->settings_list = variable_item_list_alloc();
    if(!app->settings_list) {
        FURI_LOG_E(TAG, "Failed to allocate settings list");
        multitimer_app_free(app);
        return NULL;
    }
    view_set_previous_callback(variable_item_list_get_view(app->settings_list), settings_previous_callback);
    variable_item_list_set_enter_callback(app->settings_list, settings_enter_callback, app);
    view_dispatcher_add_view(
        app->view_dispatcher, MultiTimerViewSettings, variable_item_list_get_view(app->settings_list));

    app->eink_settings_list = variable_item_list_alloc();
    if(!app->eink_settings_list) {
        FURI_LOG_E(TAG, "Failed to allocate e-ink settings list");
        multitimer_app_free(app);
        return NULL;
    }
    view_set_previous_callback(
        variable_item_list_get_view(app->eink_settings_list), eink_settings_previous_callback);
    view_dispatcher_add_view(
        app->view_dispatcher,
        MultiTimerViewEinkSettings,
        variable_item_list_get_view(app->eink_settings_list));

    // Initialize welcome popup
    app->welcome_popup = popup_alloc();
    if(!app->welcome_popup) {
        FURI_LOG_E(TAG, "Failed to allocate welcome popup");
        multitimer_app_free(app);
        return NULL;
    }
    popup_set_context(app->welcome_popup, app);
    view_dispatcher_add_view(
        app->view_dispatcher, MultiTimerViewWelcomePopup, popup_get_view(app->welcome_popup));

    // Initialize timer state
    app->state = TimerStateSetup;
    app->setup_hours = 0;
    app->setup_minutes = 0;
    app->setup_seconds = 0;
    app->setup_selection = 0;
    app->selected_timer_index = -1;
    app->selected_saved_timer_index = -1;
    app->delete_saved_timer_index = -1;
    app->timer_list_selected_index = -1;
    app->timer_list_scroll = 0;
    app->last_epaper_update_timestamp = 0;
    app->backlight_on = true;
    app->show_on_eink = true;
    app->epaper_partial_mode = false;
    app->epaper_invert_colors = false;
    app->rotate_screen = false;
    app->clear_timers_on_exit = false;
    app->epaper_on_exit = EinkExitLogo;
    app->epaper_model = EPAPER_MODEL_DEFAULT;
    app->epaper_refresh_seconds = DEFAULT_EPAPER_REFRESH_SECONDS;
    app->alarm_duration_seconds = DEFAULT_ALARM_DURATION_SECONDS;
    app->alarm_until_timestamp = 0;
    app->last_alarm_timestamp = 0;
    app->clock_screen_active = false;
    app->setup_screen_active = false;

    // Load existing timer data
    load_timer_data(app);
    multitimer_load_settings(app);
    if(app->epaper_refresh_seconds < SETTINGS_TIME_STEP_SECONDS ||
       app->epaper_refresh_seconds > SETTINGS_EPAPER_REFRESH_MAX_SECONDS ||
       app->epaper_refresh_seconds % SETTINGS_TIME_STEP_SECONDS != 0) {
        app->epaper_refresh_seconds = DEFAULT_EPAPER_REFRESH_SECONDS;
    }
    if(app->alarm_duration_seconds < SETTINGS_TIME_STEP_SECONDS ||
       app->alarm_duration_seconds > SETTINGS_ALARM_DURATION_MAX_SECONDS ||
       app->alarm_duration_seconds % SETTINGS_TIME_STEP_SECONDS != 0) {
        app->alarm_duration_seconds = DEFAULT_ALARM_DURATION_SECONDS;
    }
    save_timer_data(app);
    check_expired_timers(app);
    multitimer_epaper_normalize_model(app);
    submenu_rebuild(app);

    multitimer_apply_backlight(app);
    multitimer_apply_screen_rotation(app);

    return app;
}

void multitimer_app_free(MultiTimerApp* app) {
    if(!app) return;

    if(app->clear_timers_on_exit) {
        multitimer_clear_active_timers(app);
    }
    save_timer_data(app);
    multitimer_save_settings(app);
    multitimer_active_app = NULL;

    // Remove views from dispatcher before freeing them
    if(app->view_dispatcher) {
        if(app->submenu) {
            view_dispatcher_remove_view(app->view_dispatcher, MultiTimerViewSubmenu);
        }
        if(app->clock_view) {
            view_dispatcher_remove_view(app->view_dispatcher, MultiTimerViewClock);
        }
        if(app->timer_setup_view) {
            view_dispatcher_remove_view(app->view_dispatcher, MultiTimerViewTimerSetup);
        }
        if(app->timer_running_view) {
            view_dispatcher_remove_view(app->view_dispatcher, MultiTimerViewTimerRunning);
        }
        if(app->timer_list_view) {
            view_dispatcher_remove_view(app->view_dispatcher, MultiTimerViewTimerList);
        }
        if(app->timer_delete_view) {
            view_dispatcher_remove_view(app->view_dispatcher, MultiTimerViewDeleteTimer);
        }
        if(app->settings_list) {
            view_dispatcher_remove_view(app->view_dispatcher, MultiTimerViewSettings);
        }
        if(app->eink_settings_list) {
            view_dispatcher_remove_view(app->view_dispatcher, MultiTimerViewEinkSettings);
        }
        if(app->welcome_popup) {
            view_dispatcher_remove_view(app->view_dispatcher, MultiTimerViewWelcomePopup);
        }
    }

    // Free individual views
    if(app->submenu) {
        submenu_free(app->submenu);
        app->submenu = NULL;
    }
    if(app->clock_view) {
        view_free(app->clock_view);
        app->clock_view = NULL;
    }
    if(app->timer_setup_view) {
        view_free(app->timer_setup_view);
        app->timer_setup_view = NULL;
    }
    if(app->timer_running_view) {
        view_free(app->timer_running_view);
        app->timer_running_view = NULL;
    }
    if(app->timer_list_view) {
        view_free(app->timer_list_view);
        app->timer_list_view = NULL;
    }
    if(app->timer_delete_view) {
        view_free(app->timer_delete_view);
        app->timer_delete_view = NULL;
    }
    if(app->settings_list) {
        variable_item_list_free(app->settings_list);
        app->settings_list = NULL;
    }
    if(app->eink_settings_list) {
        variable_item_list_free(app->eink_settings_list);
        app->eink_settings_list = NULL;
    }
    if(app->welcome_popup) {
        popup_free(app->welcome_popup);
        app->welcome_popup = NULL;
    }

    if(app->epaper_ready) {
        epaper42_bw_deinit();
        app->epaper_ready = false;
    }

    if(app->notifications) {
        notification_message(app->notifications, &sequence_display_backlight_enforce_auto);
    }

    // Free view dispatcher last
    if(app->view_dispatcher) {
        view_dispatcher_free(app->view_dispatcher);
        app->view_dispatcher = NULL;
    }

    // Close records
    if(app->storage) {
        furi_record_close(RECORD_STORAGE);
        app->storage = NULL;
    }
    if(app->notifications) {
        furi_record_close(RECORD_NOTIFICATION);
        app->notifications = NULL;
    }
    if(app->gui) {
        furi_record_close(RECORD_GUI);
        app->gui = NULL;
    }

    // Finally free the app structure itself
    free(app);
}

int32_t multitimer_app(void* p) {
    UNUSED(p);

    MultiTimerApp* app = multitimer_app_alloc();
    if(!app) {
        FURI_LOG_E(TAG, "Failed to allocate app");
        return -1;
    }

    // Start with welcome popup, or finished-timer notice if any expired while away
    if(multitimer_has_finished_timers(app)) {
        int finished_count = multitimer_count_finished_timers(app);
        char finished_text[32];
        snprintf(
            finished_text,
            sizeof(finished_text),
            "%d timer(s) finished!",
            finished_count);
        popup_set_icon(app->welcome_popup, 36, 4, &I_dolphin_welcome_56x56);
        popup_set_header(app->welcome_popup, "MultiTimer", 64, 10, AlignCenter, AlignTop);
        popup_set_text(app->welcome_popup, finished_text, 64, 42, AlignCenter, AlignTop);
        popup_disable_timeout(app->welcome_popup);
        popup_set_callback(app->welcome_popup, finished_popup_callback);
        if(app->show_on_eink) {
            multitimer_request_epaper_splash(app);
        }
    } else {
        multitimer_show_splash_popup(app, welcome_popup_callback, 3000, true);
    }

    if(multitimer_has_finished_timers(app)) {
        view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewWelcomePopup);
    }

    // Run the app
    view_dispatcher_run(app->view_dispatcher);

    multitimer_app_free(app);
    return 0;
}
