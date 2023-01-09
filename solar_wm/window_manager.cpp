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
//#include <pwd.h>

#include <sys/wait.h>
//#include <unistd.h>
#include "easywsclient.hpp"
#include <assert.h>
#include "easywsclient.cpp"

#include <X11/Xatom.h>

#include <X11/XF86keysym.h>

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
  wndFull =  false;//activamos pantalla completa por default
  wSobrePanel = false;
  //wCloseApp = false;
  wUpdateTBar = true;

  demonSetBV = 0;

  /*panel[0] = 0;
  panel[1] = 0;
  clientBack[0] = 0;
  clientBack[1] = 0;
  clientFocus[0] = 0;
  clientFocus[1] = 0;
  clientFocus[2] = 0;
  clientFocus[3] = 0;*/
  //memset(panel, 0, 2);
  //memset(clientBack, 0, 2);
  //memset(clientFocus, 0, 4);
  //cambiaFoco = false;

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
    for (unsigned int i = num_top_level_windows - 1; i < num_top_level_windows; ++i){
      Frame(top_level_windows[i], true);
    }
  else
    for (unsigned int i = 0; i < num_top_level_windows; ++i){
      if(demonSetBV == 0){
        /////////////////////////////////para no maximizar ni normalizar los splash/////////////////////////////////////
        Atom WM_WINDOW_TYPE = XInternAtom(display_,"_NET_WM_WINDOW_TYPE",false);
        Atom type;
        int format;
        unsigned long nitems, after;
        unsigned char *data = 0;

        XGetWindowProperty(display_, top_level_windows[i], WM_WINDOW_TYPE, 0, 65536,false, XA_ATOM, &type, &format,&nitems, &after, &data);
        
        if(data){
          if(*(Atom*)data == (XInternAtom(display_,"_NET_WM_WINDOW_TYPE_SPLASH_BV", false))){
            demonSetBV = top_level_windows[i];
            std::string message = "{\"message\":{\"call\":\"ctlBVS\", \"window\": " + std::to_string(demonSetBV) + "}}";
            rcmSend(message);
            continue;
          }else{
            Frame(top_level_windows[i], true);
          }
          XFree(data);  
        }//////////////////////////////////////////////////////////////////////
        else{
          Frame(top_level_windows[i], true);
        }
      }else{
        Frame(top_level_windows[i], true);
      }
      //Frame(top_level_windows[i], true);
    }
  //     iii. Free top-level window array.
  XFree(top_level_windows);

  //   e. Ungrab X server.
  XUngrabServer(display_);

  //ponemos por default la variable controladora de el foco en la barra de tareas
  focoBarraTarea = 0;

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
        XSync(display_, 0);
        break;
      case DestroyNotify:
        OnDestroyNotify(e.xdestroywindow);
        XSync(display_, 0);
        break;
      case ReparentNotify:
        OnReparentNotify(e.xreparent);
        XSync(display_, 0);
        break;
      case MapNotify:
        OnMapNotify(e.xmap);
        XSync(display_, 0);
        break;
      case UnmapNotify:
        OnUnmapNotify(e.xunmap);
        XSync(display_, 0);
        break;
      case ConfigureNotify:
        OnConfigureNotify(e.xconfigure);
        XSync(display_, 0);
        break;
      case MapRequest:
        OnMapRequest(e.xmaprequest);
        XSync(display_, 0);
        break;
      case ConfigureRequest:
        OnConfigureRequest(e.xconfigurerequest);
        XSync(display_, 0);
        break;
      case ButtonPress:
       //LOG(INFO) << "click: "; 
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
        XSync(display_, 0);      
        break;
      case ButtonRelease:
        //e.xbutton.send_event = 1;
        OnButtonRelease(e.xbutton);
        //XSendEvent(display_,e.xbutton.window,true,ButtonReleaseMask,&e);
        //XAllowEvents(display_, ReplayPointer, e.xbutton.time);
        XSync(display_, 0);
        break;
      case MotionNotify:
        // Skip any already pending motion events.
        while (XCheckTypedWindowEvent(
            display_, e.xmotion.window, MotionNotify, &e)) {}
        OnMotionNotify(e.xmotion);
        XSync(display_, 0);
        break;
      case KeyPress:
        OnKeyPress(e.xkey);
        XSync(display_, 0);
        break;
      case KeyRelease:
        OnKeyRelease(e.xkey);
        XSync(display_, 0);
        break;
      case FocusIn://Enter Focus:
        //LOG(INFO) << "Enter notify no, es focus in window " << e.xfocus.window << " foco en " << clientFocus[1];//e.xcrossing.window;
           //OnEnterNotify(e.xcrossing);
             // LOG(INFO) << e.xfocus.window;

            OnFocusIn(e.xfocus);
            XSync(display_, 0);
        break;
      case FocusOut://Out Focus:
        //LOG(INFO) << "Enter notify no, es focus in window " << e.xfocus.window << " foco en " << clientFocus[1];//e.xcrossing.window;
           //OnEnterNotify(e.xcrossing);
            OnFocusOut(e.xfocus);
            XSync(display_, 0);
        break; 

      case ClientMessage://Client Message:

            if(e.xclient.message_type == XInternAtom(display_,"SOLAR_WM",false)){      

              Solar_WM(e.xclient);
              //LOG(INFO) << "Enter notify, " << e.xclient.window << " dato en " << e.xclient.data.b;//e.xcrossing.window;
              //LOG(INFO) << "Enter notify, estado: " << XSendEvent(display_,/**/4194311,true,/*NoEventMask*/,&evt); 

            }
            XSync(display_, 0);
        break;
          
      default:
        break;
        //LOG(WARNING) << "Ignored event";
    }
  }
}

void WindowManager::rcmSend(const std::string& message){
  pid_t child_pid;
  signal(SIGCHLD, SIG_IGN); //este ya no me crea los zombies
  child_pid = fork();
  if(child_pid == 0) {//pasar puerto
    using easywsclient::WebSocket;
    std::unique_ptr<WebSocket> ws(WebSocket::from_url("ws://localhost:" + std::to_string(puerto),".wm.rcmSolar"));
    assert(ws);
    ws->send(message);
    ws->poll();
    ws->close();
    exit(0);
  }
}

void WindowManager::Solar_WM(const XClientMessageEvent& e){

  if(e.data.b[0] == 'p'){//ir al panel opcion p

      if(e.window != 0){
        clientBack[0] = clients_[e.window];
        clientBack[1] = e.window;
      }

      focoBarraTarea = 0;

      if(e.data.b[1] == '1')
        sendCountWindow(true); // actualizamos primero los titulos para que se cierre el panel...
      else
        wUpdateTBar = false;

      wSobrePanel = false;
      //setWinPanel = false;
  /////////////////////////////////////////////
      clientFocus[0] = clientFocus[2];
      clientFocus[1] = clientFocus[3];
      clientFocus[2] = panel[0];
      clientFocus[3] = panel[1];

      if(demonSetBV != 0)
        XUnmapWindow(display_,demonSetBV);

      XRaiseWindow(display_, panel[0]);
      if(e.data.b[1] == '1' || e.window != 0)// se agrega || e.window != 0 para dar foco al cerrar ventana desde barra...
        XSetInputFocus(display_, panel[1], RevertToPointerRoot, CurrentTime); 
      /*else{
        unsigned int monitores = 1;
        XWindowAttributes x_root_attr;  
        XGetWindowAttributes(display_, root_, &x_root_attr);
        if(screenWidth != 0)
          monitores = x_root_attr.width/screenWidth;
        if(monitores == 1)
          XSetInputFocus(display_, panel[1], RevertToPointerRoot, CurrentTime); 
      } */
  }

  if(e.data.b[0] == 'o'){ //poweroff opcion o

      rcmSend("{\"message\":{\"call\":\"PowerOff\"}}");

      if(e.window != 0){
        clientBack[0] = clients_[e.window];
        clientBack[1] = e.window;
      }

      focoBarraTarea = 0;

      if(e.data.b[1] == '1')
        sendCountWindow(true); // actualizamos primero los titulos para que se cierre el panel...
      else
        wUpdateTBar = false;

      wSobrePanel = false;
      //setWinPanel = false;
  /////////////////////////////////////////////
      clientFocus[0] = clientFocus[2];
      clientFocus[1] = clientFocus[3];
      clientFocus[2] = panel[0];
      clientFocus[3] = panel[1];

      if(demonSetBV != 0)
          XUnmapWindow(display_,demonSetBV);

      XRaiseWindow(display_, panel[0]);
      if(e.data.b[1] == '1')
        XSetInputFocus(display_, panel[1], RevertToPointerRoot, CurrentTime);   
  }

  if(e.data.b[0] == 'l'){ //estado numlock y capslock opcion l

      if(e.data.b[1] == '0' && e.data.b[2] == '0'){
        rcmSend("{\"message\":{\"call\":\"keyslock\", \"numlock\": false, \"capslock\": false}}");
      }else if(e.data.b[1] == '0' && e.data.b[2] == '1'){
        rcmSend("{\"message\":{\"call\":\"keyslock\", \"numlock\": false, \"capslock\": true}}");
      }else if(e.data.b[1] == '1' && e.data.b[2] == '0'){
        rcmSend("{\"message\":{\"call\":\"keyslock\", \"numlock\": true, \"capslock\": false}}");
      }else if(e.data.b[1] == '1' && e.data.b[2] == '1'){
        rcmSend("{\"message\":{\"call\":\"keyslock\", \"numlock\": true, \"capslock\": true}}");
      }  
  }

  if(e.data.b[0] == 'c'){ //cerrar ventana opcion c

      auto i = clients_.find(e.window);
      if(i != clients_.end()){

        if(demonSetBV != 0)
            XUnmapWindow(display_,demonSetBV);
        //wCloseApp = true;
        XRaiseWindow(display_, i->second);
        //XSetInputFocus(display_, i->first, RevertToPointerRoot, CurrentTime);

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
          XKillClient(display_, e.window);
        }
      }
      //if(e.data.s[0] == 1)
      //  sendCountWindow(false);
  }

  if(e.data.b[0] == 'f'){ //foco ventana opcion f

      auto i = clients_.find(e.window);
      if(i != clients_.end()){
            clientFocus[0] = clientFocus[2];
            clientFocus[1] = clientFocus[3];
            clientFocus[2] = i->second;
            clientFocus[3] = i->first;

            clientBack[0] = i->second;
            clientBack[1] = i->first;

            /*XWindowAttributes attr_Xwin; 
            XGetWindowAttributes(display_, i->second, &attr_Xwin);
            if(attr_Xwin.x < screenWidth){
              wSobrePanel = true;
            }*/

            if(e.data.b[1] == '1')
              sendCountWindow(false,i->first);
            else
              wUpdateTBar = false;  

            focoBarraTarea = e.window;
            //setWinPanel = false;
            if(demonSetBV != 0)
                XUnmapWindow(display_,demonSetBV);

            XRaiseWindow(display_, i->second);
            XSetInputFocus(display_, i->first, RevertToPointerRoot, CurrentTime);
        }

      //if(e.data.s[0] == 1)  
      //  sendCountWindow(false); // se actualiza la lista de aplicaciones  
        
  }

  if(e.data.b[0] == 'u'){ //actualizar barra de tareas opcion u
      if(wUpdateTBar){
          if(e.data.b[1] == '1')
            sendCountWindow(true);
          else
            sendCountWindow(false);
      }
      else
        wUpdateTBar = true;      
  }
 

  if(e.data.b[0] == 's'){ //Colocar foco a ventana anterior opcion s
        Window ventanaSelect;
        //if(setWinPanel){
            if(clientFocus[3] != e.window){ //si no es el panel el del foco es el back por el focusout
              ventanaSelect = clientFocus[3];
              
            }else{
              ventanaSelect = clientFocus[1];
            }
            auto i = clients_.find(ventanaSelect);
            if(i != clients_.end())
            {
              XSetInputFocus(display_, i->first, RevertToPointerRoot, CurrentTime);
            }
        /*}else{
          setWinPanel = true;
        }*/   
  }

  if(e.data.b[0] == '*'){ //actualizar ctlbv
       demonSetBV = e.window;
       std::string message = "{\"message\":{\"call\":\"ctlBVS\", \"window\": " + std::to_string(demonSetBV) + "}}";
       rcmSend(message);
  }

  if(e.data.b[0] == '{' || e.data.b[0] == '}'){ //adelantar y atrazar ventana opcion { y }

        for( auto wAll = clients_.begin(); wAll != clients_.end(); wAll++){
            Status retorno;
            XClassHint wndClas;
            //********************************checamos si la primera ventana no es fantasma
            retorno = XGetClassHint(display_, wAll->first, &wndClas);
            if(retorno == 0){
              clients_.erase(wAll->first);
            }else{
              XFree(wndClas.res_class);
              XFree(wndClas.res_name); 
            }
            //*****************************************
        } 
    //********************************************************
        Window ventanaSelect;
        if(e.window == 0){
          if(clientFocus[3] != panel[1]){ //si no es el panel el del foco es el back por el focusout
            ventanaSelect = clientFocus[3];
            
          }else{
            ventanaSelect = clientFocus[1];
          }
        }else{
          ventanaSelect = e.window;
        }
    
/////////////////////////////////////////////////////////////////

        auto i = clients_.find(ventanaSelect);
        if(i != clients_.end())
        {  
          if(e.data.b[0] == '{'){
            /////////////////////Invertir seleccion de ventana
                auto wAnd = clients_.begin();
                auto wAndBuff = wAnd;

                Status retorno;
                XClassHint wndClas;

                for( auto x = clients_.begin(); x != clients_.end(); x++){
                    retorno = XGetClassHint(display_, x->first, &wndClas);
                    if(retorno == 0){
                      continue;
                    }
                   if(x == i){
                     wAndBuff = wAnd;
                   }else{
                     wAnd = x;
                   }
                   XFree(wndClas.res_class);
                   XFree(wndClas.res_name); 
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

                Status retorno;
                XClassHint wndClas;
                retorno = XGetClassHint(display_, i->first, &wndClas);
                bool flag = false;
                if(retorno == 0){
                  for(auto xy = clients_.begin(); xy != clients_.end(); xy++){
                    if(xy == i)
                      flag = true;
                    if(flag){
                      retorno = XGetClassHint(display_, xy->first, &wndClas);
                      if(retorno != 0){
                        i = xy;
                        XFree(wndClas.res_class);
                        XFree(wndClas.res_name); 
                        break;
                      }
                    }  
                  }
                }
                
            //////////////////////////////Fin  

          }
        /////////////////////////////////////////////
          clientFocus[0] = clientFocus[2];
          clientFocus[1] = clientFocus[3];
          clientFocus[2] = i->second;
          clientFocus[3] = i->first;

          clientBack[0] = i->second;
          clientBack[1] = i->first;

          focoBarraTarea = i->first;
          sendCountWindow(false,i->first); // se actualiza la lista de aplicaciones  
          if(demonSetBV != 0)
            XUnmapWindow(display_,demonSetBV);

          XRaiseWindow(display_, i->second);
          XSetInputFocus(display_, i->first, RevertToPointerRoot, CurrentTime);
          //if(e.data.s[0] == 1)
            
   
    ///////////////////////////////////////////// 

        }else{
          if(clients_.size() > 0)
          {
            auto wtmp = clients_.find(clientBack[1]);

            if(wtmp != clients_.end() && clients_[clientBack[1]] != 0)
            {
                clientFocus[0] = clientFocus[2];
                clientFocus[1] = clientFocus[3];
                clientFocus[2] = clientBack[0];
                clientFocus[3] = clientBack[1];

                focoBarraTarea = clientBack[1];
                sendCountWindow(false,clientBack[1]);

                if(demonSetBV != 0)
                    XUnmapWindow(display_,demonSetBV);

                XRaiseWindow(display_, clientBack[0]);
                XSetInputFocus(display_, clientBack[1], RevertToPointerRoot, CurrentTime);
                //if(e.data.s[0] == 1)
                  
            }else{
              if(wtmp != clients_.end())clients_.erase(clientBack[1]); 
              wtmp = clients_.begin();
              CHECK(wtmp != clients_.end());
              ++wtmp;
              if (wtmp == clients_.end()) {
                wtmp = clients_.begin();
              }

            /////////////////////////////////////////////
                  clientFocus[0] = clientFocus[2];
                  clientFocus[1] = clientFocus[3];
                  clientFocus[2] = wtmp->second;
                  clientFocus[3] = wtmp->first;

                  clientBack[0] = wtmp->second;
                  clientBack[1] = wtmp->first;

                  focoBarraTarea = wtmp->first;
                  sendCountWindow(false,wtmp->first);

                  if(demonSetBV != 0)
                    XUnmapWindow(display_,demonSetBV);

                  XRaiseWindow(display_, wtmp->second);
                  XSetInputFocus(display_, wtmp->first, RevertToPointerRoot, CurrentTime);
                  //if(e.data.s[0] == 1)
                    
            ///////////////////////////////////////////// 

            }
             
          }
        }
  }
   
 // if(e.data.b[0] == 't'){
 // } 
}

bool WindowManager::Frame(Window w, bool was_created_before_window_manager) {

 //LOG(INFO) << "Parametro " << configHome;

  // Visual properties of the frame to create.
  const unsigned int BORDER_WIDTH = 0;//1;
  unsigned long BORDER_COLOR_S = BORDER_COLOR;
  unsigned long BG_COLOR_S = BG_COLOR;

  unsigned int monitores = 1;
  int width = 0;

  // We shouldn't be framing windows we've already framed.
  //CHECK(!clients_.count(w));
  // Se cambia por que el check bota la aplicacion  si no se cumple la condicion
  //es mas quenada para no repintar un frame si ya le pusimos uno a la ventana.
  if(clients_.count(w))return false;

  // 1. Retrieve attributes of window to frame.
  XWindowAttributes x_window_attrs, x_root_attr;

  //CHECK(XGetWindowAttributes(display_, w, &x_window_attrs)); // crash con unas ventanas como fantasmas
  XGetWindowAttributes(display_, w, &x_window_attrs);
  //LOG(INFO) << "Sabes que ...your_event_mask  " << x_window_attrs.your_event_mask << " root " << x_window_attrs.root << " evtAll " << x_window_attrs.all_event_masks;
  if(x_window_attrs.root != root_ ){// estas ventanas son fantasmas hasta donde se detecto.
      return false;
    }

   //LOG(INFO) << "Sabes que ...colormap  " << x_window_attrs.colormap << " -- " << x_window_attrs.map_installed;
  /*if((int)x_window_attrs.root == 0)// estas ventanas son fantasmas hasta donde se detecto.
      return;*/

  CHECK(XGetWindowAttributes(display_, root_, &x_root_attr));
  /*XGetWindowAttributes(display_, w, &x_window_attrs);
  XGetWindowAttributes(display_, root_, &x_root_attr);*/

  // 2. If window was created before window manager started, we should frame
  // it only if it is visible and doesn't set override_redirect.
  if (was_created_before_window_manager) {
    if (x_window_attrs.override_redirect ||
        x_window_attrs.map_state != IsViewable) {
      return false;
    }
  }

//if (/*!was_created_before_window_manager*/ x_window_attrs.x == 0 && x_window_attrs.y == 0) {// ya no es necesario...
/*  if((x_window_attrs.width + 1) >= x_root_attr.width)
     x_window_attrs.width = x_root_attr.width - 2;

  if((x_window_attrs.height + 1) >= x_root_attr.height)
     x_window_attrs.height = x_root_attr.height - 2; 
  } */

 /*XClassHint wndClas;
 XGetClassHint(display_, w, &wndClas); 
  LOG(INFO) << "Atributos Ventana Classes " << wndClas.res_name << " -- " << wndClas.res_class;*/
//  LOG(INFO) << "Atributos Root" << x_root_attr.width << " -- " << x_root_attr.height;

//LOG(INFO) << "Atributos wnd " << x_window_attrs.x << ","<< x_window_attrs.y<< ","<< x_window_attrs.width<< ","<< x_window_attrs.height;
//LOG(INFO) << "Atributos override_redirect " << x_window_attrs.override_redirect;
  if(screenWidth != 0)
    monitores = x_root_attr.width/screenWidth;

  if(monitores > 1){
    int mouse_root_x, mouse_root_y,win_x, win_y;
    unsigned int mask_return;
    Window window_returned;

    XQueryPointer(display_, root_, &window_returned,
        &window_returned, &mouse_root_x, &mouse_root_y, &win_x, &win_y,
        &mask_return);

    if(mouse_root_x >= screenWidth && x_window_attrs.x < screenWidth){
          x_window_attrs.x = x_window_attrs.x + screenWidth;
      }

    width = screenWidth;
  }else
      width = x_root_attr.width;


  if(was_created_before_window_manager && (wndPanel == 0 || wndPanel == w)){
    BORDER_COLOR_S = 0;//xff0000;
    BG_COLOR_S = 0;
    //if(wndPanel == w){
    x_window_attrs.x = 0;
    x_window_attrs.y = 0;
    //}
  } 

  // 3. Create frame.
  const Window frame = XCreateSimpleWindow(
      display_,
      root_,
      x_window_attrs.x,
      x_window_attrs.y,
      /*x_window_attrs.width,
      x_window_attrs.height,*/
      //x_root_attr.width,
      width,
      x_root_attr.height,
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
        //if(x_window_attrs.map_installed != 1){
        //se cambia para agregar siempre al inicio la ventana a abrir
          clients_.insert(clients_.begin(),{w,frame});
          //clients_[w] = frame; 
          clientBack[0] = frame;
          clientBack[1] = w;

          clientFocus[0] = clientFocus[2];
          clientFocus[1] = clientFocus[3];
          clientFocus[2] = frame;
          clientFocus[3] = w;

          focoBarraTarea = w;
        //}
      }
    }
  }
  else
  {
     //if(x_window_attrs.map_installed != 1){
    //se cambia para agregar siempre al inicio la ventana a abrir
        clients_.insert(clients_.begin(),{w,frame});
        //clients_[w] = frame;
        clientBack[0] = frame;
        clientBack[1] = w;

        clientFocus[0] = clientFocus[2];
        clientFocus[1] = clientFocus[3];
        clientFocus[2] = frame;
        clientFocus[3] = w;

        focoBarraTarea = w;
    //}
  }
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
    XGrabButton(
          display_,
          Button1,
          AnyModifier,
          w,
          false,
          ButtonPressMask,
          GrabModeAsync,
          GrabModeAsync,
          None,
          None);

///////////////////////////////////Mover ventana y combinaciones posibles//////////////////////////////
     //   a. Move windows with ctrl + alt + left button.
    //---> se pasan para cuando la ventana recibe el foco de entrada.
   /* XGrabButton(
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
        None);*/
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

//////////////////////////////////Mostrar Panel y combinaciones posibles////////////////////////////// 
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

///////////////////////////////////Cambiar ventana y combinaciones posibles//////////////////////////////   
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

  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XF86XK_PowerOff),//KEY_PowerOff,//XKeysymToKeycode(display_, 124)//116 = 28 --> 124 ya no es necesaria la converción a XKey ya que la tomamos directamente.
      AnyModifier,//none
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);
    XGrabKey(
      display_,
      XKeysymToKeycode(display_, XF86XK_MonBrightnessUp),//KEY_PowerOff,//XKeysymToKeycode(display_, 124)//116 = 28 --> 124 ya no es necesaria la converción a XKey ya que la tomamos directamente.
      AnyModifier,//none
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);
  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XF86XK_MonBrightnessDown),//KEY_PowerOff,//XKeysymToKeycode(display_, 124)//116 = 28 --> 124 ya no es necesaria la converción a XKey ya que la tomamos directamente.
      AnyModifier,//none
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);
  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XF86XK_AudioRaiseVolume),//KEY_PowerOff,//XKeysymToKeycode(display_, 124)//116 = 28 --> 124 ya no es necesaria la converción a XKey ya que la tomamos directamente.
      AnyModifier,//none
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);
  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XF86XK_AudioLowerVolume),//KEY_PowerOff,//XKeysymToKeycode(display_, 124)//116 = 28 --> 124 ya no es necesaria la converción a XKey ya que la tomamos directamente.
      AnyModifier,//none
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);
  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XF86XK_AudioMute),//KEY_PowerOff,//XKeysymToKeycode(display_, 124)//116 = 28 --> 124 ya no es necesaria la converción a XKey ya que la tomamos directamente.
      AnyModifier,//none
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);

  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Delete),
      ControlMask | Mod1Mask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);
  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Delete),
     ControlMask | Mod1Mask | Mod2Mask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);
  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Delete),
      ControlMask | Mod1Mask | LockMask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);
  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Delete),
      ControlMask | Mod1Mask | Mod2Mask | LockMask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);

  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_Print),
      AnyModifier,//none
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);
   XGrabKey(
      display_,
      XKeysymToKeycode(display_, XF86XK_Calculator),
      AnyModifier,//none
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);

    //   d. Switch windows with Ctrl + alt + s (suspend) 
  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_S),
      Mod1Mask | ControlMask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);

  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_S),
      Mod1Mask | Mod2Mask | ControlMask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);

  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_S),
      Mod1Mask | LockMask | ControlMask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);

  XGrabKey(
      display_,
      XKeysymToKeycode(display_, XK_S),
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
      FocusChangeMask);//ButtonPressMask);//FocusChangeMask);//EnterWindowMask);*/

    /*XGrabButton(
        display_,
        Button1,
        AnyModifier,
        w,
        false,
        ButtonPressMask,// | ButtonReleaseMask | ButtonMotionMask,
        GrabModeAsync,
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

  normalizarWindows(/*w*/);
  sendCountWindow(false,w);
  //sendWindowFocus(w);   

  return true;
}

void WindowManager::Unframe(Window w) {
  CHECK(clients_.count(w));

  // We reverse the steps taken in Frame().
  const Window frame = clients_[w];

  bool isPanel = false;

  
  //wSobrePanel = false;
  

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

if(demonSetBV != 0)
     XUnmapWindow(display_,demonSetBV);
/***********Cambiar de Ventana**********************/
  if(clients_.size() > 1 /* && clientBack[1] != w*/ && w == clientFocus[3]){ //se agrega que la pantalla que se cierra es la del foco, 
  //lo comentado es para enviar al panel,
  // pero creo que lo correto es que las ventanas intereactuen con ellas ya que el panel es aparte
    auto i = clients_.find(clientFocus[1]/*w*/);
    if(i != clients_.end() && w != clientFocus[1])
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

      clientBack[0] = clientFocus[0];
      clientBack[1] = clientFocus[1];

      
      XWindowAttributes attr_Xwin; 
      XGetWindowAttributes(display_, clientFocus[0], &attr_Xwin);
      if(attr_Xwin.x >= screenWidth){
          clientFocus[0] = clientFocus[2];
          clientFocus[1] = clientFocus[3];
          clientFocus[2] = panel[0];
          clientFocus[3] = panel[1];

          focoBarraTarea = 0;

          XRaiseWindow(display_, panel[0]);
          XSetInputFocus(display_, panel[1], RevertToPointerRoot, CurrentTime);
          //LOG(INFO) << "Unframed window 1" << w << " [" << panel[0] << "]";
          wSobrePanel = false;
          isPanel = true;
      }else{
          focoBarraTarea = clientFocus[1];
          wSobrePanel = true;
          //wCloseApp = false;
          XRaiseWindow(display_, clientFocus[0]);
          XSetInputFocus(display_, clientFocus[1], RevertToPointerRoot, CurrentTime);
          //LOG(INFO) << "Unframed window 2" << clientFocus[0] << " [" << attr_Xwin.x << "]";
      }
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

          if(i->first == clientFocus[1]){
            ++i;
          }

          // 2. Raise and set focus.
         /* XRaiseWindow(display_, i->second);
          XSetInputFocus(display_, i->first, RevertToPointerRoot, CurrentTime);*/

    ////////////////////////////////////////////
          clientFocus[0] = clientFocus[2];
          clientFocus[1] = clientFocus[3];
          clientFocus[2] = i->second;
          clientFocus[3] = i->first;

          clientBack[0] = i->second;
          clientBack[1] = i->first;

          ///////////////////////validamos la ventana///////////////////////
          Status retorno;
          XClassHint wndClas;
          retorno = XGetClassHint(display_, i->first, &wndClas);
          if(retorno != 0){
            XFree(wndClas.res_class);
            XFree(wndClas.res_name);
          }
          ////////////////////////////////////////////////////

          XWindowAttributes attr_Xwin; 
          XGetWindowAttributes(display_, clientBack[0], &attr_Xwin);
          if(attr_Xwin.x >= screenWidth || retorno == 0){// o si la ventana es fantasa nos vamos al panel
              clientFocus[0] = clientFocus[2];
              clientFocus[1] = clientFocus[3];
              clientFocus[2] = panel[0];
              clientFocus[3] = panel[1];

              focoBarraTarea = 0;

              XRaiseWindow(display_, panel[0]);
              XSetInputFocus(display_, panel[1], RevertToPointerRoot, CurrentTime);
              //LOG(INFO) << "Unframed window 3" << w << " [" << panel[0] << "]";
              wSobrePanel = false;
              isPanel = true;
              if(retorno == 0)//si la ventana es fantasa la borramos
                clients_.erase(i->first);
          }else{
              focoBarraTarea = clientBack[1];
              wSobrePanel = true;
              //wCloseApp = false;
              XRaiseWindow(display_, i->second);
              XSetInputFocus(display_, i->first, RevertToPointerRoot, CurrentTime);
              //LOG(INFO) << "Unframed window 4" << i->first << " [" << i->second << "]";
          }
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
          //LOG(INFO) << "Unframed window 5" << w << " [" << panel[0] << "]";
          focoBarraTarea = 0;
          wSobrePanel = false;
          isPanel = true;
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
            //LOG(INFO) << "Unframed window 6" << w << " [" << panel[0] << "]";
            focoBarraTarea = 0;
            wSobrePanel = false;
            isPanel = true;
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
      //LOG(INFO) << "Unframed window 7" << w << " [" << panel[0] << "]";
      focoBarraTarea = 0;
      wSobrePanel = false;
      isPanel = true;
///////////////////////////////////////////// 
  }

/////////////////////////////////////////////////

  clients_.erase(w);

  sendCountWindow(isPanel);
  /*if(clientsAttr_.find(w) != clientsAttr_.end())
     clientsAttr_.erase(w);*/
  //LOG(INFO) << "Unframed window " << w << " [" << frame << "]";

}

void WindowManager::updateTBarExt(bool isPanel){
  Status estado;
  XEvent evt;
  evt.xclient.type = ClientMessage;
  evt.xclient.serial = 0;
  evt.xclient.send_event = true;
  evt.xclient.message_type = XInternAtom(display_,"SOLAR_WM",false);;
  evt.xclient.format = 8;
  evt.xclient.window = panel[1];
  evt.xclient.data.b[0] = 'u';

  if(isPanel)
    evt.xclient.data.b[1] = '1';
  else
    evt.xclient.data.b[1] = '0';

  estado = XSendEvent(display_,
    panel[1],//se puede madar la misma ventana se decide mandar a root
    true,//si no se propaga el mensaje no es cachado por wm
    (SubstructureRedirectMask | SubstructureNotifyMask),//si no se agregan las mascaras que procesa wm es ignorado por el mismo
    &evt);

  if(estado)
    XFlush(display_);
}

void WindowManager::setFocoWindow(void){
  Status estado;
  XEvent evt;
  evt.xclient.type = ClientMessage;
  evt.xclient.serial = 0;
  evt.xclient.send_event = true;
  evt.xclient.message_type = XInternAtom(display_,"SOLAR_WM",false);;
  evt.xclient.format = 8;
  evt.xclient.window = panel[1];
  evt.xclient.data.b[0] = 's';

  estado = XSendEvent(display_,
    panel[1],//se puede madar la misma ventana se decide mandar a root
    true,//si no se propaga el mensaje no es cachado por wm
    (SubstructureRedirectMask | SubstructureNotifyMask),//si no se agregan las mascaras que procesa wm es ignorado por el mismo
    &evt);

  if(estado)
    XFlush(display_);
}

//por si una ventana obtiene el foco y esta no pone la ventanta hasta arriba.      
void WindowManager::OnFocusIn(const XFocusChangeEvent& e){

  //se quita para que al precionar la barra no mande a la pantalla y si es diferente el foco del panel ejecute la accion
    if(e.window == panel[1] && e.window != clientFocus[3]) 
       {
          /*auto i = clients_.find(clientBack[1]);
          if(i != clients_.end())
          {
              clientFocus[0] = clientFocus[2];
              clientFocus[1] = clientFocus[3];
              clientFocus[2] = i->second;
              clientFocus[3] = i->first;

              XRaiseWindow(display_, clientBack[0]);
              XSetInputFocus(display_, clientBack[1], RevertToPointerRoot, CurrentTime);
              if(focoBarraTarea != clientBack[1])
                sendCountWindow(false);
            //clientBack[0] = i->second;
            //clientBack[1] = i->first;
              
          }else{*/
//if(focoBarraTarea != 0){
//if(e.window == clientFocus[1]){ 
            clientBack[0] = clientFocus[2];
            clientBack[1] = clientFocus[3];

            clientFocus[0] = clientFocus[2];
            clientFocus[1] = clientFocus[3];
            clientFocus[2] = panel[0];
            clientFocus[3] = panel[1];

            //XRaiseWindow(display_, panel[0]);
            //dejamos solo el foco al panel para poder ejecutar la haccion de la barra y 
            //le delegamos al panel la accion de cambio de ventana
            XSetInputFocus(display_, panel[1], RevertToPointerRoot, CurrentTime);
            //XSetInputFocus(display_, clientBack[1], RevertToPointerRoot, CurrentTime);
    //LOG(INFO) << "window " << focoBarraTarea << " focus in " << wSobrePanel << " - "<<wUpdateTBar;
            //setWinPanel = true;
            if(focoBarraTarea != 0){
                if(!wSobrePanel){ //pasamos el foco al panel 
                  //sendCountWindow(true);
                  focoBarraTarea = 0;
                  updateTBarExt(true);
                  
                  //wCloseApp = false;
                }
                else{//regresamos el foco a la ventana seleccionada
                  setFocoWindow();
                }  
            }
//}              
          //}
          ///////////////////////////////////////////// 
       }else{
            auto i = clients_.find(e.window);
            if(i != clients_.end())
            {
              //quitamos la toma del click
              XUngrabButton(display_, Button1, AnyModifier, e.window);
              //volvemos a agregar el click :) de esta manera ya recibe el foco con un cli///////////////////////////////////Mover ventana y combinaciones posibles//////////////////////////////
               //   a. Move windows with ctrl + alt + left button.
              XGrabButton(
                  display_,
                  Button1,
                  ControlMask | Mod1Mask,
                  e.window,
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
                  e.window,
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
                  e.window,
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
                  e.window,
                  false,
                  ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
                  GrabModeAsync,
                  GrabModeAsync,
                  None,
                  None);
          ///////////////////////////////////////////////////////////////////////////////////////////////////////////
              if(e.window != clientFocus[3]){
                if(demonSetBV != 0)
                    XUnmapWindow(display_,demonSetBV);
                XRaiseWindow(display_, i->second);
              }
                  

              clientFocus[0] = clientBack[0];//clientFocus[2];
              clientFocus[1] = clientBack[1];//clientFocus[3];
              clientFocus[2] = i->second;
              clientFocus[3] = i->first;

              //clientBack[0] = i->second;
              //clientBack[1] = i->first;
              XWindowAttributes attr_Xwin; 
              XGetWindowAttributes(display_, i->second, &attr_Xwin);
              if(attr_Xwin.x < screenWidth){
                wSobrePanel = true;
              }

            //if(cambiaFoco){ 
              //XRaiseWindow(display_, i->second); 
              //XSetInputFocus(display_, i->first, RevertToPointerRoot, CurrentTime);

              if(focoBarraTarea != i->first){
              //if(focoBarraTarea == 0)
                /*focoBarraTarea = i->first;
                sendCountWindow(false);*/
                focoBarraTarea = i->first;
                updateTBarExt(false); 
              } 
                
    //}            
     //sendCountWindow(false); //se agrega ya que al obtener el foco por otro lado no se refresca en la barra de tareas
          
          } 
       }
   /* return;
    if( e.window != clientFocus[3])
    {
       if(e.window == panel[1]) //se quita para que al precionar la barra no mande a la pantalla
       {
              //clientBack[0] = clients_[clientFocus[3]];
              //clientBack[1] = clientFocus[3];
          /////////////////////////////////////////////
              //  clientFocus[0] = clientFocus[2];
              //  clientFocus[1] = clientFocus[3];
              //  clientFocus[2] = panel[0];
              //  clientFocus[3] = panel[1];* /

                auto i = clients_.find(clientBack[1]);
                if(i != clients_.end())
                {
                  //XWindowAttributes attr_Xwin;
                  //XGetWindowAttributes(display_, clientBack[0], &attr_Xwin);
                  //if(attr_Xwin.x < screenWidth){* /
                    XRaiseWindow(display_, clientBack[0]);
                    XSetInputFocus(display_, clientBack[1], RevertToPointerRoot, CurrentTime);
                 // }else{
                 //   XRaiseWindow(display_, panel[0]);
                 //   XSetInputFocus(display_, panel[0], RevertToPointerRoot, CurrentTime);
                 //   sendCountWindow(true);
                 // }* /
                }else{
                  XRaiseWindow(display_, panel[0]);
                  XSetInputFocus(display_, panel[0], RevertToPointerRoot, CurrentTime);
                  sendCountWindow(true); //se comenta para tratar de evitar los ciclados
                }
                //XRaiseWindow(display_, panel[0]);
                //XSetInputFocus(display_, panel[0], RevertToPointerRoot, CurrentTime);
                //sendCountWindow(); // se actualiza la lista de aplicaciones

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

            clientBack[0] = i->second;
            clientBack[1] = i->first;

            XRaiseWindow(display_, i->second); 
            XSetInputFocus(display_, i->first, RevertToPointerRoot, CurrentTime);
            sendCountWindow(false); // se actualiza la lista de aplicaciones //se comenta para tratar de evitar los ciclados 
          }

       }
    }*/
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
void WindowManager::OnFocusOut(const XFocusChangeEvent& e){

  //if(focoBarraTarea == e.window) focoBarraTarea = 0;
  /*if(demonSetBV != 0)
     XUnmapWindow(display_,demonSetBV);*/
  if(e.window != panel[1]){
      XGrabButton(
          display_,
          Button1,
          AnyModifier,
          e.window,
          false,
          ButtonPressMask,
          GrabModeAsync,
          GrabModeAsync,
          None,
          None); 

      auto i = clients_.find(e.window);
      if(i != clients_.end())
      {
        clientBack[0] = i->second;
        clientBack[1] = i->first;
      } 
  }/*else{
    if(!cambiaFoco){ 
      clientFocus[0] = clientFocus[2];
      clientFocus[1] = clientFocus[3];
      clientFocus[2] = panel[0];
      clientFocus[3] = panel[1];

      XRaiseWindow(display_, panel[0]);
      XSetInputFocus(display_, panel[1], RevertToPointerRoot, CurrentTime); 
    }else
      cambiaFoco = false;
  }*/         
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
/////////////////////////////////para no maximizar ni normalizar los splash/////////////////////////////////////
  Atom WM_WINDOW_TYPE = XInternAtom(display_,"_NET_WM_WINDOW_TYPE",false);
  Atom type;
  int format;
  unsigned long nitems, after;
  unsigned char *data = 0;

  XGetWindowProperty(display_, e.window, WM_WINDOW_TYPE, 0, 65536,false, XA_ATOM, &type, &format,&nitems, &after, &data);
  
  if(data){
    if(*(Atom*)data == (XInternAtom(display_,"_NET_WM_WINDOW_TYPE_SPLASH", false)) || *(Atom*)data == (XInternAtom(display_,"_NET_WM_WINDOW_TYPE_SPLASH_BV", false))){
      XFree(data);
      XMapWindow(display_, e.window);
      return;
    }
    XFree(data);  
  }
  //////////////////////////////////////////////////////////////////////

  // 1. Frame or re-frame window.
  if(!Frame(e.window, false)) return;
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
 //normalizarWindows(/*w*/);

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
  normalizarWindows(/*w*/); //se agrega aqui ya que hay ventanas que se reconfiguran una vez creadas.

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

          clientBack[0] = frame;
          clientBack[1] = e.window;

          focoBarraTarea = e.window;

          sendCountWindow(false,e.window); // se actualiza la lista de aplicaciones  

          if(demonSetBV != 0)
              XUnmapWindow(display_,demonSetBV);    

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
  
    focusMonitor = 1;

    if(screenWidth != 0)
      focusMonitor = (e.x_root/screenWidth) + 1;      
}

void WindowManager::OnButtonRelease(const XButtonEvent& e) {
  if (e.state & Button1Mask ) {
      unsigned int focusMonitorRelease = 1; 

      if(screenWidth != 0)
        focusMonitorRelease = (e.x_root/screenWidth) + 1;
      if(focusMonitor != focusMonitorRelease){
        normalizarWindows();
        sendCountWindow(false, e.window);
      } 
    }
}

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
  
  //LOG(INFO) << "key  " << e.keycode; // mate-power-manager tiene agarrada la tecla.
  //se controla el poweroff ya que si se cierra sesion ya no lo toma el gnome.
  //if(e.keycode == KEY_PowerOff){
    //system("poweroff >/dev/null");
    /*Window frame = XCreateSimpleWindow(
      display_,
      root_,
      0,
      0,
      640,
      480,
      0,
      BORDER_COLOR,
      BG_COLOR);
    XClearWindow(display_, frame);
    XMapWindow(display_, frame);

    GC myGC = XCreateGC(display_, frame, 0, 0);
    XSetForeground(display_, myGC, BORDER_COLOR);
    XSetBackground(display_, myGC, BG_COLOR);

    XDrawString(display_, frame, myGC, 0, 0, "hola mundo", strlen("hola mundo")); 

    //sleep(2);
    //XFreeGC(display_, myGC);
    //XDestroyWindow(display_, frame);*/

    //LOG(INFO) << "key  " << e.keycode;
    /*LOG(INFO) << "setBV  " << demonSetBV;
  }*/
  if(e.keycode == XKeysymToKeycode(display_, XF86XK_MonBrightnessUp)){
    XEvent evt;
    evt.xclient.type = ClientMessage;
    evt.xclient.serial = 0;
    evt.xclient.send_event = true;
    evt.xclient.message_type = XInternAtom(display_,"DISPLAY_BRIGHTNESS_VOLUME_SCREEN",false);;
    evt.xclient.format = 8;
    evt.xclient.window = demonSetBV;
    evt.xclient.data.b[0] = BrightnessUp;

    XSendEvent(display_,demonSetBV,false,ExposureMask,&evt);
    XFlush(display_);
    return;
  }

  if(e.keycode == XKeysymToKeycode(display_, XF86XK_MonBrightnessDown)){ 
    XEvent evt;
    evt.xclient.type = ClientMessage;
    evt.xclient.serial = 0;
    evt.xclient.send_event = true;
    evt.xclient.message_type = XInternAtom(display_,"DISPLAY_BRIGHTNESS_VOLUME_SCREEN",false);;
    evt.xclient.format = 8;
    evt.xclient.window = demonSetBV;
    evt.xclient.data.b[0] = BrightnessDown;

    XSendEvent(display_,demonSetBV,false,ExposureMask,&evt);
    XFlush(display_);
    return;
  }

  if(e.keycode == XKeysymToKeycode(display_, XF86XK_AudioRaiseVolume)){ 
    XEvent evt;
    evt.xclient.type = ClientMessage;
    evt.xclient.serial = 0;
    evt.xclient.send_event = true;
    evt.xclient.message_type = XInternAtom(display_,"DISPLAY_BRIGHTNESS_VOLUME_SCREEN",false);;
    evt.xclient.format = 8;
    evt.xclient.window = demonSetBV;
    evt.xclient.data.b[0] = VolumeUp;

    XSendEvent(display_,demonSetBV,false,ExposureMask,&evt);
    XFlush(display_);
    return;
  }

  if(e.keycode == XKeysymToKeycode(display_, XF86XK_AudioLowerVolume)){ 
    XEvent evt;
    evt.xclient.type = ClientMessage;
    evt.xclient.serial = 0;
    evt.xclient.send_event = true;
    evt.xclient.message_type = XInternAtom(display_,"DISPLAY_BRIGHTNESS_VOLUME_SCREEN",false);;
    evt.xclient.format = 8;
    evt.xclient.window = demonSetBV;
    evt.xclient.data.b[0] = VolumeDown;

    XSendEvent(display_,demonSetBV,false,ExposureMask,&evt);
    XFlush(display_);
    return;
  }

  if(e.keycode == XKeysymToKeycode(display_, XF86XK_AudioMute)){ 
    XEvent evt;
    evt.xclient.type = ClientMessage;
    evt.xclient.serial = 0;
    evt.xclient.send_event = true;
    evt.xclient.message_type = XInternAtom(display_,"DISPLAY_BRIGHTNESS_VOLUME_SCREEN",false);;
    evt.xclient.format = 8;
    evt.xclient.window = demonSetBV;
    evt.xclient.data.b[0] = Mute;

    XSendEvent(display_,demonSetBV,false,ExposureMask,&evt);
    XFlush(display_);
    return;
  }
  if(e.keycode == XKeysymToKeycode(display_, XK_Print)){ 
    XEvent evt;
    evt.xclient.type = ClientMessage;
    evt.xclient.serial = 0;
    evt.xclient.send_event = true;
    evt.xclient.message_type = XInternAtom(display_,"DISPLAY_BRIGHTNESS_VOLUME_SCREEN",false);;
    evt.xclient.format = 8;
    evt.xclient.window = demonSetBV;
    evt.xclient.data.b[0] = PrintScreen;

    XSendEvent(display_,demonSetBV,false,ExposureMask,&evt);
    XFlush(display_);
    //LOG(INFO) << "key  " << e.keycode;
    return;
  }
  
  if(demonSetBV != 0)
     XUnmapWindow(display_,demonSetBV);

  if(e.keycode == XKeysymToKeycode(display_, XF86XK_PowerOff)){
    rcmSend("{\"message\":{\"call\":\"PowerOff\"}}");

    focoBarraTarea = 0;
    sendCountWindow(true); // actualizamos primero los titulos para que se cierre el panel...
    wSobrePanel = false;
  /////////////////////////////////////////////
    clientFocus[0] = clientFocus[2];
    clientFocus[1] = clientFocus[3];
    clientFocus[2] = panel[0];
    clientFocus[3] = panel[1];

    XRaiseWindow(display_, panel[0]);
    XSetInputFocus(display_, panel[1], RevertToPointerRoot, CurrentTime);
  } 

  if(e.keycode == XKeysymToKeycode(display_, XK_Delete)){
    rcmSend("{\"message\":{\"call\":\"Ctl+Alt+Del\"}}");
  }

  if(e.keycode == XKeysymToKeycode(display_,   XF86XK_Calculator)){
    rcmSend("{\"message\":{\"call\":\"Calculator\"}}");
    //LOG(INFO) << "key  " << e.keycode;
  }

  if(e.keycode == XKeysymToKeycode(display_, XK_S)){
    rcmSend("{\"message\":{\"call\":\"Suspend\"}}");
  }

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

/******************limpiamos las ventanas fantasmas*/  
    for( auto wAll = clients_.begin(); wAll != clients_.end(); wAll++){
        Status retorno;
        XClassHint wndClas;
        //********************************checamos si la primera ventana no es fantasma
        retorno = XGetClassHint(display_, wAll->first, &wndClas);
        if(retorno == 0){
          clients_.erase(wAll->first);
        }else{
          XFree(wndClas.res_class);
          XFree(wndClas.res_name); 
        }
        //*****************************************
    } 
//********************************************************

    auto i = clients_.find(e.window);
    if(i != clients_.end())
    {  
      if((e.state & ControlMask)){
        /////////////////////Invertir seleccion de ventana
            auto wAnd = clients_.begin();
            auto wAndBuff = wAnd;

            Status retorno;
            XClassHint wndClas;

            for( auto x = clients_.begin(); x != clients_.end(); x++){
                retorno = XGetClassHint(display_, x->first, &wndClas);
                if(retorno == 0){
                  continue;
                }
               if(x == i){
                 wAndBuff = wAnd;
               }else{
                 wAnd = x;
               }
               XFree(wndClas.res_class);
               XFree(wndClas.res_name); 
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

            Status retorno;
            XClassHint wndClas;
            retorno = XGetClassHint(display_, i->first, &wndClas);
            bool flag = false;
            if(retorno == 0){
              for(auto xy = clients_.begin(); xy != clients_.end(); xy++){
                if(xy == i)
                  flag = true;
                if(flag){
                  retorno = XGetClassHint(display_, xy->first, &wndClas);
                  if(retorno != 0){
                    i = xy;
                    XFree(wndClas.res_class);
                    XFree(wndClas.res_name); 
                    break;
                  }
                }  
              }
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

      clientBack[0] = i->second;
      clientBack[1] = i->first;

      focoBarraTarea = e.window;
      sendCountWindow(false,i->first); // se actualiza la lista de aplicaciones  
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

                focoBarraTarea = e.window;
                sendCountWindow(false,clientBack[1]);

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

              clientBack[0] = wtmp->second;
              clientBack[1] = wtmp->first;

              focoBarraTarea = e.window;
              sendCountWindow(false,wtmp->first);
              
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
    //LOG(INFO) << "Teclaaaaaa: [" << e.keycode  << "]";
    //const Window frame = clients_[panel];
    /*******************Este es el origen del back***************************/ 
    clientBack[0] = clients_[e.window];
    clientBack[1] = e.window;
    /*XRaiseWindow(display_, panel[0]);
    XSetInputFocus(display_, panel[1], RevertToPointerRoot, CurrentTime);*/
    focoBarraTarea = 0;

    sendCountWindow(true); // actualizamos primero los titulos para que se cierre el panel...
    wSobrePanel = false;
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
    
    /*CHECK(clients_.count(e.window));
    const Window frame = clients_[e.window];

    auto wnd = clientsAttr_.find(e.window);

    if(wnd == clientsAttr_.end()){//espantalla completa

      XWindowAttributes x_root_attr, wndAt;
      CHECK(XGetWindowAttributes(display_, root_, &x_root_attr));
      CHECK(XGetWindowAttributes(display_, frame, &wndAt));

      clientsAttr_[e.window] = wndAt;*/
      wndFull = !wndFull;
      normalizarWindows(/*e.window*/);
       
      /*XMoveWindow(
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
            x_root_attr.width - 2, x_root_attr.height - 2);*/

    /*}
    else{// es pantalla normal*/

      /*XMoveWindow(
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
            wnd->second.width, wnd->second.height);*/
       /*normalizarWindows(false);

        clientsAttr_.erase(e.window);
    }*/
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

void WindowManager::sendCountWindow(bool isPanel, Window w) {

  //static char *home = NULL;
  //static char *rcm;
  //static char rcm[10225];//[10225];//[256];
  //const char *path = "/bin/rcmSend"; //"/.containerrcm/.wm.rcmSolar";
  const char *jIni = "{\"message\":{\"call\":\"nNativeApps\",\"number\":\"";
  const char *jFin = "\",\"window\":[";
  const char *jWndFin = "]}}";

  char coma[2];
  char jMed[12];
  char wnd_id[500];
  char json_wnd[10111];
  //char *json;
  char json[10211];
  char letra[6];
  //FILE *pFile;

  //sprintf(wnd_id,"");
  //sprintf(coma,"");
  memset(coma, 0, 2);
  memset(jMed, 0, 12);
  memset(wnd_id, 0, 508);
  memset(json_wnd, 0, 10119);
  memset(json, 0, 10219);

  auto window = clients_.begin();
  while (window != clients_.end()) {
        XTextProperty text_prop; //para el titulo de las vemtanas
        XClassHint wndClas; //para el nombre d ela classe
        XWindowAttributes attr_next;
        Status retorno; 
        bool mm = true;
        retorno = XGetWindowAttributes(display_, window->second, &attr_next);
        if(attr_next.x >= screenWidth)
          mm = false;
        //LOG(INFO) << "XGetWindowAttributes " <<mm;
        retorno = XGetWMName(display_, window->first, &text_prop);
        //LOG(INFO) << "XGetWMName " <<retorno;
        retorno = XGetClassHint(display_, window->first, &wndClas);
        //LOG(INFO) << "XGetClassHint " <<retorno << " -- " << clients_.size(); ///////////////////////validar status de funciones de arriba//////////////////////////////////////////////
        //LOG(INFO) << "XGetClassHint " <<BadWindow; //3
        if(/*attr_next.map_installed != 1 ||*/ retorno != 0){
          sprintf(wnd_id,"%s{\"id\":\"%ld\",\"mm\":\"%d\",\"name\":\"",coma,window->first,mm);
          strcpy(json_wnd+strlen(json_wnd),wnd_id);
          if(text_prop.value != NULL)
            for (long unsigned int i = 0; i < strlen((char *)text_prop.value); i++){
              memset(letra, 0, 6);
              //LOG(INFO) << strlen((char *)text_prop.value) << " - letra " << (int) text_prop.value[i] << text_prop.value[i];
              if (((int)text_prop.value[i] >= 160 && (int)text_prop.value[i] < 256) || ((int)text_prop.value[i] == 34)) { //https://www.utf8-chartable.de/unicode-utf8-table.pl?number=1024&utf8=dec&unicodeinhtml=dec&htmlent=1
                    sprintf(letra,"&#%d;",(int)text_prop.value[i]);
                }else{
                  if ((int)text_prop.value[i] > 31 && (int)text_prop.value[i] < 127) 
                      sprintf(letra,"%c",text_prop.value[i]);
                }
                strcpy(json_wnd+strlen(json_wnd),letra);
            }
           strcpy(json_wnd+strlen(json_wnd),"\",\"class\":\"");
           //if(attr_next.map_installed != 1) 
           if(strcmp(wndClas.res_class,"Google-chrome") == 0 && strcmp(wndClas.res_name,"google-chrome") != 0)
            strcpy(json_wnd+strlen(json_wnd),wndClas.res_name);
           else
              if(strcmp(wndClas.res_class,"Chromium-browser") == 0 && strcmp(wndClas.res_name,"chromium-browser") != 0)
                strcpy(json_wnd+strlen(json_wnd),wndClas.res_name);
              else
                strcpy(json_wnd+strlen(json_wnd),wndClas.res_class);
           //else
              //strcpy(json_wnd+strlen(json_wnd),"NULL");
           strcpy(json_wnd+strlen(json_wnd),"\"}");

          //sprintf(wnd_id,"%s{\"id\":\"%d\",\"name\":\"%s\"}",coma,(int)window->first,text_prop.value);
          //strcpy(json_wnd+strlen(json_wnd),wnd_id);
          sprintf(coma,",");

          XFree(wndClas.res_class);
          XFree(wndClas.res_name); 
        }/*else{
          sprintf(wnd_id,"%s{\"id\":\"-1\",\"name\":\"",coma);
          strcpy(json_wnd+strlen(json_wnd),wnd_id);
          strcpy(json_wnd+strlen(json_wnd),"\",\"class\":\"");
          strcpy(json_wnd+strlen(json_wnd),"\"}");
          sprintf(coma,",");
        }*/  
        ++window;        
    }

  //sprintf(jMed,"%d",(int)clients_.size());
  //LOG(INFO) << "X11 " <<clients_.size();
  if(!isPanel){
    /*sprintf(jMed,"%ld",clientBack[1]);
    focoBarraTarea = clientBack[1];*/
    /*Window focusedWindow;
    int focusRevert;
    XGetInputFocus(display_, &focusedWindow, &focusRevert);
    sprintf(jMed,"%ld",focusedWindow);*/
    //LOG(INFO) << "clientFocus " << clientFocus[3] << " clientBack " << clientBack[1];
    if(w == 0)
    {
      if(clientFocus[3] != panel[1]){ //si no es el panel el del foco es el back por el focusout
        sprintf(jMed,"%ld",clientFocus[3]);
        //focoBarraTarea = clientFocus[3];
      }else{
        sprintf(jMed,"%ld",clientBack[1]);
        //focoBarraTarea = clientBack[1];
      }
    }else{
      sprintf(jMed,"%ld",w);
    }
    
  }   
  else{
    sprintf(jMed,"0");
    //*focoBarraTarea = 0;
  }
 
  //json = (char *) malloc(strlen(jIni) + strlen(jMed) + strlen(jFin) + 1 + strlen(wnd_id));
  strcpy(json,jIni);
  strcat(json,jMed);
  strcat(json,jFin);
  strcat(json,json_wnd);
  strcat(json,jWndFin);

//LOG(INFO) << "JSON Window " << json;
 /* 
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
  //fwrite (json , sizeof(char), sizeof(json), pFile);
  fclose(pFile); 
}*/

//free(rcm);
//////////////se implementa con rcmSend///////////  
  /*memset(rcm, 0, 10225);
  strcpy(rcm,"rcmSend wm '");
  //strcpy(rcm,"wm '");
  //strcpy(rcm,"'");
  strcat(rcm,json);
  //strcat(rcm,"' 2>&1");
  strcat(rcm,"'");*/

  //system(rcm);
  //pFile = popen(rcm, "r");
  //pclose(pFile);
  //wait(&status);

  //se conserva el fork para evitar que al mandar llamar del core quedara en espera y bloqueado el sistema
  /*pid_t child_pid;
  signal(SIGCHLD, SIG_IGN); //este ya no me crea los zombies
  child_pid = fork();
  if(child_pid == 0) {//pasar puerto
    using easywsclient::WebSocket;
    std::unique_ptr<WebSocket> ws(WebSocket::from_url("ws://localhost:2999",".wm.rcmSolar"));
    assert(ws);
    ws->send(json);
    ws->poll();
    ws->close();
    exit(0);
  }*/
  rcmSend(json);
  
  /*char* arg[] = {(char*)"rcmSend",(char*)"wm",json,NULL}; 
  pid_t child_pid;
  signal(SIGCHLD, SIG_IGN); //este ya no me crea los zombies
  child_pid = fork();
  if(child_pid == 0) {
    execv(path, arg);
    exit(0);
  }*/
  /*else{
    int status;
    //wait(&status);//con este era el mismo comortamiento que popen, system... "inestabilidad en irse al panel principal"
    //se siguen creando los zombies con waitpid con WNOHANG :)
    waitpid(child_pid, &status, WNOHANG);
    //LOG(INFO) << "rcm: [" <<  status  << "]";
  }*/
  //LOG(INFO) << "rcm: [" <<  rcm  << "]"; 
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
/*void WindowManager::sendWindowFocus(Window w) {

  const char *path = "/bin/rcmSend"; //"/.containerrcm/.wm.rcmSolar";
  const char *jIni = "{\"message\":{\"call\":\"wfocusbar\"";
  const char *jWndFin = "}}";

  char wnd_id[110];//500
  char json_wnd[9721];//10111

  char json[9821];//10211
  char letra[6];

  memset(wnd_id, 0, 110);
  memset(json_wnd, 0, 9721);
  memset(json, 0, 9821);

  auto window = clients_.find(w);
  if(window != clients_.end()){
    XTextProperty text_prop; //para el titulo de las vemtanas
    XClassHint wndClas; //para el nombre d ela classe
    XWindowAttributes attr_next;
    Status retorno; 
    retorno = XGetWindowAttributes(display_, window->first, &attr_next);
    retorno = XGetWMName(display_, window->first, &text_prop);
    retorno = XGetClassHint(display_, window->first, &wndClas);
    if(retorno != 0){
          sprintf(wnd_id,",\"window\":{\"id\":\"%ld\",\"name\":\"",window->first);
          strcpy(json_wnd+strlen(json_wnd),wnd_id);
          if(text_prop.value != NULL)
            for (long unsigned int i = 0; i < strlen((char *)text_prop.value); i++){
              memset(letra, 0, 6);
              //LOG(INFO) << strlen((char *)text_prop.value) << " - letra " << (int) text_prop.value[i] << text_prop.value[i];
              if (((int)text_prop.value[i] >= 160 && (int)text_prop.value[i] < 256) || ((int)text_prop.value[i] == 34)) { //https://www.utf8-chartable.de/unicode-utf8-table.pl?number=1024&utf8=dec&unicodeinhtml=dec&htmlent=1
                    sprintf(letra,"&#%d;",(int)text_prop.value[i]);
                }else{
                  if ((int)text_prop.value[i] > 31 && (int)text_prop.value[i] < 127) 
                      sprintf(letra,"%c",text_prop.value[i]);
                }
                strcpy(json_wnd+strlen(json_wnd),letra);
            }
           strcpy(json_wnd+strlen(json_wnd),"\",\"class\":\"");

           if(strcmp(wndClas.res_class,"Google-chrome") == 0 && strcmp(wndClas.res_name,"google-chrome") != 0)
            strcpy(json_wnd+strlen(json_wnd),wndClas.res_name);
           else
              if(strcmp(wndClas.res_class,"Chromium-browser") == 0 && strcmp(wndClas.res_name,"chromium-browser") != 0)
                strcpy(json_wnd+strlen(json_wnd),wndClas.res_name);
              else
                strcpy(json_wnd+strlen(json_wnd),wndClas.res_class);
         
           strcpy(json_wnd+strlen(json_wnd),"\"}");

          XFree(wndClas.res_class);
          XFree(wndClas.res_name); 
        }

  }

  strcpy(json,jIni);
  strcat(json,json_wnd);
  strcat(json,jWndFin);

  //LOG(INFO) << " -> " << json;
 
  char* arg[] = {(char*)"rcmSend",(char*)"wm",json,NULL}; 
  pid_t child_pid;
  signal(SIGCHLD, SIG_IGN); //este ya no me crea los zombies
  child_pid = fork();
  if(child_pid == 0) {
    execv(path, arg);
    exit(0);
  }

}*/

void WindowManager::normalizarWindows(/*Window wnd*/){

  int y = 0,x = 0;
  int height = 0,width = 0;
  int tamanioTopBarra = 0;//23;se realiza por porcentaje por tamanios de pantalla // sin borde //24; //conborde
  unsigned int monitores = 1;
  XWindowAttributes x_root_attr,attr_Xwin;  

  XGetWindowAttributes(display_, root_, &x_root_attr);

  if(screenWidth != 0)
    monitores = x_root_attr.width/screenWidth;

  //////////se genera la cantidad de pixeles por 2.13 porcentaje de tamanio
  tamanioTopBarra = ((2.13*x_root_attr.height)/100);
   
  auto window = clients_.begin();
  while (window != clients_.end()){

    //if(wnd != window->first){

    /*XWMHints *hint;
    hint  = XGetWMHints(display_, window->first);


    LOG(INFO) << " sttus -->  "<< window->first << " -- " <<   hint->icon_window;*/
    if(monitores > 1){
      XGetWindowAttributes(display_, window->second, &attr_Xwin);
      y = 0;
      height = 0;
      width = screenWidth;
      if(attr_Xwin.x < screenWidth){
        x = 0;
        if(!wndFull){
          y = tamanioTopBarra; 
          height = tamanioTopBarra;
        }
      }else{
         x = attr_Xwin.x/screenWidth;
         x = screenWidth * x;
      }
    }else{ 
      x = 0;
      y = 0;
      height = 0;
      width = x_root_attr.width;
      if(!wndFull){
        y = tamanioTopBarra; 
        height = tamanioTopBarra;
      }
    }

      XMoveWindow(
            display_,
            window->second,
            x,y);
      
////////////////Colocamos la ventana en 0 tambien ////////////////////////////////////////////////////////
      XMoveWindow(
            display_,
            window->first,
            0,0);
////////////////////////////////////////////////////////////////////////


        XResizeWindow(
            display_,
            window->second,
            width - 0, x_root_attr.height - height);
        // 2. Resize client window.
        XResizeWindow(
            display_,
            window->first,
            width - 0, x_root_attr.height - height);

   // }

        ++window;
    }

    //LOG(INFO) << " Full -->  " <<   wndFull ;

  /*XMoveWindow(
            display_,
            clients_[wnd],
            0,y);
      
////////////////Colocamos la ventana en 0 tambien ////////////////////////////////////////////////////////
      XMoveWindow(
            display_,
            wnd,
            0,0);
////////////////////////////////////////////////////////////////////////


        XResizeWindow(
            display_,
            clients_[wnd],
            x_root_attr.width - 2, x_root_attr.height - height);
        // 2. Resize client window.
        XResizeWindow(
            display_,
            wnd,
            x_root_attr.width - 2, x_root_attr.height - height);*/

}