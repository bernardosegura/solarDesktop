class FilesystemDisplay {
    constructor(opts) {
        if (!opts.parentId) throw "Missing options";

        const fs = require("fs");
        const path = require("path");
        //const execSync = require("child_process").execSync; //para el cambio de lectura de directorios con ls -a
        window.pathWs = [];
        window.pathActual = '';
        this.cwd = [];
        this.iconcolor = `rgb(${window.theme.r}, ${window.theme.g}, ${window.theme.b})`;
        this._formatBytes = (a,b) => {if(0==a)return"0 Bytes";var c=1024,d=b||2,e=["Bytes","KB","MB","GB","TB","PB","EB","ZB","YB"],f=Math.floor(Math.log(a)/Math.log(c));return parseFloat((a/Math.pow(c,f)).toFixed(d))+" "+e[f]};
        this.fileIconsMatcher = require("./assets/misc/file-icons-match.js");
        this.icons = require("./assets/icons/file-icons.json");
        /*let iconext = require(path.join(require("electron").remote.app.getPath("userData"),"iconext.json"));
        Object.assign(this.icons,iconext);*/

        this.edexIcons = {
            theme: {
                width: 24,
                height: 24,
                svg: '<path d="M 17.9994,3.99805L 17.9994,2.99805C 17.9994,2.44604 17.5514,1.99805 16.9994,1.99805L 4.9994,1.99805C 4.4474,1.99805 3.9994,2.44604 3.9994,2.99805L 3.9994,6.99805C 3.9994,7.55005 4.4474,7.99805 4.9994,7.99805L 16.9994,7.99805C 17.5514,7.99805 17.9994,7.55005 17.9994,6.99805L 17.9994,5.99805L 18.9994,5.99805L 18.9994,9.99805L 8.9994,9.99805L 8.9994,20.998C 8.9994,21.55 9.4474,21.998 9.9994,21.998L 11.9994,21.998C 12.5514,21.998 12.9994,21.55 12.9994,20.998L 12.9994,11.998L 20.9994,11.998L 20.9994,3.99805L 17.9994,3.99805 Z"/>'
            },
            themesDir: {
                width: 24,
                height: 24,
                svg: `<path d="m9.9994 3.9981h-6c-1.105 0-1.99 0.896-1.99 2l-0.01 12c0 1.104 0.895 2 2 2h16c1.104 0 2-0.896 2-2v-9.9999c0-1.104-0.896-2-2-2h-8l-1.9996-2z" stroke-width=".2"/><path stroke-linejoin="round" d="m18.8 9.3628v-0.43111c0-0.23797-0.19314-0.43111-0.43111-0.43111h-5.173c-0.23797 0-0.43111 0.19313-0.43111 0.43111v1.7244c0 0.23797 0.19314 0.43111 0.43111 0.43111h5.1733c0.23797 0 0.43111-0.19314 0.43111-0.43111v-0.43111h0.43111v1.7244h-4.3111v4.7422c0 0.23797 0.19314 0.43111 0.43111 0.43111h0.86221c0.23797 0 0.43111-0.19314 0.43111-0.43111v-3.879h3.449v-3.4492z" stroke-width=".086221" fill="${window.theme.colors.light_black}"/>`
            },
            kblayout: {
                width: 24,
                height: 24,
                svg: '<path d="M 18.9994,9.99807L 16.9994,9.99807L 16.9994,7.99807L 18.9994,7.99807M 18.9994,12.9981L 16.9994,12.9981L 16.9994,10.9981L 18.9994,10.9981M 15.9994,9.99807L 13.9994,9.99807L 13.9994,7.99807L 15.9994,7.99807M 15.9994,12.9981L 13.9994,12.9981L 13.9994,10.9981L 15.9994,10.9981M 15.9994,16.9981L 7.99941,16.9981L 7.99941,14.9981L 15.9994,14.9981M 6.99941,9.99807L 4.99941,9.99807L 4.99941,7.99807L 6.99941,7.99807M 6.99941,12.9981L 4.99941,12.9981L 4.99941,10.9981L 6.99941,10.9981M 7.99941,10.9981L 9.99941,10.9981L 9.99941,12.9981L 7.99941,12.9981M 7.99941,7.99807L 9.99941,7.99807L 9.99941,9.99807L 7.99941,9.99807M 10.9994,10.9981L 12.9994,10.9981L 12.9994,12.9981L 10.9994,12.9981M 10.9994,7.99807L 12.9994,7.99807L 12.9994,9.99807L 10.9994,9.99807M 19.9994,4.99807L 3.99941,4.99807C 2.89441,4.99807 2.0094,5.89406 2.0094,6.99807L 1.99941,16.9981C 1.99941,18.1021 2.89441,18.9981 3.99941,18.9981L 19.9994,18.9981C 21.1034,18.9981 21.9994,18.1021 21.9994,16.9981L 21.9994,6.99807C 21.9994,5.89406 21.1034,4.99807 19.9994,4.99807 Z"/>'
            },
            kblayoutsDir: {
                width: 24,
                height: 24,
                svg: `<path d="m9.9994 3.9981h-6c-1.105 0-1.99 0.896-1.99 2l-0.01 12c0 1.104 0.895 2 2 2h16c1.104 0 2-0.896 2-2v-9.9999c0-1.104-0.896-2-2-2h-8l-1.9996-2z" stroke-width=".2"/><path stroke-linejoin="round" d="m17.48 11.949h-1.14v-1.14h1.14m0 2.8499h-1.14v-1.14h1.14m-1.7099-0.56999h-1.14v-1.14h1.14m0 2.8499h-1.14v-1.14h1.14m0 3.4199h-4.56v-1.14h4.56m-5.13-2.85h-1.1399v-1.14h1.14m0 2.8499h-1.1399v-1.14h1.14m0.56998 0h1.14v1.14h-1.14m0-2.8499h1.14v1.14h-1.14m1.7099 0.56999h1.14v1.14h-1.14m0-2.8499h1.14v1.14h-1.14m5.13-2.8494h-9.1199c-0.62982 0-1.1343 0.51069-1.1343 1.14l-0.0057 5.6998c0 0.62925 0.51013 1.14 1.14 1.14h9.1196c0.62925 0 1.14-0.5107 1.14-1.14v-5.6998c0-0.62926-0.5107-1.14-1.14-1.14z" stroke-width="0.114" fill="${window.theme.colors.light_black}"/>`
            },
            settings: {
                width: 24,
                height: 24,
                svg: '<path d="M 11.9994,15.498C 10.0664,15.498 8.49939,13.931 8.49939,11.998C 8.49939,10.0651 10.0664,8.49805 11.9994,8.49805C 13.9324,8.49805 15.4994,10.0651 15.4994,11.998C 15.4994,13.931 13.9324,15.498 11.9994,15.498 Z M 19.4284,12.9741C 19.4704,12.6531 19.4984,12.329 19.4984,11.998C 19.4984,11.6671 19.4704,11.343 19.4284,11.022L 21.5414,9.36804C 21.7294,9.21606 21.7844,8.94604 21.6594,8.73004L 19.6594,5.26605C 19.5354,5.05005 19.2734,4.96204 19.0474,5.04907L 16.5584,6.05206C 16.0424,5.65607 15.4774,5.32104 14.8684,5.06903L 14.4934,2.41907C 14.4554,2.18103 14.2484,1.99805 13.9994,1.99805L 9.99939,1.99805C 9.74939,1.99805 9.5434,2.18103 9.5054,2.41907L 9.1304,5.06805C 8.52039,5.32104 7.95538,5.65607 7.43939,6.05206L 4.95139,5.04907C 4.7254,4.96204 4.46338,5.05005 4.33939,5.26605L 2.33939,8.73004C 2.21439,8.94604 2.26938,9.21606 2.4574,9.36804L 4.5694,11.022C 4.5274,11.342 4.49939,11.6671 4.49939,11.998C 4.49939,12.329 4.5274,12.6541 4.5694,12.9741L 2.4574,14.6271C 2.26938,14.78 2.21439,15.05 2.33939,15.2661L 4.33939,18.73C 4.46338,18.946 4.7254,19.0341 4.95139,18.947L 7.4404,17.944C 7.95639,18.34 8.52139,18.675 9.1304,18.9271L 9.5054,21.577C 9.5434,21.8151 9.74939,21.998 9.99939,21.998L 13.9994,21.998C 14.2484,21.998 14.4554,21.8151 14.4934,21.577L 14.8684,18.9271C 15.4764,18.6741 16.0414,18.34 16.5574,17.9431L 19.0474,18.947C 19.2734,19.0341 19.5354,18.946 19.6594,18.73L 21.6594,15.2661C 21.7844,15.05 21.7294,14.78 21.5414,14.6271L 19.4284,12.9741 Z"/>'
            }
        };

//+++++++++++++++++++++++++++++++++Solar+++++++++++++++++++++++++++
this.cmdPath = async e =>{
    if(e.button == 2)
    {
        if(!window.settings.fileManager)
            window.term[window.currentTerm].write(' "'+window.pathActual/*document.getElementById("fs_disp_title_dir").innerHTML*/+'"');
        else
            if(window.settings.fileManager == '')
                window.term[window.currentTerm].write(' "'+window.pathActual/*document.getElementById("fs_disp_title_dir").innerHTML*/+'"');
            else
            {
                if(!window.settings.fileManager.endsWith('*'))//para wm no consola
                {
                    //const { exec } = require("child_process");
                
                    if(!window.teclaCTRL)
                        //window.term[window.currentTerm].writelr(window.settings.fileManager + ' "'+document.getElementById("fs_disp_title_dir").innerHTML+'"');
                        require("child_process").exec(window.settings.fileManager + ' "'+window.pathActual/*document.getElementById("fs_disp_title_dir").innerHTML*/+'"', (error, stdout, stderr) => {
                                if (error) {
                                    new Modal({
                                        type: "warning",
                                        title: `Error ${window.settings.fileManager}`,
                                        message: error.message
                                    });
                                }
                            });
                    else{
                        //window.term[window.currentTerm].writelr('sudo ' + window.settings.fileManager + ' "'+document.getElementById("fs_disp_title_dir").innerHTML+'"');
                        window.teclaCTRL = false;
                        xWndExecGksu("id_"+window.settings.fileManager,window.settings.fileManager + " \\'"+window.pathActual+"\\'");
                        setTimeout(() =>{document.getElementById('id_pasroot_'+"id_"+window.settings.fileManager).focus();}, 25);
                        
                        //if(!window.settings.sudoGUI) window.settings.sudoGUI = '';
                        //exec(window.settings.sudoGUI + ' ' + window.settings.fileManager + ' "'+window.pathActual/*document.getElementById("fs_disp_title_dir").innerHTML*/+'"', (error, stdout, stderr) => {
                        /*        if (error) {
                                    new Modal({
                                        type: "warning",
                                        title: `Error ${window.settings.fileManager}`,
                                        message: error.message
                                    });
                                }
                            });*/
                    }
                }else{
                    if(!window.teclaCTRL)
                        window.term[window.currentTerm].writelr(window.settings.fileManager.replace('*','') + ' "'+window.pathActual/*document.getElementById("fs_disp_title_dir").innerHTML*/+'"');
                    else{
                        window.term[window.currentTerm].writelr('sudo ' + window.settings.fileManager.replace('*','') + ' "'+window.pathActual/*document.getElementById("fs_disp_title_dir").innerHTML*/+'"');
                        window.teclaCTRL = false;
                    }
                }
            }
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


        const container = document.getElementById(opts.parentId);
         //se modifica para cambiar el filesystem por FILESYSTEM-APPLICATIONS
        container.innerHTML = `
            <h3 class="title"><p>FILESYSTEM-APPLICATIONS</p><p style="display: none;" id="fs_disp_title_dir"></p></h3>
            <!--h2 id="fs_disp_loading">LOADING...</h2-->
            <div id="fs_disp_container" onmousedown="window.fsDisp.cmdPath(event)">
            </div>
            <div id="fs_space_bar">
                <h1>EXIT DISPLAY</h1>
                <!--h3>Calculating available space...</h3><progress value="100" max="100"></progress-->
                <h3 id="fs_disp_loading">LOADING...</h3><progress id="fs_disp_load" max="100"></progress>
            </div>`;
        this.filesContainer = document.getElementById("fs_disp_container");
        /*this.space_bar = {
            text: document.querySelector("#fs_space_bar > h3"),
            bar: document.querySelector("#fs_space_bar > progress")
        };*/
        this.fsBlock = {};
        this.dirpath = "";
        this.failed = false;
        this._noTracking = false;
        this._runNextTick = false;
        this._reading = false;

        this._timer = setInterval(() => {
            if (this._runNextTick === true) {
                this._runNextTick = false;
                this.readFS(this.dirpath);
            }
        }, 1000);

        this._asyncFSwrapper = new Proxy(fs, {
            get: function(fs, prop) {
                if (prop in fs) {
                    return function(...args) {
                        return new Promise((resolve, reject) => {
                            fs[prop](...args, (err, d) => {
                                if (typeof err !== "undefined" && err !== null) reject(err);
                                if (typeof d !== "undefined") resolve(d);
                                if (typeof d === "undefined" && typeof err === "undefined") resolve();
                            });
                        });
                    }
                }
            },
            set: function() {
                return false;
            }
        });

        this.setFailedState = () => {
            this.failed = true;
            container.innerHTML = `
            <h3 class="title"><p>FILESYSTEM</p><p id="fs_disp_title_dir">EXECUTION FAILED</p></h3>
            <h2 id="fs_disp_error">CANNOT ACCESS CURRENT WORKING DIRECTORY</h2>`;
        };

        this.followTab = () => {
            // Don't follow tabs when running in detached mode, see #432
            if (this._noTracking) return false;

            let num = window.currentTerm;

            window.term[num].oncwdchange = cwd => {
                // See #501
                //se valida y comenta para deshabilitar el filemanager y que solo funcione enla carpeta de modulos.
                //if(cwd == path.join(require("electron").remote.app.getPath("home"),"modulos")) // se cambia para carga con problema 
                if(document.getElementById("fs_disp_title_dir").innerHTML == '')
                { 
                    /*if (this._noTracking) return false;

                    if (cwd && window.currentTerm === num) {
                        if (this._fsWatcher) {
                            this._fsWatcher.close();
                        }
                        if (cwd.startsWith("FALLBACK |-- ")) {
                            this.readFS(cwd.slice(13));
                            this._noTracking = true;
                        } else {*/
                            this.readFS(path.join(require("electron").remote.app.getPath("home"),"modulos")); 
                            /*this.watchFS(cwd);
                        }
                    }*/
                }
                window.pathActual = cwd; // este es para poder acceder a la ruta actual mediante fm
            };
        };
        this.followTab();

        this.watchFS = dir => {
            if (this._fsWatcher) {
                this._fsWatcher.close();
            }
            this._fsWatcher = fs.watch(dir, () => {
                this._runNextTick = true;
            });

            if(!window.pathWs.includes(dir))
                window.pathWs.push(dir);
        };

        this.toggleHidedotfiles = () => {
            if (window.settings.hideDotfiles) {
                container.classList.remove("hideDotfiles");
                window.settings.hideDotfiles = false;
            } else {
                container.classList.add("hideDotfiles");
                window.settings.hideDotfiles = true;
            }
        };

        this.toggleListview = () => {
            if (window.settings.fsListView) {
                container.classList.remove("list-view");
                window.settings.fsListView = false;
            } else {
                container.classList.add("list-view");
                window.settings.fsListView = true;
            }
        };

        this.readFS = async dir => {
            if (this.failed === true || this._reading) return false;
            this._reading = true;

            document.getElementById("fs_disp_loading").style.display = 'block';
            document.getElementById("fs_disp_load").style.display = 'block';

            document.getElementById("fs_disp_title_dir").innerText = dir;//this.dirpath;

            this.filesContainer.setAttribute("class", "");
            this.filesContainer.innerHTML = "";
            if (this._noTracking) {
                document.querySelector("section#filesystem > h3.title > p:first-of-type").innerText = "FILESYSTEM - TRACKING FAILED, RUNNING DETACHED FROM TTY";
            }

            //if (process.platform === "win32" && dir.endsWith(":")) dir = dir+"\\";
            let tcwd = dir;
          
          //se comenta y agregamos codigo para camniar la lectura de directorio
            
            /*let content = await this._asyncFSwrapper.readdir(tcwd).catch(err => {
                console.warn(err);
                
                if (this._noTracking === true && this.dirpath) { // #262
                    this.setFailedState();
                    setTimeout(() => {
                        this.readFS(this.dirpath);
                    }, 1000);
                } else {
                    this.setFailedState();
                }
            });*/

            let content = [];
            try {
                
                 content = await fs.readdirSync(tcwd);
                 //execSync("ls -a " + tcwd).toString().replace(".\n","").replace("..\n","").split("\n");
                 //content.pop();

            }catch(err) { // manejar variable error para cuando el directorio no tenga elementos
                console.warn(err);
                
                if (this._noTracking === true && this.dirpath) { // #262
                    this.setFailedState();
                    setTimeout(() => {
                        this.readFS(this.dirpath);
                    }, 1000);
                } else {
                    this.setFailedState();
                }
            }
            

///////////////////////////Quitamos la barra para calcular o usado///////////////////////////
            //this.reCalculateDiskUsage(tcwd);

            this.cwd = [];

            await new Promise((resolve, reject) => {
                if (content.length === 0) resolve(); // NOTA si no hay archivos el wach iria aqui tambien 

                content.forEach(async (file, i) => {
                    

                    //se comenta y agregamos codigo para cambiar la lectura del estado de archivos
                    
                    /*let fstat =  await this._asyncFSwrapper.lstat(path.join(tcwd, file)).catch(e => {
                        if (!e.message.includes("EPERM") && !e.message.includes("EBUSY")) {
                            reject();
                        }
                    });*/


                    let fstat = {};

                    try {
                
                         fstat =  await fs.lstatSync(path.join(tcwd, file));

                    }catch(e) {
                        
                        if (!e.message.includes("EPERM") && !e.message.includes("EBUSY")) {
                                reject();
                        }
                    }


                    let e = {
                        name: window._escapeHtml(file),
                        type: "other",
                        category: "other",
                        hidden: false
                    };

                    if (typeof fstat !== "undefined") {
                        e.lastAccessed = fstat.mtime;

                        if (fstat.isDirectory()) {
                            e.category = "dir";
                            e.type = "dir";
                        }
                        if (e.category === "dir" && tcwd === settingsDir && file === "themes") e.type="edex-themesDir";
                        if (e.category === "dir" && tcwd === settingsDir && file === "keyboards") e.type = "edex-kblayoutsDir";

                        if (fstat.isSymbolicLink()) {
                            e.category = "symlink";
                            //e.type = "symlink"; // se utilizaran los ln (enlaces) en panel principal para colocar icono,titulo a las apps de los grupos
                             e.type = "file";
                        }

                        if (fstat.isFile()) {
                            e.category = "file";
                            e.type = "file";
                            e.size = fstat.size;
                        }
                    } else {
                        e.type = "system";
                        e.hidden = true;
                    }

                    if (e.category === "file" && tcwd === themesDir && file.endsWith(".json")) e.type = "edex-theme";
                    if (e.category === "file" && tcwd === keyboardsDir && file.endsWith(".json")) e.type = "edex-kblayout";
                    if (e.category === "file" && tcwd === settingsDir && file === "settings.json") e.type = "edex-settings";

                    if (file.startsWith(".")) e.hidden = true;

                    this.cwd.push(e);
                    if (i === content.length-1) resolve();
                });
            }).catch(() => { this.setFailedState() });

            if (this.failed) return false;

            let ordering = {
                dir: 0,
                symlink: 1,
                file: 2,
                other: 3
            };

            this.cwd.sort((a, b) => {
                return (ordering[a.category] - ordering[b.category] || a.name.localeCompare(b.name));
            });


            this.cwd.splice(0, 0, {
                name: "Show disks",
                type: "showDisks"
            });

            if (tcwd !== "/" && /^[A-Z]:\\$/i.test(tcwd) === false) {
                this.cwd.splice(1, 0, {
                    name: "Go up",
                    type: "up"
                });

                /*this.cwd.splice(2, 0, {
                    name: "Update",
                    type: "update"
                });*/
            }

             this.cwd.splice(0, 0, {
                name: "PowerOff",
                type: "PowerOff"
            });

            this.cwd.splice(1, 0, {
                name: "Battery",
                type: "Battery"
            }); 

            this.cwd.splice(2, 0, {
                name: "Num Lock",
                type: "num-lock"
            }); 

            this.cwd.splice(3, 0, {
                name: "Caps Lock",
                type: "caps-lock"
            }); 

             /*this.cwd.splice(4, 0, {
                    name: "Go up",
                    type: "up"
                });*/

            this.dirpath = tcwd;
            this.render(this.cwd);
            this._reading = false;
        };

        this.readDevices = async () => {
            if (this.failed === true) return false;

            let blocks = await window.si.blockDevices();
            let devices = [];
            blocks.forEach(block => {
                if (fs.existsSync(block.mount)) {
                    let type = (block.type === "rom") ? "rom" : "disk";
                    if (block.removable && block.type !== "rom") {
                        type = "usb";
                    }

                    devices.push({
                        name: (block.label !== "") ? `${block.label} (${block.name})` : `${block.mount} (${block.name})`,
                        type,
                        path: block.mount
                    });
                }
            });

            this.render(devices, true);
        };

        this.render = async (blockList, isDiskView) => {
            if (this.failed === true) return false;

            if (isDiskView) {
                document.getElementById("fs_disp_title_dir").innerText = "Showing available block devices";
                this.filesContainer.setAttribute("class", "disks");
            } else {
                document.getElementById("fs_disp_title_dir").innerText = this.dirpath;
                this.filesContainer.setAttribute("class", "");
            }
            if (this._noTracking) {
                document.querySelector("section#filesystem > h3.title > p:first-of-type").innerText = "FILESYSTEM - TRACKING FAILED, RUNNING DETACHED FROM TTY";
            }

            let filesDOM = ``;
            let fileMainPanel = ``;
            let appsinPanel = 0;
            let inpanel = false;
            let idInpanel = '';

            let iconext = require(path.join(require("electron").remote.app.getPath("userData"),"iconext.json"));
            Object.assign(this.icons,iconext);



            if(this.dirpath == path.join(require("electron").remote.app.getPath("home"),"modulos"))
            {
                window.cApps.xobjFile = [];
                window.cApps.xobjTitle = [];
            }
            blockList.forEach(e => {
                let hidden = e.hidden ? " hidden" : "";

                let cmd;

                if (!this._noTracking) {
                    if (e.type === "dir" || e.type.endsWith("Dir")) {
                        cmd = `window.fsDisp.readFS('${path.resolve(this.dirpath, e.name).replace(/\\/g, '\\\\')}')`; //`window.term[window.currentTerm].writelr('cd \\'${e.name.replace(/\\/g, "\\\\")}\\'')`;
                    } else if (e.type === "up") {
                        // se limita a que los grupos el nuvel superior sea la carpeta de modulos
                        const rutaOrigen = path.join(electron.remote.app.getPath("home"), "modulos");
                        if(this.dirpath.startsWith(rutaOrigen))
                            cmd = `window.fsDisp.readFS('${path.resolve(this.dirpath, '..').replace(/\\/g, '\\\\')}')`; //`window.term[window.currentTerm].writelr('cd ..')`;
                        else
                            cmd = `window.fsDisp.readFS('${rutaOrigen.replace(/\\/g, '\\\\')}')`; //`window.term[window.currentTerm].writelr('cd ..')`;

                    } else if (e.type === "disk" || e.type === "rom" || e.type === "usb") {
                        if (process.platform === "win32") {
                            cmd = `window.term[window.currentTerm].writelr('${e.path.replace(/\\/g, "\\\\")}')`;
                        } else {
                            cmd = `window.term[window.currentTerm].writelr('cd \\'${e.path.replace(/\\/g, "\\\\")}\\'')`;
                        }
                    } else {
//+++++++++++++++++++++++++++++Solar++++                        
                        cmd = `window.term[window.currentTerm].write('./\\'${e.name}\\'')`;
//++++++++++++++++++++++++++++++++++++++                          
                    }
                } else {
                    if (e.type === "dir" || e.type.endsWith("Dir")) {
                        cmd = `window.fsDisp.readFS('${path.resolve(this.dirpath, e.name).replace(/\\/g, '\\\\')}')`;
                    } else if (e.type === "up") {
                        // se limita a que los grupos el nuvel superior sea la carpeta de modulos
                        const rutaOrigen = path.join(electron.remote.app.getPath("home"), "modulos");
                        if(this.dirpath.startsWith(rutaOrigen))
                            cmd = `window.fsDisp.readFS('${path.resolve(this.dirpath, '..').replace(/\\/g, '\\\\')}')`; //`window.term[window.currentTerm].writelr('cd ..')`;
                        else
                            cmd = `window.fsDisp.readFS('${rutaOrigen.replace(/\\/g, '\\\\')}')`; //`window.term[window.currentTerm].writelr('cd ..')`;

                    } else if (e.type === "disk" || e.type === "rom" || e.type === "usb") {
                        cmd = `window.fsDisp.readFS('${e.path.replace(/\\/g, '\\\\')}')`;
                    } else {
                        cmd = `window.term[window.currentTerm].write('\\'${path.resolve(this.dirpath, e.name)}\\'')`;
                    }
                }

                /*if (e.type === "update") {
                    cmd = "window.fsDisp.readFS(document.getElementById('fs_disp_title_dir').innerText)";
                }*/

                if (e.type === "system") {
                    cmd = "";
                }

                if (e.type === "showDisks") {
                    cmd = `window.fsDisp.readDevices()`;
                }

                if (e.type === "edex-theme") {
                    cmd = `window.themeChanger('${e.name.slice(0, -5)}')`;
                }
                if (e.type === "edex-kblayout") {
                    cmd = `window.remakeKeyboard('${e.name.slice(0, -5)}')`;
                }
                if (e.type === "edex-settings") {
                    cmd = `window.openSettings()`;
                }

                if (e.type === "PowerOff") {
                    cmd = `systemPoweroff()`;
                }

                if (e.type === "Battery") { // para agregar comando en bateria
                    cmd = `powerPreferences()`;
                }

                let icon = "";
                let type = "";
                switch(e.type) {
                    case "PowerOff":
                        icon = this.icons.poweroff;
                        type = "--";
                        e.category = "PowerOff";
                        inpanel = true;
                        break;
                    case "Battery":
                        icon = this.icons.battery;
                        type = "--";
                        e.category = "Indicator";
                        inpanel = true;
                        break;    
                    case "num-lock":
                        icon = this.icons["num-lock"];
                        type = "--";
                        cmd = "execKeyLock('num')";
                        e.category = "Indicator";
                        inpanel = true;
                        idInpanel = 'num-lock';
                        break;   
                    case "caps-lock":
                        icon = this.icons["caps-lock"];
                        type = "--";
                        cmd = "execKeyLock('caps')";
                        e.category = "Indicator";
                        inpanel = true;
                        idInpanel = 'caps-lock';
                        break;       
                    case "showDisks":
                        icon = this.icons.showDisks;
                        type = "--";
                        e.category = "showDisks";
                        break;
                    case "up":
                        icon = this.icons.up;
                        type = "--";
                        e.category = "up";
                        break;
                    case "symlink":
                        icon = this.icons.symlink;
                        break;
                    case "disk":
                        icon = this.icons.disk;
                        break;
                    case "rom":
                        icon = this.icons.rom;
                        break;
                    case "usb":
                        icon = this.icons.usb;
                        break;
                    case "edex-theme":
                        icon = this.edexIcons.theme;
                        type = "eDEX-UI theme";
                        break;
                    case "edex-kblayout":
                        icon = this.edexIcons.kblayout;
                        type = "eDEX-UI keyboard layout";
                        break;
                    case "edex-settings":
                        icon = this.edexIcons.settings;
                        type = "eDEX-UI config file";
                        break;
                    case "system":
                        icon = this.edexIcons.settings;
                        break;
                    case "edex-themesDir":
                        icon = this.edexIcons.themesDir;
                        type = "eDEX-UI themes folder";
                        break;
                    case "edex-kblayoutsDir":
                        icon = this.edexIcons.kblayoutsDir;
                        type = "eDEX-UI keyboards folder";
                        break;
                    default:
                        //if (e.type === "dir") type = "folder";
                        let iconName = this.fileIconsMatcher(e.name);
                        icon = this.icons[iconName];
                        if (typeof icon === "undefined") {
                            if (e.type === "file") icon = this.icons.file;
                            if (e.type === "dir") {
                                icon = this.icons.dir; //this.icons.group; //aqui icono grupo, se quita para anexar la solucion con enlace symbolico
                            }
                            if (typeof icon === "undefined") icon = this.icons.other;
                        } else {
                            type = iconName.replace("icon-", "");
                        }
                        break;
                }

                if (type === "") type = e.type;

                if (typeof e.size === "number") {
                    e.size = this._formatBytes(e.size);
                } else {
                    e.size = "--";
                }
                if (typeof e.lastAccessed === "object") {
                    e.lastAccessed = e.lastAccessed.toString().substr(0, e.lastAccessed.toString().indexOf(" ("));
                } else {
                    e.lastAccessed = "--";
                }

//+++++++++++++++++++++++++++++Solar+++++++++++++++++
                let tamanio = (e.type === "file")? " - " + e.size: "";
                if(e.type === "file")
                {
                   if(e.name.toLowerCase().endsWith(".xobj"))
                    {

                        cmd="appXwnd('"+e.name+"');";
                        e.name = e.name.substring(0, e.name.length - 5);

                        if(e.name.toLowerCase().startsWith("inpanel")){
                            inpanel = true;
                            idInpanel = (idInpanel != '')?idInpanel:e.name.toLowerCase();
                        }

                        if(this.dirpath == path.join(require("electron").remote.app.getPath("home"),"modulos"))
                        { 
                           window.cApps.xobjFile.push(e.name);
                        }

                        if(!window.xobjDB[e.name.toLowerCase()])
                         {
                            //if(idInpanel.startsWith("inpanel")){
                            //    e.name = idInpanel.replace(idInpanel.split('-')[0] + '-',"").replace(window.entorno + '-',''); 
                            //    e.name = e.name.replace(/-/g,' ').replace(/_/g,' ').trim().toLowerCase().replace(/\w\S*/g, (w) => (w.replace(/^\w/, (c) => c.toUpperCase())));
                            //    e.name = e.name.replace(e.name.split(" ")[0] + ' ',"");
                            //}
                            let icono = e.name;
                            if(inpanel){
                                icono = icono.replace(icono.split('-')[0] + '-',"");                                
                           }
                           icono = icono.toLowerCase().replace(window.entorno + '-','').split('-')[0];
                           icono = icono.toLowerCase().split('_')[0];
                           icono = icono.toLowerCase().split(' ')[0];

                           icono = (this.fileIconsMatcher(e.name.toLowerCase(),true) != "")? this.fileIconsMatcher(e.name.toLowerCase()):icono;
                         
                           e.name = getTitleAppsDesktop(e.name + '.desktop');

                            if(!this.icons[icono]){
                                icon = this.icons[this.fileIconsMatcher(icono)];
                                if(typeof icon === "undefined")
                                    icon = this.icons.appXwnd; 
                            }    
                            else
                               icon = this.icons[icono];
                            //e.name = e.name.trim().toLowerCase().replace(/\w\S*/g, (w) => (w.replace(/^\w/, (c) => c.toUpperCase())));
                         }else{

                            if(!this.icons[window.xobjDB[e.name.toLowerCase()].icon]){ //se agregan icons svg personalizados de 100x100
                                
                                if(window.xobjDB[e.name.toLowerCase()].icon.startsWith('svg=')){

                                    icon = {
                                        width: 100,
                                        height: 100,
                                        svg: '<path d="' + window.xobjDB[e.name.toLowerCase()].icon.replace('svg=','') + '"/>'
                                    }

                                }else{
                                   icon = this.icons[this.fileIconsMatcher(window.xobjDB[e.name.toLowerCase()].icon)];
                                   icon = (this.fileIconsMatcher(e.name.toLowerCase(),true) != "")? this.icons[this.fileIconsMatcher(e.name.toLowerCase())]:icon;
                                   if(typeof icon === "undefined")
                                        icon = this.icons.appXwnd;  
                                }                                   
                            }
                            else
                               icon = this.icons[window.xobjDB[e.name.toLowerCase()].icon];
                            
                            e.name = window.xobjDB[e.name.toLowerCase()].title;
                         }                      
  
                        if(this.dirpath == path.join(require("electron").remote.app.getPath("home"),"modulos"))
                        {  
                           window.cApps.xobjTitle.push(e.name);
                        }

                        e.type = "xobj";
                        // se utilizaran los ln (enlaces) en panel principal para colocar icono,titulo a las apps de los grupos
                        if(e.category != "symlink")
                            e.category = tamanio;
                        type = "Application";
                    }
                }/*else  //se comenta ya que este se ejecuta en el boton de salida de discos
                if(e.type === "xobj")
                {  
                    cmd="appXwnd('"+e.name+"');";
                    icon = this.icons.appXwnd;
                    tamanio = e.category;
                    //e.name = e.name.substring(0, e.name.length - 5);
                }*/
                /*if(e.lastAccessed.replace(/-/g,'') != "")
                    e.lastAccessed = " - " + e.lastAccessed;
                else*/
                    //e.lastAccessed = "";

                //filesDOM += `<div class="fs_disp_${e.type}${hidden} animationWait" onclick="${cmd}" title="${e.name}${tamanio}${e.lastAccessed}">
            if(e.name.startsWith(".")) e.name = e.name.replace('.','');
            if(e.category != "showDisks" /*&& e.category != "up"*/){ //se comenta para deshabilitar el filemanager
                if(inpanel){
                    let none = '<-keys->';
                    let modKeys = '';

                    if(e.type == 'Battery')
                       window.maxBattery = icon.max;

                   if(idInpanel == 'num-lock'){
                        none = 'style="<-keys->"';

                        if(window.settings.numlock)
                            modKeys += 'display: none; ';

                        let active = require("child_process").execSync("statuskeyslock num" ).toString().startsWith('true');
                        if(!active)
                            modKeys += 'opacity: 0.5;'; 
                   }

                   if(idInpanel == 'caps-lock'){
                        none = 'style="<-keys->"';

                        if(window.settings.capslock)
                            modKeys += 'display: none; ';

                        let active = require("child_process").execSync("statuskeyslock caps" ).toString().startsWith('true');
                        if(!active)
                            modKeys += 'opacity: 0.5;'; 
                   }

                    none = none.replace('<-keys->',modKeys);
                                                  
                    fileMainPanel += `<div class="icono_panel" ${(idInpanel != '')?'id="' + idInpanel + '"':''} ${(e.type == 'Battery')?'id="' + window.idBattery + '"':''} ${(e.type == 'Battery')?'style="opacity: 0.5; ' +window.batteryNone+ '"':''} ${(idInpanel == 'inpanel-wireless' || idInpanel == 'inpanel-red' || idInpanel == 'inpanel-network')?'style="opacity: 0.5;"':''} ${none} onclick="${cmd}" title="${e.name}">
                                         <svg viewBox="0 0 ${icon.width} ${icon.height}" fill="${this.iconcolor}" style="width: 100%; height: 100%;">
                                            ${icon.svg}
                                            ${(e.type == 'Battery')?icon.plug.replace("<--id_plug-->",window.idBattery + '_plug') + icon.energy.replace("<--id_energy-->",window.idBattery + '_energy'):""}
                                        </svg>                                    
                                      </div>`;
                    appsinPanel++;  
                    inpanel = false;  
                    idInpanel = "";              
                }else{

                    let ocultarSymlnk = '';
                    // se utilizaran los ln (enlaces) en panel principal para colocar icono,titulo a las apps de los grupos
                    if(e.category === "symlink" || e.type === "dir")
                        ocultarSymlnk = 'style="display: none"';

                    if(path.join(require("electron").remote.app.getPath("home"),"modulos") == this.dirpath){
                        if(e.category != "up")
                            filesDOM += `<div class="fs_disp_${e.type}${hidden} animationWait" ${ocultarSymlnk} onclick="${cmd}" title="${e.name}${tamanio}">
                                        <svg viewBox="0 0 ${icon.width} ${icon.height}" fill="${this.iconcolor}">
                                            ${icon.svg}
                                        </svg>
                                        <h3>${e.name}</h3>
                                        <h4>${type}</h4>
                                        <h4>${e.size}</h4>
                                        <h4>${e.lastAccessed}</h4>
                                    </div>`;            
                     }else
                         filesDOM += `<div class="fs_disp_${e.type}${hidden} animationWait" ${ocultarSymlnk} onclick="${cmd}" title="${e.name}${tamanio}">
                                    <svg viewBox="0 0 ${icon.width} ${icon.height}" fill="${this.iconcolor}">
                                        ${icon.svg}
                                    </svg>
                                    <h3>${e.name}</h3>
                                    <h4>${type}</h4>
                                    <h4>${e.size}</h4>
                                    <h4>${e.lastAccessed}</h4>
                                </div>`;           
                }
            }
//+++++++++++++++++++++++++++++++++++++++++++++++++++

                /*filesDOM += `<div class="fs_disp_${e.type}${hidden} animationWait" onclick="${cmd}">
                                <svg viewBox="0 0 ${icon.width} ${icon.height}" fill="${this.iconcolor}">
                                    ${icon.svg}
                                </svg>
                                <h3>${e.name}</h3>
                                <h4>${type}</h4>
                                <h4>${e.size}</h4>
                                <h4>${e.lastAccessed}</h4>
                            </div>`;*/
            });

            document.getElementById("fs_disp_loading").style.display = 'none';
            document.getElementById("fs_disp_load").style.display = 'none';
            
            this.filesContainer.innerHTML = filesDOM;
            document.getElementById("main_panel").innerHTML = fileMainPanel;

            if (this.filesContainer.getAttribute("class").endsWith("disks")) {
                document.getElementById("fs_space_bar").setAttribute("onclick", "window.fsDisp.render(window.fsDisp.cwd)");
            } else {
                document.getElementById("fs_space_bar").setAttribute("onclick", "");
            }

            // Render animation
            let id = 0;
            while (this.filesContainer.childNodes[id]) {
                let e = this.filesContainer.childNodes[id];
                e.setAttribute("class", e.className.replace(" animationWait", ""));

                if (window.settings.hideDotfiles !== true || e.className.indexOf("hidden") === -1) {
                    window.audioManager.folder.play();
                    await _delay(30);
                }
                id++;

                /////agregamos el control de rutas para el watch, al finalizar la carga de los iconos del directorio
                /*if(!window.pathWs.includes(this.dirpath) && id == this.filesContainer.childNodes.length)
                    this.watchFS(this.dirpath);*/
                //console.log(id,this.filesContainer.childNodes.length,this.dirpath);
            }

//para tratar de solucionar que no se muestren todos los elementos del directorio

            if((blockList.length - ((path.join(require("electron").remote.app.getPath("home"),"modulos") == this.dirpath?2:1)/*2*/ + appsinPanel)) !== this.filesContainer.childNodes.length) // -2 por el showDisk y Up y mas las aplicaciones en el nuevo panel superior
            {//le reste 1 para reintegrar el up
                 this.readFS(this.dirpath);
            }

        };

        this.reCalculateDiskUsage = async path => {
            this.fsBlock = null;
            this.space_bar.text.innerHTML = "Calculating available space...";
            this.space_bar.bar.removeAttribute("value");

            window.si.fsSize().catch(() => {
                this.space_bar.text.innerHTML = "Could not calculate mountpoint usage.";
                this.space_bar.bar.value = 100;
            }).then(d => {
                d.forEach(fsBlock => {
                    if (path.startsWith(fsBlock.mount)) {
                        this.fsBlock = fsBlock;
                    }
                });
                this.renderDiskUsage(this.fsBlock);
            });
        };

        this.renderDiskUsage = async fsBlock => {
            if (document.getElementById("fs_space_bar").getAttribute("onclick") !== "" || fsBlock === null) return;

            let splitter = (process.platform === "win32") ? "\\" : "/";
            let displayMount = (fsBlock.mount.length < 18) ? fsBlock.mount : "..."+splitter+fsBlock.mount.split(splitter).pop();

            // See #226
            if (!isNaN(fsBlock.use)) {
                this.space_bar.text.innerHTML = `Mount <strong>${displayMount}</strong> used <strong>${Math.round(fsBlock.use)}%</strong>`;
                this.space_bar.bar.value = Math.round(fsBlock.use);
            } else if (!isNaN((fsBlock.size / fsBlock.used) * 100)) {
                let usage = Math.round((fsBlock.size / fsBlock.used) * 100);

                this.space_bar.text.innerHTML = `Mount <strong>${displayMount}</strong> used <strong>${usage}%</strong>`;
                this.space_bar.bar.value = usage;
            } else {
                this.space_bar.text.innerHTML = "Could not calculate mountpoint usage.";
                this.space_bar.bar.value = 100;
            }
        };

        // Automatically start indexing supposed beginning CWD
        // See #365
        // ...except if we're hot-reloading, in which case this can mess up the rendering
        // See #392
        if (window.performance.navigation.type === 0) {
            //this.readFS(window.term[window.currentTerm].cwd || window.settings.cwd);
            //se modifica para el filemanager y se agrega el watchFS para que se actualicen los cambios.
            let appsDir = path.join(require("electron").remote.app.getPath("home"),"modulos");
            this.readFS(appsDir);
            //this.watchFS(appsDir); // se crea variable window.pathWs para tratar de evitar que no se muestren bien los directorios.
             
             /////agregamos el watch directamente con esto desactializamos el metodos watchFS

            fs.watch(appsDir, (event,file) => { // nota: con el shift+ctl+r para reiniciar este ya no se ejecuta.

                if(event === 'rename') //cuando se escribe contenido archivo es change
                {
                    if(file.toLowerCase().endsWith(".xobj"))
                    {
                       file = file.substring(0, file.length - 5);
                       let titulo = getTitleAppsDesktop(file + '.desktop');

                        if(!window.xobjDB[file.toLowerCase()])
                        {
                            //if(titulo == file)
                            //     window.xobjDB[file.toLowerCase()] = {title:file.trim().toLowerCase().replace(/\w\S*/g, (w) => (w.replace(/^\w/, (c) => c.toUpperCase()))), icon:file.toLowerCase()};
                            // else
                            if(file.toLowerCase().startsWith("inpanel")){
                                /*let nameBuff = '';
                                nameBuff = titulo.split(' ')[0] + ' ';
                                nameBuff = titulo.replace(nameBuff,'');
                                titulo = nameBuff;*/
                                let icono = file.replace(file.split('-')[0] + '-',"");
                                icono = icono.toLowerCase().replace(window.entorno + '-','').split('-')[0];
                                icono = icono.toLowerCase().split('_')[0];
                                icono = icono.toLowerCase().split(' ')[0];
                                window.xobjDB[file.toLowerCase()] = {title: titulo, icon:icono.toLowerCase()};                               
                           }else{
                                let icono = file.toLowerCase().replace(window.entorno + '-','').split('-')[0];
                                icono = icono.toLowerCase().split('_')[0];
                                icono = icono.toLowerCase().split(' ')[0];
                                window.xobjDB[file.toLowerCase()] = {title: titulo, icon:icono.toLowerCase()};
                           }
                        }else{
                            //window.xobjDB.files = window.xobjDB.files.filter( el => el[file] === window.xobjDB.files[file]); 
                            if(!file.toLowerCase().endsWith('_chrome'))
                            { 
                                 delete  window.xobjDB[file.toLowerCase()]; 
                            }
                        }
                        
                        fs.writeFileSync(path.join(require("electron").remote.app.getPath("userData"), "xobjDB.json"), JSON.stringify(window.xobjDB, 4));
                           
                        this.readFS(appsDir);
                        
                    }else{
                       this.readFS(appsDir); 
                    }
                }

                //console.log(event,file); // cuando se crea es rename y despues change, cuando se elimina solo es rename
            });
            //{"title":"","x":0,"y":0,"w":0,"h":0,"code":"xWndExecFDesktop('id_inkscape','/usr/share/applications/inkscape.desktop')","id":"id_inkscape","hidden":"true"}
        }
    }
}

module.exports = {
    FilesystemDisplay
};
