class Sysinfo {

    constructor(parentId) {
        window.conectedBateryFlag = 0;
        window.conectedBateryInit = 0;
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

        // Create DOM
        this.parent = document.getElementById(parentId);
        //quitamos el type por que todo sera Linux
        this.parent.innerHTML += `<div id="mod_sysinfo">
            <div>
                <h1>1970</h1>
                <h2>JAN 1</h2>
            </div>
            <div>
                <h1>UPTIME</h1>
                <h2>0:0:0</h2>
            </div>
            <div ${(!window.wm)?'style="display:none;"':''}>
                <h1 title="Opened Native Window">WND</h1>
                <h2 title="Opened Native Window">0</h2>
            </div>
            <div id="lbl_porcentCharge" title="">
                <h1>POWER</h1>
                <h2>00%</h2>
            </div>
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

        window.si.battery().then(bat => {
            if (bat.hasbattery) {
                this.playConectBattery();

                this.batteryConectPlay = setInterval(() => {
                    this.playConectBattery();
                }, 1000); 
            } 
        });
        

    }

    playConectBattery(){
            this.isCharging().then(result => {
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
                
            });
    }

    updateDate() {
        let time = new Date();

        document.querySelector("#mod_sysinfo > div:first-child > h1").innerHTML = time.getFullYear();

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
            let indicator = document.querySelector("#mod_sysinfo > div:last-child > h2");
            if (bat.hasbattery) {
                let porcentCharge = document.querySelector("#lbl_porcentCharge");

                if (bat.ischarging) {
                     /*if(window.conectedBateryFlag == 0 &&  window.conectedBateryInit == 1)
                     {
                        window.audioManager.conectBatery.play();
                        window.conectedBateryFlag = 1;
                     }else{
                        window.conectedBateryFlag = 1;
                     }*/
                    if(bat.percent >= 100)
                    {
                        porcentCharge.setAttribute("title","Charged - " + bat.percent+"%");
                        indicator.innerHTML = "CHARGED";
                    }
                    else
                    {
                        porcentCharge.setAttribute("title","Charging - " + bat.percent+"%");
                        indicator.innerHTML = "CHARGE";
                    }
                    indicator.setAttribute("class","");
                } else if (bat.acconnected || bat.timeremaining === -1) {
                    porcentCharge.setAttribute("title","");
                    indicator.innerHTML = "WIRED";

                } else {
                    if(bat.percent >= 100)
                    {
                        porcentCharge.setAttribute("title","Charged - 100%");
                        indicator.innerHTML = "CHARGED";
                    }
                    else
                    {
                        if(bat.percent <= window.batAlert){ //indicador de necesita cargar

                            indicator.setAttribute("class","parpadea");

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

                        }    
                        else{
                            indicator.setAttribute("class","");
                        }

                        porcentCharge.setAttribute("title","");
                        indicator.innerHTML = bat.percent+"%";

                       /* if(window.conectedBateryFlag == 1)
                         {
                            window.audioManager.conectBatery.play();
                            window.conectedBateryFlag = 0;
                         }
                         window.conectedBateryInit = 1;*/
                    }
                }
            } else {
                indicator.innerHTML = "ON";
            }
        });
    }
}

module.exports = {
    Sysinfo
};
