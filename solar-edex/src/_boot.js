const signale = require("signale");
const {app, BrowserWindow, dialog, shell} = require("electron");


process.on("uncaughtException", e => {
    signale.fatal(e);
    //Se comenta para evitar que se cierre la aplicacion
    /*dialog.showErrorBox("Solar-eDEX crashed", e.message || "Cannot retrieve error message.");
    if (tty) {
        tty.close();
    }
    if (extraTtys) {
        Object.keys(extraTtys).forEach(key => {
            if (extraTtys[key] !== null) {
                extraTtys[key].close();
            }
        });
    }
///////////////////////////////////Para el crashed//////////////////////////////////////
    process.exit(1);*/
});



signale.start(`Starting Solar-eDEX v${app.getVersion()}`);
signale.info(`With Node ${process.versions.node} and Electron ${process.versions.electron}`);
signale.info(`Renderer is Chrome ${process.versions.chrome}`);

let dev = false;

process.argv.forEach(function (val, index, array) {
  if(val == 'dev')
      dev = true;
});

if(!dev){
    const gotLock = app.requestSingleInstanceLock();
    if (!gotLock) {
        signale.fatal("Error: Another instance of Solar-eDEX is already running. Cannot proceed.");
        app.exit(1);
    }
}

signale.time("Startup");

const electron = require("electron");
const ipc = electron.ipcMain;
const path = require("path");
const url = require("url");
const fs = require("fs");
const Terminal = require("./classes/terminal.class.js").Terminal;

ipc.on("log", (e, type, content) => {
    signale[type](content);
});

var win, tty, extraTtys;
const settingsFile = path.join(electron.app.getPath("userData"), "settings.json");
const themesDir = path.join(electron.app.getPath("userData"), "themes");
const innerThemesDir = path.join(__dirname, "assets/themes");
const kblayoutsDir = path.join(electron.app.getPath("userData"), "keyboards");
const innerKblayoutsDir = path.join(__dirname, "assets/kb_layouts");
const fontsDir = path.join(electron.app.getPath("userData"), "fonts");
const innerFontsDir = path.join(__dirname, "assets/fonts");

///////////////////////////////////keysRegister Solar//////////////////////////////
const registerKeys = path.join(electron.app.getPath("userData"), "kinit.json");
const startUp = path.join(electron.app.getPath("userData"), "startUp.json");
const xobjDB = path.join(electron.app.getPath("userData"), "xobjDB.json");
const entornosDisponibles = ["mate"];
var entorno = '';
entornosDisponibles.forEach((val, index) => {
   if(process.argv[0].endsWith(val)){
      entorno = val;
   }
});

///////////////////////////////////////////////////////////////////////////////////

// Unset proxy env variables to avoid connection problems on the internal websockets
// See #222
if (process.env.http_proxy) delete process.env.http_proxy;
if (process.env.https_proxy) delete process.env.https_proxy;

// Fix userData folder not setup on Windows
try {
    fs.mkdirSync(electron.app.getPath("userData"));
    signale.info(`Created config dir at ${electron.app.getPath("userData")}`);
} catch(e) {
    signale.info(`Base config dir is ${electron.app.getPath("userData")}`);
}
// Create default settings file
if (!fs.existsSync(settingsFile)) {
    fs.writeFileSync(settingsFile, JSON.stringify({
        shell: (process.platform === "win32") ? "powershell.exe" : "bash",
        cwd: /*path.join(*/electron.app.getPath("home")/*,"modulos")*/,
        keyboard: "en-US",
        theme: "espacial",
        termFontSize: 15,
        audio: true,
        disableFeedbackAudio: false,
        //pingAddr: "1.1.1.1",
        port: 3000,
        //nointro: true,
        //nocursor: false,
        //allowWindowed: false,
        excludeThreadsFromToplist: true,
        hideDotfiles: true,
        fsListView: false,
        //experimentalGlobeFeatures: false,
        //experimentalFeatures: false,
        fileManager: "",
        //enablePing: false,
        enableKeyboar: false,
        //sudoGUI: "",
        //showIP: true
        showPanel: false
    }, 4));
}


// Create default register keys file
if (!fs.existsSync(registerKeys)) {
    fs.writeFileSync(registerKeys, JSON.stringify({register:[{"Meta+R":path.join(electron.app.getPath("home"), "modulos/run.xobj")}]}, 4));
}

// Create default startUp file
//////////////////////////////////////////////////////////////////////////
if(entorno != '')
{
  switch(entorno)
  {
     case "mate": 
        if (!fs.existsSync(startUp)) {
            fs.writeFileSync(startUp, JSON.stringify({startApp:["mate-settings-daemon","mate-power-manager","mbexev"]}, 4));
        }break;
  }
}
else
{
    if (!fs.existsSync(startUp)) {
        fs.writeFileSync(startUp, JSON.stringify({startApp:[]}, 4));
    }
}
/////////////////////////////////////////////////////////////////////////

//carpeta modulos
try {
    fs.mkdirSync(path.join(electron.app.getPath("home"), "modulos"));
    signale.info(`Created modulos dir at ${path.join(electron.app.getPath("home"), "modulos")}`);
} catch(e) {
    signale.info(`Base modulos dir is ${path.join(electron.app.getPath("home"), "modulos")}`);
}

if (!fs.existsSync(path.join(electron.app.getPath("home"), "modulos/run.xobj"))) {
    fs.writeFileSync(path.join(electron.app.getPath("home"), "modulos/run.xobj"), fs.readFileSync(path.join(__dirname, "apps/run.xobj"), {encoding:"utf-8"}));
}

if (!fs.existsSync(path.join(electron.app.getPath("home"), "modulos/d2m.xobj"))) {
    fs.writeFileSync(path.join(electron.app.getPath("home"), "modulos/d2m.xobj"), fs.readFileSync(path.join(__dirname, "apps/d2m.xobj"), {encoding:"utf-8"}));
}

if (!fs.existsSync(path.join(electron.app.getPath("home"), "modulos/appcnfgmnu.xobj"))) {
    fs.writeFileSync(path.join(electron.app.getPath("home"), "modulos/appcnfgmnu.xobj"), fs.readFileSync(path.join(__dirname, "apps/appcnfgmnu.xobj"), {encoding:"utf-8"}));
}



//////////////////////////////TOoLSSSS///////////////////////////////////////////////
let pathStacer = path.join(electron.app.getPath("home"), "modulos","stacer.xobj");     

if (fs.existsSync(pathStacer)) {
        fs.unlinkSync(pathStacer); 
   }

if (fs.existsSync(path.join(path.dirname(process.argv[0]),"tools"))) {
    
       //let stacerList = require("child_process").execSync(`ls ${path.join(path.dirname(process.argv[0]),"tools")}`).toString().split("\n");
    let stacerObj = {title:"",x:0,y:0,w:0,h:0,code:"xWndExec('id_stacer','sudo ",id:"id_stacer",hidden:"true"};
    let existStacer = 0;

    let toolsList = [];
    let pathTools = path.join(path.dirname(process.argv[0]),"tools");
                
    toolsList = fs.readdirSync(pathTools);

    new Promise((resolve, reject) => {
         
         if (toolsList.length === 0) resolve(); 
         
         toolsList.forEach(async (file, i) => {
            if(file.toLowerCase().startsWith("stacer") && file.toLowerCase().endsWith("appimage") && existStacer == 0){
                existStacer = 1;
                stacerObj.code += path.join(pathTools,file) + "')";  
                resolve(); 
            }

            if (i === toolsList.length-1) resolve();
         });
    });
                  
    if(existStacer == 1){
        fs.writeFileSync(pathStacer, JSON.stringify(stacerObj, 4));
    }  
}
////////////////////////////Carga objDB/////////////////////////////////// 
if (!fs.existsSync(xobjDB)) {

    let xobjs = [];
    let jsonXObjDB = {};
    try {
        
         xobjs = fs.readdirSync(path.join(electron.app.getPath("home"), "modulos"));

    }catch(err) { 
        signale.info(err);
    }

      new Promise((resolve, reject) => {
        if (xobjs.length === 0) resolve();
        
        xobjs.forEach(async (file, i) => {

        if(file.toLowerCase().endsWith(".xobj")){    

             let ftobd = file.substring(0, file.length - 5).toLowerCase();   
             
             switch(ftobd)
                {
                    case 'd2m': jsonXObjDB[ftobd] = {title: "App Desktop to Module", icon:ftobd};
                                break;

                    case 'appcnfgmnu': jsonXObjDB[ftobd] = {title: "App in Menu", icon: "config"};
                                break;            

                    default: let tit_ftobd = ftobd.replace(/-/g,' ').replace(/_/g,' ').trim().toLowerCase().replace(/\w\S*/g, (w) => (w.replace(/^\w/, (c) => c.toUpperCase())));         
                             if(tit_ftobd.toLowerCase().startsWith("inpanel"))
                                   tit_ftobd = tit_ftobd.replace(tit_ftobd.split(' ')[0] + ' ',"");
                                
                                jsonXObjDB[ftobd] = {title:tit_ftobd, icon:tit_ftobd.toLowerCase()}; 
                                //jsonXObjDB[ftobd] = {title:ftobd.trim().toLowerCase().replace(/\w\S*/g, (w) => (w.replace(/^\w/, (c) => c.toUpperCase()))), icon:ftobd}; 
                                break;            
                }

                if(xobjs.length - 1 === i)
                    resolve(); 
         }              

        });

        fs.writeFileSync(xobjDB, JSON.stringify(jsonXObjDB, 4));
    
    }).catch((err) => { signale.info(err); });
}else{

    let xobjs = [];
    let jsonXObjDB = require(xobjDB);


    try {
        
         xobjs = fs.readdirSync(path.join(electron.app.getPath("home"), "modulos"));

    }catch(err) { 
        signale.info(err);
    }

      new Promise((resolve, reject) => {
        if (xobjs.length === 0) resolve();

        xobjs.forEach(async (file, i) => {

         if(file.toLowerCase().endsWith(".xobj")){   

             let ftobd = file.substring(0, file.length - 5).toLowerCase();   

             if(!jsonXObjDB[ftobd])
             {
                switch(ftobd)
                {
                    case 'd2m': jsonXObjDB[ftobd] = {title: "App Desktop to Module", icon:ftobd};
                                break;

                    case 'appcnfgmnu': jsonXObjDB[ftobd] = {title: "App in Menu", icon: "config"};
                                break;            

                    default: let tit_ftobd = ftobd.replace(/-/g,' ').replace(/_/g,' ').trim().toLowerCase().replace(/\w\S*/g, (w) => (w.replace(/^\w/, (c) => c.toUpperCase())));         
                             if(tit_ftobd.toLowerCase().startsWith("inpanel"))
                                   tit_ftobd = tit_ftobd.replace(tit_ftobd.split(' ')[0] + ' ',"");
                                
                                jsonXObjDB[ftobd] = {title:tit_ftobd, icon:tit_ftobd.toLowerCase()}; 
                                //jsonXObjDB[ftobd] = {title:ftobd.trim().toLowerCase().replace(/\w\S*/g, (w) => (w.replace(/^\w/, (c) => c.toUpperCase()))), icon:ftobd}; 
                             break;            
                }
             }

             xobjs[i] = ftobd;

             if(xobjs.length - 1 === i){

                    for(const key in jsonXObjDB){

                        if(xobjs.indexOf(key) == -1){
                            delete jsonXObjDB[key];
                        }
                    }
                    resolve();
                } 
          }       
                
        });

        fs.writeFileSync(xobjDB, JSON.stringify(jsonXObjDB, 4));

    }).catch((err) => { signale.info(err); });
}


if (!fs.existsSync(path.join(electron.app.getPath("userData"), "wm"))) {
    fs.writeFileSync(path.join(electron.app.getPath("userData"), "wm"), fs.readFileSync(path.join(__dirname, "apps/solar_wm")));

    const { exec } = require("child_process");
    let cmd = 'chmod +x ' + path.join(electron.app.getPath("userData"), "wm");
    exec(cmd, (error, stdout, stderr) => {});
}
// Copy default themes & keyboard layouts & fonts
try {
    fs.mkdirSync(themesDir);
} catch(e) {
    // Folder already exists
}
fs.readdirSync(innerThemesDir).forEach(e => {
    if (!fs.existsSync(path.join(themesDir, e))) {
        fs.writeFileSync(path.join(themesDir, e), fs.readFileSync(path.join(innerThemesDir, e), {encoding:"utf-8"}));
    }
});
try {
    fs.mkdirSync(kblayoutsDir);
} catch(e) {
    // Folder already exists
}
fs.readdirSync(innerKblayoutsDir).forEach(e => {
    fs.writeFileSync(path.join(kblayoutsDir, e), fs.readFileSync(path.join(innerKblayoutsDir, e), {encoding:"utf-8"}));
});
try {
    fs.mkdirSync(fontsDir);
} catch(e) {
    // Folder already exists
}
fs.readdirSync(innerFontsDir).forEach(e => {
    fs.writeFileSync(path.join(fontsDir, e), fs.readFileSync(path.join(innerFontsDir, e)));
});

function createWindow(settings) {
    signale.info("Creating window...");

    let display;
    if (!isNaN(settings.monitor)) {
        display = electron.screen.getAllDisplays()[settings.monitor] || electron.screen.getPrimaryDisplay();
    } else {
        display = electron.screen.getPrimaryDisplay();
    }
    let {x, y, width, height} = display.bounds;
    width++; height++;

    win = new BrowserWindow({
        title: "Solar-eDEX",
        x,
        y,
        width,
        height,
        show: false,
        resizable: true,
        movable: settings.allowWindowed || false,
        fullscreen: true,
        autoHideMenuBar: true,
        frame: settings.allowWindowed || false,
        backgroundColor: '#000000',
        webPreferences: {
            devTools: true,
            backgroundThrottling: false,
            webSecurity: true,
            nodeIntegration: true,
            nodeIntegrationInSubFrames: false,
            allowRunningInsecureContent: true,
            experimentalFeatures: /*settings.experimentalFeatures ||*/ false
        }
    });

    win.loadURL(url.format({
        pathname: path.join(__dirname, 'ui.html'),
        protocol: 'file:',
        slashes: true
    }));



   	signale.complete("Frontend window created!");
    win.show();
    if (!settings.allowWindowed) {
        win.setResizable(false);
    }

    signale.watch("Waiting for frontend connection...");
//  window['idPanel'] = win.id;

    win.webContents.executeJavaScript('window.entorno = "' + entorno + '";');
}

app.on('ready', async () => {
    signale.pending(`Loading settings file...`);
    let settings = require(settingsFile);
    signale.success(`Settings loaded!`);

    if (!require("fs").existsSync(settings.cwd)) throw new Error("Configured cwd path does not exist.");

    // See #366
    let cleanEnv = await require("shell-env")(settings.shell.split(" ")[0]).catch(e => { throw e; });

    Object.assign(cleanEnv, process.env, settings.env, {
        TERM: "xterm-256color",
        COLORTERM: "truecolor",
        TERM_PROGRAM: "Solar-eDEX",
        TERM_PROGRAM_VERSION: app.getVersion()
    });

    signale.pending(`Creating new terminal process on port ${settings.port || '3000'}`);
    tty = new Terminal({
        role: "server",
        shell: settings.shell.split(" ")[0],
        params: settings.shell.split(" ").splice(1),
        cwd: settings.cwd,
        env: cleanEnv,
        port: settings.port || 3000
    });
    signale.success(`Terminal back-end initialized!`);
    tty.onclosed = (code, signal) => {
        tty.ondisconnected = () => {};
        signale.complete("Terminal exited", code, signal);
        app.quit();
    };
    tty.onopened = () => {
        signale.success("Connected to frontend!");
        signale.timeEnd("Startup");
    };
    tty.onresized = (cols, rows) => {
        signale.info("Resized TTY to ", cols, rows);
    };
    tty.ondisconnected = () => {
        signale.error("Lost connection to frontend");
        signale.watch("Waiting for frontend connection...");
    };

    // Support for multithreaded systeminformation calls
    signale.pending("Starting multithreaded calls controller...");
    require("./_multithread.js");

    createWindow(settings);

    // Support for more terminals, used for creating tabs (currently limited to 4 extra terms)
    extraTtys = {};
    let basePort = settings.port || 3000;
    basePort = Number(basePort) + 2;

    for (let i = 0; i < 4; i++) {
        extraTtys[basePort+i] = null;
    }

    ipc.on("ttyspawn", (e, arg) => {
        let port = null;
        Object.keys(extraTtys).forEach(key => {
            if (extraTtys[key] === null && port === null) {
                extraTtys[key] = {};
                port = key;
            }
        });

        if (port === null) {
            signale.error("TTY spawn denied (Reason: exceeded max TTYs number)");
            e.sender.send("ttyspawn-reply", "ERROR: max number of ttys reached");
        } else {
            signale.pending(`Creating new TTY process on port ${port}`);
            let term = new Terminal({
                role: "server",
                shell: settings.shell.split(" ")[0],
                params: settings.shell.split(" ").splice(1),
                cwd: tty.tty._cwd || settings.cwd,
                env: cleanEnv,
                port: port
            });
            signale.success(`New terminal back-end initialized at ${port}`);
            term.onclosed = (code, signal) => {
                term.ondisconnected = () => {};
                term.wss.close();
                signale.complete(`TTY exited at ${port}`, code, signal);
                extraTtys[term.port] = null;
                term = null;
            };
            term.onopened = pid => {
                signale.success(`TTY ${port} connected to frontend (process PID ${pid})`);
            };
            term.onresized = () => {};
            term.ondisconnected = () => {
                term.onclosed = () => {};
                term.close();
                term.wss.close();
                extraTtys[term.port] = null;
                term = null;
            };

            extraTtys[port] = term;
            e.sender.send("ttyspawn-reply", "SUCCESS: "+port);
        }
    });

    // Backend support for theme and keyboard hotswitch
    let themeOverride = null;
    let kbOverride = null;
    ipc.on("getThemeOverride", (e, arg) => {
        e.sender.send("getThemeOverride", themeOverride);
    });
    ipc.on("getKbOverride", (e, arg) => {
        e.sender.send("getKbOverride", kbOverride);
    });
    ipc.on("setThemeOverride", (e, arg) => {
        themeOverride = arg;
    });
    ipc.on("setKbOverride", (e, arg) => {
        kbOverride = arg;
    });
});

app.on('web-contents-created', (e, contents) => {
    // Prevent creating more than one window
    contents.on('new-window', (e, url) => {
        e.preventDefault();
        shell.openExternal(url);
    });
    // Prevent loading something else than the UI
    contents.on('will-navigate', (e, url) => {
        if (url !== contents.getURL()) e.preventDefault();
    });
});

app.on('window-all-closed', () => {
    signale.info("All windows closed");
    app.quit();
});

app.on('before-quit', () => {
    tty.close();
    Object.keys(extraTtys).forEach(key => {
        if (extraTtys[key] !== null) {
            extraTtys[key].close();
        }
    });
    signale.complete("Shutting down...");
});