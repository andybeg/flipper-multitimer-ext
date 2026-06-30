#include "multitimer.h"
#include "epaper42_bw.h"

bool eink_settings_rebuilding = false;

void multitimer_apply_backlight(MultiTimerApp* app) {
    if(!app || !app->notifications) return;

    if(app->backlight_on) {
        notification_message(app->notifications, &sequence_display_backlight_on);
        notification_message(app->notifications, &sequence_display_backlight_enforce_on);
    } else {
        notification_message(app->notifications, &sequence_display_backlight_enforce_auto);
        notification_message(app->notifications, &sequence_display_backlight_off);
    }
}

void settings_format_seconds(uint32_t seconds, char* buffer, size_t buffer_size) {
    if(seconds < 60 || seconds % 60 != 0) {
        snprintf(buffer, buffer_size, "%lu sec", seconds);
    } else {
        snprintf(buffer, buffer_size, "%lu min", seconds / 60);
    }
}

uint8_t settings_seconds_to_index(uint32_t seconds, uint32_t max_seconds) {
    if(seconds < SETTINGS_TIME_STEP_SECONDS || seconds > max_seconds ||
       seconds % SETTINGS_TIME_STEP_SECONDS != 0) {
        seconds = SETTINGS_TIME_STEP_SECONDS;
    }
    return (seconds / SETTINGS_TIME_STEP_SECONDS) - 1;
}

uint32_t settings_index_to_seconds(uint8_t index, uint32_t max_seconds) {
    uint32_t seconds = (index + 1) * SETTINGS_TIME_STEP_SECONDS;
    if(seconds > max_seconds) seconds = max_seconds;
    return seconds;
}

void settings_backlight_changed(VariableItem* item) {
    MultiTimerApp* app = variable_item_get_context(item);
    if(!app) return;

    uint8_t index = variable_item_get_current_value_index(item);
    app->backlight_on = index != 0;
    variable_item_set_current_value_text(item, app->backlight_on ? "On" : "Off");
    multitimer_apply_backlight(app);
    multitimer_save_settings(app);
}

void settings_show_on_eink_changed(VariableItem* item) {
    MultiTimerApp* app = variable_item_get_context(item);
    if(!app || eink_settings_rebuilding) return;

    uint8_t index = variable_item_get_current_value_index(item);
    bool new_value = index != 0;
    if(new_value == app->show_on_eink) return;

    app->show_on_eink = new_value;
    variable_item_set_current_value_text(item, app->show_on_eink ? "On" : "Off");

    if(!app->show_on_eink && app->epaper_ready) {
        epaper42_bw_deinit();
        app->epaper_ready = false;
    } else if(app->show_on_eink) {
        multitimer_request_epaper_refresh(app);
    }

    multitimer_save_settings(app);
}

void settings_epaper_model_changed(VariableItem* item) {
    MultiTimerApp* app = variable_item_get_context(item);
    if(!app || eink_settings_rebuilding) return;

    uint8_t index = variable_item_get_current_value_index(item);
    if(index >= EpaperModelCount) index = EPAPER_MODEL_DEFAULT;

    app->epaper_model = index;
    variable_item_set_current_value_text(
        item, epaper_model_value_label((EpaperModel)app->epaper_model));
    multitimer_epaper_normalize_model(app);
    multitimer_save_settings(app);
    multitimer_request_epaper_refresh(app);
}

void settings_epaper_mode_changed(VariableItem* item) {
    MultiTimerApp* app = variable_item_get_context(item);
    if(!app || eink_settings_rebuilding) return;

    uint8_t index = variable_item_get_current_value_index(item);
    app->epaper_partial_mode = index != 0;
    variable_item_set_current_value_text(item, app->epaper_partial_mode ? "Partial" : "Full");
    multitimer_save_settings(app);
    multitimer_request_epaper_refresh(app);
}

void settings_epaper_invert_changed(VariableItem* item) {
    MultiTimerApp* app = variable_item_get_context(item);
    if(!app || eink_settings_rebuilding) return;

    uint8_t index = variable_item_get_current_value_index(item);
    app->epaper_invert_colors = index != 0;
    variable_item_set_current_value_text(item, app->epaper_invert_colors ? "On" : "Off");
    multitimer_save_settings(app);
    multitimer_request_epaper_refresh(app);
}

void settings_epaper_refresh_changed(VariableItem* item) {
    MultiTimerApp* app = variable_item_get_context(item);
    if(!app || eink_settings_rebuilding) return;

    uint8_t index = variable_item_get_current_value_index(item);
    app->epaper_refresh_seconds =
        settings_index_to_seconds(index, SETTINGS_EPAPER_REFRESH_MAX_SECONDS);
    settings_format_seconds(
        app->epaper_refresh_seconds,
        app->epaper_refresh_text,
        sizeof(app->epaper_refresh_text));
    variable_item_set_current_value_text(item, app->epaper_refresh_text);
    multitimer_save_settings(app);
}

void settings_alarm_duration_changed(VariableItem* item) {
    MultiTimerApp* app = variable_item_get_context(item);
    if(!app) return;

    uint8_t index = variable_item_get_current_value_index(item);
    app->alarm_duration_seconds =
        settings_index_to_seconds(index, SETTINGS_ALARM_DURATION_MAX_SECONDS);
    settings_format_seconds(
        app->alarm_duration_seconds, app->alarm_duration_text, sizeof(app->alarm_duration_text));
    variable_item_set_current_value_text(item, app->alarm_duration_text);
    multitimer_save_settings(app);
}

void settings_rotate_screen_changed(VariableItem* item) {
    MultiTimerApp* app = variable_item_get_context(item);
    if(!app) return;

    uint8_t index = variable_item_get_current_value_index(item);
    app->rotate_screen = index != 0;
    variable_item_set_current_value_text(item, app->rotate_screen ? "On" : "Off");
    multitimer_apply_screen_rotation(app);
    multitimer_save_settings(app);
}

void settings_clear_timers_on_exit_changed(VariableItem* item) {
    MultiTimerApp* app = variable_item_get_context(item);
    if(!app) return;

    uint8_t index = variable_item_get_current_value_index(item);
    app->clear_timers_on_exit = index != 0;
    variable_item_set_current_value_text(item, app->clear_timers_on_exit ? "On" : "Off");
    multitimer_save_settings(app);
}

void settings_open_eink_changed(VariableItem* item) {
    UNUSED(item);
}

uint32_t settings_previous_callback(void* context) {
    UNUSED(context);
    if(multitimer_active_app) {
        multitimer_save_settings(multitimer_active_app);
    }
    return MultiTimerViewSubmenu;
}

uint32_t eink_settings_previous_callback(void* context) {
    UNUSED(context);
    if(multitimer_active_app) {
        multitimer_save_settings(multitimer_active_app);
    }
    return MultiTimerViewSettings;
}

void settings_enter_callback(void* context, uint32_t index) {
    MultiTimerApp* app = context;
    if(!app || !app->view_dispatcher) return;

    if(index == 4) {
        eink_settings_refresh(app);
        view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewEinkSettings);
    }
}

void settings_refresh(MultiTimerApp* app) {
    if(!app || !app->settings_list) return;

    variable_item_list_reset(app->settings_list);

    VariableItem* backlight_item = variable_item_list_add(
        app->settings_list, "Backlight On", 2, settings_backlight_changed, app);
    variable_item_set_current_value_index(backlight_item, app->backlight_on ? 1 : 0);
    variable_item_set_current_value_text(backlight_item, app->backlight_on ? "On" : "Off");

    uint8_t alarm_index =
        settings_seconds_to_index(app->alarm_duration_seconds, SETTINGS_ALARM_DURATION_MAX_SECONDS);
    VariableItem* alarm_item = variable_item_list_add(
        app->settings_list,
        "alarm time",
        SETTINGS_ALARM_DURATION_MAX_SECONDS / SETTINGS_TIME_STEP_SECONDS,
        settings_alarm_duration_changed,
        app);
    variable_item_set_current_value_index(alarm_item, alarm_index);
    settings_format_seconds(
        app->alarm_duration_seconds, app->alarm_duration_text, sizeof(app->alarm_duration_text));
    variable_item_set_current_value_text(alarm_item, app->alarm_duration_text);

    VariableItem* rotate_item = variable_item_list_add(
        app->settings_list, "Rotate Screen", 2, settings_rotate_screen_changed, app);
    variable_item_set_current_value_index(rotate_item, app->rotate_screen ? 1 : 0);
    variable_item_set_current_value_text(rotate_item, app->rotate_screen ? "On" : "Off");

    VariableItem* clear_timers_item = variable_item_list_add(
        app->settings_list, "Clear on exit", 2, settings_clear_timers_on_exit_changed, app);
    variable_item_set_current_value_index(clear_timers_item, app->clear_timers_on_exit ? 1 : 0);
    variable_item_set_current_value_text(
        clear_timers_item, app->clear_timers_on_exit ? "On" : "Off");

    VariableItem* eink_settings_item = variable_item_list_add(
        app->settings_list, "E-Ink Settings", 1, settings_open_eink_changed, app);
    variable_item_set_current_value_text(eink_settings_item, "Open");
}

void eink_settings_refresh(MultiTimerApp* app) {
    if(!app || !app->eink_settings_list) return;

    eink_settings_rebuilding = true;
    variable_item_list_reset(app->eink_settings_list);

    VariableItem* eink_item = variable_item_list_add(
        app->eink_settings_list, "Show on E-Ink", 2, settings_show_on_eink_changed, app);
    variable_item_set_current_value_index(eink_item, app->show_on_eink ? 1 : 0);
    variable_item_set_current_value_text(eink_item, app->show_on_eink ? "On" : "Off");

    if(app->epaper_model >= EpaperModelCount) {
        app->epaper_model = EPAPER_MODEL_DEFAULT;
    }
    VariableItem* epaper_model_item = variable_item_list_add(
        app->eink_settings_list,
        epaper_model_menu_label((EpaperModel)app->epaper_model),
        EpaperModelCount,
        settings_epaper_model_changed,
        app);
    variable_item_set_current_value_index(epaper_model_item, app->epaper_model);
    variable_item_set_current_value_text(
        epaper_model_item, epaper_model_value_label((EpaperModel)app->epaper_model));

    VariableItem* epaper_mode_item = variable_item_list_add(
        app->eink_settings_list, "E-Ink Mode", 2, settings_epaper_mode_changed, app);
    variable_item_set_current_value_index(epaper_mode_item, app->epaper_partial_mode ? 1 : 0);
    variable_item_set_current_value_text(
        epaper_mode_item, app->epaper_partial_mode ? "Partial" : "Full");

    VariableItem* epaper_invert_item = variable_item_list_add(
        app->eink_settings_list, "Invert colors", 2, settings_epaper_invert_changed, app);
    variable_item_set_current_value_index(epaper_invert_item, app->epaper_invert_colors ? 1 : 0);
    variable_item_set_current_value_text(
        epaper_invert_item, app->epaper_invert_colors ? "On" : "Off");

    uint8_t epaper_index =
        settings_seconds_to_index(app->epaper_refresh_seconds, SETTINGS_EPAPER_REFRESH_MAX_SECONDS);
    VariableItem* epaper_item = variable_item_list_add(
        app->eink_settings_list,
        "E-Ink refresh",
        SETTINGS_EPAPER_REFRESH_MAX_SECONDS / SETTINGS_TIME_STEP_SECONDS,
        settings_epaper_refresh_changed,
        app);
    variable_item_set_current_value_index(epaper_item, epaper_index);
    settings_format_seconds(
        app->epaper_refresh_seconds,
        app->epaper_refresh_text,
        sizeof(app->epaper_refresh_text));
    variable_item_set_current_value_text(epaper_item, app->epaper_refresh_text);
    eink_settings_rebuilding = false;
}
