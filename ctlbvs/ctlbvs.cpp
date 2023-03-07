#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdio.h> //printf
//#include <stdlib.h> //1
#include <string.h>
//#include <unistd.h> //sleep
#include <X11/extensions/Xrandr.h>

#include <X11/cursorfont.h>

#include <thread>
#include <ctime> 
//#include <sys/wait.h>

//#include "util.hpp"
#include <sstream>

#include <dirent.h> // opendir, readdir
#include <limits.h> // NAME_MAX

#include <pwd.h>

#include <alsa/asoundlib.h> //libasound2-dev

#include <libpng16/png.h>

#include <sys/wait.h> //signal

#define dev_class "/sys/class"
#define app_config ".config/Solar_eDEX"


#define Mute '0'
#define VolumeUp '1'
#define VolumeDown '2'
#define BrightnessUp '3'
#define BrightnessDown '4'
#define PrintScreen '5'
#define TimeOut 't'
#define ThemeColor 'T'

//#include <X11/xpm.h>
//unsigned tiempo;
bool pTarea = true;
bool strTime = false;
unsigned char porcentaje = 5;

//usr/lib/udev/rules.d
//g++ ctlbvs.cpp -o ctlbvs -lX11 -lXrandr -pthread -lasound -lpng

void hideCursor(Display *display, Window w){
    Cursor invisibleCursor;
    Pixmap bitmapNoData;
    XColor black;
    static char noData[] = { 0,0,0,0,0,0,0,0 };
    black.red = black.green = black.blue = 0;

    bitmapNoData = XCreateBitmapFromData(display, w, noData, 8, 8);
    invisibleCursor = XCreatePixmapCursor(display, bitmapNoData, bitmapNoData, 
                                         &black, &black, 0, 0);
    XDefineCursor(display,w, invisibleCursor);
    XFreeCursor(display, invisibleCursor);
    XFreePixmap(display, bitmapNoData);
}

void showCursor(Display *display, Window w){
    Cursor cursor;
    cursor=XCreateFontCursor(display,XC_left_ptr);
    XDefineCursor(display, w, cursor);
    XFreeCursor(display, cursor);
}

void subProceso(int limitT,Window win)
{
    //bool pTarea = true;
    unsigned tiempo = clock(); // para poder resetear el relog
    bool execAccion = true;
    while(pTarea){
        //cout<< double(tiempo)/CLOCKS_PER_SEC << " "<< double(tiempoEnd)/CLOCKS_PER_SEC << " ";
        
        if((double(clock() - tiempo)/CLOCKS_PER_SEC) >= limitT && execAccion){
            //Status resp; 
            XEvent evt;
            Display* display = XOpenDisplay(XDisplayName(NULL));

            if(display != NULL){
                evt.xclient.type = ClientMessage;
                evt.xclient.serial = 0;
                evt.xclient.send_event = true;
                evt.xclient.message_type = XInternAtom(display,"DISPLAY_BRIGHTNESS_VOLUME_SCREEN",false);;
                evt.xclient.format = 8;
                evt.xclient.window = win;
                evt.xclient.data.b[0] = TimeOut;

                /*resp =*/ XSendEvent(display,win,false,ExposureMask,&evt);

                XFlush(display);
                execAccion = false;
                
            }

            XCloseDisplay(display);

            //pTarea = false;
            //printf("pTarea; %f\n",double(tiempo)/CLOCKS_PER_SEC);

            //return resp; 
        }

        if(strTime){
            tiempo = clock();
            strTime = false;
            execAccion = true;
        }

        if(!execAccion){
            usleep(100);
        }
    }
}

Window showIndicador(Display *display, int screen, int w_window, int h_window, long bgColor){
    int x_window, y_window,rMononitor, mouse_root_x, mouse_root_y,win_x, win_y,porcentajeH = 32;
    unsigned int mask_return;
    Window w,window_returned,root = RootWindow(display, screen);
    XRRMonitorInfo *info = XRRGetMonitors(display, root,0,&rMononitor);

    XQueryPointer(display, root, &window_returned,
                &window_returned, &mouse_root_x, &mouse_root_y, &win_x, &win_y,
                &mask_return);
    if(mouse_root_x >= info->width){
        x_window = ((info->width/2)-(w_window/2) + info->width);
    }else{
        x_window = ((info->width/2)-(w_window/2));
    }

    if(mouse_root_y >= info->height){
        y_window = (info->height - ((info->height*porcentajeH)/100)) + info->height; //((info->height/2)-(h_window/2) + info->height);
    }else{
        y_window = info->height - ((info->height*porcentajeH)/100);//((info->height/2)-(h_window/2));
    }

    w = XCreateSimpleWindow(display, root, x_window, y_window, w_window, h_window, 0,BlackPixel(display, screen), bgColor/*WhitePixel(display, screen)*/);
    Atom type = XInternAtom(display,"_NET_WM_WINDOW_TYPE", false);
    Atom value = XInternAtom(display,"_NET_WM_WINDOW_TYPE_SPLASH_BV", false);
    Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", false);
    XChangeProperty(display, w, type, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&value), 1);
    XSetWMProtocols(display, w, &wmDeleteMessage, 1);
    XSelectInput(display, w, ExposureMask);
    return w;
}

void drawIndicador(Display *display, Window w,XColor color,int wAncho, int wAlto,int cantidad){
    GC lienzo;
    XColor colorFondo;
    Colormap mapa;
    //char bgColorBr[] = {'\0','\0','\0','\0','\0','\0','\0'};

    lienzo = XCreateGC( display, DefaultRootWindow(display), 0, 0 );
    cantidad = (cantidad*200)/200;
    if(cantidad > 200)
        cantidad = 200;
    if(cantidad < 0)
        cantidad = 0;

    //sprintf(bgColorBr,"#%02x%02x%02x\n", (255-(color.red/257)), (255-(color.green/257)), (255-(color.blue/257)));

    colorFondo.red = 257 * (255 - (color.red/257));//0x8c;
    colorFondo.green = 257 * (255 - (color.green/257));// 0x8d;
    colorFondo.blue = 257 * (255 - (color.blue/257));//0x8f;
    
    mapa = DefaultColormap(display, DefaultScreen(display));
    //XParseColor(display, mapa, bgColorBr,&colorFondo);//este
    XAllocColor(display, mapa, &colorFondo);
    XSetForeground( display, lienzo, colorFondo.pixel);
    XFillRectangle(display, w, /*DefaultGC(display, screen)*/lienzo,10,wAlto-((wAlto*20)/100),(wAncho-20),10);
    //printf("#%02x%02x%02x\n", ((colorFondo.red/257)), ((colorFondo.green/257)), ((colorFondo.blue/257)));

    mapa = DefaultColormap(display, DefaultScreen(display));
    XAllocColor(display, mapa, &color);
    XSetForeground( display, lienzo, color.pixel);
    XFillRectangle(display, w, lienzo,10,wAlto-((wAlto*20)/100),cantidad,10);
}

void drawBocina(Display *display, Window win,GC lienzo,XColor color, int x, int y){
    
    XPoint baseBocina[] = { 
    { 50,45 }, { 5,5},{ 10,0}, { 0,-30 }, { -10,0 },{ -5,5}}; 
    baseBocina[0].x = x;
    baseBocina[0].y = y;
    XPoint bocina[] = { 
    { 67,50 }, { 0,-30},{ 15,-15}, { 5,0}, { 0,60 },{ -5,0}};
    bocina[0].x = x+17;
    bocina[0].y = y+5;

    XFillPolygon( display, win, lienzo, baseBocina, 6, Convex, 
                      CoordModePrevious );
    XFillPolygon( display, win, lienzo, bocina, 6, Convex, 
                      CoordModePrevious );
} 

void drawSound(Display *display, Window win,GC lienzo, int x, int y, bool mute=false){
    int tamanioOnda = 5;

    if(mute)
        XFillRectangle(display, win, lienzo,x,y,30,2);
    else{
        XDrawArc( display, win, lienzo, x-(tamanioOnda/2), y-(tamanioOnda/2),//se resta la mitad de el ancho para ajustar cordenadas x e y
          tamanioOnda,//ancho
          tamanioOnda,//alto
          270*64, //lo va girando
          180*64 ); //es el arco
        x += 1;
        XDrawArc( display, win, lienzo, x-(tamanioOnda/2), y-(tamanioOnda/2),tamanioOnda,tamanioOnda,270*64,180*64 ); 

        x += 5; tamanioOnda *= 2;
        XDrawArc( display, win, lienzo, x-(tamanioOnda/2),y-(tamanioOnda/2),tamanioOnda,tamanioOnda,270*64,180*64 );
        x += 1;
        XDrawArc( display, win, lienzo, x-(tamanioOnda/2), y-(tamanioOnda/2),tamanioOnda,tamanioOnda,270*64,180*64 );

        x += 5; tamanioOnda *= 2;
        XDrawArc( display, win, lienzo, x-(tamanioOnda/2),y-(tamanioOnda/2),tamanioOnda,tamanioOnda,270*64,180*64 );
        x += 1;
        XDrawArc( display, win, lienzo, x-(tamanioOnda/2), y-(tamanioOnda/2),tamanioOnda,tamanioOnda,270*64,180*64 );

        x += 5; tamanioOnda *= 2;
        XDrawArc( display, win, lienzo, x-(tamanioOnda/2),y-(tamanioOnda/2),tamanioOnda,tamanioOnda,270*64,180*64 );
        x += 1;
        XDrawArc( display, win, lienzo, x-(tamanioOnda/2), y-(tamanioOnda/2),tamanioOnda,tamanioOnda,270*64,180*64 );
    }
}

void drawVolumen(Display *display, Window win,int wAncho, int wAlto,XColor color, int x, int y,int nivel, bool mute=false){
    GC lienzo;
    Colormap mapa;

    lienzo = XCreateGC( display, DefaultRootWindow(display), 0, 0 );
    mapa = DefaultColormap(display, DefaultScreen(display));
    XAllocColor(display, mapa, &color);
    XSetForeground( display, lienzo, color.pixel);
    XSetFillRule( display, lienzo, WindingRule );

    XClearArea(display,win,x,0,(wAncho-70),(wAlto-22),false);
    drawBocina(display, win,lienzo,color, x, y);
    x += 40; y -= 10;
    drawSound(display,win,lienzo,x,y,mute);
    if(mute) nivel = 0;
    drawIndicador(display, win,color,wAncho,wAlto,nivel);
    XFlush(display);
}

void drawSol(Display *display, Window win,GC lienzo,int x, int y){
    int tamanioCentro = 40;
    XPoint cord = {0,0};
    cord.x = x; cord.y = y;

    XDrawArc( display, win, lienzo, x-(tamanioCentro/2), y-(tamanioCentro/2),tamanioCentro,tamanioCentro,0,360*64 );
    XFillRectangle(display, win, lienzo,(x-12),(y-15),26,30);
    for (int i = 0; i < tamanioCentro; ++i)
    {
        XDrawArc( display, win, lienzo, x-((tamanioCentro-i)/2), y-((tamanioCentro-i)/2),(tamanioCentro-i),(tamanioCentro-i),0,360*64 );
        if(i < 6){
            XDrawArc( display, win, lienzo, x-(tamanioCentro/2), (y+i)-(tamanioCentro/2),tamanioCentro,tamanioCentro,0,180*64 );
          //  XDrawArc( display, win, lienzo, x-(tamanioOnda/2), (y-i)-(tamanioOnda/2),tamanioOnda,tamanioOnda,180*64,180*64 );
        }
    }
    cord.y -= 31;
    XPoint picoA[] = {cord, { -10,10},{ 20,0}};
    cord.y = y + 32;
    XPoint picoB[] = { cord , { 10,-10},{ -20,0}};
    cord.x = x - 31; cord.y = y;
    XPoint picoC[] = { cord , { 10,10},{ 0,-20}};
    cord.x = x + 32;
    XPoint picoD[] = { cord , { -10,-10},{ 0,20}};
    cord.x = x - 20; cord.y = y - 20;
    XPoint picoAC[] = { cord , { 3,6},{ 4,-5}};
    cord.x = x - 19; cord.y = y + 20;
    XPoint picoCB[] = { cord , { 5,-3},{ -5,-4}};
    cord.x = x + 21; cord.y = y - 19;
    XPoint picoAD[] = { cord , { -3,6},{ -4,-5}};
    cord.x = x + 20; cord.y = y + 21;
    XPoint picoDB[] = { cord , { -5,-3},{ 4,-5}};
    XFillPolygon( display, win, lienzo, picoA, 3, Convex, CoordModePrevious );
    XFillPolygon( display, win, lienzo, picoB, 3, Convex, CoordModePrevious );
    XFillPolygon( display, win, lienzo, picoC, 3, Convex, CoordModePrevious );
    XFillPolygon( display, win, lienzo, picoD, 3, Convex, CoordModePrevious );
    XFillPolygon( display, win, lienzo, picoAC, 3, Convex, CoordModePrevious );
    XFillPolygon( display, win, lienzo, picoCB, 3, Convex, CoordModePrevious );
    XFillPolygon( display, win, lienzo, picoAD, 3, Convex, CoordModePrevious );
    XFillPolygon( display, win, lienzo, picoDB, 3, Convex, CoordModePrevious );
}

void drawBrillo(Display *display, Window win,int wAncho, int wAlto,XColor color, int x, int y,int nivel){
    GC lienzo;
    Colormap mapa;

    lienzo = XCreateGC( display, DefaultRootWindow(display), 0, 0 );
    mapa = DefaultColormap(display, DefaultScreen(display));
    XAllocColor(display, mapa, &color);
    XSetForeground( display, lienzo, color.pixel);
    XSetFillRule( display, lienzo, WindingRule );

    XClearArea(display,win,(x-40),0,(wAncho-70),(wAlto-22),false);
    drawSol(display, win,lienzo, x, y);
    drawIndicador(display, win,color,wAncho,wAlto,nivel);
    XFlush(display);
}

uint64_t get_Brightness(char const *filename)
{
    FILE *fp;
    uint64_t data = -1;

    fp = fopen(filename, "r");
    if(!fp)
    {
        printf("error reading %s\n",filename);
        return data;
    }

    if(fscanf(fp, "%lu", &data) != 1)
    {
        printf("Couldn't parse an unsigned integer from '%s'", filename);
        fclose(fp);
        return -2;
    }

    fclose(fp);
    return data;
}

struct stdObj
{
    uint64_t max_value;
    uint64_t value;
    char driver[NAME_MAX];
    char type[NAME_MAX];
    bool active;
};

struct ctlScreenShot
{
    long bgcolor; 
    const char *fileScreenshot;
    long size_screenshot;
    Atom XA_image_png;
    Atom XA_TARGETS;
    bool active;
};

struct ctlObj
{
    stdObj *backlight; 
    stdObj *volume;
    ctlScreenShot *ScreenShot;
    bool mute;
};

unsigned char set_Brightness(stdObj *data)
{
    FILE *fp;
    char brightness[NAME_MAX];

    snprintf(brightness, sizeof(brightness), "%s/%s/%s/brightness",dev_class,data->type,data->driver);
        
    fp = fopen(brightness, "w");
    if(!fp)
    {
        //printf("error writing %s\n",brightness);
        return 1;
    }

    if(fprintf(fp, "%lu", data->value) < 0)
    {
        //printf("fprintf failed");
        fclose(fp);
        return 2;
    }

    fclose(fp);
    return 0;
}

bool flush_brightness(stdObj *data){
    FILE *pFicheroDat;
    static char *home = NULL;
    char dev_dir[NAME_MAX];

     if(!home){
        home = getenv("HOME");
      }

      if(!home){
        struct passwd *pw = getpwuid(getuid());
        if(pw)
          home = pw->pw_dir;
      }

    snprintf(dev_dir, sizeof(dev_dir), "%s/%s/%s.dat",home,app_config,data->type);

    pFicheroDat=fopen(dev_dir,"wb");
    if(pFicheroDat!=NULL){
        fwrite(data,sizeof(stdObj),1,pFicheroDat);
        fclose(pFicheroDat);
        return true;
    }else{
        return false;
    }
}
bool init_brightness(stdObj **data,std::string type){
    DIR *brightness_dir;
    struct dirent *curr_entry;
    bool status = false;

    char brightness[NAME_MAX],max_brightness[NAME_MAX], dev_dir[NAME_MAX];

    stdObj *return_Data = (stdObj*)malloc(sizeof(stdObj));

    FILE *pFicheroExist;
    static char *home = NULL;

     if(!home){
        home = getenv("HOME");
      }

      if(!home){
        struct passwd *pw = getpwuid(getuid());
        if(pw)
          home = pw->pw_dir;
      }

    snprintf(dev_dir, sizeof(dev_dir), "%s/%s/%s.dat",home,app_config,type.c_str());

    pFicheroExist=fopen(dev_dir,"rb");
    if(pFicheroExist!=NULL){
        fread(return_Data,sizeof(stdObj),1,pFicheroExist);
        fclose(pFicheroExist);
        snprintf(brightness, sizeof(brightness), "%s/%s/%s/brightness",dev_class,return_Data->type,return_Data->driver);
        return_Data->value = get_Brightness(brightness);
        status = true;
    }else{
        
        snprintf(dev_dir, sizeof(dev_dir), "%s/%s", dev_class,type.c_str());
    
        if((brightness_dir = opendir(dev_dir)) == NULL)
        {
            printf("failed to open backlight controller directory for reading");
            return status;
        }
        
        return_Data->max_value = 0;
        //memset(return_Data->type, 0, sizeof(return_Data->type));
        snprintf(return_Data->type, sizeof(return_Data->type), "%s", type.c_str());

        while((curr_entry = readdir(brightness_dir)) != NULL)
        {
            // Skip dot entries
            if(curr_entry->d_name[0] == '.')
            {
                continue;
            }
            
            // Setup the target data 
            snprintf(max_brightness, sizeof(max_brightness), "%s/%s/max_brightness", dev_dir,curr_entry->d_name);
            
            // Read the max brightness to get the best one
            uint64_t curr_value = get_Brightness(max_brightness);

            if(curr_value > return_Data->max_value)
            {
                return_Data->max_value = curr_value;
                //memset(return_Data->driver, 0, sizeof(return_Data->driver));
                snprintf(return_Data->driver, sizeof(return_Data->driver), "%s", curr_entry->d_name);
                status = true;
            }
            
        }

        closedir(brightness_dir);
        snprintf(brightness, sizeof(brightness), "%s/%s/brightness",dev_dir, return_Data->driver);
        return_Data->value = get_Brightness(brightness);

        status = flush_brightness(return_Data);
    }

    *data = return_Data;
    return status;
}
void send_WN(Display *display, Window win){
    XEvent evt;
    evt.xclient.type = ClientMessage;
    evt.xclient.serial = 0;
    evt.xclient.send_event = true;
    evt.xclient.message_type = XInternAtom(display,"SOLAR_WM",false);;
    evt.xclient.format = 8;
    evt.xclient.window = win;
    evt.xclient.data.b[0] = '*';

    XSendEvent(display, DefaultRootWindow(display),true,(SubstructureRedirectMask | SubstructureNotifyMask),&evt);
    XFlush(display);
}
bool init_volume(stdObj **data,std::string card, std::string type, bool *mute){
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    snd_mixer_elem_t* elem;
    long min_and_val, max;
    stdObj *data_volume = (stdObj*)malloc(sizeof(stdObj));
    bool status = false;
    int val_mute;

    data_volume->active = true;
    snprintf(data_volume->driver, sizeof(data_volume->driver), "%s",card.c_str());
    snprintf(data_volume->type, sizeof(data_volume->type), "%s",type.c_str());

    if(snd_mixer_open(&handle, 0) != 0){
        data_volume->max_value = -3;
        *data = data_volume;
        return status;
    }

    if(snd_mixer_attach(handle, data_volume->driver) != 0){
        snd_mixer_close(handle);
        data_volume->max_value = -4;
        *data = data_volume;
        return status;
    }

    if(snd_mixer_selem_register(handle, NULL, NULL) != 0){
        snd_mixer_close(handle);
        data_volume->max_value = -5;
        *data = data_volume;
        return status;
    }

    if(snd_mixer_load(handle) != 0){
        snd_mixer_close(handle);
        data_volume->max_value = -6;
        *data = data_volume;
        return status;
    }

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, data_volume->type);
    elem = snd_mixer_find_selem(handle, sid);
    if(elem == NULL){
        snd_mixer_close(handle);
        data_volume->max_value = -7;
        *data = data_volume;
        return status;
    }

    snd_mixer_selem_get_playback_switch(elem,SND_MIXER_SCHN_MONO,&val_mute); 
    snd_mixer_selem_get_playback_volume_range(elem, &min_and_val, &max); 
    snd_mixer_selem_get_playback_volume(elem,SND_MIXER_SCHN_MONO,&min_and_val); 
    snd_mixer_close(handle);

    data_volume->value = min_and_val;
    data_volume->max_value = max;
    *mute = !(bool)val_mute;
    *data = data_volume;

    return !status;
}

unsigned char set_volume(stdObj *data_volume, bool mute){
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    snd_mixer_elem_t* elem;

    if(snd_mixer_open(&handle, 0) != 0){
        return 3;
    }

    if(snd_mixer_attach(handle, data_volume->driver) != 0){
        snd_mixer_close(handle);
        return 4;
    }

    if(snd_mixer_selem_register(handle, NULL, NULL) != 0){
        snd_mixer_close(handle);
        return 5;
    }

    if(snd_mixer_load(handle) != 0){
        snd_mixer_close(handle);
        return 6;
    }

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, data_volume->type);
    elem = snd_mixer_find_selem(handle, sid);
    if(elem == NULL){
        snd_mixer_close(handle);
        return 7;
    }

    snd_mixer_selem_set_playback_switch(elem,SND_MIXER_SCHN_MONO,!mute);
    snd_mixer_selem_set_playback_volume_all(elem, data_volume->value);


    snd_mixer_close(handle);


    return 0;
}

void drawCamera(Display *display, Window w,GC lienzo,char *bgcolor, int x, int y){
    int tamanioCentro = 25;
    XColor colorLente;
    Colormap mapa;

    XFillRectangle(display, w, lienzo,x-30,y-20,60,40);
    XFillRectangle(display, w, lienzo,x-30,y-23,10,5);
    XFillRectangle(display, w, lienzo,x-15,y-28,35,10);

    mapa = DefaultColormap(display, DefaultScreen(display));
    XParseColor(display, mapa, bgcolor,&colorLente);
    XAllocColor(display, mapa, &colorLente);
    XSetForeground( display, lienzo, colorLente.pixel);
    XDrawArc( display, w, lienzo, x-(tamanioCentro/2), y-(tamanioCentro/2),tamanioCentro,tamanioCentro,0,360*64 );
    for (int i = 0; i < tamanioCentro-20; ++i)
    {
        XDrawArc( display, w, lienzo, x-((tamanioCentro-i)/2), y-((tamanioCentro-i)/2),(tamanioCentro-i),(tamanioCentro-i),0,360*64 );
    }
}

void drawScreenShot(Display *display, Window win,int wAncho, int wAlto,XColor color, long bgcolor, int x, int y,int nivel){
    GC lienzo;
    Colormap mapa;
    char BGColor[7];

    lienzo = XCreateGC( display, DefaultRootWindow(display), 0, 0 );
    mapa = DefaultColormap(display, DefaultScreen(display));
    XAllocColor(display, mapa, &color);
    XSetForeground( display, lienzo, color.pixel);
    XSetFillRule( display, lienzo, WindingRule );

    sprintf(BGColor,"#%06x", bgcolor);

    XClearArea(display,win,(x-40),0,(wAncho-70),(wAlto-22),false);
    drawCamera(display, win,lienzo,BGColor, x, y);
    drawIndicador(display, win,color,wAncho,wAlto,nivel);
    XFlush(display);
}

int write_png_for_image(XImage *image, int width, int height, const char *filename) {

    int code = 0;
    FILE *fp;
    png_structp png_ptr;
    png_infop png_info_ptr;
    png_bytep png_row;

    //char buffer[50];
    //int n;

    //n = sprintf(buffer, filename);

// Open file
    fp = fopen(filename, "wb");
    if (fp == NULL) {
        //fprintf(stderr, "Could not open file for writing\n");
        code = -1;
        return code;
    }

// Initialize write structure
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        //fprintf(stderr, "Could not allocate write struct\n");
        code = -2;
        return code;
    }

// Initialize info structure
    png_info_ptr = png_create_info_struct(png_ptr);
    if (png_info_ptr == NULL) {
        //fprintf(stderr, "Could not allocate info struct\n");
        code = -3;
        return code;
    }

// Setup Exception handling
    if (setjmp(png_jmpbuf (png_ptr))) {
        //fprintf(stderr, "Error during png creation\n");
        code = -4;
        return code;
    }

    png_init_io(png_ptr, fp);

// Write header (8 bit colour depth)
    png_set_IHDR(png_ptr, png_info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

// Set title
    /*char *title = "Screenshot";
    if (title != NULL) {
        png_text title_text;
        title_text.compression = PNG_TEXT_COMPRESSION_NONE;
        title_text.key = "Title";
        title_text.text = title;
        png_set_text(png_ptr, png_info_ptr, &title_text, 1);
    }*/

    png_write_info(png_ptr, png_info_ptr);

// Allocate memory for one row (3 bytes per pixel - RGB)
    png_row = (png_bytep) malloc(3 * width * sizeof(png_byte));

    unsigned long red_mask = image->red_mask;
    unsigned long green_mask = image->green_mask;
    unsigned long blue_mask = image->blue_mask;

// Write image data
//int xxx, yyy;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned long pixel = XGetPixel(image, x, y);
            unsigned char blue = pixel & blue_mask;
            unsigned char green = (pixel & green_mask) >> 8;
            unsigned char red = (pixel & red_mask) >> 16;
            png_byte *ptr = &(png_row[x * 3]);
            ptr[0] = red;
            ptr[1] = green;
            ptr[2] = blue;
        }
        png_write_row(png_ptr, png_row);
    }

// End write
    png_write_end(png_ptr, NULL);

// Read size file
    fseek(fp,0,SEEK_END);
    code = ftell(fp);

// Free
    fclose(fp);
    if (png_info_ptr != NULL)
        png_free_data(png_ptr, png_info_ptr, PNG_FREE_ALL, -1);
    if (png_ptr != NULL)
        png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
    if (png_row != NULL)
        free(png_row);

    return code;
}

int getScreenShot(Display *display, const char *fileScreenshot){
   int x_pos, y_pos, mouse_root_x, mouse_root_y;
    int _rMononitor, _winX, _winY;
    unsigned int _mask_return;
    Window _window_returned;
    XRRMonitorInfo *info = XRRGetMonitors(display,DefaultRootWindow(display),0,&_rMononitor);
    XQueryPointer(display, DefaultRootWindow(display), &_window_returned,
                &_window_returned, &mouse_root_x, &mouse_root_y, &_winX, &_winY,
                &_mask_return);
    if(mouse_root_x >= info->width){
        x_pos = info->width;
    }else{
        x_pos = 0;
    }

    if(mouse_root_y >= info->height){
        y_pos = info->height;
    }else{
        y_pos = 0;
    }

    XImage *image = XGetImage(display,DefaultRootWindow(display), x_pos, y_pos, info->width, info->height, AllPlanes, ZPixmap);
    return write_png_for_image(image, info->width, info->height,fileScreenshot); 
}

void selectAction(int option,Display *display, Window win,ctlObj *ctl,XColor color){
   if(option == BrightnessUp){
        unsigned char porc = ((ctl->backlight->value*100)/ctl->backlight->max_value);//+porcentaje; 
        if(ctl->backlight->active){
            porc += porcentaje;
            ctl->backlight->value = ((porc*ctl->backlight->max_value)/100);
            XMapRaised(display, win);
        }
        if(ctl->backlight->value > ctl->backlight->max_value)
            ctl->backlight->value = ctl->backlight->max_value;
        drawBrillo(display,win,220,90,color,110,35,porc*2);
        if(ctl->backlight->active)
            set_Brightness(ctl->backlight);
    } 
    if(option == BrightnessDown){
        /*unsigned char */int porc = ((ctl->backlight->value*100)/ctl->backlight->max_value);//-porcentaje; 
        if(ctl->backlight->active){
            porc -= porcentaje;
            ctl->backlight->value = ((porc*ctl->backlight->max_value)/100);
            XMapRaised(display, win);
        }
        if(porc < 0)
            porc = ctl->backlight->value = 0;

         drawBrillo(display,win,220,90,color,110,35,porc*2);
        if(ctl->backlight->active)
            set_Brightness(ctl->backlight);
    }

    if(option == VolumeUp){
        unsigned char porc = ((ctl->volume->value*100)/ctl->volume->max_value);//+porcentaje; 
        if(ctl->volume->active){
            porc += porcentaje;
            ctl->volume->value = ((porc*ctl->volume->max_value)/100);
            XMapRaised(display, win);
        }
        if(ctl->volume->value > ctl->volume->max_value)
            ctl->volume->value = ctl->volume->max_value;
        drawVolumen(display,win,220,90,color,70,45,porc*2);
        if(ctl->volume->active){
            ctl->mute = false;
            set_volume(ctl->volume,ctl->mute);
        }
    } 

    if(option == VolumeDown){
        int porc = ((ctl->volume->value*100)/ctl->volume->max_value);//-porcentaje; 
        if(ctl->volume->active){
            porc -= porcentaje;
            ctl->volume->value = ((porc*ctl->volume->max_value)/100);
            XMapRaised(display, win);
        }
        if(porc < 0)
            porc = ctl->volume->value = 0;
        
        drawVolumen(display,win,220,90,color,70,45,porc*2);
        if(ctl->volume->active){
            ctl->mute = false;
            set_volume(ctl->volume,ctl->mute);
        }
    }

    if(option == Mute){
        unsigned char porc = ((ctl->volume->value*100)/ctl->volume->max_value);

        if(ctl->volume->active){
            ctl->mute = !ctl->mute;
            XMapRaised(display, win);
        }
        drawVolumen(display,win,220,90,color,70,45,porc*2,ctl->mute);
        
        if(ctl->volume->active)
            set_volume(ctl->volume,ctl->mute);   
    }

    if(option == PrintScreen){
        //char* arg[] =  {(char *)"-sel",(char *)"clip",(char *)"-i",(char *)"-t",(char *)ctl->ScreenShot->XA_image_png.c_str(),(char *)"<",(char *)ctl->ScreenShot->fileScreenshot};
        //drawScreenShot(display, win,220, 90,color,ctl->ScreenShot->bgcolor, 110, 40,200);
        if(ctl->ScreenShot->active){
            ctl->ScreenShot->size_screenshot = getScreenShot(display,ctl->ScreenShot->fileScreenshot);
            
            XMapRaised(display, win);
            /* pid_t child_pid;
            signal(SIGCHLD, SIG_IGN); //este ya no me crea los zombies
            child_pid = fork();
            if(child_pid == 0) {
                execv(ctl->ScreenShot->clipboard.c_str(),arg);
                exit(EXIT_SUCCESS);
            }
            */
            /* Claim ownership of the clipboard. */
            XSetSelectionOwner(display,XInternAtom(display, "CLIPBOARD", false), win, CurrentTime);
        }
        drawScreenShot(display, win,220, 90,color,ctl->ScreenShot->bgcolor, 110, 40,200);
    }
}

void movWindow(Display *display, Window win, int w_win){
    int x_window, y_window,rMononitor, mouse_root_x, mouse_root_y,win_x, win_y,porcentajeH = 32;
    unsigned int mask_return;
    Window window_returned;
    XRRMonitorInfo *info = XRRGetMonitors(display,DefaultRootWindow(display),0,&rMononitor);
    XQueryPointer(display, DefaultRootWindow(display), &window_returned,
                &window_returned, &mouse_root_x, &mouse_root_y, &win_x, &win_y,
                &mask_return);
    if(mouse_root_x >= info->width){
        x_window = ((info->width/2)-(w_win/2) + info->width);
    }else{
        x_window = ((info->width/2)-(w_win/2));
    }

    if(mouse_root_y >= info->height){
        y_window = (info->height - ((info->height*porcentajeH)/100)) + info->height;
    }else{
        y_window = info->height - ((info->height*porcentajeH)/100);
    }
    XMoveWindow(display,win,x_window,y_window);

}

void sendNoClipboard(Display *display, XSelectionRequestEvent *sev)
{
    XSelectionEvent ssev;

    // All of these should match the values of the request. 
    ssev.type = SelectionNotify;
    ssev.requestor = sev->requestor;
    ssev.selection = sev->selection;
    ssev.target = sev->target;
    ssev.property = None;  // signifies "nope" 
    ssev.time = sev->time;

    XSendEvent(display, sev->requestor, true, NoEventMask, (XEvent *)&ssev);
}

void sendPNGClipboard(Display *display, XSelectionRequestEvent *sev, ctlScreenShot *clipboard)
{
    XSelectionEvent ssev;
    FILE *source;
    char *contenido;

    source = fopen(clipboard->fileScreenshot, "rb");

    if (source) {
        contenido = (char*) malloc(clipboard->size_screenshot);
        fread(contenido, 1, clipboard->size_screenshot, source);
    } else {
        sendNoClipboard(display,sev);
    }

    fclose(source);

    XChangeProperty(display, sev->requestor, sev->property, clipboard->XA_image_png, 8, PropModeReplace,
                    reinterpret_cast<const unsigned char*>(contenido), clipboard->size_screenshot);

    ssev.type = SelectionNotify;
    ssev.requestor = sev->requestor;
    ssev.selection = sev->selection;
    ssev.target = sev->target;
    ssev.property = sev->property;
    ssev.time = sev->time;

    XSendEvent(display, sev->requestor, true, NoEventMask, (XEvent *)&ssev);

   /* source = fopen("/tmp/in_screenshot", "wb");
    fwrite(contenido,1,clipboard->size_screenshot,source);
    fclose(source);*/
}

void sendTargets(Display *display, XSelectionRequestEvent *sev,ctlScreenShot *clipboard)
{
    XSelectionEvent ssev;
    Atom targets[] = {clipboard->XA_TARGETS,clipboard->XA_image_png};

    XChangeProperty(display, sev->requestor, sev->property, XA_ATOM, 32, PropModeReplace,
                    (unsigned char*)(&targets), sizeof(targets));

    ssev.type = SelectionNotify;
    ssev.requestor = sev->requestor;
    ssev.selection = sev->selection;
    ssev.target = sev->target;
    ssev.property = sev->property;
    ssev.time = sev->time;

    XSendEvent(display, sev->requestor, true, NoEventMask, (XEvent *)&ssev);

   /* source = fopen("/tmp/in_screenshot", "wb");
    fwrite(contenido,1,clipboard->size_screenshot,source);
    fclose(source);*/
}

int main(int arc, char** arv)
{
    XEvent e;
    Display *display;
    Window win;
    ctlObj ctlBVS;//*ctlBV = (ctlObj*)malloc(sizeof(ctlObj));
    //stdObj *backlight, *volume;
    int option = -1;
    int screen,w_win=220;
    bool done = false;
    long bgColor = 0;
    XColor color;
    color.red = 0;
    color.green = 0;
    color.blue = 0;
    ctlBVS.ScreenShot = (ctlScreenShot*)malloc(sizeof(ctlScreenShot));
    

    if(arc > 1){
        if(strlen(arv[1]) == 6){
            char col[1];
            col[0] = arv[1][0];
            col[1] = arv[1][1];
            color.red = 257 * std::stol(col,nullptr,16);
            col[0] = arv[1][2];
            col[1] = arv[1][3];
            color.green = 257 * std::stol(col,nullptr,16);
            col[0] = arv[1][4];
            col[1] = arv[1][5];
            color.blue = 257 * std::stol(col,nullptr,16);
        }
    }

    if(arc > 2)
        bgColor = std::stol(arv[2],nullptr,16);

    //color.red = 257 * 0x35;
    //color.green = 257 * 0x37;
   // color.blue = 257 * 0xb8;

    /* open connection with the server */
    display = XOpenDisplay(NULL);
    if (display == NULL)
    {
        //fprintf(stderr, "Cannot open display\n");
        //exit(1);
        return -1;
    }
    screen = DefaultScreen(display);
    win = showIndicador(display, screen, w_win, 90,bgColor);
    
    
    if(!init_brightness(&ctlBVS.backlight,"backlight")){
        return -2;
    }

    if(!init_volume(&ctlBVS.volume,"default", "Master",&ctlBVS.mute)){
        return ctlBVS.volume->max_value;
    }
    
    ctlBVS.ScreenShot->bgcolor = bgColor; 
    ctlBVS.ScreenShot->fileScreenshot = {"/tmp/out_screenshot"};
    ctlBVS.ScreenShot->size_screenshot = 0;
    ctlBVS.ScreenShot->XA_image_png = /*"image/png";*/ XInternAtom(display, "image/png", false);
    ctlBVS.ScreenShot->XA_TARGETS = XInternAtom(display, "TARGETS", false);

    //return 0;
    //ctlBVS.ScreenShot->clipboard = "xclip";
    //size_screenshot = getScreenShot(display,fileScreenshot);
    //XMapRaised(display, win);
   /* GC lienzo;
    Colormap mapa;

    lienzo = XCreateGC( display, DefaultRootWindow(display), 0, 0 );
    mapa = DefaultColormap(display, DefaultScreen(display));
    XAllocColor(display, mapa, &color);
    XSetForeground( display, lienzo, color.pixel);
    XSetFillRule( display, lienzo, WindingRule );


    //XClearArea(display,win,x,0,(wAncho-70),(wAlto-22),false);
    drawIndicador(display, win,color,w_win,90,100 * 2);*/
    //printf("Listonessss...%d\n",size_screenshot);
    
    /*FILE *source;
    char *contenido;

    source = fopen(fileScreenshot, "rb");

    if (source) {
        contenido = (char*) malloc(size_screenshot);
        fread(contenido, 1, size_screenshot, source);
    } else {
        printf("fail\n");
    }

    fclose(source);*/

    /*source = fopen("/tmp/in_screenshot", "wb");
    fwrite(contenido,1,size_screenshot,source);
    fclose(source);*/

//printf("%s \n",ctlBVS.ScreenShot->fileScreenshot);
//printf("\ncontenido %d \n",size);
    //return 1;
    //XMapRaised(display, e.xclient.window);

    //ctlBV.mute = false;
    //printf("Driver = %s Tipo = %s Max = %lu Actual = %lu \n",backlight->driver,backlight->type,backlight->max_value,backlight->value);
    //printf("Driver = %s Max = %lu Actual = %lu \n",backlight->driver,backlight->max_value,backlight->value);
    /*long value, max,min;
    bool muted = true;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "Master";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume(elem,SND_MIXER_SCHN_MONO,&value);
    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);

    //snd_mixer_selem_get_playback_switch(elem,SND_MIXER_SCHN_MONO,&val); 
    snd_mixer_selem_set_playback_switch(elem,SND_MIXER_SCHN_MONO,!muted); //mute val 0
//sleep(1);
    //snd_mixer_selem_get_playback_switch(elem,SND_MIXER_SCHN_MONO,&val); 
    snd_mixer_selem_set_playback_switch(elem,SND_MIXER_SCHN_MONO,muted); //unmute val 1
   
    snd_mixer_close(handle);*/
    //printf("valor=%lu Min=lu Max=%lu \n",volume->value,volume->max_value);
   // return 0;

    std::thread tarea(subProceso,2,win);

    
    
    //tarea.join(); // espera a terminar tarea
    //tarea.detach();
    
    //hideCursor(display,win);
    //showCursor(display,win);                 

 /*///////////////////////////////////////////////////////////////////////////////////////
 //int wid = DisplayWidth(display, screen),hei = DisplayHeight(display, screen);
    
 ////////////////////////////////////////////////////////////////////////////////////////*/
    /* register interest in the delete window message */
    //XMapWindow(display, win);
    /*XEvent evt;
            //Display* display = XOpenDisplay(XDisplayName(NULL));

            //if(display != NULL){
                evt.xclient.type = ClientMessage;
                evt.xclient.serial = 0;
                evt.xclient.send_event = true;
                evt.xclient.message_type = XInternAtom(display,"DISPLAY_BRIGHTNESS_VOLUME_SCREEN",false);;
                evt.xclient.format = 32;
                evt.xclient.window = win;
                evt.xclient.data.b[0] = BrightnessUp;

                XSendEvent(display,win,false,ExposureMask,&evt);

                XMapWindow(display, win);
                XFlush(display);*/
           // }    
    //printf("%lu\n",win);

    tarea.detach();
    /* event loop */
    //int inc = 0;
    send_WN(display,win);
    while (!done)
    {
        XNextEvent(display, &e);
        /* draw or redraw the window */
        if (e.type == Expose)//se ejecuta con el mapeo
        {
            ctlBVS.ScreenShot->active = ctlBVS.volume->active = ctlBVS.backlight->active = false;
            //selectActionBrightness(option,display,win,ctlBVS.backlight,color);
            selectAction(option,display,win,&ctlBVS,color);
        }
        if(e.type == ClientMessage){

            if(e.xclient.message_type == XInternAtom(display,"DISPLAY_BRIGHTNESS_VOLUME_SCREEN",false)){
                if(e.xclient.data.b[0] == TimeOut){//time out

                    /*if(inc == 0){
                        //tiempo = clock();
                        inc++;
                        //XUnmapWindow(display,win);
                        XMapWindow(display,win);
                        drawIndicador(display, win,color,220,90,170);
                        XFlush(display);
                        strTime = true;
                    }else{
                        XMapWindow(display,win);
                        drawIndicador(display, win,color,220,90,200);
                        XFlush(display);
                        sleep(1);
                        pTarea = false;
                        done = true;
                    }*/
                    XUnmapWindow(display,win);
                }else{
                    if(e.xclient.data.b[0] == ThemeColor){
                        if(strlen(e.xclient.data.b) >= 13){
                            char col[] = {'\0','\0','\0','\0','\0','\0'};
                            col[0] = e.xclient.data.b[1];
                            col[1] = e.xclient.data.b[2];
                            color.red = 257 * std::stol(col,nullptr,16);
                            col[0] = e.xclient.data.b[3];
                            col[1] = e.xclient.data.b[4];
                            color.green = 257 * std::stol(col,nullptr,16);
                            col[0] = e.xclient.data.b[5];
                            col[1] = e.xclient.data.b[6];
                            color.blue = 257 * std::stol(col,nullptr,16);

                            col[0] = e.xclient.data.b[7];
                            col[1] = e.xclient.data.b[8];
                            col[2] = e.xclient.data.b[9];
                            col[3] = e.xclient.data.b[10];
                            col[4] = e.xclient.data.b[11];
                            col[5] = e.xclient.data.b[12];

                            bgColor = std::stol(col,nullptr,16);
                            XSetWindowBackground(display,win,bgColor);
                        }
                    }else{                        
                        /*if(e.xclient.data.b[0] == 'w'){
                        send_WN(display,win);
                        }else{*/
                        movWindow(display,win,w_win); //lo movi primero
                        ctlBVS.ScreenShot->active = ctlBVS.volume->active = ctlBVS.backlight->active = strTime = true;
                        option = e.xclient.data.b[0];
                        //XMapRaised(display, e.xclient.window);
                        selectAction(option,display,win,&ctlBVS,color);
                        //selectActionVolume(option,display,win,ctlBVS.volume,color);
                        //}  
                    } 
                } 
                //printf("Opcion %c\n",e.xclient.data.b[0]);
            }
        }
        if (e.type == SelectionRequest)
        {
            XSelectionRequestEvent *sev = (XSelectionRequestEvent*)&e.xselectionrequest;
            if(sev->target == ctlBVS.ScreenShot->XA_TARGETS){
                sendTargets(display, sev,ctlBVS.ScreenShot);
            }else{
                if (sev->target != ctlBVS.ScreenShot->XA_image_png || sev->property == None)
                    sendNoClipboard(display, sev);
                else
                    sendPNGClipboard(display, sev, ctlBVS.ScreenShot);

            /*Window owner     = e.xselectionrequest.owner;
            Atom selection   = e.xselectionrequest.selection;
            Atom target      = e.xselectionrequest.target;
            Atom property    = e.xselectionrequest.property;
            Window requestor = e.xselectionrequest.requestor;


            printf("%ld -- %ld\n",owner,win );
            printf("Selection atom = %s\n",XGetAtomName(display, selection));
            printf("Target atom    = %s\n", XGetAtomName(display, target));
            printf("Property atom  = %s\n" , XGetAtomName(display, property));
            printf("Requestor = %ld\n", requestor);*/

        }
                
            
        }

    }
    
    XUnmapWindow(display,win);
    XDestroyWindow(display, win);
    XFlush(display);
    XCloseDisplay(display);

    return 0;
}