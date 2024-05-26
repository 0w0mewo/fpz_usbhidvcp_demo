#include "helloworld_view.h"
#include <furi.h>
#include <math.h>

// callbacks
static void view_on_enter(void* ctx);
static void view_on_draw(Canvas* canvas, void* ctx);
static bool view_on_input(InputEvent* event, void* ctx);
static void timer_cb(void* ctx);

HelloWorldView* helloworld_view_new() {
    HelloWorldView* hwv = (HelloWorldView*)(malloc(sizeof(HelloWorldView)));

    // config usb
    composite_connect();

    // allocate view
    hwv->view = view_alloc();

    // timer
    hwv->timer = furi_timer_alloc(timer_cb, FuriTimerTypePeriodic, hwv);

    // pass HelloWorldView as context to callbacks
    view_set_context(hwv->view, hwv);

    // pass HelloWorldModel as context to the draw callback as ctx, (it's only for draw callback)
    view_allocate_model(hwv->view, ViewModelTypeLocking, sizeof(HelloWorldModel));

    // attatch draw, input and other callbacks to the view
    view_set_draw_callback(hwv->view, view_on_draw);
    view_set_input_callback(hwv->view, view_on_input);
    view_set_enter_callback(hwv->view, view_on_enter);

    return hwv;
}

void helloworld_view_delete(HelloWorldView* hwv) {
    furi_assert(hwv);

    view_free(hwv->view);
    furi_timer_stop(hwv->timer);
    furi_timer_free(hwv->timer);

    // restore usb config
    composite_disconnect();

    free(hwv);
}

View* helloworld_view_get_view(HelloWorldView* hwv) {
    return hwv->view;
}

void helloworld_init(HelloWorldView* hwv) {
    // initial model
    with_view_model(
        hwv->view, HelloWorldModel * model, { model->start = true; }, true);

    // set sensor reporting state
    start_sensor_report_toggle(hwv);
}

void start_sensor_report_toggle(HelloWorldView* hwv) {
    bool on;

    with_view_model(
        hwv->view, HelloWorldModel * model, { on = model->start; }, true);

    if(!on) {
        furi_timer_start(hwv->timer, furi_kernel_get_tick_frequency() * 2); // every 2 seconds
    } else {
        furi_timer_stop(hwv->timer);
    }

    with_view_model(
        hwv->view, HelloWorldModel * model, { model->start = !on; }, true);
}

// on enter callback, HelloWorldView as ctx
static void view_on_enter(void* ctx) {
    furi_assert(ctx);

    HelloWorldView* hwv = (HelloWorldView*)ctx;
    helloworld_init(hwv);
}

// view draw callback, HelloWorldModel as ctx
static void view_on_draw(Canvas* canvas, void* ctx) {
    furi_assert(ctx);
    HelloWorldModel* model = (HelloWorldModel*)ctx;

    if(model == NULL) {
        FURI_LOG_E(LOG_TAG, "model is null");
        return;
    }

    // draw border
    canvas_draw_frame(canvas, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    bool start = model->start;
    if(start) {
        canvas_draw_str_aligned(
            canvas, SCREEN_CENTER_X, SCREEN_CENTER_Y, AlignCenter, AlignCenter, "start");
    } else {
        canvas_draw_str_aligned(
            canvas, SCREEN_CENTER_X, SCREEN_CENTER_Y, AlignCenter, AlignCenter, "stop");
    }
}

// keys input event callback, HelloWorldView as ctx
static bool view_on_input(InputEvent* event, void* ctx) {
    furi_assert(ctx);
    bool consumed = false; // flag to notify view_dispacther
        //that the callback function is processed.

    HelloWorldView* hw = (HelloWorldView*)ctx;

    // move
    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        switch(event->key) {
        case InputKeyOk:
            start_sensor_report_toggle(hw);
            break;

        default:
            break;
        }
        consumed = true;
    }

    return consumed;
}

static void timer_cb(void* ctx) {
    furi_assert(ctx);

    composite_hid_send_temp();
}
