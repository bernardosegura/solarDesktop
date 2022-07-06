const electron = require("electron");
window.solar = {versions : electron.remote.app.getVersion() + "-0606.22 Beta"};
// Disable eval()
window["cApps"] = {id: '', xobjFile: [], xobjTitle: [], osPathApps: "/usr/share/applications"};
window.setBGI = { change: false, transparency: false};
window.setColorT = "#2E344080";
window.idBattery = 'id_battery';
window.maxBattery = 0;
window.upowerFlag = false;
window.batteryNone = '';
window.alertLowBattery = false;
window.backWnd = '0';

window.eval = global.eval = function () {
    throw new Error("eval() is disabled for security reasons.");
};
// Security helper :)
window._escapeHtml = text => {
    let map = {
        '&': '&amp;',
        '<': '&lt;',
        '>': '&gt;',
        '"': '&quot;',
        "'": '&#039;'
    };
    return text.replace(/[&<>"']/g, m => {return map[m];});
};
window._purifyCSS = str => {
    return str.replace(/[<]/g, "");
};
window._delay = ms => {
    return new Promise((resolve, reject) => {
        setTimeout(resolve, ms);
    });
};

// Initiate basic error handling
window.onerror = (msg, path, line, col, error) => {
    document.getElementById("boot_screen").innerHTML += `${error} :  ${msg}<br/>==> at ${path}  ${line}:${col}`;
};

const path = require("path");
const fs = require("fs");
//const electron = require("electron");
const ipc = electron.ipcRenderer;

const settingsDir = electron.remote.app.getPath("userData");
const themesDir = path.join(settingsDir, "themes");
const keyboardsDir = path.join(settingsDir, "keyboards");
const fontsDir = path.join(settingsDir, "fonts");
const settingsFile = path.join(settingsDir, "settings.json");

window.ifaceNetFile = path.join(settingsDir, "ifaceNet.json");
window.ifaceNet = require(ifaceNetFile);

// Load config
window.settings = require(settingsFile);

//load registerKeys
const registerKeys = require(path.join(settingsDir, "kinit.json"));

//load startUp
const startUp = require(path.join(settingsDir, "startUp.json"));

//load xobjDB
window.xobjDB = require(path.join(settingsDir, "xobjDB.json"));

// Load CLI parameters
/*if (electron.remote.process.argv.includes("--nointro")) {
    window.settings.nointroOverride = true;
} else {
    window.settings.nointroOverride = false;
}*/
/*if (electron.remote.process.argv.includes("--nocursor")) {
    window.settings.nocursorOverride = true;
} else {
    window.settings.nocursorOverride = false;
}*/

// Retrieve theme override (hotswitch)
ipc.once("getThemeOverride", (e, theme) => {
    if (theme !== null) {
        window.settings.theme = theme;
        //window.settings.nointroOverride = true;
        _loadTheme(require(path.join(themesDir, window.settings.theme+".json")));
    } else {
        _loadTheme(require(path.join(themesDir, window.settings.theme+".json")));
    }
});
ipc.send("getThemeOverride");
// Same for keyboard override/hotswitch
ipc.once("getKbOverride", (e, layout) => {
    if (layout !== null) {
        window.settings.keyboard = layout;
        //window.settings.nointroOverride = true;
    }
});
ipc.send("getKbOverride");


// Load UI theme
window._loadTheme = theme => {

    if (document.querySelector("style.theming")) {
        document.querySelector("style.theming").remove();
    }

    // Load fonts
    let mainFont = new FontFace(theme.cssvars.font_main, `url("${path.join(fontsDir, theme.cssvars.font_main.toLowerCase().replace(/ /g, '_')+'.woff2').replace(/\\/g, '/')}")`);
    let lightFont = new FontFace(theme.cssvars.font_main_light, `url("${path.join(fontsDir, theme.cssvars.font_main_light.toLowerCase().replace(/ /g, '_')+'.woff2').replace(/\\/g, '/')}")`);
    let termFont = new FontFace(theme.terminal.fontFamily, `url("${path.join(fontsDir, theme.terminal.fontFamily.toLowerCase().replace(/ /g, '_')+'.woff2').replace(/\\/g, '/')}")`);

    document.fonts.add(mainFont);
    document.fonts.load("12px "+theme.cssvars.font_main);
    document.fonts.add(lightFont);
    document.fonts.load("12px "+theme.cssvars.font_main_light);
    document.fonts.add(termFont);
    document.fonts.load("12px "+theme.terminal.fontFamily);    

    /*if(!theme.backgroundImage) theme.backgroundImage = '';
    else
        theme.backgroundImage = `background-size: cover; background-image: url('${theme.backgroundImage}');`;*/

    document.querySelector("head").innerHTML += `<style class="theming">
    :root {
        --font_main: "${theme.cssvars.font_main}";
        --font_main_light: "${theme.cssvars.font_main_light}";
        --color_r: ${theme.colors.r};
        --color_g: ${theme.colors.g};
        --color_b: ${theme.colors.b};
        --color_black: ${theme.colors.black};
        --color_light_black: ${theme.colors.light_black};
        --color_grey: ${theme.colors.grey};
        --color_r1: ${theme.colors.r + 25};
        --color_g1: ${theme.colors.g + 25};
        --color_b1: ${theme.colors.b + 25};
        --color_r2: ${theme.colors.r - 25};
        --color_g2: ${theme.colors.g - 25};
        --color_b2: ${theme.colors.b - 25};
    }

    body {
        font-family: var(--font_main), sans-serif;
        cursor: ${/*(window.settings.nocursorOverride || window.settings.nocursor) ? "none" :*/ "default"} !important;
        ${""/*theme.backgroundImage*/}
    }

    * {
   	   ${/*(window.settings.nocursorOverride || window.settings.nocursor) ? "cursor: none !important;" :*/ ""}
	}
    ${((!window.settings.enableKeyboar)?window._purifyCSS("section#filesystem{left:0;width:100vw}section#filesystem>h3.title,section#filesystem>div{width:99vw}section#keyboard{display:none;}"):"")}
    ${window._purifyCSS(theme.injectCSS || "")}
    </style>`;

    //se cambia la colocacion de la imagen de fondo para realizar cambio en linea
    if(theme.backgroundImage){
        document.body.style.setProperty('background-size','cover');
        document.body.style.setProperty('background-image',`url(${theme.backgroundImage})`);
        theme.terminal.backgroundtmp=theme.terminal.background;
        theme.terminal.background=(theme.terminal.allowTransparency)?window.setColorT:theme.terminal.background;
    }
    
//se cambia section#filesystem>div{width:100vw} por section#filesystem>div{width:99vw} ya que no sale la barra en el sistema real.
    window.theme = theme;
    window.theme.r = theme.colors.r;
    window.theme.g = theme.colors.g;
    window.theme.b = theme.colors.b;

    loadWM(((1 << 24) + (theme.colors.r << 16) + (theme.colors.g << 8) + theme.colors.b).toString(16).slice(1),theme.colors.light_black.replace('#',''));
};

function initGraphicalErrorHandling() {
    window.edexErrorsModals = [];
    window.onerror = (msg, path, line, col, error) => {
        let errorModal = new Modal({
            type: "error",
            title: error,
            message: `${msg}<br/>        at ${path}  ${line}:${col}`
        });
        window.edexErrorsModals.push(errorModal);

        ipc.send("log", "error", `${error}: ${msg}`);
        ipc.send("log", "debug", `at ${path} ${line}:${col}`);
    };
}

function waitForFonts() {
    return new Promise(resolve => {
        if (document.readyState !== "complete" || document.fonts.status !== "loaded") {
            document.addEventListener("readystatechange", () => {
                if (document.readyState === "complete") {
                    if (document.fonts.status === "loaded") {
                        resolve();
                    } else {
                        document.fonts.onloadingdone = () => {
                            if (document.fonts.status === "loaded") resolve();
                        };
                    }
                }
            });
        } else {
            resolve();
        }
    });
}

// A proxy function used to add multithreading to systeminformation calls - see backend process manager @ _multithread.js
function initSystemInformationProxy() {
    const nanoid = require("nanoid/non-secure");

    window.si = new Proxy({}, {
        apply: () => {throw new Error("Cannot use sysinfo proxy directly as a function")},
        set: () => {throw new Error("Cannot set a property on the sysinfo proxy")},
        get: (target, prop, receiver) => {
            return function(...args) {
                let callback = (typeof args[args.length - 1] === "function") ? true : false;

                return new Promise((resolve, reject) => {
                    let id = nanoid();
                    ipc.once("systeminformation-reply-"+id, (e, res) => {
                        if (callback) {
                            args[args.length - 1](res);
                        }
                        resolve(res);
                    });
                    ipc.send("systeminformation-call", prop, id, ...args);
                });
            };
        }
    });
}

// Init audio
window.audioManager = new AudioManager();

// See #223
electron.remote.app.focus();

let i = 0;
//if (window.settings.nointro || window.settings.nointroOverride) {
    initGraphicalErrorHandling();
    initSystemInformationProxy();
    document.getElementById("boot_screen").remove();
    document.body.setAttribute("class", "");
    waitForFonts().then(initUI);
    
/*} else {
    displayLine();
}*/

// Startup boot log
/*function displayLine() {
    let bootScreen = document.getElementById("boot_screen");
    let log = fs.readFileSync(path.join(__dirname, "assets", "misc", "boot_log.txt")).toString().split('\n');

    function isArchUser() {
        return require("os").platform() === "linux"
                && fs.existsSync("/etc/os-release")
                && fs.readFileSync("/etc/os-release").toString().includes("arch");
    }

    if (typeof log[i] === "undefined") {
        setTimeout(displayTitleScreen, 300);
        return;
    }

    if (log[i] === "Boot Complete") {
        window.audioManager.granted.play();
    } else {
        window.audioManager.stdout.play();
    }
    bootScreen.innerHTML += log[i]+"<br/>";
    i++;

    switch(true) {
        case i === 2:
            bootScreen.innerHTML += `eDEX-UI Kernel version ${electron.remote.app.getVersion()} boot at ${Date().toString()}; root:xnu-1699.22.73~1/RELEASE_X86_64`;
        case i === 4:
            setTimeout(displayLine, 500);
            break;
        case i > 4 && i < 25:
            setTimeout(displayLine, 30);
            break;
        case i === 25:
            setTimeout(displayLine, 400);
            break;
        case i === 42:
            setTimeout(displayLine, 300);
            break;
        case i > 42 && i < 82:
            setTimeout(displayLine, 25);
            break;
        case i === 83:
            if (isArchUser())
                bootScreen.innerHTML += "btw i use arch<br/>";
            setTimeout(displayLine, 25);
            break;
        case i >= log.length-2 && i < log.length:
            setTimeout(displayLine, 300);
            break;
        default:
            setTimeout(displayLine, Math.pow(1 - (i/1000), 3)*25);
    }
}*/

// Show "logo" and background grid
/*async function displayTitleScreen() {
    let bootScreen = document.getElementById("boot_screen");
    if (bootScreen === null) {
        bootScreen = document.createElement("section");
        bootScreen.setAttribute("id", "boot_screen");
        bootScreen.setAttribute("style", "z-index: 9999999");
        document.body.appendChild(bootScreen);
    }
    bootScreen.innerHTML = "";
    window.audioManager.theme.play();

    await _delay(400);

    document.body.setAttribute("class", "");
    bootScreen.setAttribute("class", "center");
    bootScreen.innerHTML = "<h1>Solar-eDEX</h1>";
    let title = document.querySelector("section > h1");

    await _delay(200);

    document.body.setAttribute("class", "solidBackground");

    await _delay(100);

    title.setAttribute("style", `background-color: rgb(${window.theme.r}, ${window.theme.g}, ${window.theme.b});border-bottom: 5px solid rgb(${window.theme.r}, ${window.theme.g}, ${window.theme.b});`);

    await _delay(300);

    title.setAttribute("style", `border: 5px solid rgb(${window.theme.r}, ${window.theme.g}, ${window.theme.b});`);

    await _delay(100);

    title.setAttribute("style", "");
    title.setAttribute("class", "glitch");

    await _delay(500);

    document.body.setAttribute("class", "");
    title.setAttribute("class", "");
    title.setAttribute("style", `border: 5px solid rgb(${window.theme.r}, ${window.theme.g}, ${window.theme.b});`);

    await _delay(1000);
    if (window.term) {
        bootScreen.remove();
        return true;
    }
    initGraphicalErrorHandling();
    initSystemInformationProxy();
    waitForFonts().then(() => {
        bootScreen.remove();
        initUI();
    });
}*/

// Create the UI's html structure and initialize the terminal client and the keyboard
async function initUI() {
    let tAdmin = 'mate-time-admin';
    let execTime = `onclick="timeAdmin('${tAdmin}')" style="cursor: pointer;" title="Time Admin"`;
    let which = require("child_process").execSync("which " + tAdmin + ' | wc -l').toString();
    let time = (parseInt(which) != 0)? true:false;
    /*let time = true;
    let execTime = `onclick="mostrarPanel()" style="cursor: pointer;" title="Hide X Window System"`;*/

    document.body.innerHTML += `<section class="top_panel">
        <!--div style="position: absolute;height: 23px;width: 100%; background: linear-gradient(90deg,var(--color_light_black) 1.85vh,transparent 1%) center,linear-gradient(var(--color_light_black) 1.85vh,transparent 1%) center,var(--color_grey); opacity: 0.4;"></div-->
        <section id="main_panel"></section>
        <section  class="task_panel">
            <div class="task_app" id="id_closeXWindPanel" style="cursor: pointer; visibility: hidden;" onclick="closeNativeWindow()" title="Close Window"></div>
            <div id="id_task_reloj" class="task_reloj" ${((!window.settings.showclocktopbar)?'style="display: block; cursor: pointer; margin-right: 0.5vh;"':'style="display: none; cursor: pointer; margin-right: 0.5vh;"')} ${(time)?execTime:''}></div>
            <div id='id_task_panel' style="float: left;">                         
            </div>                          
        </section>
    </section>
    <section class="mod_column" id="mod_column_left">
        <h3 class="title"><p id='idUser'></p><p id='idName'></p></h3>
    </section>
    <section id="main_shell" augmented-ui="tl-clip br-clip exe" style="height:0%;width:0%;opacity:0;margin-bottom:30vh;">
        <h3 class="title" style="opacity:0;"><p>TERMINAL</p><p>MAIN SHELL</p></h3>
        <h1 id="main_shell_greeting"></h1>
    </section>
    <section class="mod_column" id="mod_column_right" style="display: none;">
        <h3 class="title"><p>OPENED NATIVE WINDOW</p><p>LINUX</p></h3>
        <div id="id_panel_xwindow" class="panel_xwindow"></div>
    </section>`;

    await _delay(10);

    window.audioManager.expand.play();
    document.getElementById("main_shell").setAttribute("style", "height:0%;margin-bottom:30vh;");

    await _delay(500);

    document.getElementById("main_shell").setAttribute("style", "margin-bottom: 30vh;");
    document.querySelector("#main_shell > h3.title").setAttribute("style", "");

    await _delay(700);

    document.getElementById("main_shell").setAttribute("style", "opacity: 0;");
    document.body.innerHTML += `
    <section id="filesystem" style="width: 0px;" class="${window.settings.hideDotfiles ? "hideDotfiles" : ""} ${window.settings.fsListView ? "list-view" : ""}">
    </section>
    <section id="keyboard" style="opacity:0;">
    </section>`;
    window.keyboard = new Keyboard({
        layout: path.join(keyboardsDir, settings.keyboard+".json"),
        container: "keyboard"
    });

    await _delay(10);

    document.getElementById("main_shell").setAttribute("style", "");

    await _delay(270);

    let greeter = document.getElementById("main_shell_greeting");

    require("username")().then(user => {
        greeter.innerHTML += `Welcome, <em>${user}</em>`; //solar
        document.getElementById("idUser").innerHTML = "USERNAME";
        document.getElementById("idName").innerHTML = user.toUpperCase();
    }).catch(() => {
        greeter.innerHTML += "Welcome";//solar
    });
    greeter.setAttribute("style", "opacity: 1;");

    document.getElementById("filesystem").setAttribute("style", "");
    document.getElementById("keyboard").setAttribute("style", "");
    document.getElementById("keyboard").setAttribute("class", "animation_state_1");
    window.audioManager.keyboard.play();

    await _delay(100);

    document.getElementById("keyboard").setAttribute("class", "animation_state_1 animation_state_2");

    await _delay(1000);

    greeter.setAttribute("style", "opacity: 0;");

    await _delay(100);

    document.getElementById("keyboard").setAttribute("class", "");

    await _delay(400);

    greeter.remove();

    //verificamos si colocar panel visible o no.
    showTogglePanel(true);

    // Initialize modules
    window.mods = {};

    // Left column
    window.mods.clock = new Clock("mod_column_left");
    window.mods.sysinfo = new Sysinfo("mod_column_left");
    window.mods.hardwareInspector = new HardwareInspector("mod_column_left");
    window.mods.cpuinfo = new Cpuinfo("mod_column_left");
    window.mods.ramwatcher = new RAMwatcher("mod_column_left");
    window.mods.toplist = new Toplist("mod_column_left");

    // Right column
    //window.mods.netstat = new Netstat("mod_column_right");
    window.objRedId = '';
    window.objRedTitle = '';
    window.ssidWifi = '';
    window.mods.netstat = new Netstat("inpanel-wireless|inpanel-red|inpanel-network");
    // se quitan modulos de mundo y monitoreo red, aprovechar panel en futura version.
    //window.mods.globe = new LocationGlobe("mod_column_right");
    //window.mods.conninfo = new Conninfo("mod_column_left");//("mod_column_right");

    // Fade-in animations
    document.querySelectorAll(".mod_column").forEach(e => {
        e.setAttribute("class", "mod_column activated");
    });
    let i = 0;
    let left = document.querySelectorAll("#mod_column_left > div");
    //let right = document.querySelectorAll("#mod_column_right > div");
    let x = setInterval(() => {
        if (!left[i] /*&& !right[i]*/) {
            clearInterval(x);
        } else {
            window.audioManager.panels.play();
            if (left[i]) {
                left[i].setAttribute("style", "animation-play-state: running;");
            }
            /*if (right[i]) {
                right[i].setAttribute("style", "animation-play-state: running;");
            }*/
            i++;
        }
    }, 500);

    await _delay(100);

    // Initialize the terminal
    let shellContainer = document.getElementById("main_shell");
    shellContainer.innerHTML += `
        <ul id="main_shell_tabs">
            <li id="shell_tab0" onclick="window.focusShellTab(0);" class="active"><p>MAIN SHELL</p></li>
            <li id="shell_tab1" onclick="window.focusShellTab(1);"><p>EMPTY</p></li>
            <li id="shell_tab2" onclick="window.focusShellTab(2);"><p>EMPTY</p></li>
            <li id="shell_tab3" onclick="window.focusShellTab(3);"><p>EMPTY</p></li>
            <li id="shell_tab4" onclick="window.focusShellTab(4);"><p>EMPTY</p></li>
        </ul>
        <div id="main_shell_innercontainer">
            <pre id="terminal0" class="active"></pre>
            <pre id="terminal1"></pre>
            <pre id="terminal2"></pre>
            <pre id="terminal3"></pre>
            <pre id="terminal4"></pre>
        </div>`;
    window.term = {
        0: new Terminal({
            role: "client",
            parentId: "terminal0",
            port: window.settings.port || 3000
        })
    };
    window.currentTerm = 0;
    window.term[0].onprocesschange = p => {
        document.getElementById("shell_tab0").innerHTML = `<p>MAIN - ${(p.startsWith('krnlcastnow'))?p.replace('krnl',''):p}</p>`;
    };
    // Prevent losing hardware keyboard focus on the terminal when using touch keyboard
    window.onmouseup = e => {
        if (window.keyboard.linkedToTerm) window.term[window.currentTerm].term.focus();
    };
    //window.term[0].term.writeln("\033[1m"+`Welcome to eDEX-UI v${electron.remote.app.getVersion()} - Electron v${process.versions.electron}`+"\033[0m");
    window.term[0].term.writeln("\033[1m"+`Welcome to Solar-eDEX v${window.solar.versions}`+"\033[0m");

    await _delay(100);

    window.fsDisp = new FilesystemDisplay({
        parentId: "filesystem"
    });

    await _delay(200);

    document.getElementById("filesystem").setAttribute("style", "opacity: 1;");

    // Resend terminal CWD to fsDisp if we're hot reloading
    if (window.performance.navigation.type === 1) {
        window.term[window.currentTerm].resendCWD();
    }

    await _delay(200);

    //////////////////////////////////////////////////////////////////////////////////////
    let icons = require("./assets/icons/file-icons.json").closeWin;
    document.querySelector("#id_closeXWindPanel").innerHTML = `<svg viewBox="0 0 ${icons.width} ${icons.height}" fill="rgb(${window.theme.r}, ${window.theme.g}, ${window.theme.b})" style="width: 100%; height: 100%;">
                                            ${icons.svg}
                                        </svg>`;
    /////////////////////////////////////////////////////////////////////////////////////

    //window.updateCheck = new UpdateChecker(); /* implementar uno en su momento.*/
}

window.themeChanger = theme => {
    ipc.send("setThemeOverride", theme);
    setTimeout(() => {
        window.location.reload(true);
    }, 100);
};

window.remakeKeyboard = layout => {
    document.getElementById("keyboard").innerHTML = "";
    window.keyboard = new Keyboard({
        layout: path.join(keyboardsDir, layout+".json" || settings.keyboard+".json"),
        container: "keyboard"
    });
    ipc.send("setKbOverride", layout);
};

window.focusShellTab = (number, cmd="") => {
    window.audioManager.folder.play();

    if (number !== window.currentTerm && window.term[number]) {

        window.currentTerm = number;

        document.querySelectorAll(`ul#main_shell_tabs > li:not(:nth-child(${number+1}))`).forEach(e => {
            e.setAttribute("class", "");
        });
        document.getElementById("shell_tab"+number).setAttribute("class", "active");

        document.querySelectorAll(`div#main_shell_innercontainer > pre:not(:nth-child(${number+1}))`).forEach(e => {
            e.setAttribute("class", "");
        });
        document.getElementById("terminal"+number).setAttribute("class", "active");

        window.term[number].fit();
        window.term[number].term.focus();
        window.term[number].resendCWD();

        window.fsDisp.followTab();

        if(cmd != ""){
            window.term[number].writelr(cmd);
        }
    } else if (number > 0 && number <= 4 && window.term[number] !== null && typeof window.term[number] !== "object") {
        window.term[number] = null;

        document.getElementById("shell_tab"+number).innerHTML = "<p>LOADING...</p>";
        ipc.send("ttyspawn", "true");
        ipc.once("ttyspawn-reply", (e, r) => {
            if (r.startsWith("ERROR")) {
                document.getElementById("shell_tab"+number).innerHTML = "<p>ERROR</p>";
            } else if (r.startsWith("SUCCESS")) {
                let port = Number(r.substr(9));

                window.term[number] = new Terminal({
                    role: "client",
                    parentId: "terminal"+number,
                    port
                });


                if(window.setBGI.change){
                    if(window.setBGI.transparency){
                      window.termThemeTmp.background = window.setColorT;  
                      window.term[number].term.setOption("allowTransparency",window.setBGI.transparency);
                      window.term[number].term.setOption("theme",window.termThemeTmp);  
                    }else{
                      window.termThemeTmp.background = window.theme.terminal.backgroundtmp;
                      window.term[number].term.setOption("theme",window.termThemeTmp);                    
                    }
               }

                window.term[number].onclose = e => {
                    delete window.term[number].onprocesschange;
                    document.getElementById("shell_tab"+number).innerHTML = "<p>EMPTY</p>";
                    document.getElementById("terminal"+number).innerHTML = "";
                    delete window.term[number];
                    window.focusShellTab(0);
                };

                window.term[number].onprocesschange = p => {
                    document.getElementById("shell_tab"+number).innerHTML = `<p>#${number+1} - ${(p.startsWith('krnlcastnow'))?p.replace('krnl',''):p}</p>`;
                };

                document.getElementById("shell_tab"+number).innerHTML = `<p>::${port}</p>`;
                setTimeout(() => {
                    window.focusShellTab(number,cmd);
                }, 500);
            }
        });
    }
};

// Settings editor
window.openSettings = async () => {
    // Build lists of available keyboards, themes, monitors
    let keyboards, themes, monitors; //, ifaces;
    fs.readdirSync(keyboardsDir).forEach(kb => {
        if (!kb.endsWith(".json")) return;
        kb = kb.replace(".json", "");
        if (kb === window.settings.keyboard) return;
        keyboards += `<option>${kb}</option>`;
    });
    fs.readdirSync(themesDir).forEach(th => {
        if (!th.endsWith(".json")) return;
        th = th.replace(".json", "");
        if (th === window.settings.theme) return;
        themes += `<option>${th}</option>`;
    });
    for (let i = 0; i < electron.remote.screen.getAllDisplays().length; i++) {
        if (i !== window.settings.monitor) monitors += `<option>${i}</option>`;
    }
    /*let nets = await window.si.networkInterfaces();
    nets.forEach(net => {
        if (net.iface !== window.mods.netstat.iface) ifaces += `<option>${net.iface}</option>`;
    });*/

    if(!window.settings.ampm)
        window.settings.ampm = false;

    if(!window.settings.numlock)
        window.settings.numlock = false;

    if(!window.settings.capslock)
        window.settings.capslock = false;

    if(!window.settings.showclocktopbar)
        window.settings.showclocktopbar = false;
    // Unlink the tactile keyboard from the terminal emulator to allow filling in the settings fields
    window.keyboard.detach();

    new Modal({
        type: "custom",
        title: `Settings <i>(v${window.solar.versions})</i>`,
        html: `<table id="settingsEditor">
                    <tr>
                        <th>Key</th>
                        <th>Description</th>
                        <th>Value</th>
                    </tr>
                    <tr>
                        <td>shell</td>
                        <td>The program to run as a terminal emulator</td>
                        <td><input type="text" id="settingsEditor-shell" value="${window.settings.shell}"></td>
                    </tr>
                    <tr>
                        <td>cwd</td>
                        <td>Working Directory to start in</td>
                        <td><input type="text" id="settingsEditor-cwd" value="${window.settings.cwd}"></td>
                    </tr>
                    <tr style="display:none">
                        <td>env</td>
                        <td>Custom shell environment override</td>
                        <td><input type="text" id="settingsEditor-env" value="${window.settings.env}"></td>
                    </tr>
                    <tr>
                        <td>keyboard</td>
                        <td>On-screen keyboard layout code</td>
                        <td><select id="settingsEditor-keyboard">
                            <option>${window.settings.keyboard}</option>
                            ${keyboards}
                        </select></td>
                    </tr>
                    <tr>
                        <td>theme</td>
                        <td>Name of the theme to load</td>
                        <td><select id="settingsEditor-theme">
                            <option>${window.settings.theme}</option>
                            ${themes}
                        </select></td>
                    </tr>
                    <tr>
                        <td>termFontSize</td>
                        <td>Size of the terminal text in pixels</td>
                        <td><input type="number" id="settingsEditor-termFontSize" value="${window.settings.termFontSize}"></td>
                    </tr>
                    <tr>
                        <td>audio</td>
                        <td>Activate audio sound effects</td>
                        <td><select id="settingsEditor-audio">
                            <option>${window.settings.audio}</option>
                            <option>${!window.settings.audio}</option>
                        </select></td>
                    </tr>
                    <tr>
                        <td>disableFeedbackAudio</td>
                        <td>Disable recurring feedback sound FX (input/output, mostly)</td>
                        <td><select id="settingsEditor-disableFeedbackAudio">
                            <option>${window.settings.disableFeedbackAudio}</option>
                            <option>${!window.settings.disableFeedbackAudio}</option>
                        </select></td>
                    </tr>
                    <tr>
                        <td>port</td>
                        <td>Local port to use for UI-shell connection</td>
                        <td><input type="number" id="settingsEditor-port" value="${window.settings.port}"></td>
                    </tr>
                    <!--tr>
                        <td>pingAddr</td>
                        <td>IPv4 address to test Internet connectivity</td>
                        <td><input type="text" id="settingsEditor-pingAddr" value="${/*window.settings.pingAddr ||*/ "1.1.1.1"}"></td>
                    </tr-->
                    <tr>
                        <td>monitor</td>
                        <td>Which monitor to spawn the UI in (defaults to primary display)</td>
                        <td><select id="settingsEditor-monitor">
                            ${(typeof window.settings.monitor !== "undefined") ? "<option>"+window.settings.monitor+"</option>" : ""}
                            ${monitors}
                        </select></td>
                    </tr>
                    <!--tr style="display:none">
                        <td>nointro</td>
                        <td>Skip the intro boot log and logo${(window.settings.nointroOverride) ? " (Currently overridden by CLI flag)" : ""}</td>
                        <td><select id="settingsEditor-nointro">
                            <option>${window.settings.nointro}</option>
                            <option>${!window.settings.nointro}</option>
                        </select></td>
                    </tr-->
                    <!--tr>
                        <td>nocursor</td>
                        <td>Hide the mouse cursor${/*(window.settings.nocursorOverride) ? " (Currently overridden by CLI flag)" :*/ ""}</td>
                        <td><select id="settingsEditor-nocursor">
                            <option>${/*window.settings.nocursor*/ ""}</option>
                            <option>${/*!window.settings.nocursor*/ ""}</option>
                        </select></td>
                    </tr-->
                    <!--tr>
                        <td>iface</td>
                        <td>Override the interface used for network monitoring</td>
                        <td><select id="settingsEditor-iface">
                            <option>${/*window.mods.netstat.iface*/ ""}</option>
                            ${/*ifaces*/ ""}
                        </select></td>
                    </tr-->
                    <!--tr>
                        <td>allowWindowed</td>
                        <td>Allow using F11 key to set the UI in windowed mode</td>
                        <td><select id="settingsEditor-allowWindowed">
                            <option>${/*window.settings.allowWindowed*/ ""}</option>
                            <option>${/*!window.settings.allowWindowed*/""}</option>
                        </select></td>
                    </tr-->
                    <tr>
                        <td>excludeThreadsFromToplist</td>
                        <td>Display threads in the top processes list</td>
                        <td><select id="settingsEditor-excludeThreadsFromToplist">
                            <option>${window.settings.excludeThreadsFromToplist}</option>
                            <option>${!window.settings.excludeThreadsFromToplist}</option>
                        </select></td>
                    </tr>
                    <tr>
                        <td>hideDotfiles</td>
                        <td>Hide files and directories starting with a dot in file display</td>
                        <td><select id="settingsEditor-hideDotfiles">
                            <option>${window.settings.hideDotfiles}</option>
                            <option>${!window.settings.hideDotfiles}</option>
                        </select></td>
                    </tr>
                    <tr>
                        <td>fsListView</td>
                        <td>Show files in a more detailed list instead of an icon grid</td>
                        <td><select id="settingsEditor-fsListView">
                            <option>${window.settings.fsListView}</option>
                            <option>${!window.settings.fsListView}</option>
                        </select></td>
                    </tr>
                    <!--tr>
                        <td>experimentalGlobeFeatures</td>
                        <td>Toggle experimental features for the network globe</td>
                        <td><select id="settingsEditor-experimentalGlobeFeatures">
                            <option>${/*window.settings.experimentalGlobeFeatures*/ ""}</option>
                            <option>${/*!window.settings.experimentalGlobeFeatures*/ ""}</option>
                        </select></td>
                    </tr>
                    <tr>
                        <td>experimentalFeatures</td>
                        <td>Toggle Chrome's experimental web features (DANGEROUS)</td>
                        <td><select id="settingsEditor-experimentalFeatures">
                            <option>${/*window.settings.experimentalFeatures*/ ""}</option>
                            <option>${/*!window.settings.experimentalFeatures*/ ""}</option>
                        </select></td>
                    </tr-->
                    <tr>
                        <td>fileManager</td>
                        <td>Name file manager to open the directory</td>
                        <td><input type="text" id="settingsEditor-fileManager" value="${(!window.settings.fileManager)?"":window.settings.fileManager}"></td>
                    </tr>
                    <!--tr>
                        <td>enabledPing</td>
                        <td>Send ping to host</td>
                        <td><select id="settingsEditor-enablePing">
                            <option>${/*window.settings.enablePing*/ ""}</option>
                            <option>${/*!window.settings.enablePing*/""}</option>
                        </select></td>
                    </tr-->
                    <tr>
                        <td>enableKeyboar</td>
                        <td>Enable keyboard on screen</td>
                        <td><select id="settingsEditor-enableKeyboar">
                            <option>${window.settings.enableKeyboar}</option>
                            <option>${!window.settings.enableKeyboar}</option>
                        </select></td>
                    </tr>
                    <!--tr>
                        <td>sudoGUI</td>
                        <td>GUI for sudo</td>
                        <td><input type="text" id="settingsEditor-sudoGUI" value="${(!window.settings.sudoGUI)?"":window.settings.sudoGUI}"></td>
                    </tr-->
                    <tr>
                        <td>enableAMPM</td>
                        <td>Enable format am and pm, save to disk for keep change</td>
                        <td><select id="settingsEditor-enableampm" onchange="setAmPm();">
                            <option>${window.settings.ampm}</option>
                            <option>${!window.settings.ampm}</option>
                        </select></td>
                    </tr>
                    <tr>
                        <td>showClockTopBar</td>
                        <td>Hide clock in top bar</td>
                        <td><select id="settingsEditor-showclocktopbar" onchange="showclocktopbar();">
                            <option>${window.settings.showclocktopbar}</option>
                            <option>${!window.settings.showclocktopbar}</option>
                        </select></td>
                    </tr>
                    <tr>
                        <td>autoClosePanel</td>
                        <td>Automatic closing right panel, with XWindow opening and closing event</td>
                        <td><select id="settingsEditor-autoClosePanel">
                            <option>${window.settings.autoClosePanel}</option>
                            <option>${!window.settings.autoClosePanel}</option>
                        </select></td>
                    </tr>
                    <tr>
                        <td>numLock</td>
                        <td>Hide num lock indicator</td>
                        <td><select id="settingsEditor-numlock" onchange="setnumlock();">
                            <option>${window.settings.numlock}</option>
                            <option>${!window.settings.numlock}</option>
                        </select></td>
                    </tr>
                    <tr>
                        <td>capsLock</td>
                        <td>Hide caps lock indicator</td>
                        <td><select id="settingsEditor-capslock" onchange="setcapsock();">
                            <option>${window.settings.capslock}</option>
                            <option>${!window.settings.capslock}</option>
                        </select></td>
                    </tr>
                </table>
                <h6 id="settingsEditorStatus">Loaded values from memory</h6>
                <br>`,
        buttons: [
            {label: "Open in External Editor", action:`electron.shell.openExternal('file://${settingsFile}')`},
            {label: "Save to Disk", action: "window.writeSettingsFile()"},
            //{label: "Reload UI", action: "window.location.reload(true);"},
            //{label: "Restart Solar", action: "electron.remote.app.relaunch();electron.remote.app.quit();"}
            {label: "Exit Solar", action: "electron.remote.app.quit();"}
        ]
    }, () => { 
        if(document.querySelectorAll(".appXwnd").length == 0 /*&& document.querySelectorAll("#settingsEditor").length == 0*/){
            // Link the keyboard back to the terminal
            window.keyboard.attach();

            // Focus back on the term
            window.term[window.currentTerm].term.focus();
        }
      
    });
};

function showclocktopbar(){
    window.settings.showclocktopbar = (!window.settings.showclocktopbar);
    
    if(!window.settings.showclocktopbar)
        document.getElementById('id_task_reloj').style.display = 'block';
    else
        document.getElementById('id_task_reloj').style.display = 'none';
}

function setnumlock(){
    window.settings.numlock = (!window.settings.numlock);
    
    if(!window.settings.numlock)
        document.getElementById('num-lock').style.display = 'block';
    else
        document.getElementById('num-lock').style.display = 'none';
}

function setcapsock(){
    window.settings.capslock = (!window.settings.capslock);
    
    if(!window.settings.capslock)
        document.getElementById('caps-lock').style.display = 'block';
    else
        document.getElementById('caps-lock').style.display = 'none';
}

window.writeSettingsFile = () => {
    window.settings = {
        shell: document.getElementById("settingsEditor-shell").value,
        cwd: document.getElementById("settingsEditor-cwd").value,
        env: document.getElementById("settingsEditor-env").value,
        keyboard: document.getElementById("settingsEditor-keyboard").value,
        theme: document.getElementById("settingsEditor-theme").value,
        termFontSize: Number(document.getElementById("settingsEditor-termFontSize").value),
        audio: (document.getElementById("settingsEditor-audio").value === "true"),
        disableFeedbackAudio: (document.getElementById("settingsEditor-disableFeedbackAudio").value === "true"),
        //pingAddr: document.getElementById("settingsEditor-pingAddr").value,
        port: Number(document.getElementById("settingsEditor-port").value),
        monitor: Number(document.getElementById("settingsEditor-monitor").value),
        //nointro: (document.getElementById("settingsEditor-nointro").value === "true"),
        //nocursor: (document.getElementById("settingsEditor-nocursor").value === "true"),
        //iface: document.getElementById("settingsEditor-iface").value,
        //allowWindowed: (document.getElementById("settingsEditor-allowWindowed").value === "true"),
        excludeThreadsFromToplist: (document.getElementById("settingsEditor-excludeThreadsFromToplist").value === "true"),
        hideDotfiles: (document.getElementById("settingsEditor-hideDotfiles").value === "true"),
        fsListView: (document.getElementById("settingsEditor-fsListView").value === "true"),
        //experimentalGlobeFeatures: (document.getElementById("settingsEditor-experimentalGlobeFeatures").value === "true"),
        //experimentalFeatures: (document.getElementById("settingsEditor-experimentalFeatures").value === "true"),
        fileManager:document.getElementById("settingsEditor-fileManager").value,
        //enablePing:(document.getElementById("settingsEditor-enablePing").value === "true"),
        enableKeyboar:(document.getElementById("settingsEditor-enableKeyboar").value === "true"),
        //sudoGUI:document.getElementById("settingsEditor-sudoGUI").value,
        //showIP: true,//netShowIP(), *revisar con el ocultamiento
        showPanel: window.settings.showPanel,
        ampm: window.settings.ampm,
        autoClosePanel: (document.getElementById("settingsEditor-autoClosePanel").value === "true"),
        capslock: (document.getElementById("settingsEditor-capslock").value === "true"),
        numlock: (document.getElementById("settingsEditor-numlock").value === "true"),
        showclocktopbar: (document.getElementById("settingsEditor-showclocktopbar").value === "true")
    };

    Object.keys(window.settings).forEach(key => {
        if (window.settings[key] === "undefined") {
            delete window.settings[key];
        }
    });

    fs.writeFileSync(settingsFile, JSON.stringify(window.settings, "", 4));
    document.getElementById("settingsEditorStatus").innerText = "New values written to settings.json file at "+new Date().toTimeString();
};

// Display available keyboard shortcuts
window.openShortcutsHelp = () => {
    new Modal({
        type: "custom",
        title: `Available Keyboard Shortcuts <i>(v${window.solar.versions}</i>`,
        html: `<h5>Using either the on-screen or a physical keyboard, you can use the following shortcuts:</h5>
                <table id="shortcutsHelp" style="width: 100%;">
                    <tr>
                        <th>Trigger</th>
                        <th>Action</th>
                    </tr>
                    <tr>
                        <td>${process.platform === "darwin" ? "Command" : "Ctrl + Shift"} + C</td>
                        <td>Copy selected buffer from the terminal.</td>
                    </tr>
                    <tr>
                        <td>${process.platform === "darwin" ? "Command" : "Ctrl + Shift"} + V</td>
                        <td>Paste system clipboard to the terminal.</td>
                    </tr>
                    <tr>
                        <td>Ctrl + Tab</td>
                        <td>Switch to the next opened terminal tab (left to right order).</td>
                    </tr>
                    <tr>
                        <td>Ctrl + Shift + Tab</td>
                        <td>Switch to the previous opened terminal tab (right to left order).</td>
                    </tr>
                    <tr>
                        <td>Ctrl + [1-5]</td>
                        <td>Switch to a specific terminal tab, or create it if it hasn't been opened yet.</td>
                    </tr>
                    <tr>
                        <td>Ctrl + Shift + S</td>
                        <td>Open the settings editor.</td>
                    </tr>
                    <tr>
                        <td>Ctrl + Shift + K</td>
                        <td>List available keyboard shortcuts.</td>
                    </tr>
                    <tr>
                        <td>Ctrl + Shift + H</td>
                        <td>Toggle show hidden module.</td>
                    </tr>
                    <tr>
                        <td>Ctrl + Shift + P</td>
                        <td>Toggle the on-screen keyboard's "Password Mode", that allows you to safely type<br> sensitive information even if your screen might be recorded (disables visual input feedback).</td>
                    </tr>
                    <!--tr>
                        <td>Ctrl + Shift + I</td>
                        <td>Open Chromium Dev Tools (for debugging purposes).</td>
                    </tr>
                    <tr>
                        <td>F5</td>
                        <td>Update Panel File's.</td>
                    </tr-->
                    <tr>
                        <td>Alt + W</td>
                        <td>Toggle Show Right Panel</td>
                    </tr>
                    <tr>
                        <td>Alt + F4</td>
                        <td>Close XWindow</td>
                    </tr>
                    <tr>
                        <td>Alt + Tab</td>
                        <td>Toggle Rigth XWindow</td>
                    </tr>
                    <tr>
                        <td>Ctrl + Alt + Tab</td>
                        <td>Toggle Left XWindow</td>
                    </tr>
                    <tr>
                        <td>Win + Tab</td>
                        <td>Toggle Main Panel</td>
                    </tr>
                    <tr>
                        <td><button onClick='electron.shell.openExternal("file://${require('path').join(require('electron').remote.app.getPath("userData"), "kinit.json")}")' style='position: relative; left: 0px; top: 0px;'>Open kinit</button></td>
                        <td>Open the config file to register the keys that trigger the application, ({register:['combination keys','path and file name xobj']}).</td>
                    </tr>
                </table>
                <br>`
    });
};

// Global keyboard shortcuts
const globalShortcut = electron.remote.globalShortcut;
globalShortcut.unregisterAll();

function registerKeyboardShortcuts() {
    // Open settings
    globalShortcut.register("Control+Shift+S", () => {
        if (!document.getElementById("settingsEditor")) {
            window.openSettings();
        }
    });

    // Open list of keyboard shortcuts
    globalShortcut.register("Control+Shift+K", () => {
        if (!document.getElementById("shortcutsHelp")) {
            window.openShortcutsHelp();
        }
    });

    // Copy and paste shortcuts

    if (process.platform === "darwin") {
        // See #342, we have an actual available key on macOS to do this
        globalShortcut.register("Command+C", () => {
            window.term[window.currentTerm].clipboard.copy();
        });
        globalShortcut.register("Command+V", () => {
            window.term[window.currentTerm].clipboard.paste();
        });
    } else {
        // Use Ctrl+shift on other OSs
        globalShortcut.register("Ctrl+Shift+C", () => {
            window.term[window.currentTerm].clipboard.copy();
        });
        globalShortcut.register("Ctrl+Shift+V", () => {
            window.term[window.currentTerm].clipboard.paste();
        });
    }

    // Switch tabs
    // Next
    globalShortcut.register("Control+Tab", () => {
        if (window.term[window.currentTerm+1]) {
            window.focusShellTab(window.currentTerm+1);
        } else if (window.term[window.currentTerm+2]) {
            window.focusShellTab(window.currentTerm+2);
        } else if (window.term[window.currentTerm+3]) {
            window.focusShellTab(window.currentTerm+3);
        } else if (window.term[window.currentTerm+4]) {
            window.focusShellTab(window.currentTerm+4);
        } else {
            window.focusShellTab(0);
        }
    });
    // Previous
    globalShortcut.register("Control+Shift+Tab", () => {
        let i = window.currentTerm ? window.currentTerm : 4;
        if (window.term[i] && i !== window.currentTerm) {
            window.focusShellTab(i);
        } else if (window.term[i-1]) {
            window.focusShellTab(i-1);
        } else if (window.term[i-2]) {
            window.focusShellTab(i-2);
        } else if (window.term[i-3]) {
            window.focusShellTab(i-3);
        } else if (window.term[i-4]) {
            window.focusShellTab(i-4);
        }
    });
    // By tab number
    globalShortcut.register("Control+1", () => {
        window.focusShellTab(0);
    });
    globalShortcut.register("Control+2", () => {
        window.focusShellTab(1);
    });
    globalShortcut.register("Control+3", () => {
        window.focusShellTab(2);
    });
    globalShortcut.register("Control+4", () => {
        window.focusShellTab(3);
    });
    globalShortcut.register("Control+5", () => {
        window.focusShellTab(4);
    });

    // Toggle hiding dotfiles in fsDisp
    globalShortcut.register("Control+Shift+H", () => {
        window.fsDisp.toggleHidedotfiles();
    });

    // Toggle list view in fsDisp
    globalShortcut.register("Control+Shift+L", () => {
        window.fsDisp.toggleListview();
    });

    // Hide on-screen keyboard visual feedback (#394)
    globalShortcut.register("Alt+Shift+P", () => {
        window.keyboard.togglePasswordMode();
    });

    // Hide on-screen lateral panel visual
    globalShortcut.register("Alt+W"/*Control+Alt+P"*/, () => {
        showTogglePanel();
    });


    // Open inspector es retirado
   /* globalShortcut.register("Control+Shift+I", () => {
        electron.remote.getCurrentWindow().webContents.toggleDevTools();
    });

     // Hide on-screen keyboard visual feedback (#394)
    globalShortcut.register("F5", () => {
        window.fsDisp.readFS(document.getElementById("fs_disp_title_dir").innerHTML);
         //se comenta para deshabilitar el filemanager y que solo funcione en la carpeta de modulos.
        //window.fsDisp.watchFS(document.getElementById("fs_disp_title_dir").innerHTML);
    });*/

//se cambia por que no se propaga el ESC y eso afecto el funcionamiento del admon archivos mc
    globalShortcut.register("Control+Escape", () => { //antes "Escape" pro no funciono esc + tab en mc.
       closeModal();
    });

    globalShortcut.register("Super+Tab", () => { //registramos win + tab para evitar que nos detecte el tab en la consola principal.
    });// lo que antes fue un problema, ahora es un beneficio.


    fnRegisterKeys();
}
registerKeyboardShortcuts();

//////////////////////////////////Se coloca el evento en la ventana/////////////////////////////////
//funciono pero al usar esc se comia la pulsacion siguiente en la consola debido a que la terminal
//usa el escape como comando, se valido en terminal normal y tenia mismo comportamiento
/*window.addEventListener('keyup', (e) =>{
    if(e.key == "Escape"){
        closeModal();
        
    }

});*/

/*electron.remote.getCurrentWindow().webContents.sendInputEvent({
          type: "keyDown",
          keyCode: 'Escape'
        });

        electron.remote.getCurrentWindow().webContents.sendInputEvent({
          type: "keyUp",
          keyCode: 'Escape'
        });*/
///////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////Solar registerKeys////////////////////////////
function fnRegisterKeys()
{
    let registerExist = [,"Alt+W","Control+Shift+S","Control+Shift+K","Command+C","Command+V","Ctrl+Shift+C","Ctrl+Shift+V","Control+Tab","Control+Shift+Tab","Control+1","Control+2","Control+3","Control+4","Control+5","Control+Shift+H","Control+Shift+L","Control+Shift+P","Control+Escape"];
    
    for(let i=0; i < registerKeys.register.length; i++)
    { 
       Object.keys(registerKeys.register[i]).forEach(key => {
            if(!registerExist.includes(key))
            {
                let app = registerKeys.register[i][key];
                window[key] = () =>{window.appXwnd(app);};
                globalShortcut.register(key,window[key]);
                registerExist.push(key);
            }
        });
    }
}

///////////////////////////////////////////////////////////////

// See #361
window.addEventListener("focus", () => {
    registerKeyboardShortcuts();
});

window.addEventListener("blur", () => {
    globalShortcut.unregisterAll();
});

// Prevent showing menu, exiting fullscreen or app with keyboard shortcuts
document.addEventListener("keydown", e => {
    if (e.key === "Alt") {
        e.preventDefault();
    }
    /*if (e.key === "F11" && !settings.allowWindowed) {
        e.preventDefault();
    }*/
    if (e.code === "KeyD" && e.ctrlKey) {
        e.preventDefault();
    }
    if (e.code === "KeyA" && e.ctrlKey) {
        e.preventDefault();
    }

    if (e.key === "Control") {
        window.teclaCTRL =  true;
        e.preventDefault();
    }

    /*if (e.key === "Meta") { //Meta windows key
        //window.teclaCTRL =  true;
        //pantallaX();
        e.preventDefault();
    }*/

});

// Fix #265
window.addEventListener("keyup", e => {
   /*if (e.key === "F4" && e.altKey === true) { //quitamos el cerrar alt + F4
        electron.remote.app.quit();
    }*/

    if (e.key === "Control") {
        window.teclaCTRL =  false;
        e.preventDefault();
    }
});

// Fix double-tap zoom on touchscreens
electron.webFrame.setVisualZoomLevelLimits(1, 1);

// Resize terminal with window
window.onresize = () => {
    if (typeof window.currentTerm !== "undefined") {
        if (typeof window.term[window.currentTerm] !== "undefined") {
            window.term[window.currentTerm].fit();
        }
    }
};

// See #413
window.resizeTimeout = null;
let electronWin = electron.remote.getCurrentWindow();
electronWin.on("resize", () => {
    clearTimeout(window.resizeTimeout);
    window.resizeTimeout = setTimeout(() => {
        let win = electron.remote.getCurrentWindow();
        if (win.isFullScreen()) return false;
        if (win.isMaximized()) {
            win.unmaximize();
            win.setFullScreen(true);
            return false;
        }

        let size = win.getSize();

        if (size[0] >= size[1]) {
            win.setSize(size[0], parseInt(size[0] * 9 / 16));
        } else {
            win.setSize(size[1], parseInt(size[1] * 9 / 16));
        }
    }, 100);
});

electronWin.on("leave-full-screen", () => {
    electron.remote.getCurrentWindow().setSize(960, 540);
});
///////////////////////////////////////////Solar////////////////////////////////
function xWindow(obj,f,audioOff)
{
    if(!f) f = '';
    if(!obj) obj = {};
    if(!obj.id)
    {
          if(!audioOff)  
              window.audioManager.info.play(); //style="position:relative; top: 2px;"
          let titleClose = (!obj.titleClose)?"Close":obj.titleClose;
          let titleWnd = (!obj.title)?"Window":obj.title;
          let id = "id_pantalla-"+ require("nanoid")();
          let pX = (!obj.x)?0:obj.x;
          let pY = (!obj.y)?0:obj.y;
          let numPX = document.querySelectorAll(".appXwnd").length;
          let zIndex = (!obj.zIndex)?((numPX != 0)?document.querySelectorAll(".appXwnd")[numPX - 1].style.zIndex + 1 :400):obj.zIndex;
          let code = (!obj.code)?'':obj.code;
          let noLimit = (!obj.noLimit)?'':((obj.noLimit > 0)?'noLimit':''); 
          let agExec = (!obj.noLimit)?'exe':(obj.noLimit < 1)?'exe':''; 
          let bgFnd = (!obj.noLimit)?'bgFnd':(obj.noLimit < 1)?'bgFnd':''; 
           

          let tmp = document.createElement("div");
          recClient = document.body.getBoundingClientRect();
          let iHeight = (!obj.h)?parseInt(recClient.height) - 35:obj.h;
          let iWidth = (!obj.w)?parseInt(recClient.width) - 37:obj.w;
//////////////////////////////se agrega para centrado automatico
          if(!obj.x){
            pX = ((parseInt(recClient.width)/2) - (iWidth/2));
          }

          if(!obj.y){
            pY = ((parseInt(recClient.height)/2) - (iHeight + 9));
          }
//////////////////////////////////////////////////////////////////
          let wndHidden = (!obj.hidden)?'':(obj.hidden == 'true')?'display:none':'';
          iHeight = (pY + iHeight > (parseInt(recClient.height) - 35))?iHeight - pY:iHeight;
          iWidth = (pX + iWidth > (parseInt(recClient.width) - 37))?iWidth - pX:iWidth;
          let sContenido = (!obj.content)?"":obj.content;
          let hBarraT = 2;
          tmp.innerHTML = `<div id="${id}" class="info custom focus appXwnd ${noLimit} ${bgFnd}" augmented-ui="tl-clip br-clip ${agExec}" style="z-index:${zIndex}; left: ${pX}px; top: ${pY}px; width: ${iWidth/10}vh; height: ${iHeight/10}vh; opacity: 0.8; ${wndHidden}">
             <div ontouchstart="xwTouchStart(event,'${id}')" onmousedown="xwMouseDown(event,'${id}')" style="border-bottom: 1px solid; position:absolute; left: 0px; top: 0px; width: 100%; height: ${hBarraT}vh;">
                <table style="width: 100%;">
                    <tr style="height: ${hBarraT}vh;">
                        <td>
                            <div style="position: relative; top: -1px; width: 97%; float: left;">${titleWnd}</div>
                            <b style="position: relative; top: 0px; left: -5px; float: right; width: 2%; border-left: 1px solid;  cursor: pointer;" title="${titleClose}" onclick="xWindow({id:'${id}'})">&nbsp;x</b>
                        </td>
                    </tr>
                </table>        
            </div>
            <div id="xWnd_contenido" style="border: 0px solid; position:absolute; left: 0px; top: ${hBarraT}vh; width: 100%; height: ${iHeight/10}vh;">
                ${sContenido}
            </div>
            <input type="button" id="code_${id}" onclick="${code}" style="display:none"/>
            <input type="hidden" id="file_${id}" value="${f}"/>
        </div>`;
         let element = tmp.firstChild;
         window.keyboard.detach();
         document.body.appendChild(element);
         return "code_" + id;
    }
    else
   {
     if(document.getElementById(obj.id))
     {
        //se borra seccionpara variables globales dentro de la ventana
        delete window[obj.id];

        let modalElement = document.getElementById(obj.id);

        /////////////////////////////Para la seleccion con las esquinas recortadas////////////////////////////////
        if(modalElement.getAttribute('augmented-ui') != null){
            if(!modalElement.getAttribute('augmented-ui').trim().endsWith("exe")){
                modalElement.setAttribute('augmented-ui', modalElement.getAttribute('augmented-ui').trim() + " exe tmp");
            }
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////////////

        modalElement.setAttribute("class", "custom blink appXwnd");
        window.audioManager.denied.play();
        setTimeout(() => {
                    modalElement.remove();
                    if(document.querySelectorAll(".appXwnd").length == 0 && document.querySelectorAll("#settingsEditor").length == 0)
                    {
                        window.keyboard.attach();
                        window.term[window.currentTerm].term.focus();
                    }
                }, 100);
     }
     else
     {
        //se define seccionpara variables globales dentro de la ventana
        window[obj.id] = {};

          if(!audioOff)  
            window.audioManager.info.play(); //style="position:relative; top: 2px;"
          let titleClose = (!obj.titleClose)?"Close":obj.titleClose;
          let titleWnd = (!obj.title)?"Window":obj.title;
          let id = obj.id;
          let pX = (!obj.x)?0:obj.x;
          let pY = (!obj.y)?0:obj.y;
          let numPX = document.querySelectorAll(".appXwnd").length;
          let zIndex = (!obj.zIndex)?((numPX != 0)?document.querySelectorAll(".appXwnd")[numPX - 1].style.zIndex + 1 :400):obj.zIndex;
          let code = (!obj.code)?'':obj.code;
          let noLimit = (!obj.noLimit)?'':((obj.noLimit > 0)?'noLimit':''); 
          let agExec = (!obj.noLimit)?'exe':(obj.noLimit < 1)?'exe':''; 
          let bgFnd = (!obj.noLimit)?'bgFnd':(obj.noLimit < 1)?'bgFnd':'';  

          let tmp = document.createElement("div");
          recClient = document.body.getBoundingClientRect();
          let iHeight = (!obj.h)?parseInt(recClient.height) - 35:obj.h;
          let iWidth = (!obj.w)?parseInt(recClient.width) - 37:obj.w;
//////////////////////////////se agrega para centrado automatico
          if(!obj.x){
            pX = ((parseInt(recClient.width)/2) - (iWidth/2));
          }
          
          if(!obj.y){
            pY = ((parseInt(recClient.height)/2) - (iHeight + 9));
          }
////////////////////////////////////////////////////////////////////
          iHeight = (pY + iHeight > (parseInt(recClient.height) - 35))?iHeight - pY:iHeight;
          iWidth = (pX + iWidth > (parseInt(recClient.width) - 37))?iWidth - pX:iWidth;
          let wndHidden = (!obj.hidden)?'':(obj.hidden == 'true')?'display:none':'';
          let sContenido = (!obj.content)?"":obj.content;
          let hBarraT = 2;
          tmp.innerHTML = `<div id="${id}" class="info custom focus appXwnd ${noLimit} ${bgFnd}" augmented-ui="tl-clip br-clip ${agExec}" style="z-index:${zIndex}; left: ${pX}px; top: ${pY}px; width: ${iWidth/10}vh; height: ${iHeight/10}vh; opacity: 0.8; ${wndHidden}">
                 <div ontouchstart="xwTouchStart(event,'${id}')" onmousedown="xwMouseDown(event,'${id}')"  style="border-bottom: 1px solid; position:absolute; left: 0px; top: 0px; width: 100%; height: ${hBarraT}vh;">
                    <table style="width: 100%;">
                        <tr style="height: ${hBarraT}vh;">
                            <td>
                                <div style="position: relative; top: -1px; width: 97%; float: left;">${titleWnd}</div>
                                <b style="position: relative; top: -1px; left: -5px; float: right; width: 2%; border-left: 1px solid;  cursor: pointer;" title="${titleClose}" onclick="xWindow({id:'${id}'})">&nbsp;x</b>
                            </td>
                        </tr>
                    </table>        
                </div>
                <div id="xWnd_contenido" style="border: 0px solid; position:absolute; left: 0px; top: ${hBarraT}vh; width: 100%; height: ${iHeight/10}vh;">
                    ${sContenido}
                </div>
                <input type="button" id="code_${id}" onclick="${code}" style="display:none"/>
                <input type="hidden" id="file_${id}" value="${f}"/>
            </div>`;
         let element = tmp.firstChild;
         window.keyboard.detach();
         document.body.appendChild(element);
         return "code_" + id;
     }
   }  
};

function appXwnd(f)
{
    const path = require("path");
    
    f = f.replace("[~]",electron.remote.app.getPath("userData"));
    f = f.replace("~",electron.remote.app.getPath("home"));

    if(f == "") return false;

////////////////////////////Para ejecutar con titulo////////////////////////////////////////
    if(window.cApps.xobjFile.indexOf(f) == -1)
    {
        let fIndex = window.cApps.xobjTitle.indexOf(f);
        f = (fIndex != -1)? window.cApps.xobjFile[fIndex]: f;
    }
////////////////////////////////////////////////////////////////////////////////////////////


    if(!f.toLowerCase().endsWith('.xobj'))
        f += '.xobj';

    let file = f;
    //let fs = require('fs');

    if(!fs.existsSync(file)) 
        {
            file = path.join(document.getElementById("fs_disp_title_dir").innerHTML,f);
            if(!fs.existsSync(file)) 
            {
                file = path.join(electron.remote.app.getPath("home"), "modulos/" + f); 
                if(!fs.existsSync(file)) 
                {
                    new Modal({
                                type: "error",
                                title: `Error ${f}`,
                                message: "File not Found"
                            });
                    return false;
                }
            }
        }
   
    let rawdata = fs.readFileSync(file,"utf8");
    let wnd = JSON.parse(rawdata.replace(/\r?\n|\r/g,''));
    let code = '';
    if(!wnd.id)
        code = xWindow(wnd,file);
    else
        if(!document.getElementById(wnd.id))
            code = xWindow(wnd,file);
    code = document.getElementById(code);
    if(!code)
        return true;
    code.click();
    return true;
    //let cons_tructor = "constructor";
    //let code = 'return ' + 'alert("'+file+'");'
    //cons_tructor[cons_tructor][cons_tructor](code)();
}

//Modificar para las app en terminal y c2m tambien
function xWndExec(id,app)
{
    if(!document.getElementById(id))
    {
      return false;
    }
    else
    {
        if(app.endsWith("::")){ //para la nueva forma de ejecutar en terminal directamente
            app = app.replace("::",'');
            //validar aqui
            let which = require("child_process").execSync("which " + app.split(' ')[0].replace(/(\r\n|\n|\r)/gm, "") + ' | wc -l').toString();
            if(parseInt(which) == 0){
                desinstalarModulo(id,document.getElementById("file_" + id).value);
            }else{
                xExecInTrm(app);
                xWindow({"id":id});
            }
        }else{

            if(!app.startsWith("sudo ")){
                //validar aqui
                let which = require("child_process").execSync("which " + app.split(' ')[0].replace(/(\r\n|\n|\r)/gm, "") + ' | wc -l').toString();               
                if(parseInt(which) == 0){
                    desinstalarModulo(id,document.getElementById("file_" + id).value);
                }else{
                   const { exec } = require("child_process");
                   exec(app, (error, stdout, stderr) => {
                        if (error) {
                        	if(error.message.length > 100 || error.message.includes('stderr'))
                        		errorLog(app,error.message);
                        	else
    	                        new Modal({
    	                            type: "warning",
    	                            title: `Error ${app}`,
    	                            message: error.message
    	                        });
                        }
                    });
                   xWindow({"id":id});
               }
            }else{
                app = app.replace("sudo ",'');
                //validar aqui
                let which = require("child_process").execSync("which " + app.split(' ')[0].replace(/(\r\n|\n|\r)/gm, "") + ' | wc -l').toString();
                if(parseInt(which) == 0){
                    desinstalarModulo(id,document.getElementById("file_" + id).value);
                }else{
                    xWndExecGksu(id,app);
                    xWindow({"id":id});
                }
            }

        }       
    }
}

function xWndCmd(id,app)
{
    if(!document.getElementById(id))
    {
      return false;
    }
    else
    {
       window.term[window.currentTerm].writelr(app);
       xWindow({"id":id});
    }
}

function xwMouseDown(e,wndId)
{
    e.preventDefault();
      
    let wnd = document.getElementById(wndId);

    /////////////////////////////Para la seleccion con las esquinas recortadas////////////////////////////////
    if(!wnd.getAttribute('augmented-ui').trim().endsWith("exe")){
        wnd.setAttribute('augmented-ui', wnd.getAttribute('augmented-ui').trim() + " exe tmp");
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////


    wnd.style.background = "rgba(var(--color_r), var(--color_g), var(--color_b), 0.5)";
    wnd.classList.add("selected");
    
    window.wndActive = {};
    window.wndActive.pX = e.clientX; 
    window.wndActive.pY = e.clientY;
    window.wndActive.id = wnd.id;

    window.addEventListener("mousemove", xwMouseMove);
    window.addEventListener("mouseup", xwMouseUp);
}

function xwMouseUp(e)
{

    window.removeEventListener("mousemove", xwMouseMove);
    
    if(window.wndActive.id)
     {
        let wnd = document.getElementById(window.wndActive.id);

        /////////////////////////////Para la seleccion con las esquinas recortadas////////////////////////////////
        if(wnd.getAttribute('augmented-ui').trim().endsWith("tmp")){
            wnd.setAttribute('augmented-ui', wnd.getAttribute('augmented-ui').trim().replace(" exe tmp",''));
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////////////

        wnd.style.background = "";
        wnd.classList.remove("selected");
        window.wndActive = {};

     }   
    
    window.removeEventListener("mouseup", xwMouseUp);
}

function xwMouseMove(e)
{
    e.preventDefault();
    if(window.wndActive.id)
    {

        let wnd = document.getElementById(window.wndActive.id);
       
        let posX = wnd.offsetLeft; //wnd.style.left.replace("px",'');
        let posY = wnd.offsetTop; //wnd.style.top.replace("px",'');
        posX = parseInt(posX) + (e.clientX - window.wndActive.pX);
        posY = parseInt(posY) + (e.clientY - window.wndActive.pY);
       
        window.wndActive.pX = e.clientX;
        window.wndActive.pY = e.clientY;

        if(wnd.classList.contains('selected'))
         { 
           wnd.style.left = posX + "px";
           wnd.style.top = posY + "px";
         }
     }    
}
/////////////////////////////Touch no probado/////////////////////////////////////////////// 
function xwTouchStart(e,wndId)
{
    e.preventDefault();
      
    let wnd = document.getElementById(wndId);

    /////////////////////////////Para la seleccion con las esquinas recortadas////////////////////////////////
    if(!wnd.getAttribute('augmented-ui').trim().endsWith("exe")){
        wnd.setAttribute('augmented-ui', wnd.getAttribute('augmented-ui').trim() + " exe tmp");
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    wnd.style.background = "rgba(var(--color_r), var(--color_g), var(--color_b), 0.5)";
    wnd.classList.add("selected");
    
    window.wndActive = {};
    window.wndActive.pX = e.clientX; 
    window.wndActive.pY = e.clientY;
    window.wndActive.id = wnd.id;

    window.addEventListener("touchmove", xwTouchMove);
    window.addEventListener("touchend", xwTouchEnd);
}

function xwTouchMove(e)
{
     e.preventDefault();
    if(window.wndActive.id)
    {

        let wnd = document.getElementById(window.wndActive.id);
       
        let posX = wnd.offsetLeft; //wnd.style.left.replace("px",'');
        let posY = wnd.offsetTop; //wnd.style.top.replace("px",'');
        posX = parseInt(posX) + (e.changedTouches[0].clientX - window.wndActive.pX);
        posY = parseInt(posY) + (e.changedTouches[0].clientY - window.wndActive.pY);
       
        window.wndActive.pX = e.clientX;
        window.wndActive.pY = e.clientY;

        if(wnd.classList.contains('selected'))
         { 
           wnd.style.left = posX + "px";
           wnd.style.top = posY + "px";
         }
     } 
}

function xwTouchEnd(e)
{
    window.removeEventListener("touchmove", xwTouchMove);
    
    if(window.wndActive.id)
     {
        let wnd = document.getElementById(window.wndActive.id);

        /////////////////////////////Para la seleccion con las esquinas recortadas////////////////////////////////
        if(wnd.getAttribute('augmented-ui').trim().endsWith("tmp")){
            wnd.setAttribute('augmented-ui', wnd.getAttribute('augmented-ui').trim().replace(" exe tmp",''));
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////////////

        wnd.style.background = "";
        wnd.classList.remove("selected");
        window.wndActive = {};

     }   
    
    window.removeEventListener("touchend", xwTouchEnd);
}

function xWndExecFDesktop(id,desk)
{
    if(!document.getElementById(id))
    {
      return false;
    }
    else
    {
        if (!/*require('fs')*/fs.existsSync(path.join(window["cApps"].osPathApps, desk + '.desktop'))) {
                desinstalarModulo(id,document.getElementById("file_" + id).value);
                return false;
            }
            
       const { exec } = require("child_process");
       //exec(`grep '^Exec' ${path.join(window["cApps"].osPathApps, desk + '.desktop')} | tail -1 | sed 's/^Exec=//' | sed 's/%.//' | sed 's/^"//g' | sed 's/" *$//g'`, (error, stdout, stderr) => {
       let ini = require('ini');

       if(fs.existsSync(`${path.join(window["cApps"].osPathApps, desk + '.desktop')}`)){
            
            var config = ini.parse(fs.readFileSync(`${path.join(window["cApps"].osPathApps, desk + '.desktop')}`, 'utf8'));

            if(config["Desktop Entry"].Exec)
                xWndExec(id,config["Desktop Entry"].Exec);
       }

       
                    
       /*exec(`grep '^Exec' ${path.join(window["cApps"].osPathApps, desk + '.desktop')} | head -1 | sed 's/^Exec=//' | sed 's/%.//' | sed 's/^"//g' | sed 's/" *$//g'`, (error, stdout, stderr) => {    
            if (error) {
                new Modal({
                    type: "warning",
                    title: `Error ${desk}`,
                    message: error.message
                });
            }

            if(stdout != '')
            {
                xWndExec(id,stdout);
            }
        });*/
    }
}

function createXln(appDesk)
{
    let code = {title:'',x: 0,y: 0,w: 0,h: 0,code:'',id:'',hidden: 'true'},mName;

    code.id = 'id_' + ((appDesk == '')?require("nanoid")():appDesk.replace(" ",'_'));
    if(appDesk == ''){
        if(window.settings.fileManager.endsWith("*"))
            code.code = `xWndCmd('${code.id}','${window.settings.fileManager.replace('*','') + " " + window.cApps.osPathApps}')`; 
         else    
            code.code = `xWndExec('${code.id}','${window.settings.fileManager + " " + window.cApps.osPathApps}')`; 
        mName = 'applications';
    }
    else{
        code.code = `xWndExecFDesktop('${code.id}','${appDesk}')`;
        mName = appDesk;
        
    }
    //let fs = require('fs');
    if (!fs.existsSync(require('path').join(require('electron').remote.app.getPath('home'), 'modulos/' + mName + '.xobj'))) {
        fs.writeFileSync(require('path').join(require('electron').remote.app.getPath('home'), 'modulos/' + mName + '.xobj'), JSON.stringify(code));

        new Modal({
                    type: "info",
                    title: `Susseful`,
                    message: "Module " + mName +" create in " + require('path').join(require('electron').remote.app.getPath('home'), 'modulos')
                });
     }else{
        new Modal({
                    type: "info",
                    title: `Exist`,
                    message: "The module " + mName +" exist in " + require('path').join(require('electron').remote.app.getPath('home'), 'modulos') + " if necessary remove this file."
                });
     }   
}

function loadWM(cBorder,cBack, wndPanel)
{
    const { exec,spawn, execSync } = require("child_process");
    var onError = 0;
    let wm = 'pwd';
    //exec(wm, (e, sout, serr) => {
        wm = /*`export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${sout} ` +*/ path.join(electron.remote.app.getPath("userData"), "wm");// + ` ${parseInt(cBorder,16)} ${parseInt(cBack,16)}`;
    let hWm = '';
    window.wm = true;

    if(!wndPanel){
        window['wndPanel'] = {};
        window['wndPanel'].cBorder = cBorder;
        window['wndPanel'].cBack = cBack;
        window['wndPanel'].wndPanel = 0;
        hWm = spawn(wm,[electron.remote.screen.getPrimaryDisplay().bounds.width,parseInt(cBorder,16),parseInt(cBack,16)]);
    }    
    else
       hWm = spawn(wm,[electron.remote.screen.getPrimaryDisplay().bounds.width,parseInt(cBorder,16),parseInt(cBack,16),wndPanel]); 


    hWm.stderr.on('data', (data)=>{

        
        if(data.includes("<--wndPanel-->")){

            if(window['wndPanel'].wndPanel == 0){

                exec("xsetroot -cursor_name left_ptr", (error, stdout, stderr) => {});

                let demonio = spawn(startUp.startApp[0]);// ejecutamos el demonio de mate este debe ser primero que power manager
                demonio.stderr.on('data',(data)=>{
                    if(!window.demonioMate){
                       exec(startUp.startApp[1], (error, stdout, stderr) => {}); // ejecutamos el administrador de corriente sin este el boton poweroff apaga el equipo. 
                       window.demonioMate = true;
                    }
                });

                for(let i=2; i < startUp.startApp.length; i++)
                { //primero cerramos la aplicacion si esta en ejecucion y posteriormente ejecutamos de nuevo.
                    //exec("kill -9 $(ps aux | grep " + startUp.startApp[i] + " | awk '{print $2}')", (error, stdout, stderr) => {
                    //exec("ps aux | grep " + startUp.startApp[i] + " | wc -l", (error, stdout, stderr) => {   
                    //exec("ps aux | grep " + startUp.startApp[i], (error, stdout, stderr) => {  

                   //exec('ps ax | grep -e "sh -c ' + startUp.startApp[i] + '" | grep -v grep | wc -l', (error, stdout, stderr) => {
                     exec("kill -9 $(ps ax | grep -e " + startUp.startApp[i] + " | grep -v grep | awk '{print $1}')", (error, stdout, stderr) => {
                	// alert(stdout);
		            //if(parseInt(stdout) == 0 ) // se valida que no se este ejecutando ya las aplicacion.
                         exec(startUp.startApp[i], (error, stdout, stderr) => {});  
                    });
                }

                window['wndPanel'].wndPanel = data.toString('utf8').split("<--wndPanel-->")[1];

                //User init theme and more
                let user = require(path.join(settingsDir, "user.json"));

                if(user.init)
                    exec("dconf " + user.dconf, (error, stdout, stderr) => {});
            }
        }else{

            if(data.includes("Detected another window manager on display")){
                new Modal({
                    type: "info",
                    title: 'Problem of external module',
                    message: data
                });
                 onError = -1;
                 window.wm = false;
                 //document.getElementById("lbl_ona").style.display = "none";
                 
            }else{

                new Modal({
                    type: "info",
                    title: 'Problem of external module',
                    message: data
                });
            }
            
        }
    });

    hWm.on('exit', code => {
        if(onError != -1)
            loadWM(window['wndPanel'].cBorder,window['wndPanel'].cBack, window['wndPanel'].wndPanel);
    });
        /*new Modal({
                    type: "info",
                    title: 'Problem of external module',
                    message: "Se ejecuto: " + wm
                });*/
        
        /*exec(wm, (error, stdout, stderr) => {
        new Modal({
                    type: "info",
                    title: 'Problem of external module',
                    message: stderr
                });
         });*/
    //});   
}

function completListCbx(obj)
{

    //if( window.cApps.id != txt_id)
    //{
        //window.cApps.id = txt_id;
        if(!obj.id) obj.id = '<null>';
        if(!obj.wnd_id) obj.wnd_id = '<null>';
        if(!obj.var_list) obj.var_list = '<null>';
        if(!obj.cbxAct) obj.cbxAct = '<null>';
        
        autocompleteAppsInit(document.getElementById(obj.id),obj.wnd_id,obj.var_list,obj.cbxAct);
   // }

    document.getElementById(obj.id).focus(); 
}

function autocompleteAppsInit(inp,wnd_id,var_list,cbxAct) {
  /*the autocomplete function takes two arguments,
  the text field element and an array of possible autocompleted values:*/
  var currentFocus, tamanioLista = 400;//352;
  /*execute a function when someone writes in the text field:*/
  inp.addEventListener("input", function(e) {

///////////////////////Se valida para no tapar la lista 
    if((parseInt((document.getElementById(wnd_id).style.top).replace('px','')) + tamanioLista) >=  (document.body.clientHeight - 103))
          document.getElementById(wnd_id).style.top = ((document.body.clientHeight - 120) - tamanioLista ) + 'px';
////////////////////////////////////////////////////////

      var a, b, i, val = this.value;
      /*close any already open lists of autocompleted values*/
      closeAllLists();
      if (!val) { return false;}
      
      if(!(!document.getElementById(this.id + "autocomplete-list"))) document.getElementById(this.id + "autocomplete-list").remove(); //se agrega para evariar que no se cierre

      currentFocus = -1;
      /*create a DIV element that will contain the items (values):*/
      a = document.createElement("DIV");
      a.setAttribute("id", this.id + "autocomplete-list");
      a.setAttribute("class", "autocomplete-items class_" + this.id);

      /*if(cbxAct == '<null>')
         a.style.borderBottom = 'none';*/
      /*append the DIV element as a child of the autocomplete container:*/
      this.parentNode.appendChild(a);
////////////////////////////////////Unims los dos arreglos para buscar por titulo o archivo//////////////////////////////////
      arr = (!window[wnd_id][var_list])?[]:window[wnd_id][var_list]; //window.cApps.xobjTitle;
      
      //Se comenta para hacer la busqueda por titulo nada mas si es necesario descomentar y/o modificar
      /*arr = arr.concat(window.cApps.xobjFile);

      for(var i=0; i<arr.length; ++i) {
        for(var j=i+1; j<arr.length; ++j) {
            if(arr[i] === arr[j])
                arr.splice(j--, 1);
        }
    }*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////      
      /*for each item in the array...*/
      for (i = 0; i < arr.length; i++) {
        /*check if the item starts with the same letters as the text field value:*/
        if (arr[i].substr(0, val.length).toUpperCase() == val.toUpperCase()) {
          /*create a DIV element for each matching element:*/
          b = document.createElement("DIV");
          /*make the matching letters bold:*/
          b.innerHTML = "<strong>" + arr[i].substr(0, val.length) + "</strong>";
          b.innerHTML += arr[i].substr(val.length);
          /*insert a input field that will hold the current array item's value:*/
          b.innerHTML += "<input type='hidden' value='" + arr[i] + "'>"; //para poner de valor archivo xobjFile
          /*execute a function when someone clicks on the item value (DIV element):*/
          b.addEventListener("click", function(e) {
              /*insert the value for the autocomplete text field:*/
              inp.value = this.getElementsByTagName("input")[0].value;
              /*close the list of autocompleted values,
              (or any other open lists of autocompleted values:*/
              closeAllLists();
          });
          a.appendChild(b);
          //console.log(a.height);
        }
      }
  });

if(cbxAct != '<null>'){

    inp.style.cursor = "pointer";
    let ph = inp.getAttribute('placeholder');
    if(ph === null)
        inp.setAttribute('placeholder','Select Item');

    inp.addEventListener("click", function(e) { //se agrega para que carge todo al click como combobox

///////////////////////Se valida para no tapar la lista 
    if((parseInt((document.getElementById(wnd_id).style.top).replace('px','')) + tamanioLista) >=  (document.body.clientHeight - 103))
          document.getElementById(wnd_id).style.top = ((document.body.clientHeight - 120) - tamanioLista ) + 'px';
////////////////////////////////////////////////////////


      var a, b, i;//, val = this.value;
      /*close any already open lists of autocompleted values*/
      closeAllLists();
     //if (!val) { return false;}
      if(!(!document.getElementById(this.id + "autocomplete-list"))) document.getElementById(this.id + "autocomplete-list").remove(); //se agrega para evariar que no se cierre

      currentFocus = -1;
      /*create a DIV element that will contain the items (values):*/
      a = document.createElement("DIV");
      a.setAttribute("id", this.id + "autocomplete-list");
      a.setAttribute("class", "autocomplete-items class_" + this.id);

      /*append the DIV element as a child of the autocomplete container:*/
      this.parentNode.appendChild(a);
////////////////////////////////////Unims los dos arreglos para buscar por titulo o archivo//////////////////////////////////
      arr = (!window[wnd_id][var_list])?[]:window[wnd_id][var_list]; //

      //Se comenta para hacer la busqueda por titulo nada mas si es necesario descomentar y/o modificar
      /*arr = arr.concat(window.cApps.xobjFile);

      for(var i=0; i<arr.length; ++i) {
        for(var j=i+1; j<arr.length; ++j) {
            if(arr[i] === arr[j])
                arr.splice(j--, 1);
        }
    }*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////   
      /*for each item in the array...*/
      for (i = 0; i < arr.length; i++) {
        /*check if the item starts with the same letters as the text field value:*/
       // if (arr[i].substr(0, val.length).toUpperCase() == val.toUpperCase()) {
          /*create a DIV element for each matching element:*/
          b = document.createElement("DIV");
          /*make the matching letters bold:*/
          b.innerHTML = ""; //"<strong>" + arr[i].substr(0, val.length) + "</strong>";
          b.innerHTML += arr[i];//.substr(val.length);
          /*insert a input field that will hold the current array item's value:*/
          b.innerHTML += "<input type='hidden' value='" + arr[i] + "'>";
          /*execute a function when someone clicks on the item value (DIV element):*/
              b.addEventListener("click", function(e) {
              /*insert the value for the autocomplete text field:*/
              inp.value = this.getElementsByTagName("input")[0].value;
              /*close the list of autocompleted values,
              (or any other open lists of autocompleted values:*/
              closeAllLists();
          });
          a.appendChild(b);
          
       // }
      }
  });

}

  /*execute a function presses a key on the keyboard:*/
  inp.addEventListener("keydown", function(e) {
    //document.getElementById('id_selectDesktopautocomplete-list').scrollBy(0,70);
      var x = document.getElementById(this.id + "autocomplete-list");
      if (x) x = x.getElementsByTagName("div");
      if (e.keyCode == 40) {
        /*If the arrow DOWN key is pressed,
        increase the currentFocus variable:*/
        currentFocus++;
        /*and and make the current item more visible:*/
        addActive(x);

      } else if (e.keyCode == 38) { //up
        /*If the arrow UP key is pressed,
        decrease the currentFocus variable:*/
        currentFocus--;
        /*and and make the current item more visible:*/
        addActive(x);
      } else if (e.keyCode == 13) {
        /*If the ENTER key is pressed, prevent the form from being submitted,*/
        e.preventDefault();
        if (currentFocus > -1) {
          /*and simulate a click on the "active" item:*/
          if (x) x[currentFocus].click();
        }
      }
    
  });

  function addActive(x) {
    /*a function to classify an item as "active":*/
    if (!x) return false;
    /*start by removing the "active" class on all items:*/
    removeActive(x);
    if (currentFocus >= x.length) currentFocus = 0;
    if (currentFocus < 0) currentFocus = (x.length - 1);
    /*add class "autocomplete-active":*/
    x[currentFocus].classList.add("autocomplete-active");

    x[currentFocus].scrollIntoView(false); // se agrega para que el scroll este en el elemento seleccionado
       
  }
  function removeActive(x) {
    /*a function to remove the "active" class from all autocomplete items:*/
    for (var i = 0; i < x.length; i++) {
      x[i].classList.remove("autocomplete-active");
    }
  }
  function closeAllLists(elmnt) {
    /*close all autocomplete lists in the document,
    except the one passed as an argument:*/
    var x = document.getElementsByClassName("class_" + inp.id);//"autocomplete-items");
    for (var i = 0; i < x.length; i++) {
      if (elmnt != x[i] && elmnt != inp && elmnt) {
        //x[i].parentNode.removeChild(x[i]);
            if(!elmnt.id){
                x[i].parentNode.removeChild(x[i]);
            }else{
                if(elmnt.id != inp.id)
                    x[i].parentNode.removeChild(x[i]);
            }
      }
    }
  }
  /*execute a function when someone clicks in the document:*/
  document.addEventListener("click", function (e) {

         closeAllLists(e.target);

  });
}

function changeTitIcnMnu(app,title,icon){
    ////////////////////////////Para ejecutar con titulo////////////////////////////////////////
        if(window.cApps.xobjFile.indexOf(app) == -1)
        {
            let fIndex = window.cApps.xobjTitle.indexOf(app);
            app = (fIndex != -1)? window.cApps.xobjFile[fIndex]: '-1';
        }

        if(app != '-1')
        {
            if(!window.xobjDB[app])
            {
                new Modal({
                    type: "warning",
                    title: `Error X(`,
                    message: "Application not Found"
                }); 
                return false;

            }else{
                let icono = app;
                if(icono.startsWith("inpanel"))
                    icono = icono.replace(icono.split('-')[0] + '-',"");

                icono = icono.replace(window.entorno + '-','').split('-')[0];
                icono = icono.toLowerCase().split('_')[0];
                icono = icono.toLowerCase().split(' ')[0];

                window.xobjDB[app].title = (document.getElementById(title).value.trim() != '')? document.getElementById(title).value: getTitleAppsDesktop(app + '.desktop');
                window.xobjDB[app].icon = (document.getElementById(icon).value.trim() != '')? document.getElementById(icon).value: icono;

                fs.writeFileSync(path.join(require("electron").remote.app.getPath("userData"), "xobjDB.json"), JSON.stringify(window.xobjDB, 4));
                window.fsDisp.readFS(path.join(require("electron").remote.app.getPath("home"),"modulos"));
                return true;
                
            }
        }else{

            new Modal({
                type: "warning",
                title: `Error X(`,
                message: "Application not Found"
            });

            return false;
        }
    ////////////////////////////////////////////////////////////////////////////////////////////
}

function loadCnfgmnu(app,title,icon)
{
    ////////////////////////////Para ejecutar con titulo////////////////////////////////////////
        if(window.cApps.xobjFile.indexOf(app) == -1)
        {
            let fIndex = window.cApps.xobjTitle.indexOf(app);
            app = (fIndex != -1)? window.cApps.xobjFile[fIndex]: '-1';
        }

        if(app != '-1')
        {
            if(!window.xobjDB[app])
            {
                new Modal({
                    type: "warning",
                    title: `Error X(`,
                    message: "Application not Found"
                });
            }else{
                document.getElementById(title).value = window.xobjDB[app].title;
                document.getElementById(icon).value = window.xobjDB[app].icon;
            }
        }else{

            new Modal({
                type: "warning",
                title: `Error X(`,
                message: "Application not Found"
            });
        }
    ////////////////////////////////////////////////////////////////////////////////////////////
}

function getTitleAppsDesktop(app){
    let retorno = '';
    if(app.toLowerCase().startsWith("inpanel")){
        let inpanelName = app.split('-')[0];
        app = app.replace(inpanelName + '-',"");
    }
    try {
             
      //retorno = require("child_process").execSync(`grep '^Name=' ${path.join(window["cApps"].osPathApps, app)} | tail -1 | sed 's/^Name=//' | sed 's/%.//' | sed 's/^"//g' | sed 's/" *$//g'`).toString().replace("\n",'');
      let ini = require('ini');

      if(fs.existsSync(`${path.join(window["cApps"].osPathApps, app)}`)){

        var config = ini.parse(fs.readFileSync(`${path.join(window["cApps"].osPathApps, app)}`, 'utf8'));

        if(config["Desktop Entry"].Name)
            retorno = config["Desktop Entry"].Name; //require("child_process").execSync(`grep '^Name='  | head -1 | sed 's/^Name=//' | sed 's/%.//' | sed 's/^"//g' | sed 's/" *$//g'`).toString().replace("\n",'');
        else
            retorno = ''; 
      }
    }catch(err) { // manejar variable error para cuando el directorio no tenga elementos
        new Modal({
                type: "warning",
                title: `Error X(`,
                message: err
            });
    }

    return (retorno != '')? retorno:app.replace('.desktop', '').replace(/-/g,' ').replace(/_/g,' ').trim().toLowerCase().replace(/\w\S*/g, (w) => (w.replace(/^\w/, (c) => c.toUpperCase())));
}

function closeModal(){

    if(document.querySelector(".modal_popup.focus") != null)
        window.modals[document.querySelector(".modal_popup.focus").id.replace("modal_",'')].close();
    else
        if(document.querySelector(".modal_popup") != null)
             window.modals[document.querySelector(".modal_popup").id.replace("modal_",'')].close();
}

function xWndExecSudo(id,app,pass)
{
   if(!document.getElementById(id))
    {
      return false;
    }
    else
    {
       const comando = `echo "${pass}" | sudo -S -k ${app}`;  
       const { exec } = require("child_process");
       exec(comando, (error, stdout, stderr) => {
            if (error) {

                if(stderr.toLowerCase().indexOf("sorry, try again") != -1){

                    new Modal({
                        type: "warning",
                        title: `Error ${app}`,
                        message: "Incorrect password attempts"
                    });

                }else{
                    if(!id.startsWith("wnd_rootar_id_time-admin-")){
                        new Modal({
                            type: "warning",
                            title: `Error ${app}`,
                            message: stderr.replace(/sudo/g,'')
                       });
                    }
                }

            }
        });
       xWindow({"id":id});
    }
}

function xWndExecGksu(id,app){
  
  recClient = document.body.getBoundingClientRect();

  let wnd = {
  title:"Root Authorization Required",
  /*x: 270,
  y: 200,*/
  x: ((parseInt(recClient.width)/2) - 250),
  y: ((parseInt(recClient.height)/2) - 109),
  w: 490,
  h: 150,
  id: "wnd_rootar_"+id,
  content:`<table style='width: 100%; height: 100%;'>
  <tr>
   <td colspan='2'>
     I need root's password
   </td>
  </tr>
  <tr>
    <td colspan='2'>
      <label style='position: relative; left: 5px; top: 3px; float: left;'>
        Password:
      </label>
      <div style='width: 75%; float:left; position: relative; left: 23px;'>
        <div>
          <input type='password'id='id_pasroot_${id}' onkeyup="if(event.keyCode == 13){xWndExecSudo('wnd_rootar_${id}','${app}',document.getElementById('id_pasroot_${id}').value); xWindow({ id:'wnd_rootar_${id}'});}"/>
        </div>
      </div>  
    </td>
  </tr>
  <tr>
     <td>
       <button onClick="xWndExecSudo('wnd_rootar_${id}','${app}',document.getElementById('id_pasroot_${id}').value); xWindow({ id:'wnd_rootar_${id}'});" style='position: relative; left: 0px; top: 0px;'>
         Accept
       </button>
     </td>
     <td>
       <button onClick='xWindow({ id:"wnd_rootar_${id}"})' style='position: relative; left: 25%; top: 0px;'>
         Cancel
       </button>
      </td>
   </tr>
 </table>`,
  code:`document.getElementById('id_pasroot_${id}').focus();`,
  noLimit: 0
};
    let code = '';
    if(!wnd.id)
        code = xWindow(wnd);
    else
        if(!document.getElementById(wnd.id))
            code = xWindow(wnd);
    code = document.getElementById(code);
    if(!code)
        return true;
    code.click();
    return true;
}

//echo '{"message":{"call":"MsgBox","title":"Titulo","text":"Holiss"}}' > ~/.containerrcm/.rcmC2m.rcmSolar

async function showTogglePanel(show){
    return false;
    if(!show){ //si es falso o nula interactuar
        if(window.settings.showPanel){
            document.getElementById("mod_column_right").setAttribute("style", "display: none;");
            document.getElementById("main_panel").setAttribute("style", "");
            document.getElementById("main_shell").setAttribute("style", "left: 8%; width: 82%;");

        }else{
            //document.getElementById("main_panel").setAttribute("style", "width: 83%;"); //este reduce la barra principal.
            document.getElementById("main_shell").setAttribute("style", "left: 0%; width: 65%;");
            await _delay(500);
            document.getElementById("mod_column_right").setAttribute("style", "");
        }
        window.settings.showPanel = !window.settings.showPanel;
        Object.keys(window.settings).forEach(key => {
            if (window.settings[key] === "undefined") {
                delete window.settings[key];
            }
        });

        fs.writeFileSync(settingsFile, JSON.stringify(window.settings, "", 4));
    }else{
        //if(!window.settings.showPanel){ //de inicio siempre estara cerrado el panel
            window.settings.showPanel = false;
            document.getElementById("mod_column_right").setAttribute("style", "display: none;");
            document.getElementById("main_panel").setAttribute("style", "");
            document.getElementById("main_shell").setAttribute("style", "left: 8%; width: 82%;");

        /*}else{
            document.getElementById("main_panel").setAttribute("style", "");
            document.getElementById("main_shell").setAttribute("style", "");
            document.getElementById("mod_column_right").setAttribute("style", "");
        }*/
    }

    
  /*if(!show){
        if(show !== false)
            return (document.querySelector("#mod_netstat_innercontainer > div:nth-child(2)").id == "true")?true:false;
        else
          document.querySelector("#mod_netstat_innercontainer > div:nth-child(2)").id = "false";  
  }else
        document.querySelector("#mod_netstat_innercontainer > div:nth-child(2)").id = "true";

  window.settings.showIP = netShowIP();

    Object.keys(window.settings).forEach(key => {
        if (window.settings[key] === "undefined") {
            delete window.settings[key];
        }
    });

    fs.writeFileSync(settingsFile, JSON.stringify(window.settings, "", 4));*/
}

function systemPoweroff(){
  recClient = document.body.getBoundingClientRect();
  mostrarPanel();

  let wnd = {
  title:"Power Off",
  x: ((parseInt(recClient.width)/2) - 250),
  y: ((parseInt(recClient.height)/2) - 109),
  w: 450,
  h: 100,
  id: "wnd_powewroff",
  content:`<table style='width: 100%; height: 100%;'>
  <tr>
    <td>
    </td>
  </tr>
  <tr>
    <td>
        <button id='btn_pwof' onClick='xWndExec("wnd_powewroff","systemctl poweroff")' onkeyup='poweroffKeyup(this,event)' style='position: relative; left: 5px; top: -15px;'>Poweroff</button>
    </td>
    <td>
        <button id='btn_ret' onClick='xWndExec("wnd_powewroff","systemctl reboot")' onkeyup='poweroffKeyup(this,event)' style='position: relative; left: 2.5vh; top: -15px;'>Reboot</button>
    </td>
    <td>
        <button id='btn_cle' onClick='electron.remote.app.exit(0);' onkeyup='poweroffKeyup(this,event)' style='position: relative; left: 2vh; top: -15px;'>Exit Session</button>
    </td>
  </tr></table>`,
  code:`document.getElementById('btn_pwof').focus();`,
  noLimit: 0
};
    let code = '';
    //console.log(wnd);
    if(!wnd.id){
        code = xWindow(wnd);
        //console.log(code);
    }
    else
        if(!document.getElementById(wnd.id))
            code = xWindow(wnd);
    code = document.getElementById(code);
    if(!code)
        return true;
    code.click();
    return true;
}

function poweroffKeyup(elemento, ev){

    switch(ev.keyCode){
        case 37: if(elemento) if(elemento.id == 'btn_pwof') document.getElementById('btn_cle').focus(); if(elemento.id == 'btn_cle') document.getElementById('btn_ret').focus(); if(elemento.id == 'btn_ret') document.getElementById('btn_pwof').focus(); break;
        case 39: if(elemento) if(elemento.id == 'btn_pwof') document.getElementById('btn_ret').focus(); if(elemento.id == 'btn_ret') document.getElementById('btn_cle').focus(); if(elemento.id == 'btn_cle') document.getElementById('btn_pwof').focus(); break;
        case 27: xWindow({id:'wnd_powewroff'}); break;
    }
}

function setAmPm(){
    window.settings.ampm = (!window.settings.ampm);
}

function escToClose(idW, ev){
    if(idW){
        switch(ev.keyCode){
           case 27: xWindow({id:idW}); break;
        }
    }    
}

new restCommMesg("rcmSolar",(data)=>{
    if(data.message.call){

        if(data.message.call.toLowerCase() == 'msgbox'){
            data.message.title = (!data.message.title)?"":data.message.title;
            data.message.text = (!data.message.text)?"":data.message.text;
            new Modal({
                        type: "warning",
                        title: data.message.title,
                        message: data.message.text
                   });            
        }

        if(data.message.call.toLowerCase() == 'changeimage'){
            if(!data.message.img){
                document.body.style.setProperty('background-size','');
                document.body.style.setProperty('background-image',"");
                //document.body.style.setProperty('--color_light_black',window.theme.terminal.background);  
                window.termThemeTmp.background = window.theme.terminal.backgroundtmp;
                Object.keys(window.term).forEach(key => {
                    window.term[key].term.setOption("theme",window.termThemeTmp);
                });
                window.setBGI.transparency = false;
                
            }else{
                data.message.img = (data.message.img != '')? "url(" + data.message.img + ")":"";
                document.body.style.setProperty('background-size','cover');
                document.body.style.setProperty('background-image',data.message.img);

                if(data.message.transparency){
                    window.termThemeTmp.background = window.setColorT;
                    Object.keys(window.term).forEach((key,inx) => {
                        window.term[key].term.setOption("allowTransparency",data.message.transparency);
                        window.term[key].term.setOption("theme",window.termThemeTmp);
                    });
                    window.setBGI.transparency = true;
                    
                }else{
                    window.termThemeTmp.background = window.theme.terminal.backgroundtmp;
                    Object.keys(window.term).forEach((key,inx) => {
                        window.term[key].term.setOption("theme",window.termThemeTmp);
                    });
                    window.setBGI.transparency = false;
                }                
            }
            window.setBGI.change = true;
        }

        if(data.message.call.toLowerCase() == 'appxwnd'){
            if(data.message.file){
                appXwnd(data.message.file);
            }
        }

        if(data.message.call.toLowerCase() == 'nnativeapps'){
            if(!data.message.number) data.message.number = 0;
            if(data.message.window){
                if(window.settings.autoClosePanel)
                    showTogglePanel(true);
                //let xapp = '';
                let xappBarr = '';
                window.backWnd = '0';
                document.querySelector("#id_closeXWindPanel").style.visibility = 'hidden';
                if(document.querySelector("#mod_sysinfo > div:nth-child(3) > h2"))
                    document.querySelector("#mod_sysinfo > div:nth-child(3) > h2").innerHTML = data.message.window.length;
                if(data.message.window.length > 0){
                    //let wndBack;
                    let icons = require("./assets/icons/file-icons.json");
                    let iconext = require(path.join(electron.remote.app.getPath("userData"),"iconext.json"));
                    let fileIconsMatcher = require("./assets/misc/file-icons-match.js");
                    let icon = "";
                    let iconname ='';
                    let wndBackOpaco = '';
                    //let showCloseWindow = false;
                    Object.assign(icons,iconext);

                    for (let i = 0; i < data.message.window.length; i++) {
                        if(data.message.number == data.message.window[i].id /*&& data.message.window.length > 1*/){
                           // wndBack = 'style="opacity: 0.5"';
                            wndBackOpaco = ' opacity: 0.5';
                            window.backWnd = data.message.number;
                            document.querySelector("#id_closeXWindPanel").style.visibility = 'visible';
                            //showCloseWindow = true;
                        }
                        else{
                            //wndBack = '';
                            wndBackOpaco = '';
                            /*if(!showCloseWindow)
                                document.querySelector("#id_closeXWindPanel").style.display = 'none';*/
                        }
                        //crear icono con data.message.window[i].class --> buscar en minusculas y si no partilo y biscar y el primero que se encuentre es el icono y si no poner un default
                        data.message.window[i].class = data.message.window[i].class.toLowerCase();
                        //console.log(data.message.window[i].class);
                        icon = icons[data.message.window[i].class];
                        if (typeof icon === "undefined") {
                            iconName = fileIconsMatcher(data.message.window[i].class);
                            icon = icons[iconName];
                            if (typeof icon === "undefined") {
                                let classNameApp = data.message.window[i].class.replace('-',' ').replace('_',' ');
                                classNameApp = classNameApp.split(' ');
                                for(let x=0; x < classNameApp.length; x++){
                                    icon = icons[classNameApp[x]];
                                    if (typeof icon === "undefined") {
                                        iconName = fileIconsMatcher(classNameApp[x]);
                                        icon = icons[iconName];
                                        if (typeof icon === "undefined") {
                                            if (x == (classNameApp.length - 1)) {
                                                icon = icons["x11-window"]; //.appXwnd;
                                            }
                                        }else
                                            break;
                                    }else
                                        break;
                                }
    
                            }
                        }
                        
                       //console.log(data.message.window[i].class); 
                       /*iconname = `<div class="task_app" style="left: -0.5vh; cursor: pointer;" onclick="goNativeWindow('${data.message.window[i].id}')" title="Go Window">
                                         <svg viewBox="0 0 ${icon.width} ${icon.height}" fill="rgb(${window.theme.r}, ${window.theme.g}, ${window.theme.b})" style="width: 100%; height: 100%;">
                                            ${icon.svg}
                                        </svg>                                    
                                      </div> `;*/

                      xappBarr += `<div class="task_app" style="left: -0.5vh; cursor: pointer;${wndBackOpaco}" onclick="goNativeWindowTask(this,'${data.message.window[i].id}')" title="${((data.message.window[i].name != '')?data.message.window[i].name:"xWindow - Untitled")}">
                                         <svg viewBox="0 0 ${icon.width} ${icon.height}" fill="rgb(${window.theme.r}, ${window.theme.g}, ${window.theme.b})" style="width: 100%; height: 100%;">
                                            ${icon.svg}
                                        </svg>                                    
                                      </div> `;                
                        

                       //xapp += '<h1 '/*+wndBack*/+'>'+iconname+'[ <b onclick="closeNativeWindow('+data.message.window[i].id+')" title="Close Window">X</b> ] <b onclick="goNativeWindow('+data.message.window[i].id+')" title="Go Window">'+((data.message.window[i].name != '')?data.message.window[i].name:"xWindow - Untitled")+'</b></h1>';
                    }
                }

                if(document.querySelector("#id_task_panel"))
                    document.querySelector("#id_task_panel").innerHTML = decodeURIComponent(escape(xappBarr));

                /*if(document.querySelector("#id_panel_xwindow")){
                    document.querySelector("#id_panel_xwindow").innerHTML = xapp;
                    
                    let h1 = document.querySelectorAll("#id_panel_xwindow > h1");
                    for(let i=0; i < h1.length; i++){
                        try {
                          h1[i].innerHTML = decodeURIComponent(escape(h1[i].innerHTML));
                        } catch (error) {
                            continue;
                        }
                    } 
                }*/
                
                //console.log(data.message.window);
            }else{
                if(document.querySelector("#mod_sysinfo > div:nth-child(3) > h2"))
                    document.querySelector("#mod_sysinfo > div:nth-child(3) > h2").innerHTML = "0";
            }
        }

        if(data.message.call.toLowerCase() == 'poweroff'){
            systemPoweroff();
        }

        if(data.message.call.toLowerCase() == 'keyslock'){
            procKeysLock(data.message);
        }

        //

        /*if(data.message.call.toLowerCase() == 'netshowip'){
            //if(data.message.value){
                netShowIP(data.message.value);
           // }
        }*/
    }

});

function procKeysLock(data){

    if(document.getElementById('num-lock') && document.getElementById('caps-lock'))
    {
        if(!data.numlock){
            //document.getElementById('num-lock').setAttribute("style","opacity: 0.5");
            document.getElementById('num-lock').style.opacity = 0.5;
        }else{
            //document.getElementById('num-lock').setAttribute("style","");
            document.getElementById('num-lock').style.opacity = 1;
        }

        if(!data.capslock){
             //document.getElementById('caps-lock').setAttribute("style","opacity: 0.5");
             document.getElementById('caps-lock').style.opacity = 0.5;
        }else{
            //document.getElementById('caps-lock').setAttribute("style","");
            document.getElementById('caps-lock').style.opacity = 1;
        }

    }
    
}

function xExecInTrm(cmd){
    let execute = false;
    if(cmd){

        if(((document.getElementById("shell_tab"+window.currentTerm).querySelectorAll("p")[0]).innerHTML).endsWith("bash")){
            window.term[window.currentTerm].writelr(cmd);
            execute =  true;
        }else{
            for (var i = 0; i < Object.keys(window.term).length; i++) {
               if(((document.getElementById("shell_tab"+i).querySelectorAll("p")[0]).innerHTML).endsWith("bash")){
                  window.focusShellTab(i,cmd);
                  execute =  true;
                  break;
               } 
            }
        }
        if(!execute && Object.keys(window.term).length < 5){
           let i = Object.keys(window.term).length;
           window.focusShellTab(i,cmd);
           execute =  true;
        }
        if(!execute){
            new Modal({
                type: "warning",
                title: `Not terminals available`,
                message: "This application is executed in terminal but there are not available."
            });
        }else{
           mostrarPanel();
        }

    }
}
function errorLog(name,message,noshwomsg){
	let now= new Date();
	let app = name;
	now = ((now.getDate() < 10)?'0'+now.getDate():now.getDate()) + '-' + ((now.getMonth() < 10)?'0'+now.getMonth():now.getMonth()) + '-' + now.getFullYear();
	name = '/tmp/' + name +'-'+ now +'.log';
	fs.writeFileSync(name, message);
    if(!noshwomsg){
        new Modal({
            type: "warning",
            title: `Error ${app}`,
            message: `Log in ${name}`
        });
        mostrarPanel();
    }
}

function powerPreferences(){
    const { exec } = require("child_process");
    let app = '';
    app = 'mate-power-preferences';
    exec(app, (error, stdout, stderr) => {
        if (error) {
            if(error.message.length > 100 || error.message.includes('stderr'))
                errorLog(app,error.message);
            else
                new Modal({
                    type: "warning",
                    title: `Error ${app}`,
                    message: error.message
                });
        }
    });
}

function timeAdmin(app){
    /*const { exec } = require("child_process");
    exec("kill -9 $(ps ax | grep -e " + app + " | grep -v grep | awk '{print $1}')", (error, stdout, stderr) => {*/
            let id = 'id_time-admin-' + require("nanoid")();
            xWndExecGksu(id,app);
            mostrarPanel();
            /*exec(app, (error, stdout, stderr) => {
            //if (error) {
                //if(error.message.length > 100 || error.message.includes('stderr'))
                    errorLog(app,error.message,true);*/
                /*else
                    new Modal({
                        type: "warning",
                        title: `Error ${app}`,
                        message: error.message
                    });*/
            //}
        //}); 
    //});
}

function goNativeWindowTask(obj,wnd){

    if(obj.style.opacity != 0.5){

        if(wnd == window.backWnd || window.backWnd == '0'){

            const { exec } = require("child_process");
            let cmd = "xdotool windowfocus " + wnd;   
            if(window.settings.autoClosePanel)
                showTogglePanel(true);
            window.audioManager.stdin.play();
         
            exec(cmd, (error, stdout, stderr) => {
                if (error) {
                        errorLog("goNativeWindowTask",error.message);
                }

            });

        }else{
            const { exec } = require("child_process");
            let cmd = "xdotool windowfocus " + window.backWnd;     
            exec(cmd, (error, stdout, stderr) => {

                cmd = "xdotool windowfocus " + wnd;   
                if(window.settings.autoClosePanel)
                    showTogglePanel(true);
                window.audioManager.stdin.play();
             
                exec(cmd, (error, stdout, stderr) => {
                    if (error) {
                            errorLog("goNativeWindowTask",error.message);
                    }

                });

                if (error) {
                        errorLog("goNativeWindowTask",error.message);
                }

            });
        }

    }else{
        obj.style.opacity = 1;
        mostrarPanel();
    }

    
    
}

function goNativeWindow(wnd){
    /*const fs = require('fs');
    const dir = require('path');
    let urlDefault = dir.join(process.env.HOME,".containerrcm");
    fs.writeFileSync(dir.join(urlDefault,".rcmSolar.wm"), wnd);*/
    const { exec } = require("child_process");
    let cmd = "xdotool windowfocus " + wnd;   
    if(window.settings.autoClosePanel)
        showTogglePanel(true);
    window.audioManager.stdin.play();
 
    exec(cmd, (error, stdout, stderr) => {
        if (error) {
            //if(error.message.length > 100 || error.message.includes('stderr'))
                errorLog("goNativeWindow",error.message);
            /*else
                new Modal({
                    type: "warning",
                    title: `Error ${"goNativeWindow"}`,
                    message: error.message
                });*/
        }

    });

}

function closeNativeWindow(wnd){
    const { exec } = require("child_process");
    //let cmd = "xdotool windowclose " + wnd;
    wnd = !wnd ? "": " windowfocus " + wnd;
    let cmd = "xdotool" + wnd + " key --clearmodifiers --delay 100 alt+F4";   
    if(window.settings.autoClosePanel) 
        showTogglePanel(true);
    window.audioManager.stdin.play();

    exec(cmd, (error, stdout, stderr) => {
        if (error) {
            //if(error.message.length > 100 || error.message.includes('stderr'))
                errorLog("closeNativeWindow",error.message);
            /*else
                new Modal({
                    type: "warning",
                    title: `Error ${"closeNativeWindow"}`,
                    message: error.message
                });*/
        }

    });

}

function showX11App(id_list){ //funcion creada para uso en aplicaciones XOBJ
    
    if(!id_list)id_list = 0;
    
    if(document.querySelectorAll('#id_panel_xwindow > h1')[id_list])
        document.querySelectorAll('#id_panel_xwindow > h1')[id_list].querySelectorAll('b')[1].click();
} 

function closeX11App(id_list){ //funcion creada para uso en aplicaciones XOBJ
    
    if(!id_list)id_list = 0;
    
    if(document.querySelectorAll('#id_panel_xwindow > h1')[id_list])
        document.querySelectorAll('#id_panel_xwindow > h1')[id_list].querySelectorAll('b')[0].click();
} 

function getTitleX11App(id_list){ //funcion creada para uso en aplicaciones XOBJ
    
    if(!id_list)id_list = 0;
    
    if(document.querySelectorAll('#id_panel_xwindow > h1')[id_list])
        return document.querySelectorAll('#id_panel_xwindow > h1')[id_list].querySelectorAll('b')[1].innerHTML;
    return "";
}

function systemAlertBatterylow(){
 
 if(window.alertLowBattery)
     return false;

    const { exec } = require("child_process");
    let cmd = "xdotool key Super+Tab";
    recClient = document.body.getBoundingClientRect();

  let wnd = {
  title:"Alert Low Battery",
  x: ((parseInt(recClient.width)/2) - 250),
  y: ((parseInt(recClient.height)/2) - 109),
  w: 500,
  h: 100,
  id: "wnd_batterylow",
  content:`<table style='width: 100%; height: 100%;'>
  <tr>
    <td>
    </td>
  </tr>
  <tr>
    <td>
        Battery is about to life, down 5%
    </td>
  </tr>
  <tr>  
    <td>
        <button id='btn_ok' onClick='xWindow({ id:"wnd_batterylow"});' onkeyup='if(event.keyCode == 27)xWindow({ id:"wnd_batterylow"});' style='position: relative; left: 21.5vh;'>OK</button>
    </td>
  </tr></table>`,
  code:`document.getElementById('btn_ok').focus();`,
  noLimit: 0
};
    let code = '';
    //console.log(wnd);
    if(!wnd.id){
        code = xWindow(wnd);
        //console.log(code);
    }
    else
        if(!document.getElementById(wnd.id))
            code = xWindow(wnd);
    code = document.getElementById(code);
    if(code)
        code.click();

    exec(cmd, (error, stdout, stderr) => {});

    window.alertLowBattery = true;

    return true;
    
}

function desinstalarModulo(id,app){
  
  recClient = document.body.getBoundingClientRect();
  mostrarPanel();
  
  xWindow({"id":id});

  let wnd = {
  title:"App Not Installed",
  /*x: 270,
  y: 200,*/
  x: ((parseInt(recClient.width)/2) - 250),
  y: ((parseInt(recClient.height)/2) - 109),
  w: 350,
  h: 100,
  id: "wnd_ani_"+id,
  content:`<table style='width: 100%; height: 100%;'>
  <tr>
   <td colspan='2'>
     You want to uninstall the module?
   </td>
  </tr>
  <tr>
     <td>
       <button id="yes_wnd_ani_${id}" onClick="borrarXobj('wnd_ani_${id}','${app}');" onkeyup='desinstalarModuloKeyup("${id}",this,event)' style='position: relative; left: 0px; top: -1vh;'>
         Yes
       </button>
     </td>
     <td>
       <button id="no_wnd_ani_${id}" onClick='xWindow({ id:"wnd_ani_${id}"});' onkeyup='desinstalarModuloKeyup("${id}",this,event)' style='position: relative; left: 25%; top: -1vh;'>
         No
       </button>
      </td>
   </tr>
 </table>`,
  code:`document.getElementById('yes_wnd_ani_${id}').focus();`,
  noLimit: 0
};
    let code = '';
    if(!wnd.id)
        code = xWindow(wnd,'',true);
    else
        if(!document.getElementById(wnd.id))
            code = xWindow(wnd,'',true);
    code = document.getElementById(code);
    if(!code)
        return true;
    code.click();
    return true;
}

function borrarXobj(id_app,app){
// continuar borrado
  let resp = /*require('fs')*/fs.unlinkSync(app);
  xWindow({ id:id_app});
}


function desinstalarModuloKeyup(id,elemento, ev){

    switch(ev.keyCode){
        case 37: 
        case 39: if(elemento) if(elemento.id == 'yes_wnd_ani_' + id) document.getElementById('no_wnd_ani_' + id).focus(); else document.getElementById('yes_wnd_ani_' + id).focus(); break;
        case 27: xWindow({id:'wnd_ani_' + id}); break;
    }
}

function execKeyLock(keyLock){
    
    const { exec } = require("child_process");
    let cmd = "xdotool key ";   

    switch(keyLock){
        case 'num': cmd += "Num_Lock";
                    break;
        case 'caps': cmd += "Caps_Lock";
                    break;
    }
    exec(cmd, (error, stdout, stderr) => {});    

}

function mostrarPanel(){ //Win+Tab
    const { exec } = require("child_process");
    let cmd = "xdotool key Super+Tab";       
    exec(cmd, (error, stdout, stderr) => {});   
}

async function leerWebApps() {
    //const fs = require('fs');
    //const os = require("os");

    let content = [];
    let webApps = [];
    try {
         
         if(fs.existsSync(electron.remote.app.getPath('home') + "/.local/share/applications")){
            content = await fs.readdirSync(electron.remote.app.getPath('home') + "/.local/share/applications");
             content = content.filter(function(value, index, arr){ 
                return ((value + '').startsWith("chrome-") && (value + '').endsWith(".desktop"));
            });
         }    

         await new Promise((resolve, reject) => {
            if (content.length === 0) resolve();
                content.forEach(async (file, i) => {
                    ini = require('ini');

                    if(fs.existsSync(electron.remote.app.getPath('home') + "/.local/share/applications/" + file)){

                        var config = ini.parse(fs.readFileSync(/*os.homedir()*/electron.remote.app.getPath('home') + "/.local/share/applications/" + file, 'utf8'));
                        if(config["Desktop Entry"].Name)
                            webApps[config["Desktop Entry"].Name] = {cmd:config["Desktop Entry"].Exec, class:((config["Desktop Entry"].StartupWMClass)?config["Desktop Entry"].StartupWMClass:"")};
                    }               
                });
              resolve();  
         });       
         

    }catch(err) { // manejar variable error para cuando el directorio no tenga elementos
        console.warn(err);
        new Modal({
            type: "warning",
            title: `Error leerWebApps`,
            message: err.message
        });
    }

    return webApps;

}

async function getNameWApps(webApps) {
    
    let wAppsName = [];
    try {
        
        Object.keys(webApps).forEach(key => {
           wAppsName.push(key);
        });

    }catch(err) { // manejar variable error para cuando el directorio no tenga elementos
        console.warn(err);
        new Modal({
            type: "warning",
            title: `Error getNameWApps`,
            message: err.message
        });
    }

    return wAppsName;

}

async function createWmodule(key,icon){   
  //const fs = require('fs');  
  //const os = require("os");
  if (!fs.existsSync(path.join(electron.remote.app.getPath('home'), `modulos/${icon}_chrome.xobj`))){
      
      let icons = require('./assets/icons/file-icons.json');
      let iconext = require(path.join(electron.remote.app.getPath('userData'),'iconext.json'));
      let webApps = await leerWebApps();
      let newIcon = {};

      if(!icons[icon])
        newIcon = iconext[icon];
      else
        newIcon = icons[icon]; 

      window.xobjDB[`${icon}_chrome`] = {title: key, "icon":icon};

      iconext[webApps[key].class] = newIcon;

      let modulo = `{"title":"","x":0,"y":0,"w":0,"h":0,"code":"xWndExec('${icon}_chrome','${webApps[key].cmd}')","id":"${icon}_chrome","hidden":"true"}`;

      fs.writeFileSync(path.join(require("electron").remote.app.getPath("userData"), "iconext.json"), JSON.stringify(iconext, 4));
      //fs.writeFileSync(path.join(require("electron").remote.app.getPath("userData"), "xobjDB.json"), JSON.stringify(window.xobjDB, 4));
      fs.writeFileSync(path.join(electron.remote.app.getPath('home'), `modulos/${icon}_chrome.xobj`), modulo);
      
  }else{
    new Modal({
            type: "warning",
            title: `Error ${icon}_chrome`,
            message: 'Exists module'
        });
  }
}

