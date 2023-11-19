#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/FL_Image.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Round_Button.H>
#include <unistd.h>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <Savitzky_Golay_2d.h>
#include <mib-read.h>

#define MAINWIN_W       700                             // main window w()
#define MAINWIN_H       700                             // main window h()
#define BROWSER_X       10                              // browser x()
#define BROWSER_Y       10                              // browser y()
#define BROWSER_W       100                             // browser w()
#define BROWSER_H       MAINWIN_H-35                    // browser h()
#define DATA_AREA_X      (BROWSER_W + 20)                // test area x()
#define DATA_AREA_Y      10                              // test area y()
#define DATA_AREA_W      (MAINWIN_W - BROWSER_W - 30)    // test area w()
#define DATA_AREA_H      BROWSER_H                       // test area h()
int get_peak_list(double *data, int w, int h, int line_len, int box,
                  int *peaklist, int max_size,  int &count);
int box_size= 30;
void set_mul_threshold(double a) ;
void set_threshold(double a);

static int data_mode = 0;
int *peaklist;
Filter2d *filter;
class MIB_Window: public Fl_Double_Window{
public:
    MIB_Window(int w, int h, const char *l=0L) :
    Fl_Double_Window(w, h, l){}
};

class MIB_view:public Fl_Widget{
public:
    MIB_view(int x, int y, int w, int h, const char *label = 0L):Fl_Widget(x, y, w, h, label){
        data = new uint8_t [w*h];
        set_dummy();
    }
    ~MIB_view(){if(data) delete []data;}
    unsigned char* image_ptr(){return data;}
    void set_peaks(double *xy, int len){
  //      if(len > 50) len = 50;
        peak_len = len;
        int *ptr = peaks;
        double *ptr1 = xy;
        for(int i = 0; i < len; i++, ptr+=2, ptr1+=2){
            ptr[0] = ptr1[0]+x();
            ptr[1] = ptr1[1]+y();
        }
    }
    
    void draw_cross(int *ptr){
        fl_line(ptr[0]-10+x(), ptr[1]+y(), ptr[0]+10+x(), ptr[1]+y());
        fl_line(ptr[0]+x(), ptr[1]-10+y(), ptr[0]+x(), ptr[1]+10+y());
    }
    void set_dummy()
    {
        int *ptr = peaks;
        for(int i = 0; i < 10; i++, ptr+=2){
            ptr[0] = i * 30;
            ptr[1] = i * 30;
        }
        peak_len = 10;
    }
    int* peaks_ptr(){return peaks;}
    void set_peak_len(int i){peak_len = i;}
    void draw(){
        fl_draw_image(data, x(), y(), w(), h(), 1);
        fl_color(FL_RED);
        int *ptr = peaks;
        fl_begin_line();
        fl_color(FL_GREEN);
        for(int i = 0; i < peak_len/2; i++, ptr+=2){
            draw_cross(ptr);
        }
        fl_rect(10, 10, box_size, box_size);
 //       fl_arc(0., 0., 20.0, 0.0, 360.0);
        fl_end_line();
    }
protected:
    uint8_t *data;
    int peaks[400];
    int peak_len=0;
};

class YN_Int_Input: public Fl_Int_Input
{
public:
    YN_Int_Input(int x, int y, int w, int h,const char * l = 0, int v=0):
    Fl_Int_Input(x, y, w, h, l){
        set(v);
        tab_nav(1);
    }
    int set(int i){
        ival = i;
        char val[10];
        snprintf(val, 10, "%d", ival);
        value(val);
    }
    void increment(int i){
        ival += i;
        set(ival);
        do_callback();
    }
    void decrement(int i){
        ival -= i;
        set(ival);
        do_callback();
    }
    int val(){return ival;}
    int handle(int e) {
        int ret = Fl_Input::handle(e);
        if(active()){
            switch (e) {
                case FL_KEYDOWN:
                    switch(Fl::event_key()){
                        case FL_Up:
                            increment(1);
                            break;
                        case FL_Down:
                            decrement(1);
                            break;
                        case FL_Enter:
                            ret = 1;
                            ival = atoi(value());
                            do_callback();
                            break;

                    }
            }
        }
        return ret;
    }
protected:
    int ival;
};

class YN_Input: public Fl_Input
{
public:
    YN_Input(int x, int y, int w, int h,const char * l = 0, double v=0 ):
    Fl_Input(x, y, w, h, l){
        setv(v);
        tab_nav(1);
    }
    void setv(double v){
        val = v;
        set();
        value_set =1;
        set_changed();
//        do_callback();
    }
    void set(){
        char str_val[40];
        snprintf(str_val, 40, "%5.1f", val);
        value(str_val);
    }
    
    int handle(int e) {
        int ret = Fl_Input::handle(e);
        if ( e == FL_KEYBOARD && Fl::event_key() == FL_Escape ) exit(0);
        if(active())
            switch (e) {
                case FL_KEYDOWN:
                    switch(Fl::event_key()){
                        case FL_Down:
                            val /=2;
                            setv(val);
                            ret =1;
                            do_callback();
                            break;
                        case FL_Up:
                            val *=2;
                            setv(val);
                            ret = 1;
                            do_callback();
                            break;
                        case FL_Enter:
                            ret = 1;
                            val = atof(value());
                            setv(val);
                            do_callback();
                            break;
                    }
            }
        if(!value_set) val = atof(value());
        return(ret);
    }
    double get(){return val;}
protected:
    double val;
    int value_set = 0;
};


YN_Int_Input *input, *sg_size_input, *sg_order_input;
YN_Int_Input *box_size_input;
YN_Input *max_input, *sigma_x_input, *sigma_input;
YN_Input *scale_input;

mib *reader;
MIB_view *view;


void copy_mode()
{
    switch(data_mode){
        case 0:
            reader->copy(view->image_ptr());
            break;
        case 1:
            reader->copy_d(view->image_ptr());
            break;
        case 2:
            reader->copy_o(view->image_ptr());
            break;
    }
}

void file_num_changed(Fl_Widget *w, void *data)
{
    int frame  = input->val();
    reader->read(frame);
    reader->set_max(max_input->get());
    copy_mode();
    reader->find_center();

    double C = reader->bg_scale(30);
    reader->bg_subtract(C);
    reader->apply(*filter);
    int count = 0;
    get_peak_list(reader->filtered_data(), 514, 514, 514, box_size,
                  view->peaks_ptr(), 200,  count);
    view->set_peak_len(count);
    cout <<"num peaks"<< count/2 <<endl;
    view->redraw();

}

void sg_param_changed(Fl_Widget *w, void *data)
{
    int n = sg_size_input->val();
    filter->reset(n, n, 0, sg_order_input->val());
    reader->apply(*filter);
    copy_mode();
    int count = 0;
    get_peak_list(reader->filtered_data(), 514, 514, 514, 30,
                  view->peaks_ptr(), 200,  count);
    view->set_peak_len(count);
    cout <<"num peaks"<< count/2 <<endl;
    view->redraw();
}

void max_changed(Fl_Widget *w, void *data)
{
    reader->set_max(max_input->get());
    copy_mode();
    view->redraw();
}
void scale_changed(Fl_Widget *w, void *data)
{
    reader->set_scale(scale_input->get());
    file_num_changed(NULL, NULL);
 }

void box_size_changed(Fl_Widget *w, void *data)
{
    box_size = box_size_input->val();
    cout <<"box size"<< box_size <<endl;
    int count = 0;
    get_peak_list(reader->filtered_data(), 514, 514, 514, box_size,
                  view->peaks_ptr(), 200,  count);
    view->set_peak_len(count);
    cout <<"num peaks"<< count/2 <<endl;
    view->redraw();
    
}
void peak_pick_conditions_changed(Fl_Widget *w, void *data)
{
    set_mul_threshold(sigma_x_input->get());
    set_threshold(sigma_input->get());
    box_size_changed(NULL, NULL);
}
Fl_Output *cb_info=(Fl_Output *)0;

static void button_cb(Fl_Button *b, void *) {
  char msg[256];
  sprintf(msg, "Label: '%s'\nValue: %d", b->label(),b->value());
    if(! strncmp(b->label(), "subtracted", 6)) data_mode = 1;
    if(! strncmp(b->label(), "raw", 3)) data_mode = 0;
    if(! strncmp(b->label(), "filtered", 6)) data_mode = 2;
   file_num_changed(NULL, NULL);
  cb_info->value(msg);
  cb_info->redraw();
  printf("%s\n",msg);
}

using namespace std;
int main(int argc, char *argv[])
{
    reader = new mib(argv[1], .8);
    filter = new Filter2d (2, 2, 0, 2, 514, 514);
    peaklist = new int[200];
    
    MIB_Window window(700, 700, "diffraction frame");
    window.begin();
    view = new MIB_view(10, 10, 514, 514, "diffraction");
    input = new YN_Int_Input(624, 10, 50, 25, "frame num", 1);
    input->callback(file_num_changed, NULL);
    max_input = new YN_Input(624, 40, 50, 25, "max", 255);
    max_input->tooltip("maximum pixel value");
    max_input->callback(max_changed, NULL);
    scale_input =new YN_Input(624, 70, 50, 25, "scale", 0.8);
    scale_input->tooltip("scaling factor for the pixels at the border of chips");
    scale_input->callback(scale_changed, NULL);
    
    Fl_Group* o = new Fl_Group(580, 140, 70, 90, "chose display");
    o->box(FL_THIN_UP_FRAME);
    { Fl_Round_Button* o = new Fl_Round_Button(580, 140, 70, 30, "raw");
        o->tooltip("showing detector image.");
        o->type(102);
        o->set();
        o->down_box(FL_ROUND_DOWN_BOX);
        o->callback((Fl_Callback*) button_cb);
    } // Fl_Round_Button* o
    { Fl_Round_Button* o = new Fl_Round_Button(580, 170, 70, 30, "subtrated");
        o->tooltip("showing image after direct beam subtraction");
        o->type(102);
        o->down_box(FL_ROUND_DOWN_BOX);
        o->callback((Fl_Callback*) button_cb);
    }
    { Fl_Round_Button* o = new Fl_Round_Button(580, 200, 70, 30, "filtered");
        o->tooltip("showing image after Savitzky-Golay filter application");
        o->type(102);
        o->down_box(FL_ROUND_DOWN_BOX);
        o->callback((Fl_Callback*) button_cb);
    }
    { cb_info = new Fl_Output(534, 230, 160, 62, "");
        cb_info->type(12);
        cb_info->textsize(12);
        cb_info->align(Fl_Align(133));
    } // Fl_Output* cb_info
    
    o->end();
    // Fl_Group* o
    {Fl_Group* o = new Fl_Group(550, 330, 150, 60, "savitzky golay parameters");
        o->box(FL_THIN_UP_FRAME);
        sg_size_input = new YN_Int_Input(624, 330, 50, 25, "width", 2);
        sg_size_input->tooltip("the width of the windows on both sides");
        sg_size_input->callback((Fl_Callback *) sg_param_changed);
        sg_order_input = new YN_Int_Input(624, 355, 50, 25, "order", 2);
        sg_order_input->callback((Fl_Callback *) sg_param_changed);
       o->end();
    }
    {
        Fl_Group* o = new Fl_Group(550, 410, 150, 80, "peak search") ;
        o->box(FL_THIN_UP_FRAME);
        o->tooltip("only peaks with intensity above x times the local standard deviation are chosen");
        box_size_input = new YN_Int_Input(624, 410, 50, 25, "box size", box_size);
        box_size_input->callback((Fl_Callback *)box_size_changed);
        box_size_input->tooltip("size of the tile in which we try to find a peak. Should be smaller than the minimum distance between two peaks");
        sigma_x_input = new YN_Input(624, 435, 50, 25, "sigma x", 15.);
        sigma_x_input->tooltip("only peaks with intensity above x times the local standard deviation are chosen");
        sigma_x_input->callback((Fl_Callback *)peak_pick_conditions_changed );
        sigma_input = new YN_Input (624, 460, 50, 25, "cutoff", 5.);
        sigma_input->tooltip("only peaks with intensity above this value are chosen");
        sigma_input->callback((Fl_Callback *)peak_pick_conditions_changed );
//        input = new YN_Int_Input(624, 307, 50, 25, "order", 2);
        o->end();
    }
    file_num_changed(NULL, NULL);
    window.end();
    window.show(argc, argv);
    return Fl::run();
}
