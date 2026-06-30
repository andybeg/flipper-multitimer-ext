#include "multitimer.h"

void timer_list_draw_callback(Canvas* canvas, void* model) {
    MultiTimerApp* app = multitimer_view_get_app(model);
    if(!app || !canvas) return;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);

    int active_count = count_active_timers(app);
    char title[32];
    snprintf(title, sizeof(title), "Active Timers (%d)", active_count);
    canvas_draw_str_aligned(canvas, 64, 5, AlignCenter, AlignTop, title);

    canvas_set_font(canvas, FontSecondary);

    if(active_count == 0) {
        canvas_draw_str_aligned(canvas, 64, 35, AlignCenter, AlignCenter, "No active timers");
        canvas_draw_str_aligned(canvas, 64, 62, AlignCenter, AlignTop, "Back: Menu");
        return;
    }

    int y_pos = 20;
    int visible_row = 0;
    const int max_visible_rows = 4;

    for(int i = 0; i < MAX_TIMERS; i++) {
        TimerData* timer = &app->timer_storage.timers[i];
        if(!timer->active) continue;

        if(visible_row < app->timer_list_scroll) {
            visible_row++;
            continue;
        }
        if(visible_row - app->timer_list_scroll >= max_visible_rows) {
            break;
        }

        char time_str[32];
        if(timer->state == TimerStateFinished) {
            snprintf(time_str, sizeof(time_str), "DONE");
        } else {
            uint32_t remaining = get_remaining_seconds(timer);
            format_time(remaining, time_str, sizeof(time_str));
        }

        const char* state_str = "";
        if(timer->state == TimerStateRunning) {
            state_str = ">";
        } else if(timer->state == TimerStatePaused) {
            state_str = "||";
        } else if(timer->state == TimerStateFinished) {
            state_str = "!";
        }

        bool selected = i == app->timer_list_selected_index;
        if(selected) {
            canvas_draw_box(canvas, 0, y_pos - 8, 128, 10);
            canvas_set_color(canvas, ColorWhite);
        }

        char display_str[64];
        snprintf(display_str, sizeof(display_str), "%s%-10s %s", state_str, timer->name, time_str);
        canvas_draw_str(canvas, 5, y_pos, display_str);
        canvas_set_color(canvas, ColorBlack);

        y_pos += 10;
        visible_row++;
    }

    canvas_draw_str_aligned(canvas, 64, 62, AlignCenter, AlignTop, "OK:Open Left:Stop Back:Menu");
}

bool timer_list_input_callback(InputEvent* event, void* context) {
    MultiTimerApp* app = context;
    if(!app || !event) return false;

    if(event->type != InputTypePress) return false;

    int active_count = count_active_timers(app);

    if(active_count == 0) {
        if(event->key == InputKeyBack && app->view_dispatcher) {
            view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewSubmenu);
            return true;
        }
        return false;
    }

    if(app->timer_list_selected_index < 0 ||
       !app->timer_storage.timers[app->timer_list_selected_index].active) {
        timer_list_prepare_selection(app, -1);
    }

    int selected_row = get_active_timer_row_for_slot(app, app->timer_list_selected_index);

    switch(event->key) {
    case InputKeyUp:
        if(selected_row > 0) {
            app->timer_list_selected_index =
                get_active_timer_slot_by_row(app, selected_row - 1);
            if(selected_row - 1 < app->timer_list_scroll) {
                app->timer_list_scroll = selected_row - 1;
            }
            with_view_model(app->timer_list_view, MultiTimerApp** model, { UNUSED(model); }, true);
        }
        return true;

    case InputKeyDown:
        if(selected_row >= 0 && selected_row < active_count - 1) {
            app->timer_list_selected_index =
                get_active_timer_slot_by_row(app, selected_row + 1);
            if(selected_row + 1 >= app->timer_list_scroll + 4) {
                app->timer_list_scroll = selected_row + 1 - 3;
            }
            with_view_model(app->timer_list_view, MultiTimerApp** model, { UNUSED(model); }, true);
        }
        return true;

    case InputKeyOk:
        if(app->timer_list_selected_index >= 0) {
            app->selected_timer_index = app->timer_list_selected_index;
            if(app->view_dispatcher) {
                view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewTimerRunning);
            }
        }
        return true;

    case InputKeyLeft:
        if(app->timer_list_selected_index >= 0) {
            deactivate_timer(app, app->timer_list_selected_index);
            timer_list_prepare_selection(app, -1);
            with_view_model(app->timer_list_view, MultiTimerApp** model, { UNUSED(model); }, true);
        }
        return true;

    case InputKeyBack:
        if(app->view_dispatcher) {
            view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewSubmenu);
        }
        return true;

    default:
        break;
    }

    return false;
}
