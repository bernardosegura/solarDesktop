/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                           *
 *  Copyright (C) 2013-2017 Chuan Ji <ji@chu4n.com>                          *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *   http://www.apache.org/licenses/LICENSE-2.0                              *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 *   Update from solar-wm by Bernardo Segura                                 *
 *                                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "window_manager.hpp"
extern "C" {
#include <X11/Xutil.h>
}
#include <cstring>
#include <algorithm>
#include <glog/logging.h>
#include "util.hpp"

//#include <thread>

//#include <sys/type.h>
#include <pwd.h>

using ::std::max;
using ::std::mutex;
using ::std::string;
using ::std::unique_ptr;

bool WindowManager::wm_detected_;
mutex WindowManager::wm_detected_mutex_;

unique_ptr<WindowManager> WindowManager::Create(const string& display_str) {
  // 1. Open X display.
  const char* display_c_str =
        display_str.empty() ? nullptr : display_str.c_str();
  Display* display = XOpenDisplay(display_c_str);
  if (display == nullptr) {
    LOG(ERROR) << "Failed to open X display " << XDisplayName(display_c_str);
    return nullptr;
  }


  // 2. Construct WindowManager instance.
  return unique_ptr<WindowManager>(new WindowManager(display));
}

WindowManager::WindowManager(Display* display)
    : display_(CHECK_NOTNULL(display)),
      root_(DefaultRootWindow(display_)),
      WM_PROTOCOLS(XInternAtom(display_, "WM_PROTOCOLS", false)),
      WM_DELETE_WINDOW(XInternAtom(display_, "WM_DELETE_WINDOW", false)) {
}

WindowManager::~WindowManager() {
  XCloseDisplay(display_);
}

void WindowManager::Run() {
  // 1. Initialization.
  //   a. Select events on root window. Use a special error handler so we can
  //   exit gracefully if another window manager is already running.
  {
    ::std::lock_guard<mutex> lock(wm_detected_mutex_);

    wm_detected_ = false;
    XSetErrorHandler(&WindowManager::OnWMDetected);
    XSelectInput(
        display_,
        root_,
        SubstructureRedirectMask | SubstructureNotifyMask);
    XSync(display_, false);
    if (wm_detected_) {
      LOG(ERROR) << "Detected another window manager on display "
                 << XDisplayString(display_);
      return;
    }
  }
  //   b. Set error handler.
  XSetErrorHandler(&WindowManager::OnXError);
  //   c. Grab X server to prevent windows from changing under us.
  
  XGrabServer(display_);
  //   d. Reparent existing top-level windows.
  //     i. Query existing top-level windows.
  
  
  Window returned_root, returned_parent;
  Window* top_level_windows;
  unsigned int num_top_level_windows;
  CHECK(XQueryTree(
      display_,
      root_,
      &returned_root,
      &returned_parent,
      &top_level_windows,
      &num_top_level_windows));
  CHECK_EQ(returned_root, root_);

//  LOG(INFO) << "num_top_level_windows: " << num_top_level_windows << " -- " << (num_top_level_windows/2);
  //     ii. Frame each top-level window.
  if(wndPanel == 0)
    for (unsigned int i = num_top_level_windows - 1; i < num_top_level_windows; ++i) {
      Frame(top_level_windows[i], true);
    }
  else
    for (unsigned int i = 0; i < num_top_level_windows; ++i) {
      Frame(top_level_windows[i], true);
    }
  //     iii. Free top-level window array.
  XFree(top_level_windows);

  //   e. Ungrab X server.
  XUngrabServer(display_);

 //pasamos el foco al panel desde el incio del administrador de ventana
  XRaiseWindow(display_, panel[0]);
  XSetInputFocus(display_, panel[1], RevertToPointerRoot, CurrentTime);

  ///hiloa
  /*XInitThreads();
  std::thread t(hiloWnd);
  t.join();*/

  // 2. Main event loop.
  for (;;) {
    // 1. Get next event.
    XEvent e;
    XNextEvent(display_, &e);
    //LOG(INFO) << "Received event: " << ToString(e);

    // 2. Dispatch event.
    switch (e.type) {
      case CreateNotify:
        OnCreateNotify(e.xcreatewindow);
        break;
      case DestroyNotify:
        OnDestroyNotify(e.xdestroywindow);
        break;
      case ReparentNotify:
        OnReparentNotify(e.xreparent);
        break;
      case MapNotify:
        OnMapNotify(e.xmap);
        break;
      case UnmapNotify:
        OnUnmapNotify(e.xunmap);
        break;
      case ConfigureNotify:
        OnConfigureNotify(e.xconfigure);
        break;
      case MapRequest:
        OnMapRequest(e.xmaprequest);
        break;
      case ConfigureRequest:
        OnConfigureRequest(e.xconfigurerequest);
        break;
      case ButtonPress:
        //e.xbutton.send_event = 1;
        //if(ctrl_L)
          OnButtonPress(e.xbutton);
        /*else{
         // OnEnterNotify(e.xbutton);
          OnButtonPress(e.xbutton);
          XAllowEvents(display_, ReplayPointer, e.xbutton.time);
          XSync(display_, 0); 
        }*/
        //XSendEvent(display_,e.xbutton.window,true,ButtonPressMask,&e);        
        break;
      case ButtonRelease:
        //e.xbutton.send_event = 1;
        OnButtonRelease(e.xbutton);
        //XSendEvent(display_,e.xbutton.window,true,ButtonReleaseMask,&e);
        //XAllowEvents(display_, ReplayPointer, e.xbutton.time);
        //XSync(display_, 0);
        break;
      case MotionNotify:
        // Skip any already pending motion events.
        while (XCheckTypedWindowEvent(
            display_, e.xmotion.window, MotionNotify, &e)) {}
        OnMotionNotify(e.xmotion);
        break;
      case KeyPress:
        OnKeyPress(e.xkey);
        break;
      case KeyRelease:
        OnKeyRelease(e.xkey);
        break;
      case FocusIn://EnterNotify:
        //LOG(INFO) << "Enter notify no, es focus in window " << e.xfocus.window << " foco en " << clientFocus[1];//e.xcrossing.window;
           //OnEnterNotify(e.xcrossing);
            OnFocusIn(e.xfocus);
        break;
      default:
        break;
        //LOG(WARNING) << "Ignored event";
    }
  }
}

void WindowManager::Frame(Window w, bool was_created_before_window_manager) {

 //LOG(INFO) << "Parametro " << configHome;

  // Visual properties of the frame to create.
  const unsigned int BORDER_WIDTH = 1;
  unsigned long BORDER_COLOR_S = BORDER_COLOR;
  unsigned long BG_COLOR_S = BG_COLOR;

  // We shouldn't be framing windows we've already framed.
  //CHECK(!clients_.count(w));
  // Se cambia por que el check bota la aplicacion  si no se cumple la condicion
  //es mas quenada para no repintar un frame si ya le pusimos uno a la ventana.
    if(clients_.count(w))return;

  // 1. Retrieve attributes of window to frame.
  XWindowAttributes x_window_attrs, x_root_attr;
  CHECK(XGetWindowAttributes(display_, w, &x_window_attrs));
  CHECK(XGetWindowAttributes(display_, root_, &x_root_attr));
  /*XGetWindowAttributes(display_, w, &x_window_attrs);
  XGetWindowAttributes(display_, root_, &x_root_attr);*/

if(was_created_before_window_manager && (wndPanel == 0 || wndPanel == w)){
  BORDER_COLOR_S = 0;//xff0000;
  BG_COLOR_S = 0;
  if(wndPanel == w){
    x_window_attrs.x = 0;
    x_window_attrs.y = 0;
  }
} 


  // 2. If window was created before window manager started, we should frame
  // it only if it is visible and doesn't set override_redirect.
  if (was_created_before_window_manager) {
    if (x_window_attrs.override_redirect ||
        x_window_attrs.map_state != IsViewable) {
      return;
    }
  }

if (/*!was_created_before_window_manager*/ x_window_attrs.x == 0 && x_window_attrs.y == 0) {
  if((x_window_attrs.width + 1) >= x_root_attr.width)
     x_window_attrs.width = x_root_attr.width - 2;

  if((x_window_attrs.height + 1) >= x_root_attr.height)
     x_window_attrs.height = x_root_attr.height - 2; 
  } 
//  LOG(INFO) << "Atributos Ventana" << x_window_attrs.width << " -- " << x_window_attrs.height;
//  LOG(INFO) << "Atributos Root" << x_root_attr.width << " -- " << x_root_attr.height;

//LOG(INFO) << "Atributos wnd " << x_window_attrs.x << ","<< x_window_attrs.y<< ","<< x_window_attrs.width<< ","<< x_window_attrs.height;
//LOG(INFO) << "Atributos override_redirect " << x_window_attrs.override_redirect;


  // 3. Create frame.
  const Window frame = XCreateSimpleWindow(
      display_,
      root_,
      x_window_attrs.x,
      x_window_attrs.y,
      x_window_attrs.width,
      x_window_attrs.height,
      BORDER_WIDTH,
      BORDER_COLOR_S,
      BG_COLOR_S);
  // 4. Select events on frame.
  XSelectInput(
      display_,
      frame,
      SubstructureRedirectMask | SubstructureNotifyMask);
  // 5. Add client to save set, so that it will be restored and kept alive if we
  // crash.

  XAddToSaveSet(display_, w);
  // 6. Reparent client window.
 XReparentWindow(
      display_,
      w,
      frame,
    0, 0);  // Offset of client window within frame.

  // 7. Map frame.
 XMapWindow(display_, frame);

  // 8. Save frame handle.
  if(was_created_before_window_manager){
    if(wndPanel == 0){
      panel[0] = frame;
      panel[1] = w;
      LOG(INFO) << "<--wndPanel-->" << w;
/////////////////////////////////////////////
      clientFocus[0] = panel[0];
      clientFocus[1] = panel[1];
      clientFocus[2] = panel[0];
      clientFocus[3] = panel[1];
/////////////////////////////////////////////      
    }else{
      if(wndPanel == w)
      {
        panel[0] = frame;
        panel[1] = w;
        LOG(INFO) << "<--wndPanel-->" << w;
/////////////////////////////////////////////
      clientFocus[0] = panel[0];
      clientFocus[1] = panel[1];
      clientFocus[2] = panel[0];
      clientFocus[3] = panel[1];
/////////////////////////////////////////////          
      }else{
       clients_[w] = frame; 
      }
    }
  }
  else
  //{
    clients_[w] = frame;
    /*wndAt wnd;
    wnd.x = x_window_attrs.x;
    wnd.y =  x_window_attrs.y;
    wnd.w =  x_window_attrs.width;
    wnd.h = x_window_attrs.height;
    wnd.f = 0;
    clientsAttr_[w] = wnd;
  }*/

  // 9. Grab universal window management actions on client window.
/*
  Mask        | Value | Key
------------+-------+------------
ShiftMask   |     1 | Shift
LockMask    |     2 | Caps Lock
ControlMask |     4 | Ctrl
Mod1Mask    |     8 | Alt
Mod2Mask    |    16 | Num Lock
Mod3Mask    |    32 | Scroll Lock
Mod4Mask    |    64 | Windows
Mod5Mask    |   128 | ???
*/
  if(!was_created_before_window_manager || (wndPanel != 0 && wndPanel != w)){

///////////////////////////////////Mover ventana y combinaciones posibles//////////////////////////////
     //   a. Move windows with ctrl + alt + left button.
    XGrabButton(
        display_,
        Button1,
        ControlMask | Mod1Mask,
        w,
        false,
        ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
        GrabModeAsync,
        GrabModeAsync,
        None,
        None);

    //   a. Move windows with ctrl + alt + left button + numLock.
    XGrabButton(
        display_,
        Button1,
        ControlMask | Mod1Mask | Mod2Mask,
        w,
        false,
        ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
        GrabModeAsync,
        GrabModeAsync,
        None,
        None);

    //   a. Move windows with ctrl + alt + left button + Block Mayus.
    XGrabButton(
        display_,
        Button1,
        ControlMask | Mod1Mask | LockMask,
        w,
        false,
        ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
        GrabModeAsync,
        GrabModeAsync,
        None,
        None);

    //   a. Move windows with ctrl + alt + left button + Block Mayus + numLock.
    XGrabButton(
        display_,
        Button1,
        ControlMask | Mod1Mask | LockMask | Mod2Mask,
        w,
        false,
        ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
        GrabModeAsync,
        GrabModeAsync,
        None,
        None);
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////Modificar ventana y combinaciones posibles//////////////////////////////    
    //   b. Resize windows with ctrl + alt + right button.
    XGrabButton(
        display_,
        Button3,
        ControlMask | Mod1Mask,
        w,
        false,
        ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
        GrabModeAsync,
        GrabModeAsync,
        None,
        None);

    XGrabButton(
        display_,
        Button3,
        ControlMask | Mod1Mask | Mod2Mask,
        w,
        false,
        ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
        GrabModeAsync,
        GrabModeAsync,
        None,
        None);

    XGrabButton(
        display_,
        Button3,
        ControlMask | Mod1Mask | LockMask,
        w,
        false,
        ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
        GrabModeAsync,
        GrabModeAsync,
        None,
        None);

    XGrabButton(
        display_,
        Button3,
        ControlMask | Mod1Mask | Mod2Mask | LockMask,
        w,
        false,
        ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
        GrabModeAsync,
        GrabModeAsync,
        None,
        None);
////////////////////////////////////////////////////////////////////////////////////////////////////////    
///////////////////////////////////Cerrar ventana y combinaciones posibles////////////////////////////// 
    //   c. Kill windows with alt + f4.
    XGrabKey(
        display_,
        XKeysymToKeycode(display_, XK_F4),
        Mod1Mask,
        w,
        false,
        GrabModeAsync,
        GrabModeAsync);

    XGrabKey(
        display_,
        XKeysymToKeycode(display_, XK_F4),
        Mod1Mask | Mod2Mask,
        w,
        false,
        GrabModeAsync,
        GrabModeAsync);

    XGrabKey(
        display_,
        XKeysymToKeycode(display_, XK_F4),
        Mod1Mask | LockMask,
        w,
        false,
        GrabModeAsync,
        GrabModeAsync);

    XGrabKey(
        display_,
        XKeysymToKeycode(display_, XK_F4),
        Mod1Mask | Mod2Mask | LockMask,
        w,
        false,
        GrabModeAsync,
        GrabModeAsync);
/////////////////////////////////////////////////////////////////////////////////////////////////////
    // e. tecla windows
    /* XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Super_L),
      AnyModifier,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);

  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Super_R),
      AnyModifier,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);*/

///////////////////////////////////Cambiar ventana y combinaciones posibles////////////////////////////// 
 // e. tecla windows + tab
   XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Tab),
      Mod4Mask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);

   XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Tab),
      Mod4Mask | Mod2Mask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);

   XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Tab),
      Mod4Mask | LockMask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);

   XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Tab),
      Mod4Mask | Mod2Mask | LockMask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);
///////////////////////////////////////////////////////////////////////////////////////////////////////////

    XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_F11),
      AnyModifier,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);

//Se trata separar el contrar y click del mouse
    /*XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Control_L),
      AnyModifier,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);*/

  }

///////////////////////////////////Mostrar Panel y combinaciones posibles//////////////////////////////   
  //   d. Switch windows with alt + tab.
  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Tab),
      Mod1Mask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);

  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Tab),
      Mod1Mask | Mod2Mask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);

  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Tab),
      Mod1Mask | LockMask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);

  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Tab),
      Mod1Mask | Mod2Mask | LockMask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);

    //   d. Switch windows with alt + tab + Ctrl
  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Tab),
      Mod1Mask | ControlMask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);

  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Tab),
      Mod1Mask | Mod2Mask | ControlMask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);

  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Tab),
      Mod1Mask | LockMask | ControlMask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);

  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Tab),
      Mod1Mask | Mod2Mask | LockMask | ControlMask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

// Se agrega el manejador del foco en la ventana hija, para ponerla al inicio si esta obtiene el foco demanera propia 
//  en casocontrario es necessario el ctrl + click 
 XSelectInput(
      display_,
      w,
      FocusChangeMask);//EnterWindowMask);*/

    /*XGrabButton(
        display_,
        Button1,
        AnyModifier,
        w,
        false,
        ButtonPressMask,// | ButtonReleaseMask | ButtonMotionMask,
        GrabModeSync,
        GrabModeAsync,
        None,
        None);*/

   /*XGrabButton(
        display_,
        Button1,
        AnyModifier,
        frame,
        false,
        ButtonPressMask,
        GrabModeAsync,
        GrabModeAsync,
        None,
        None);*/

  //LOG(INFO) << "Framed window " << w << " [" << frame << "]";
/*
      XMoveWindow(
            display_,
            frame,
 ////////////////Colocamos la ventana en 0 tambien ////////////////////////////////////////////////////////
      XMoveWindow(
            display_,
            w,
            0,0);
////////////////////////////////////////////////////////////////////////


        XResizeWindow(
            display_,
            frame,
            x_root_attr.width - 2, x_root_attr.height - 2);
        // 2. Resize client window.
        XResizeWindow(
            display_,
            w,
            x_root_attr.width - 2, x_root_attr.height - 2);


      XMoveWindow(
            display_,
            frame,
            x_window_attrs.x,x_window_attrs.y);

        XResizeWindow(
            display_,
            frame,
            x_window_attrs.width, x_window_attrs.height);
        // 2. Resize client window.
        XResizeWindow(
            display_,
            w,
            x_window_attrs.width, x_window_attrs.height);*/

  //////////////////////////Se verifica si la ventana tiene una sub ventana esto por que no se abren de manera adecuada////////////
 // ya no fue necesario el quitar la ventana se detecto   ue l crear la ventana esta cambia la posicion y manda llamar el configurerequest
  
  /* Window root, parent, *ch;
   unsigned int nch;
   XQueryTree(display_, w, &root, &parent, &ch, &nch);
  if(nch > 0){
    //LOG(INFO) << "windows " << frame <<" -- " << parent << " -- " << nch;
    XWindowAttributes x_w_attr;
    CHECK(XGetWindowAttributes(display_, ch[0], &x_w_attr));
// LOG(INFO) << "windows x=" << x_w_attr.x <<" y=" << x_w_attr.y <<"  height=" << x_w_attr.height <<" width=" << x_w_attr.width;
    
    if((w) > ch[0])
    {
      XUnmapWindow(display_,frame); 
      Frame(ch[0],false);
    }
     XFree(ch);
  }*/

  sendCountWindow();
}

void WindowManager::Unframe(Window w) {
  CHECK(clients_.count(w));

  // We reverse the steps taken in Frame().
  const Window frame = clients_[w];

/*//////////////////////////////////////////////////////
   XWindowAttributes x_frame_attrs;
  CHECK(XGetWindowAttributes(display_, frame, &x_frame_attrs));
////////////////////////////Ponemos la ventana en l apocision de frame//////////////////////////
   XMoveWindow(
              display_,
              w,
              x_frame_attrs.x,
        x_frame_attrs.y);
 /////////////////////////////////////////////////////*/


  // 1. Unmap frame.
  XUnmapWindow(display_, frame);


  // 2. Reparent client window.
  XReparentWindow(
      display_,
      w,
      root_,
      0, 0);  // Offset of client window within root.
  // 3. Remove client window from save set, as it is now unrelated to us.
  XRemoveFromSaveSet(display_, w);
  // 4. Destroy frame.
  XDestroyWindow(display_, frame);
  // 5. Drop reference to frame handle.

/***********Cambiar de Ventana**********************/
  if(clients_.size() > 1 /* && clientBack[1] != w*/ && w == clientFocus[3]){ //se agrega que la pantalla que se cierra es la del foco, 
  //lo comentado es para enviar al panel,
  // pero creo que lo correto es que las ventanas intereactuen con ellas ya que el panel es aparte
    auto i = clients_.find(clientFocus[1]/*w*/);
    if(i != clients_.end())
    {  
      /*CHECK(i != clients_.end());
      ++i;
      if (i == clients_.end()) {
        i = clients_.begin();
      }
      // 2. Raise and set focus.
      XRaiseWindow(display_, i->second);
      XSetInputFocus(display_, i->first, RevertToPointerRoot, CurrentTime);*/

      /*XRaiseWindow(display_, clientFocus[0]);
      XSetInputFocus(display_, clientFocus[1], RevertToPointerRoot, CurrentTime);*/

/////////////////////////////////////////////
      clientFocus[2] = clientFocus[0];
      clientFocus[3] = clientFocus[1];

      XRaiseWindow(display_, clientFocus[0]);
      XSetInputFocus(display_, clientFocus[1], RevertToPointerRoot, CurrentTime);
/////////////////////////////////////////////       


    }else{

      if(clientFocus[1] != panel[1]){

        auto i = clients_.find(w);
        if(i != clients_.end())
        {  
          CHECK(i != clients_.end());
          ++i;
          if (i == clients_.end()) {
            i = clients_.begin();
          }

          // 2. Raise and set focus.
         /* XRaiseWindow(display_, i->second);
          XSetInputFocus(display_, i->first, RevertToPointerRoot, CurrentTime);*/

////////////////////////////////////////////
      clientFocus[0] = clientFocus[2];
      clientFocus[1] = clientFocus[3];
      clientFocus[2] = i->second;
      clientFocus[3] = i->first;

      XRaiseWindow(display_, i->second);
      XSetInputFocus(display_, i->first, RevertToPointerRoot, CurrentTime);
///////////////////////////////////////////// 

        }else{

          /*XRaiseWindow(display_, panel[0]);
          XSetInputFocus(display_, panel[1], RevertToPointerRoot, CurrentTime);*/
/////////////////////////////////////////////
      clientFocus[0] = clientFocus[2];
      clientFocus[1] = clientFocus[3];
      clientFocus[2] = panel[0];
      clientFocus[3] = panel[1];

      XRaiseWindow(display_, panel[0]);
      XSetInputFocus(display_, panel[1], RevertToPointerRoot, CurrentTime);
/////////////////////////////////////////////           
        }

      }else{

        /*XRaiseWindow(display_, panel[0]);
        XSetInputFocus(display_, panel[1], RevertToPointerRoot, CurrentTime);*/
/////////////////////////////////////////////
      clientFocus[0] = clientFocus[2];
      clientFocus[1] = clientFocus[3];
      clientFocus[2] = panel[0];
      clientFocus[3] = panel[1];

      XRaiseWindow(display_, panel[0]);
      XSetInputFocus(display_, panel[1], RevertToPointerRoot, CurrentTime);
/////////////////////////////////////////////         
      }
    }

    
  }else{

    /*XRaiseWindow(display_, panel[0]);
    XSetInputFocus(display_, panel[1], RevertToPointerRoot, CurrentTime);*/
/////////////////////////////////////////////
      clientFocus[0] = clientFocus[2];
      clientFocus[1] = clientFocus[3];
      clientFocus[2] = panel[0];
      clientFocus[3] = panel[1];

      XRaiseWindow(display_, panel[0]);
      XSetInputFocus(display_, panel[1], RevertToPointerRoot, CurrentTime);
///////////////////////////////////////////// 
  }

/////////////////////////////////////////////////

  clients_.erase(w);

  if(clientsAttr_.find(w) != clientsAttr_.end())
     clientsAttr_.erase(w);
  //LOG(INFO) << "Unframed window " << w << " [" << frame << "]";

  sendCountWindow();
}

//por si una ventana obtiene el foco y esta no pone la ventanta hasta arriba.      
void WindowManager::OnFocusIn(const XFocusChangeEvent& e){

   // LOG(INFO) << "window " << e.window << " focus in " << clientFocus[3];
    if( e.window != clientFocus[3])
    {
       if(e.window == panel[1])
       {
              clientBack[0] = clients_[clientFocus[3]];
              clientBack[1] = clientFocus[3];
          /////////////////////////////////////////////
                clientFocus[0] = clientFocus[2];
                clientFocus[1] = clientFocus[3];
                clientFocus[2] = panel[0];
                clientFocus[3] = panel[1];

                 XRaiseWindow(display_, panel[0]);
              //XSetInputFocus(display_, panel[0], RevertToPointerRoot, CurrentTime);
          ///////////////////////////////////////////// 
       }else{
          //const Window frame = ;
          auto i = clients_.find(e.window);
          if(i != clients_.end())
          {
            clientFocus[0] = clientFocus[2];
            clientFocus[1] = clientFocus[3];
            clientFocus[2] = i->second;
            clientFocus[3] = i->first;

            XRaiseWindow(display_, i->second); 
            //XSetInputFocus(display_, e.window, RevertToPointerRoot, CurrentTime);
          }  
       }
    }
    /*if(prosccFoc){

        auto wFin = clients_.find(e.window);

        if(wFin != clients_.end())
        {     
          prosccFoc = true;

          XRaiseWindow(display_, wFin->second);
          XSetInputFocus(display_, wFin->first, RevertToPointerRoot, CurrentTime);
          /////////////////////////////////////////////
          clientFocus[0] = clientFocus[2];
          clientFocus[1] = clientFocus[3];
          clientFocus[2] = wFin->second;
          clientFocus[3] = wFin->first;
          ////////////////////////////////////////// 
        }else{

              clientBack[0] = clients_[e.window];
              clientBack[1] = e.window;
              
              prosccFoc = true;

              XRaiseWindow(display_, panel[0]);
              XSetInputFocus(display_, panel[0], RevertToPointerRoot, CurrentTime);
          /////////////////////////////////////////////
                clientFocus[0] = clientFocus[2];
                clientFocus[1] = clientFocus[3];
                clientFocus[2] = panel[0];
                clientFocus[3] = panel[1];
          ///////////////////////////////////////////// 
          }
    }
    prosccFoc = true;*/
}

void WindowManager::OnCreateNotify(const XCreateWindowEvent& e) {}

void WindowManager::OnDestroyNotify(const XDestroyWindowEvent& e) {}

void WindowManager::OnReparentNotify(const XReparentEvent& e) {}

void WindowManager::OnMapNotify(const XMapEvent& e) {}

void WindowManager::OnUnmapNotify(const XUnmapEvent& e) {
  // If the window is a client window we manage, unframe it upon UnmapNotify. We
  // need the check because we will receive an UnmapNotify event for a frame
  // window we just destroyed ourselves.
  if (!clients_.count(e.window)) {
    //LOG(INFO) << "Ignore UnmapNotify for non-client window " << e.window;
    return;
  }

  // Ignore event if it is triggered by reparenting a window that was mapped
  // before the window manager started.
  //
  // Since we receive UnmapNotify events from the SubstructureNotify mask, the
  // event attribute specifies the parent window of the window that was
  // unmapped. This means that an UnmapNotify event from a normal client window
  // should have this attribute set to a frame window we maintain. Only an
  // UnmapNotify event triggered by reparenting a pre-existing window will have
  // this attribute set to the root window.
  if (e.event == root_) {
    //LOG(INFO) << "Ignore UnmapNotify for reparented pre-existing window "
    //          << e.window;
    return;
  }

  Unframe(e.window);
}

void WindowManager::OnConfigureNotify(const XConfigureEvent& e) {}

void WindowManager::OnMapRequest(const XMapRequestEvent& e) {
  // 1. Frame or re-frame window.
  Frame(e.window, false);
  // 2. Actually map window.

  XMapWindow(display_, e.window);

  auto i = clients_.find(e.window);

  // 2. Raise and set focus.
//  XRaiseWindow(display_, i->second);
//  XSetInputFocus(display_, i->first, RevertToPointerRoot, CurrentTime);

/////////////////////////////////////////////
      clientFocus[0] = clientFocus[2];
      clientFocus[1] = clientFocus[3];
      clientFocus[2] = clients_[e.window];
      clientFocus[3] = e.window;

   XSetInputFocus(display_, i->first, RevertToPointerRoot, CurrentTime);   
///////////////////////////////////////////// 

    ////////////////Colocamos la ventana en 0 tambien ///////////////////////
   const Window frame = clients_[e.window];

    XWindowAttributes x_window_attrs;
    CHECK(XGetWindowAttributes(display_, frame, &x_window_attrs));
    /*Status stat;
    stat = XGetWindowAttributes(display_, frame, &x_window_attrs);
    if(stat == 0) exit(-1);*/

  XMoveWindow(
          display_,
          e.window,
          0,0); 

 XResizeWindow(
            display_,
            e.window,
            x_window_attrs.width, x_window_attrs.height);
////////////////////////////////////////////////////////////////////////

}

void WindowManager::OnConfigureRequest(const XConfigureRequestEvent& e) {
  XWindowChanges changes;
  changes.x = e.x;
  changes.y = e.y;
  changes.width = e.width;
  changes.height = e.height;
  changes.border_width = e.border_width;
  changes.sibling = e.above;
  changes.stack_mode = e.detail;

  if (clients_.count(e.window)) {
    const Window frame = clients_[e.window];
    XConfigureWindow(display_, frame, e.value_mask, &changes);
    //LOG(INFO) << "Resize [" << frame << "] to " << Size<int>(e.width, e.height);
    //si tenemos un frame ponemos la ventana en la pocicion 0,0 para evitar el desplazzamiento incorrecto
    changes.x = 0;
    changes.y = 0;
  }
  XConfigureWindow(display_, e.window, e.value_mask, &changes);
  //LOG(INFO) << "Resize " << e.window << " to " << Size<int>(e.width, e.height);
 //LOG(INFO) << "windows " << changes.x  <<" -- " << changes.y <<" -- " << e.window;

}

/*void WindowManager::OnEnterNotify(const XButtonEvent& e) {

    LOG(INFO) << "Framed window OnEnterNotify window = " << e.window << " focus = " << clientFocus[1];
}*/

void WindowManager::OnButtonPress(const XButtonEvent& e) {
  CHECK(clients_.count(e.window));
  
 // LOG(INFO) << "Framed window Click ctrl_L = " << ctrl_L;
 // LOG(INFO) << "Framed window Click window = " << e.window << " focus = " << clientFocus[1];
 /* auto i = clients_.find(e.window);
  if(i != clients_.end()){*/

      const Window frame = clients_[e.window];

      // 1. Save initial cursor position.
      drag_start_pos_ = Position<int>(e.x_root, e.y_root);

      // 2. Save initial window info.
      Window returned_root;
      int x, y;
      unsigned width, height, border_width, depth;
      CHECK(XGetGeometry(
          display_,
          frame,
          &returned_root,
          &x, &y,
          &width, &height,
          &border_width,
          &depth));
      drag_start_frame_pos_ = Position<int>(x, y);
      drag_start_frame_size_ = Size<int>(width, height);

      //if(clientFocus[1] != e.window){
          // 3. Raise clicked window to top.

//          XRaiseWindow(display_, frame); 
//          XSetInputFocus(display_, e.window, RevertToPointerRoot, CurrentTime);

        /////////////////////////////////////////////
              clientFocus[0] = clientFocus[2];
              clientFocus[1] = clientFocus[3];
              clientFocus[2] = clients_[e.window];
              clientFocus[3] = e.window;

          XRaiseWindow(display_, frame); 
          XSetInputFocus(display_, e.window, RevertToPointerRoot, CurrentTime);
        ///////////////////////////////////////////// 
      /*}        
  }else{

        if(clientFocus[1] != e.window){
          XRaiseWindow(display_, panel[0]);
          XSetInputFocus(display_, panel[1], RevertToPointerRoot, CurrentTime);
      /////////////////////////////////////////////
            clientFocus[0] = clientFocus[2];
            clientFocus[1] = clientFocus[3];
            clientFocus[2] = panel[0];
            clientFocus[3] = panel[1];
      /////////////////////////////////////////////   
        }
  }*/

}

void WindowManager::OnButtonRelease(const XButtonEvent& e) {}

void WindowManager::OnMotionNotify(const XMotionEvent& e) {
  //if(ctrl_L){

    CHECK(clients_.count(e.window));
    const Window frame = clients_[e.window];
    const Position<int> drag_pos(e.x_root, e.y_root);
    const Vector2D<int> delta = drag_pos - drag_start_pos_;

    if (e.state & Button1Mask ) {
      // alt + left button: Move window.
      const Position<int> dest_frame_pos = drag_start_frame_pos_ + delta;
      XMoveWindow(
          display_,
          frame,
          dest_frame_pos.x, dest_frame_pos.y);

    } else if (e.state & Button3Mask) {
      // alt + right button: Resize window.
      // Window dimensions cannot be negative.
      const Vector2D<int> size_delta(
          max(delta.x, -drag_start_frame_size_.width),
          max(delta.y, -drag_start_frame_size_.height));
      const Size<int> dest_frame_size = drag_start_frame_size_ + size_delta;
      // 1. Resize frame.
      XResizeWindow(
          display_,
          frame,
          dest_frame_size.width, dest_frame_size.height);
      // 2. Resize client window.
      XResizeWindow(
          display_,
          e.window,
          dest_frame_size.width, dest_frame_size.height);
    }

 // }

}

void WindowManager::OnKeyPress(const XKeyEvent& e) {

  /*if(e.keycode == XKeysymToKeycode(display_, XK_Control_L)){

    ctrl_L = true;

  }*/

  if ((e.state & Mod1Mask) &&
      (e.keycode == XKeysymToKeycode(display_, XK_F4))) {
    // alt + f4: Close window.
    //
    // There are two ways to tell an X window to close. The first is to send it
    // a message of type WM_PROTOCOLS and value WM_DELETE_WINDOW. If the client
    // has not explicitly marked itself as supporting this more civilized
    // behavior (using XSetWMProtocols()), we kill it with XKillClient().
         
    Atom* supported_protocols;
    int num_supported_protocols;
    if (XGetWMProtocols(display_,
                        e.window,
                        &supported_protocols,
                        &num_supported_protocols) &&
        (::std::find(supported_protocols,
                     supported_protocols + num_supported_protocols,
                     WM_DELETE_WINDOW) !=
         supported_protocols + num_supported_protocols)) {
      //LOG(INFO) << "Gracefully deleting window " << e.window;
      // 1. Construct message.
      XEvent msg;
      memset(&msg, 0, sizeof(msg));
      msg.xclient.type = ClientMessage;
      msg.xclient.message_type = WM_PROTOCOLS;
      msg.xclient.window = e.window;
      msg.xclient.format = 32;
      msg.xclient.data.l[0] = WM_DELETE_WINDOW;
      // 2. Send message to window to be closed.
      CHECK(XSendEvent(display_, e.window, false, 0, &msg));
    } else {
      //LOG(INFO) << "Killing window " << e.window;
      XKillClient(display_, e.window);
    }
  } else if ((e.state & Mod1Mask) &&
             (e.keycode == XKeysymToKeycode(display_, XK_Tab))) {
    // alt + tab: Switch window.
      // 1. Find next window.
      auto i = clients_.find(e.window);
    if(i != clients_.end())
    {  

      if((e.state & ControlMask)){

        /////////////////////Invertir seleccion de ventana
            auto wAnd = clients_.begin();
            auto wAndBuff = wAnd;

            for( auto x = clients_.begin(); x != clients_.end(); x++){
               if(x == i){
                 wAndBuff = wAnd;
               }else{
                 wAnd = x;
               }
            } 

            if(i == clients_.begin()){
              wAndBuff = wAnd;
            }
            i = wAndBuff;
        ////////////////////////////////////////Fin 

      }else{

        ////////////////////////Seleccion de ventana 
            CHECK(i != clients_.end());
            ++i;
            if (i == clients_.end()) {
              i = clients_.begin();
            }
        //////////////////////////////Fin  

      }
    
      // 2. Raise and set focus.
     /* XRaiseWindow(display_, i->second);
      XSetInputFocus(display_, i->first, RevertToPointerRoot, CurrentTime);*/

/////////////////////////////////////////////
      clientFocus[0] = clientFocus[2];
      clientFocus[1] = clientFocus[3];
      clientFocus[2] = i->second;
      clientFocus[3] = i->first;

      XRaiseWindow(display_, i->second);
      XSetInputFocus(display_, i->first, RevertToPointerRoot, CurrentTime);
///////////////////////////////////////////// 

    }else{

      if(clients_.size() > 0)
      {
        auto wtmp = clients_.find(clientBack[1]);

        //LOG(INFO) << "window  " << clientBack[1]<< " status  " << clients_[clientBack[1]];

        if(wtmp != clients_.end() && clients_[clientBack[1]] != 0)//)
        {     
             /* XRaiseWindow(display_, clientBack[0]);
              XSetInputFocus(display_, clientBack[1], RevertToPointerRoot, CurrentTime);*/
            /////////////////////////////////////////////
             /*XWindowAttributes wtmp_attrs;
             Status stat;
             stat = XGetWindowAttributes(display_, clientBack[1], &wtmp_attrs);
             LOG(INFO) << "window  " << clientBack[1]<< " status  " << stat;*/
             //if( stat == 1){ // agrego par avalidar que si exista la ventana
                clientFocus[0] = clientFocus[2];
                clientFocus[1] = clientFocus[3];
                clientFocus[2] = clientBack[0];
                clientFocus[3] = clientBack[1];

                XRaiseWindow(display_, clientBack[0]);
                XSetInputFocus(display_, clientBack[1], RevertToPointerRoot, CurrentTime);
             /*}else{

                  //wtmp = clients_.begin();
                  //if(wtmp != clients_.end())
                  //{  
                    CHECK(wtmp != clients_.end());
                    ++wtmp;
                    if (wtmp == clients_.end()) {
                      wtmp = clients_.begin();
                    }
               
                    clientFocus[0] = clientFocus[2];
                    clientFocus[1] = clientFocus[3];
                    clientFocus[2] = wtmp->second;
                    clientFocus[3] = wtmp->first;

                    XRaiseWindow(display_, wtmp->second);
                    XSetInputFocus(display_, wtmp->first, RevertToPointerRoot, CurrentTime);
                 //} 
            ///////////////////////////////////////////// 

             } */   
    ///////////////////////////////////////////// 
        }else{

          if(wtmp != clients_.end())clients_.erase(clientBack[1]); 

          wtmp = clients_.begin();
          CHECK(wtmp != clients_.end());
          ++wtmp;
          if (wtmp == clients_.end()) {
            wtmp = clients_.begin();
          }

         /* XRaiseWindow(display_, wtmp->second);
          XSetInputFocus(display_, wtmp->first, RevertToPointerRoot, CurrentTime);*/

        /////////////////////////////////////////////
              clientFocus[0] = clientFocus[2];
              clientFocus[1] = clientFocus[3];
              clientFocus[2] = wtmp->second;
              clientFocus[3] = wtmp->first;

              XRaiseWindow(display_, wtmp->second);
              XSetInputFocus(display_, wtmp->first, RevertToPointerRoot, CurrentTime);
        ///////////////////////////////////////////// 

        }
          /*lo que quite de unframe
auto nextW = clients_.find(w);
    if(clients_.size() > 1)
    {  
      CHECK(nextW != clients_.end());
      ++nextW;
      if (nextW == clients_.end()) {
        nextW = clients_.begin();
      }
   }  
   
 if(clientBack[1] == w && clients_.size() > 0){ //el call fue el que se cerro
   LOG(INFO) << "window  " << clientBack[1]<< " newWin  " << nextW->first;
     clientBack[0] = nextW->second;
     clientBack[1] = nextW->first;
   LOG(INFO) << "window  " << clientBack[1]<< " newWin  " << nextW->first;  
  }
          */
      }
    }
    
  }

  if(/*e.keycode == XKeysymToKeycode(display_, XK_Super_L) || e.keycode == XKeysymToKeycode(display_, XK_Super_R)*/ //cambiamos a win+tab
  (e.state & Mod4Mask) && (e.keycode == XKeysymToKeycode(display_, XK_Tab))
  ){
    sendCountWindow(); // actualizamos primero los titulos para que se cierre el panel...
    //LOG(INFO) << "Teclaaaaaa: [" << e.keycode  << "]";
    //const Window frame = clients_[panel];
    /*******************Este es el origen del back***************************/ 
    clientBack[0] = clients_[e.window];
    clientBack[1] = e.window;
    /*XRaiseWindow(display_, panel[0]);
    XSetInputFocus(display_, panel[1], RevertToPointerRoot, CurrentTime);*/

/////////////////////////////////////////////
      clientFocus[0] = clientFocus[2];
      clientFocus[1] = clientFocus[3];
      clientFocus[2] = panel[0];
      clientFocus[3] = panel[1];

    XRaiseWindow(display_, panel[0]);
    XSetInputFocus(display_, panel[1], RevertToPointerRoot, CurrentTime);  
///////////////////////////////////////////// 
    
  }

  if(e.keycode == XKeysymToKeycode(display_, XK_F11)){
    
    CHECK(clients_.count(e.window));
    const Window frame = clients_[e.window];

    auto wnd = clientsAttr_.find(e.window);

    if(wnd == clientsAttr_.end()){//espantalla completa

      XWindowAttributes x_root_attr, wndAt;
      CHECK(XGetWindowAttributes(display_, root_, &x_root_attr));
      CHECK(XGetWindowAttributes(display_, frame, &wndAt));

      clientsAttr_[e.window] = wndAt;
      
      XMoveWindow(
            display_,
            frame,
            0,0);
      
////////////////Colocamos la ventana en 0 tambien ////////////////////////////////////////////////////////
      XMoveWindow(
            display_,
            e.window,
            0,0);
////////////////////////////////////////////////////////////////////////


        XResizeWindow(
            display_,
            frame,
            x_root_attr.width - 2, x_root_attr.height - 2);
        // 2. Resize client window.
        XResizeWindow(
            display_,
            e.window,
            x_root_attr.width - 2, x_root_attr.height - 2);

    }
    else{// es pantalla normal

      XMoveWindow(
            display_,
            frame,
            wnd->second.x,wnd->second.y);

        XResizeWindow(
            display_,
            frame,
            wnd->second.width, wnd->second.height);
        // 2. Resize client window.
        XResizeWindow(
            display_,
            e.window,
            wnd->second.width, wnd->second.height);

        clientsAttr_.erase(e.window);
    }
    /*wndAt tmpWnd = clientsAttr_[e.window]; //+++

    if(tmpWnd.f == 0)//+++
    {
      XWindowAttributes x_root_attr;
      CHECK(XGetWindowAttributes(display_, root_, &x_root_attr));
      
      XMoveWindow(
            display_,
            frame,
            0,0);

        XResizeWindow(
            display_,
            frame,
            x_root_attr.width - 2, x_root_attr.height - 2);
        // 2. Resize client window.
        XResizeWindow(
            display_,
            e.window,
            x_root_attr.width - 2, x_root_attr.height - 2);

        clientsAttr_[e.window].f = 1;//+++

        //LOG(INFO) << "root W " << x_root_attr.width << "root H " << x_root_attr.height;
      }
      else//+++
      {
           XMoveWindow(
            display_,
            frame,
            tmpWnd.x,tmpWnd.y);

        XResizeWindow(
            display_,
            frame,
           tmpWnd.w, tmpWnd.h);
        // 2. Resize client window.
        XResizeWindow(
            display_,
            e.window,
           tmpWnd.w, tmpWnd.h);

          clientsAttr_[e.window].f = 0;
      }*/
    }
    
    
  }

void WindowManager::OnKeyRelease(const XKeyEvent& e) {

  /*if(e.keycode == XKeysymToKeycode(display_, XK_Control_L)){

    ctrl_L = false;

  }*/
}

int WindowManager::OnXError(Display* display, XErrorEvent* e) {
  const int MAX_ERROR_TEXT_LENGTH = 1024;
  char error_text[MAX_ERROR_TEXT_LENGTH];
  XGetErrorText(display, e->error_code, error_text, sizeof(error_text));
  /*LOG(ERROR) << "Received X error:\n"
             << "    Request: " << int(e->request_code)
             << " - " << XRequestCodeToString(e->request_code) << "\n"
             << "    Error code: " << int(e->error_code)
             << " - " << error_text << "\n"
             << "    Resource ID: " << e->resourceid;*/
  // The return value is ignored.
  return 0;
}

int WindowManager::OnWMDetected(Display* display, XErrorEvent* e) {
  // In the case of an already running window manager, the error code from
  // XSelectInput is BadAccess. We don't expect this handler to receive any
  // other errors.
  CHECK_EQ(static_cast<int>(e->error_code), BadAccess);
  // Set flag.
  wm_detected_ = true;
  // The return value is ignored.
  return 0;
}

void WindowManager::sendCountWindow() {

  static char *home = NULL;
  //static char *rcm;
  static char rcm[256];
  const char *path = "/.containerrcm/.wm.rcmSolar";
  const char *jIni = "{\"message\":{\"call\":\"nNativeApps\",\"number\":\"";
  const char *jFin = "\",\"window\":[";
  const char *jWndFin = "]}}";

  char coma[2];
  char jMed[12];
  char wnd_id[1000];
  char json_wnd[10000];
  //char *json;
  char json[10100];
  char letra[6];
  FILE *pFile;

  //sprintf(wnd_id,"");
  //sprintf(coma,"");
  memset(coma, 0, 2);
  memset(jMed, 0, 12);
  memset(wnd_id, 0, 1000);
  memset(json_wnd, 0, 10000);
  memset(json_wnd, 0, 10100);
  auto window = clients_.begin();
    while (window != clients_.end()) {
        XTextProperty text_prop; //para el titulo de las vemtanas
        XGetWMName(display_, window->first, &text_prop);
        sprintf(wnd_id,"%s{\"id\":\"%d\",\"name\":\"",coma,(int)window->first);
        strcpy(json_wnd+strlen(json_wnd),wnd_id);

        if(text_prop.value != NULL)
          for (long unsigned int i = 0; i < strlen((char *)text_prop.value); i++){
            memset(letra, 0, 6);
            //LOG(INFO) << strlen((char *)text_prop.value) << " - letra " << (int) text_prop.value[i] << text_prop.value[i];
            if ((int)text_prop.value[i] >= 160 && (int)text_prop.value[i] < 256) { //https://www.utf8-chartable.de/unicode-utf8-table.pl?number=1024&utf8=dec&unicodeinhtml=dec&htmlent=1
                  sprintf(letra,"&#%d;",(int)text_prop.value[i]);
              }else{
                if ((int)text_prop.value[i] > 31 && (int)text_prop.value[i] < 127) 
                    sprintf(letra,"%c",text_prop.value[i]);
              }
              strcpy(json_wnd+strlen(json_wnd),letra);
          }

         strcpy(json_wnd+strlen(json_wnd),"\"}");
        //sprintf(wnd_id,"%s{\"id\":\"%d\",\"name\":\"%s\"}",coma,(int)window->first,text_prop.value);
        //strcpy(json_wnd+strlen(json_wnd),wnd_id);
        sprintf(coma,",");
        ++window;
    }

  sprintf(jMed,"%d",(int)clients_.size());
  //json = (char *) malloc(strlen(jIni) + strlen(jMed) + strlen(jFin) + 1 + strlen(wnd_id));
  strcpy(json,jIni);
  strcat(json,jMed);
  strcat(json,jFin);
  strcat(json,json_wnd);
  strcat(json,jWndFin);

//LOG(INFO) << "JSON Window " << json;
  if(!home){
    home = getenv("HOME");
  }

  if(!home){
    struct passwd *pw = getpwuid(getuid());
    if(pw)
      home = pw->pw_dir;
  }
memset(rcm, 0, 256);
//rcm = (char *) malloc(strlen(home) + strlen(path) + 1);
strcpy(rcm,home);
strcat(rcm,path);

pFile = fopen(rcm,"w");
if(pFile != NULL){
  fputs(json,pFile);
  fclose(pFile);
}
//free(rcm);
//LOG(INFO) << "Home: [" <<  rcm  << "]";
}

/*Window WindowManager::getWindow() {
  const char *path = "/.containerrcm/.rcmSolar.wm";
  static char *home = NULL;
  static char *rcm;
  char wnd[10];
  int iwnd;
  FILE *pFile;

  memset(wnd, 0, 10);

  if(!home){
    home = getenv("HOME");
  }

  if(!home){
    struct passwd *pw = getpwuid(getuid());
    if(pw)
      home = pw->pw_dir;
  }

  rcm = (char *) malloc(strlen(home) + strlen(path) + 1);
  strcpy(rcm,home);
  strcat(rcm,path);

  pFile = fopen(rcm,"rb");
  if(pFile != NULL){
    fgets(wnd,10,pFile);
    fclose(pFile);
    remove(rcm);
  }
  //LOG(INFO) << " File " << rcm; 
  free(rcm);
  iwnd = atoi(wnd);
  return (Window) iwnd;
}

void WindowManager::hiloWnd() {
  //for(int i = 1; i < 100; i++){
    
    LOG(INFO) << " Open WND " << getWindow();
//sleep(1000);
  //}
}*/