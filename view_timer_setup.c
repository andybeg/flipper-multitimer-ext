#include "multitimer.h"
#include <multitimerext_icons.h>

void timer_setup_draw_callback(Canvas* canvas, void* model) {
    MultiTimerApp* app = multitimer_view_get_app(model);
    if(!app || !canvas) return;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);

    // Draw clock icon at the left edge
    canvas_draw_icon(canvas, 0, 3, &I_clock_10x10);
    canvas_draw_str_aligned(canvas, 64, 5, AlignCenter, AlignTop, "New Timer");

    // Draw time inputs
    char time_str[32];
    canvas_set_font(canvas, FontBigNumbers);

    // Hours
    snprintf(time_str, sizeof(time_str), "%02lu", app->setup_hours);
    if(app->setup_selection == 0) {
        canvas_draw_box(canvas, 15, 35, 20, 20);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_str_aligned(canvas, 25, 45, AlignCenter, AlignCenter, time_str);
    canvas_set_color(canvas, ColorBlack);

    canvas_draw_str_aligned(canvas, 40, 45, AlignCenter, AlignCenter, ":");

    // Minutes
    snprintf(time_str, sizeof(time_str), "%02lu", app->setup_minutes);
    if(app->setup_selection == 1) {
        canvas_draw_box(canvas, 50, 35, 20, 20);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_str_aligned(canvas, 60, 45, AlignCenter, AlignCenter, time_str);
    canvas_set_color(canvas, ColorBlack);

    canvas_draw_str_aligned(canvas, 75, 45, AlignCenter, AlignCenter, ":");

    // Seconds
    snprintf(time_str, sizeof(time_str), "%02lu", app->setup_seconds);
    if(app->setup_selection == 2) {
        canvas_draw_box(canvas, 85, 35, 20, 20);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_str_aligned(canvas, 95, 45, AlignCenter, AlignCenter, time_str);
    canvas_set_color(canvas, ColorBlack);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 25, 28, AlignCenter, AlignTop, "HH");
    canvas_draw_str_aligned(canvas, 60, 28, AlignCenter, AlignTop, "MM");
    canvas_draw_str_aligned(canvas, 95, 28, AlignCenter, AlignTop, "SS");

}

bool timer_setup_input_callback(InputEvent* event, void* context) {
    MultiTimerApp* app = context;
    if(!app || !event) return false;

    if(event->type == InputTypePress) {
        switch(event->key) {
        case InputKeyLeft:
            if(app->setup_selection > 0) app->setup_selection--;
            timer_setup_refresh(app);
            return true;

        case InputKeyRight:
            if(app->setup_selection < 2) app->setup_selection++;
            timer_setup_refresh(app);
            return true;

        case InputKeyUp:
            if(app->setup_selection == 0) { // Hours
                if(app->setup_hours < 23) app->setup_hours++;
            } else if(app->setup_selection == 1) { // Minutes
                if(app->setup_minutes < 59) {
                    app->setup_minutes++;
                } else {
                    app->setup_minutes = 0;
                }
            } else if(app->setup_selection == 2) { // Seconds
                if(app->setup_seconds < 59) {
                    app->setup_seconds++;
                } else {
                    app->setup_seconds = 0;
                }
            }
            timer_setup_refresh(app);
            return true;

        case InputKeyDown:
            if(app->setup_selection == 0) { // Hours
                if(app->setup_hours > 0) app->setup_hours--;
            } else if(app->setup_selection == 1) { // Minutes
                if(app->setup_minutes > 0) {
                    app->setup_minutes--;
                } else {
                    app->setup_minutes = 59;
                }
            } else if(app->setup_selection == 2) { // Seconds
                if(app->setup_seconds > 0) {
                    app->setup_seconds--;
                } else {
                    app->setup_seconds = 59;
                }
            }
            timer_setup_refresh(app);
            return true;

        case InputKeyOk: {
            app->setup_screen_active = false;
            save_timer_data(app);

            // Create new timer
            uint32_t duration =
                app->setup_hours * 3600 + app->setup_minutes * 60 + app->setup_seconds;
            if(duration > 0) {
                int saved_slot = save_new_timer_preset(app, duration);
                if(saved_slot >= 0) {
                    SavedTimerData* saved_timer = &app->timer_storage.saved_timers[saved_slot];
                    app->selected_saved_timer_index = saved_slot;
                    start_timer(app, saved_timer->duration_seconds, saved_timer->name);
                }
            }
            return true;
        }

        case InputKeyBack:
            app->setup_screen_active = false;
            save_timer_data(app);
            if(app->show_on_eink) {
                if(app->timer_storage.count > 0) {
                    multitimer_epaper_update(app, true);
                } else {
                    multitimer_epaper_show_splash(app);
                }
            }
            if(app->view_dispatcher) {
                view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewSubmenu);
            }
            return true;

        default:
            break;
        }
    }

    return false;
}
