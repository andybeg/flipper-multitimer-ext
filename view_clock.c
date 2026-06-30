#include "multitimer.h"

void clock_draw_callback(Canvas* canvas, void* model) {
    UNUSED(model);
    if(!canvas) return;

    DateTime datetime;
    furi_hal_rtc_get_datetime(&datetime);

    char time_str[16];
    char date_str[16];
    snprintf(
        time_str,
        sizeof(time_str),
        "%02u:%02u:%02u",
        datetime.hour,
        datetime.minute,
        datetime.second);
    snprintf(
        date_str,
        sizeof(date_str),
        "%04u-%02u-%02u",
        datetime.year,
        datetime.month,
        datetime.day);

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 5, AlignCenter, AlignTop, "Clock");

    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, time_str);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 52, AlignCenter, AlignTop, date_str);
}

bool clock_input_callback(InputEvent* event, void* context) {
    MultiTimerApp* app = context;
    if(!app || !event) return false;

    if(event->type == InputTypePress && event->key == InputKeyBack) {
        app->clock_screen_active = false;
        multitimer_epaper_update(app, true);
        if(app->view_dispatcher) {
            view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewSubmenu);
        }
        return true;
    }

    return false;
}
