#include "multitimer.h"

void multitimer_apply_storage_settings(MultiTimerApp* app) {
    if(!app) return;

    if(app->timer_storage.setup_hours <= 23) {
        app->setup_hours = app->timer_storage.setup_hours;
    }
    if(app->timer_storage.setup_minutes <= 59) {
        app->setup_minutes = app->timer_storage.setup_minutes;
    }
    if(app->timer_storage.setup_seconds <= 59) {
        app->setup_seconds = app->timer_storage.setup_seconds;
    }
    if(app->timer_storage.setup_selection <= 2) {
        app->setup_selection = app->timer_storage.setup_selection;
    }
}

bool multitimer_storage_ensure_app_dir(MultiTimerApp* app) {
    if(!app || !app->storage) return false;
    return storage_simply_mkdir(app->storage, STORAGE_APP_DATA_PATH_PREFIX);
}

void multitimer_settings_from_app(MultiTimerApp* app, AppSettingsFile* settings) {
    memset(settings, 0, sizeof(AppSettingsFile));
    settings->magic = SETTINGS_MAGIC;
    settings->version = SETTINGS_VERSION;
    settings->show_on_eink = app->show_on_eink ? 1 : 0;
    settings->backlight_on = app->backlight_on ? 1 : 0;
    settings->epaper_partial_mode = app->epaper_partial_mode ? 1 : 0;
    settings->epaper_invert_colors = app->epaper_invert_colors ? 1 : 0;
    settings->epaper_model = app->epaper_model;
    settings->rotate_screen = app->rotate_screen ? 1 : 0;
    settings->clear_timers_on_exit = app->clear_timers_on_exit ? 1 : 0;
    settings->epaper_refresh_seconds = app->epaper_refresh_seconds;
    settings->alarm_duration_seconds = app->alarm_duration_seconds;
}

void multitimer_apply_settings_file(MultiTimerApp* app, const AppSettingsFile* settings) {
    if(!app || !settings) return;

    app->show_on_eink = settings->show_on_eink != 0;
    app->backlight_on = settings->backlight_on != 0;
    app->epaper_partial_mode = settings->epaper_partial_mode != 0;
    app->epaper_invert_colors = settings->epaper_invert_colors != 0;
    if(settings->epaper_model < EpaperModelCount) {
        app->epaper_model = settings->epaper_model;
    } else {
        app->epaper_model = EPAPER_MODEL_DEFAULT;
    }
    app->rotate_screen = settings->rotate_screen != 0;
    app->clear_timers_on_exit = settings->clear_timers_on_exit != 0;
    if(settings->epaper_refresh_seconds > 0) {
        app->epaper_refresh_seconds = settings->epaper_refresh_seconds;
    }
    if(settings->alarm_duration_seconds > 0) {
        app->alarm_duration_seconds = settings->alarm_duration_seconds;
    }
}

void multitimer_sync_settings_to_timer_storage(MultiTimerApp* app) {
    if(!app) return;

    app->timer_storage.backlight_on = app->backlight_on;
    app->timer_storage.show_on_eink = app->show_on_eink;
    app->timer_storage.epaper_partial_mode = app->epaper_partial_mode;
    app->timer_storage.epaper_invert_colors = app->epaper_invert_colors;
    app->timer_storage.epaper_model = app->epaper_model;
    app->timer_storage.epaper_refresh_seconds = app->epaper_refresh_seconds;
    app->timer_storage.alarm_duration_seconds = app->alarm_duration_seconds;
}

void multitimer_migrate_settings_from_timer_storage(MultiTimerApp* app) {
    if(!app) return;

    app->backlight_on = app->timer_storage.backlight_on;
    app->show_on_eink = app->timer_storage.show_on_eink;
    app->epaper_partial_mode = app->timer_storage.epaper_partial_mode;
    app->epaper_invert_colors = app->timer_storage.epaper_invert_colors;
    if(app->timer_storage.epaper_refresh_seconds > 0) {
        app->epaper_refresh_seconds = app->timer_storage.epaper_refresh_seconds;
    }
    if(app->timer_storage.alarm_duration_seconds > 0) {
        app->alarm_duration_seconds = app->timer_storage.alarm_duration_seconds;
    }
    if(app->timer_storage.epaper_model < EpaperModelCount) {
        app->epaper_model = app->timer_storage.epaper_model;
    }
}

void multitimer_load_settings(MultiTimerApp* app) {
    if(!app || !app->storage) return;

    AppSettingsFile settings;
    bool loaded = false;

    File* file = storage_file_alloc(app->storage);
    if(file && storage_file_open(file, SETTINGS_FILE, FSAM_READ, FSOM_OPEN_EXISTING)) {
        if(storage_file_read(file, &settings, sizeof(AppSettingsFile)) == sizeof(AppSettingsFile) &&
           settings.magic == SETTINGS_MAGIC && settings.version == SETTINGS_VERSION) {
            multitimer_apply_settings_file(app, &settings);
            loaded = true;
        }
        storage_file_close(file);
    }
    storage_file_free(file);

    if(!loaded) {
        multitimer_migrate_settings_from_timer_storage(app);
        app->show_on_eink = true;
        multitimer_save_settings(app);
    }
}

void multitimer_save_settings(MultiTimerApp* app) {
    if(!app || !app->storage) return;

    multitimer_sync_settings_to_timer_storage(app);

    AppSettingsFile settings;
    multitimer_settings_from_app(app, &settings);

    if(!multitimer_storage_ensure_app_dir(app)) {
        FURI_LOG_E(TAG, "Failed to create app data directory");
        return;
    }

    File* file = storage_file_alloc(app->storage);
    if(!file) return;

    if(storage_file_open(file, SETTINGS_FILE, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        uint16_t written = storage_file_write(file, &settings, sizeof(AppSettingsFile));
        if(written != sizeof(AppSettingsFile)) {
            FURI_LOG_E(TAG, "Failed to write settings.dat (%u/%zu)", written, sizeof(AppSettingsFile));
        }
    }
    storage_file_close(file);
    storage_file_free(file);
}

void multitimer_epaper_normalize_model(MultiTimerApp* app) {
    if(!app) return;

    if(app->epaper_model >= EpaperModelCount ||
       !epaper_model_is_tested((EpaperModel)app->epaper_model)) {
        app->epaper_model = EPAPER_MODEL_DEFAULT;
    }
    epaper_set_model((EpaperModel)app->epaper_model);
}

// Timer storage functions
void save_timer_data(MultiTimerApp* app) {
    if(!app || !app->storage) return;

    app->timer_storage.magic = TIMER_STORAGE_MAGIC;
    app->timer_storage.version = TIMER_STORAGE_VERSION;
    app->timer_storage.setup_hours = app->setup_hours;
    app->timer_storage.setup_minutes = app->setup_minutes;
    app->timer_storage.setup_seconds = app->setup_seconds;
    app->timer_storage.setup_selection = app->setup_selection;
    app->timer_storage.backlight_on = app->backlight_on;
    app->timer_storage.show_on_eink = app->show_on_eink;
    app->timer_storage.epaper_partial_mode = app->epaper_partial_mode;
    app->timer_storage.epaper_invert_colors = app->epaper_invert_colors;
    app->timer_storage.epaper_refresh_seconds = app->epaper_refresh_seconds;
    app->timer_storage.epaper_model = app->epaper_model;
    app->timer_storage.alarm_duration_seconds = app->alarm_duration_seconds;
    app->timer_storage.saved_count = 0;
    for(int i = 0; i < MAX_TIMERS; i++) {
        if(app->timer_storage.saved_timers[i].used) {
            app->timer_storage.saved_count++;
        }
    }
    app->timer_storage.count = 0;
    for(int i = 0; i < MAX_TIMERS; i++) {
        if(app->timer_storage.timers[i].active) {
            app->timer_storage.count++;
        }
    }

    if(!multitimer_storage_ensure_app_dir(app)) {
        FURI_LOG_E(TAG, "Failed to create app data directory");
        return;
    }

    File* file = storage_file_alloc(app->storage);
    if(!file) return;

    if(storage_file_open(file, TIMER_DATA_FILE, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        uint16_t written = storage_file_write(file, &app->timer_storage, sizeof(TimerStorage));
        if(written != sizeof(TimerStorage)) {
            FURI_LOG_E(
                TAG,
                "Failed to write timers.dat (%u/%zu)",
                written,
                sizeof(TimerStorage));
        } else {
            storage_file_sync(file);
        }
    } else {
        FURI_LOG_E(TAG, "Failed to open timers.dat for write");
    }
    storage_file_close(file);
    storage_file_free(file);
}

bool timer_data_is_valid(const TimerData* timer) {
    if(!timer->active) return true;
    if(timer->duration_seconds == 0 || timer->duration_seconds > 24 * 60 * 60) return false;
    if(timer->state != TimerStateRunning && timer->state != TimerStatePaused &&
       timer->state != TimerStateFinished) {
        return false;
    }
    if(timer->name[0] == '\0') return false;
    return true;
}

bool saved_timer_is_valid(const SavedTimerData* timer) {
    if(!timer->used) return true;
    if(timer->duration_seconds == 0 || timer->duration_seconds > 24 * 60 * 60) return false;
    if(timer->name[0] == '\0') return false;
    return true;
}

void ensure_default_saved_timer(MultiTimerApp* app) {
    if(!app) return;

    for(int i = 0; i < MAX_TIMERS; i++) {
        if(app->timer_storage.saved_timers[i].used) return;
    }

    const uint32_t default_durations[] = {60, 3 * 60, 5 * 60, 10 * 60};
    for(size_t i = 0; i < COUNT_OF(default_durations); i++) {
        SavedTimerData* timer = &app->timer_storage.saved_timers[i];
        timer->used = true;
        timer->duration_seconds = default_durations[i];
        format_time(timer->duration_seconds, timer->name, sizeof(timer->name));
    }
    app->timer_storage.saved_count = COUNT_OF(default_durations);
}

void load_timer_data(MultiTimerApp* app) {
    if(!app || !app->storage) return;

    memset(&app->timer_storage, 0, sizeof(TimerStorage));

    File* file = storage_file_alloc(app->storage);
    if(!file) {
        save_timer_data(app);
        return;
    }

    bool seed_default_saved_timers = false;
    bool storage_loaded = false;

    if(storage_file_open(file, TIMER_DATA_FILE, FSAM_READ, FSOM_OPEN_EXISTING)) {
        uint32_t magic = 0;
        uint32_t version = 0;

        if(storage_file_read(file, &magic, sizeof(magic)) != sizeof(magic) ||
           storage_file_read(file, &version, sizeof(version)) != sizeof(version) ||
           magic != TIMER_STORAGE_MAGIC) {
            memset(&app->timer_storage, 0, sizeof(TimerStorage));
            seed_default_saved_timers = true;
        } else if(
            version != TIMER_STORAGE_VERSION && version != TIMER_STORAGE_V7 &&
            version != TIMER_STORAGE_V6 && version != TIMER_STORAGE_V5 &&
            version != TIMER_STORAGE_V4 && version != TIMER_STORAGE_V3 &&
            version != TIMER_STORAGE_V2 && version != TIMER_STORAGE_LEGACY_VERSION) {
            memset(&app->timer_storage, 0, sizeof(TimerStorage));
            seed_default_saved_timers = true;
        } else {
            storage_loaded = true;
            app->timer_storage.magic = magic;
            app->timer_storage.version = version;

            if(version == TIMER_STORAGE_VERSION) {
                storage_file_read(file, &app->timer_storage.count, sizeof(app->timer_storage.count));
                storage_file_read(file, app->timer_storage.timers, sizeof(app->timer_storage.timers));
                storage_file_read(
                    file,
                    &app->timer_storage.setup_hours,
                    sizeof(TimerStorage) - offsetof(TimerStorage, setup_hours));
            } else {
                storage_file_read(file, app->timer_storage.timers, sizeof(app->timer_storage.timers));

                if(version == TIMER_STORAGE_V7) {
                    TimerStorageV7Broken legacy_v7;
                    storage_file_read(
                        file,
                        &legacy_v7.setup_hours,
                        sizeof(TimerStorageV7Broken) -
                            offsetof(TimerStorageV7Broken, setup_hours));
                    app->timer_storage.setup_hours = legacy_v7.setup_hours;
                    app->timer_storage.setup_minutes = legacy_v7.setup_minutes;
                    app->timer_storage.setup_seconds = legacy_v7.setup_seconds;
                    app->timer_storage.setup_selection = legacy_v7.setup_selection;
                    app->timer_storage.backlight_on = legacy_v7.backlight_on;
                    app->timer_storage.show_on_eink = legacy_v7.show_on_eink;
                    app->timer_storage.epaper_partial_mode = legacy_v7.epaper_partial_mode;
                    app->timer_storage.epaper_invert_colors = legacy_v7.epaper_invert_colors;
                    app->timer_storage.epaper_refresh_seconds = legacy_v7.epaper_refresh_seconds;
                    app->timer_storage.alarm_duration_seconds = legacy_v7.alarm_duration_seconds;
                    app->timer_storage.saved_count = legacy_v7.saved_count;
                    memcpy(
                        app->timer_storage.saved_timers,
                        legacy_v7.saved_timers,
                        sizeof(legacy_v7.saved_timers));
                    app->timer_storage.epaper_model = EPAPER_MODEL_DEFAULT;
                } else {
                    storage_file_read(
                        file,
                        &app->timer_storage.setup_hours,
                        offsetof(TimerStorage, epaper_model) -
                            offsetof(TimerStorage, setup_hours));
                    app->timer_storage.epaper_model = EPAPER_MODEL_DEFAULT;
                }
            }

            multitimer_apply_storage_settings(app);

            if(version != TIMER_STORAGE_VERSION && version != TIMER_STORAGE_V7 &&
               version != TIMER_STORAGE_V6) {
                memset(
                    &app->timer_storage.timers[TIMER_STORAGE_LEGACY_MAX_TIMERS],
                    0,
                    sizeof(TimerData) * (MAX_TIMERS - TIMER_STORAGE_LEGACY_MAX_TIMERS));
            }
        }
    } else {
        seed_default_saved_timers = true;
    }

    app->timer_storage.count = 0;
    for(int i = 0; i < MAX_TIMERS; i++) {
        if(!timer_data_is_valid(&app->timer_storage.timers[i])) {
            memset(&app->timer_storage.timers[i], 0, sizeof(TimerData));
        }
        if(app->timer_storage.timers[i].active) {
            app->timer_storage.count++;
        }
    }

    if(app->timer_storage.version < TIMER_STORAGE_V6) {
        memset(app->timer_storage.saved_timers, 0, sizeof(app->timer_storage.saved_timers));
        if(storage_loaded) {
            seed_default_saved_timers = true;
        }
    }
    app->timer_storage.saved_count = 0;
    for(int i = 0; i < MAX_TIMERS; i++) {
        if(!saved_timer_is_valid(&app->timer_storage.saved_timers[i])) {
            memset(&app->timer_storage.saved_timers[i], 0, sizeof(SavedTimerData));
        }
        if(app->timer_storage.saved_timers[i].used) {
            app->timer_storage.saved_count++;
        }
    }
    if(seed_default_saved_timers) {
        ensure_default_saved_timer(app);
    }

    storage_file_close(file);
    storage_file_free(file);
}
