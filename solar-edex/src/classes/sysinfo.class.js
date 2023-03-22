class Sysinfo {

    constructor(parentId) {
        //window.conectedBateryFlag = 0;
        //window.conectedBateryInit = 0;
        window.lowBateryFlag = 0;
        let porBatAlert = 20;
        window.batAlert = porBatAlert;
        window.batPlayAlert = porBatAlert;

        if (!parentId) throw "Missing parameters";

        // See #255
        /*let os;
        switch (require("os").platform()) {
            case "darwin":
                os = "macOS";
                break;
            case "win32":
                os = "win";
                break;
            default:
                os = require("os").platform();
        }*/

        this.isCharging = require('is-charging');
        //this.timeremainingConected = 365040; // se detecto que al estar conectada y cargada marca ese tiepo la bateria asi que se maneja como 
        this.batteryFlag = false;
        //punto de partida no se si otras lap manejen este parametro de la misma manera.
        // Create DOM
        this.parent = document.getElementById(parentId);
        //quitamos el type por que todo sera Linux
        this.parent.innerHTML += `<div id="mod_sysinfo">
            <div>
                <h1 style="cursor: pointer;" title="View">1970</h1>
                <h2 onclick="xExecInTrm('cal')" title="View" style="cursor: pointer;">JAN 1</h2>
            </div>
            <div>
                <h1>UPTIME</h1>
                <h2>0:0:0</h2>
            </div>
            <div ${(!window.wm)?'style="display:none;"':''}>
                <!--h1 title="Opened Native Window" onclick="showTogglePanel();" style="cursor:pointer;">X11</h1-->
                <h1 title="Opened Native X Window System" onclick="showTogglePanel();">X11</h1>
                <h2 title="Opened Native X Window System" onclick="showTogglePanel();">0</h2>
                <!--h2 title="Opened Native Window" onclick="showTogglePanel();" style="cursor:pointer;">0</h2-->
            </div>
            <!--div id="lbl_porcentCharge" title="">
                <h1>POWER</h1>
                <h2>00%</h2>
            </div-->
        </div>`;
        /*this.parent.innerHTML += `<div id="mod_sysinfo">
            <div>
                <h1>1970</h1>
                <h2>JAN 1</h2>
            </div>
            <div>
                <h1>UPTIME</h1>
                <h2>0:0:0</h2>
            </div>
            <div id="lbl_porcentCharge" title="">
                <h1>POWER</h1>
                <h2>00%</h2>
            </div>
        </div>`;*/

        this.updateDate();
        this.updateUptime();
        this.uptimeUpdater = setInterval(() => {
            this.updateUptime();
        }, 60000);
        this.updateBattery();
        this.batteryUpdater = setInterval(() => {
            this.updateBattery();
        }, 3000); //3000);

        /*window.si.battery().then(bat => {
            if (bat.hasbattery) {
                this.playConectBattery();

                this.batteryConectPlay = setInterval(() => {
                    this.playConectBattery();
                }, 1000); 
            } 
        });*/
        

    }

    playConectBattery(isCharging){
            /*this.isCharging().then(result => {
                if(result){
                    if(window.conectedBateryFlag == 0 &&  window.conectedBateryInit == 1)
                     {
                        window.audioManager.conectBatery.play();
                        window.conectedBateryFlag = 1;
                     }else{
                        window.conectedBateryFlag = 1;
                     }
                }else{
                    if(window.conectedBateryFlag == 1)
                     {
                        window.audioManager.conectBatery.play();
                        window.conectedBateryFlag = 0;
                     }
                     window.conectedBateryInit = 1;
                }
                
            });*/ 
            if(isCharging){
                document.getElementById(window.idBattery + '_plug').setAttribute("style", "");
                window.audioManager.conectBatery.play();
            }else{
                document.getElementById(window.idBattery + '_plug').setAttribute("style", "display: none;");
            }
    }

    updateDate() {
        let time = new Date();

        document.querySelector("#mod_sysinfo > div:first-child > h1").innerHTML = time.getFullYear();
        document.querySelector("#mod_sysinfo > div:first-child > h1").setAttribute("onclick","xExecInTrm('cal " + time.getFullYear()+"')");
        

        let month = time.getMonth();
        switch(month) {
            case 0:
                month = "JAN";
                break;
            case 1:
                month = "FEB";
                break;
            case 2:
                month = "MAR";
                break;
            case 3:
                month = "APR";
                break;
            case 4:
                month = "MAY";
                break;
            case 5:
                month = "JUN";
                break;
            case 6:
                month = "JUL";
                break;
            case 7:
                month = "AUG";
                break;
            case 8:
                month = "SEP";
                break;
            case 9:
                month = "OCT";
                break;
            case 10:
                month = "NOV";
                break;
            case 11:
                month = "DEC";
                break;
        }
        document.querySelector("#mod_sysinfo > div:first-child > h2").innerHTML = month+" "+time.getDate();

        let timeToNewDay = ((23 - time.getHours()) * 3600000) + ((59 - time.getMinutes()) * 60000);
        setTimeout(() => {
            this.updateDate();
        }, timeToNewDay);
    }
    updateUptime() {
        let uptime = {
            raw: Math.floor(require("os").uptime()),
            days: 0,
            hours: 0,
            minutes: 0
        };

        uptime.days = Math.floor(uptime.raw/86400);
        uptime.raw -= uptime.days*86400;
        uptime.hours = Math.floor(uptime.raw/3600);
        uptime.raw -= uptime.hours*3600;
        uptime.minutes = Math.floor(uptime.raw/60);

        if (uptime.hours.toString().length !== 2) uptime.hours = "0"+uptime.hours;
        if (uptime.minutes.toString().length !== 2) uptime.minutes = "0"+uptime.minutes;

        document.querySelector("#mod_sysinfo > div:nth-child(2) > h2").innerHTML = uptime.days+":"+uptime.hours+":"+uptime.minutes;
    }
    updateBattery() {
        window.si.battery().then(bat => {
            //let indicator = document.querySelector("#mod_sysinfo > div:last-child > h2");
            if(!document.getElementById(window.idBattery)) return false;

            if (bat.hasbattery) {
               // let porcentCharge = document.querySelector("#lbl_porcentCharge");
                document.getElementById(window.idBattery).setAttribute("style", "");

                if (bat.ischarging) {
                    document.getElementById(window.idBattery + '_plug').setAttribute("style", "");
                     /*if(window.conectedBateryFlag == 0 &&  window.conectedBateryInit == 1)
                     {
                        window.audioManager.conectBatery.play();
                        window.conectedBateryFlag = 1;
                     }else{
                        window.conectedBateryFlag = 1;
                     }*/
                    if(bat.percent >= 100)
                    {
                       // porcentCharge.setAttribute("title","Charged - " + bat.percent+"%");
                        //indicator.innerHTML = "CHARGED";
                        document.getElementById(window.idBattery).setAttribute("title", "Battery: Charged - " + bat.percent+"%");
                        document.getElementById(window.idBattery + '_energy').setAttribute("width", ((window.maxBattery*bat.percent)/100));
                        document.getElementById("txt_porcentajeBat").style.right = '0.7vh';
                        document.getElementById("txt_porcentajeBat").style.fontSize = '1.3vh';
                        document.getElementById("txt_porcentajeBat").style.display = (window.settings.porcentajeBat)?'block':'none';
                        document.getElementById("txt_porcentajeBat").innerHTML = "<b>" + bat.percent+"%</b>";
                        document.getElementById("txt_porcentajeBat").setAttribute("title", "Battery: Charged - " + bat.percent+"%");
                    }
                    else
                    {
                        //porcentCharge.setAttribute("title","Charging - " + bat.percent+"%");
                        //indicator.innerHTML = "CHARGE";
                        if(bat.percent > window.batAlert){
                            document.getElementById(window.idBattery).classList.remove("parpadea");
                            document.getElementById("txt_porcentajeBat").classList.remove("parpadea");
                        }
                             
                        document.getElementById(window.idBattery).setAttribute("title", "Battery: Charging - " + bat.percent+"%");
                        document.getElementById(window.idBattery + '_energy').setAttribute("width", ((window.maxBattery*bat.percent)/100));
                        document.getElementById("txt_porcentajeBat").style.right = '0.3vh';
                        document.getElementById("txt_porcentajeBat").style.fontSize = '1.3vh';
                        document.getElementById("txt_porcentajeBat").style.display = (window.settings.porcentajeBat)?'block':'none';
                        document.getElementById("txt_porcentajeBat").innerHTML = "<b>" + bat.percent+"%</b>";
                        document.getElementById("txt_porcentajeBat").setAttribute("title", "Battery: Charging - " + bat.percent+"%");
                    }
                    //this.timeremainingConected = bat.timeremaining;
                    this.batteryFlag = true;
                    //indicator.setAttribute("class","");
                } else if ((bat.acconnected || bat.timeremaining === -1) && (!this.batteryFlag && !bat.hasbattery)) {
                    //porcentCharge.setAttribute("title","");
                    //indicator.innerHTML = "WIRED"; //pendiente proba quitando la bateria
                    document.getElementById(window.idBattery).setAttribute("style", "opacity: 0.5;");
                    document.getElementById(window.idBattery).setAttribute("title", "");
                    document.getElementById(window.idBattery + '_energy').setAttribute("width", 0);
                    document.getElementById("txt_porcentajeBat").setAttribute("title", "");
                    document.getElementById("txt_porcentajeBat").innerHTML = "";
                    document.getElementById("txt_porcentajeBat").style.display = 'none';
                    document.getElementById(window.idBattery).style.display = 'none';


                } else {
                    /*if(bat.timeremaining == this.timeremainingConected) 
                         document.getElementById(window.idBattery + '_plug').setAttribute("style", "");
                    else 
                         document.getElementById(window.idBattery + '_plug').setAttribute("style", "display: none;");   */

                    this.batteryFlag = true;
                    if(bat.percent >= 100)
                    {
                        //porcentCharge.setAttribute("title","Charged - 100%");
                        //indicator.innerHTML = "CHARGED";
                        //if(bat.timeremaining == this.timeremainingConected) 
                        //* ya no es necesario con las reglas incluidas en udev
                        //if(!window.upowerFlag){//evaluar si no afecta mucho el rendimiento del equipo.
                        if(document.getElementById(window.idBattery + '_plug').getAttribute("style") != ""){
                            require("fs").readFile("/sys/class/power_supply/AC/online","utf8",function(err, online){
                                if(online == 1)
                                    document.getElementById(window.idBattery + '_plug').setAttribute("style", "");
                            });                                
                            /*const { exec } = require("child_process");
                            let cmd = "upower -i $(upower -e | grep AC) | grep 'online.*yes'";
                            //window.upowerFlag = true;
                            exec(cmd, (error, stdout, stderr) => {
                                if(stdout != ''){
                                    document.getElementById(window.idBattery + '_plug').setAttribute("style", "");
                                }*//*else{
                                    document.getElementById(window.idBattery + '_plug').setAttribute("style", "display: none;");
                                }*/
                                //window.upowerFlag = false;

                            //});
                        }
                        
                        /*if(document.getElementById(window.idBattery + '_energy').getAttribute("width") == 0)
                            document.getElementById(window.idBattery + '_energy').setAttribute("width", window.maxBattery);

                        if(document.getElementById(window.idBattery + '_energy').getAttribute("width") == ((window.maxBattery*bat.percent)/100))
                            document.getElementById(window.idBattery + '_plug').setAttribute("style", "");
                        else
                             document.getElementById(window.idBattery + '_plug').setAttribute("style", "display: none;");*/
                        
                        document.getElementById(window.idBattery).setAttribute("title", "Battery: Charged - 100%");
                        document.getElementById(window.idBattery + '_energy').setAttribute("width", window.maxBattery);
                        document.getElementById("txt_porcentajeBat").style.right = '0.7vh';
                        document.getElementById("txt_porcentajeBat").style.fontSize = '1.3vh';
                        document.getElementById("txt_porcentajeBat").style.display = (window.settings.porcentajeBat)?'block':'none';
                        document.getElementById("txt_porcentajeBat").innerHTML = "<b>100%</b>";
                        document.getElementById("txt_porcentajeBat").setAttribute("title", "Battery: Charged - 100%");
                    }
                    else
                    {
                        //* ya no es necesario con las reglas incluidas en udev
                        //document.getElementById(window.idBattery + '_plug').setAttribute("style", "display: none;");
                        if(bat.percent <= window.batAlert){ //indicador de necesita cargar

                            //indicator.setAttribute("class","parpadea");
                            document.getElementById(window.idBattery).classList.add("parpadea");
                            document.getElementById("txt_porcentajeBat").classList.add("parpadea");

                            if(window.lowBateryFlag == 0 && bat.percent == window.batPlayAlert)
                             {
                                window.audioManager.lowBatery.play();
                                window.lowBateryFlag = 1;
                             }
                             else if(window.lowBateryFlag == 1 && bat.percent < window.batPlayAlert){
                                window.lowBateryFlag = 0;
                                window.batPlayAlert -= 5; 
                             } else if(window.lowBateryFlag == 0 && bat.percent < window.batPlayAlert){
                                window.batPlayAlert -= 5; 
                             }

                             if(bat.percent <= 5)
                                 systemAlertBatterylow(); 
                             else
                                 window.alertLowBattery = false;

                        }    
                        else{
                            //indicator.setAttribute("class","");
                            document.getElementById(window.idBattery).classList.remove("parpadea");
                            document.getElementById("txt_porcentajeBat").classList.remove("parpadea");
                        }

                        //porcentCharge.setAttribute("title","");
                        //indicator.innerHTML = bat.percent+"%";
                        document.getElementById(window.idBattery).setAttribute("title", "Battery: " + bat.percent+"%");
                        document.getElementById(window.idBattery + '_energy').setAttribute("width", ((window.maxBattery*bat.percent)/100));
                        document.getElementById("txt_porcentajeBat").style.right = '0.3vh';
                        document.getElementById("txt_porcentajeBat").style.fontSize = '1.3vh';
                        document.getElementById("txt_porcentajeBat").style.display = (window.settings.porcentajeBat)?'block':'none';
                        document.getElementById("txt_porcentajeBat").innerHTML = "<b>" + bat.percent + "%</b>";
                        document.getElementById("txt_porcentajeBat").setAttribute("title", "Battery: " + bat.percent+"%");

                       /* if(window.conectedBateryFlag == 1)
                         {
                            window.audioManager.conectBatery.play();
                            window.conectedBateryFlag = 0;
                         }
                         window.conectedBateryInit = 1;*/
                    }
                }
            } else {
                //indicator.innerHTML = "ON";
                document.getElementById(window.idBattery).setAttribute("style", "display: none;");
                window.batteryNone = 'display: none;';
            }
        });
    }
}

module.exports = {
    Sysinfo
};
