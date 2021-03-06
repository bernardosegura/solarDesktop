class Netstat {
    constructor(parentId) {
        if (!parentId) throw "Missing parameters";

        // Create DOM
        this.parent = parentId.split('|'); /*document.getElementById(parentId);*/
        //this.parent = document.getElementById('mod_column_right');
        /*this.parent.innerHTML += `<div id="mod_netstat">
            <div id="mod_netstat_inner">
                <h1>NETWORK STATUS<i id="mod_netstat_iname"></i></h1>
                <div id="mod_netstat_innercontainer">
                    <div>
                        <h1>STATE</h1>
                        <h2>UNKNOWN</h2>
                    </div>
                    <div id="${(window.settings.showIP !== false)?'true':'false'}">
                        <h1>IPv4</h1>
                        <h2>--.--.--.--</h2>
                    </div>
                    <div>
                        <h1>PING</h1>
                        <h2>--ms</h2>
                    </div>
                </div>
            </div>
        </div>`;*/
        this.paren_t = null;

        this.offline = false;
        //this.lastconn = {finished: true};
        this.iface = null;
        this.iconWired = require("./assets/icons/file-icons.json").wired;
        this.iconWireless = require("./assets/icons/file-icons.json").wireless;
        /*this._httpsAgent = new require("https").Agent({
            keepAlive: false,
            maxSockets: 10
        });*/

        // Init updaters
        this.updateInfo();
        this.infoUpdater = setInterval(() => {
            this.updateInfo();
        }, 2000);
    }
    updateInfo() {
        window.si.networkInterfaces().then(async data => {

            let offline = false;

            let net = data[0];
            let netID = 0;

            if(!document.getElementById(this.parent[0]) && !document.getElementById(this.parent[1]) && !document.getElementById(this.parent[2]))
                return false;

            if(window.objRedId != ''){
                if(document.getElementById(window.objRedId))
                    this.paren_t = document.getElementById(window.objRedId);
                 else{
                     window.objRedId = '';
                     window.objRedTitle = '';
                     window.ssidWifi = '';
                     return false;
                 }

             }else{
                if(document.getElementById(this.parent[0])){
                    window.objRedId = this.parent[0];
                    this.paren_t = document.getElementById(this.parent[0]);
                    window.objRedTitle = this.paren_t.getAttribute("title");
                }else{
                    if(document.getElementById(this.parent[1])){
                        window.objRedId = this.parent[1];
                        this.paren_t = document.getElementById(this.parent[1]);
                        window.objRedTitle = this.paren_t.getAttribute("title");
                    }else{
                        if(document.getElementById(this.parent[2])){
                            window.objRedId = this.parent[2];
                            this.paren_t = document.getElementById(this.parent[2]);
                            window.objRedTitle = this.paren_t.getAttribute("title");
                        }
                    }
                }
             }

//quitamos la interfaz de la configuracion
            /*if (typeof window.settings.iface === "string") {
                while (net.iface !== window.settings.iface) {
                    netID++;
                    if (data[netID]) {
                        net = data[netID];
                    } else {
                        // No detected interface has the custom iface name, fallback to automatic detection on next loop
                        window.settings.iface = false;
                        this.paren_t.setAttribute("style", "opacity: 0.5");
                        //sufre al renderizar los saltos d elinea la primera vez
                        //this.paren_t.setAttribute("title",window.objRedTitle + "\nSatus: Offline");
                        this.paren_t.setAttribute("title",window.objRedTitle);
                        window.ssidWifi = '';
                        return false;
                    }
                }
            } else {*/
                // Find the first external, IPv4 connected networkInterface that has a MAC address set

                while (net.operstate !== "up" || net.internal === true || net.ip4 === "" || net.mac === "") {
                    netID++;
                    if (data[netID]) {
                        net = data[netID];
                    } else {
                        // No external connection!
                        this.iface = null;
                        //document.getElementById("mod_netstat_iname").innerText = "Interface: (offline)";
                        

                        //sufre al renderizar los saltos d elinea la primera vez
                        //this.paren_t.setAttribute("title",window.objRedTitle + "\nSatus: Offline");
                        

                        this.offline = true;
                        offline = true;
                        /*document.querySelector("#mod_netstat_innercontainer > div:first-child > h2").innerHTML = "OFFLINE";
                        document.querySelector("#mod_netstat_innercontainer > div:nth-child(2) > h2").innerHTML = "--.--.--.--";
                        document.querySelector("#mod_netstat_innercontainer > div:nth-child(3) > h2").innerHTML = "--ms";*/
                        break;
                    }
                }
            //}

            if(offline){
                this.paren_t.setAttribute("style", "opacity: 0.5");                    
                this.paren_t.setAttribute("title",window.objRedTitle);
                window.ssidWifi = '';
                return false;
            }

            this.iface = net.iface;
            this.internalIPv4 = net.ip4;
            //document.getElementById("mod_netstat_iname").innerText = "Interface: "+net.iface;
            //sufre al renderizar los saltos d elinea la primera vez
            //this.paren_t.setAttribute("title",window.objRedTitle + "\nInterface: " + net.iface);
            if(window.ssidWifi != '')
                 this.paren_t.setAttribute("title",window.objRedTitle + " - Interface: " + net.iface + ", " +  window.ssidWifi);
            else
               this.paren_t.setAttribute("title",window.objRedTitle + " - Interface: " + net.iface); 

            window.si.networkStats(net.iface).then(data => {
                
                        //if(data[0].operstate !== "up"){ //tambien probar rx_sec: -1 y tx_sec: -1
                        if(data[0].rx_sec === -1 && data[0].tx_sec === -1 ){    
                            this.offline =  true;
                            this.paren_t.setAttribute("style", "opacity: 0.5");                    
                            this.paren_t.setAttribute("title",window.objRedTitle);
                            window.ssidWifi = '';
                           
                        }else{
                             this.offline = false;
                             this.paren_t.setAttribute("style", "");

                            if(window.ssidWifi == '' && net.type == "wireless"){
                                const { exec } = require("child_process");
                                let cmd = "nmcli -t -f NAME connection show --active";//"nmcli -t -f active,ssid dev wifi | cut -d\\' -f2 | grep '^yes*\\|^si*'"; // funciona pero la nueva esta mejor
                                
                                document.querySelector("#" + this.paren_t.id + " > svg").innerHTML = this.iconWireless.svg;
                                exec(cmd, (error, stdout, stderr) => {

                                    if(stdout != ''){
                                        //let out = stdout.split(':');
                                        window.ssidWifi = stdout;//out[out.length - 1];
                                        this.paren_t.setAttribute("title",this.paren_t.getAttribute("title") + ", " +  window.ssidWifi);
                                    }else{
                                        this.paren_t.setAttribute("style", "opacity: 0.5");                 
                                        this.paren_t.setAttribute("title",window.objRedTitle);
                                        window.ssidWifi = '';
                                     //console.log(stderr); //contenido-->> /bin/sh: 1: nmcl: not found
                                    }

                                });
                            }else{
                                if(net.type =="wired"){
                                    document.querySelector("#" + this.paren_t.id + " > svg").innerHTML = this.iconWired.svg;
                                }
                                
                            }
                            if(window.ifaceNet.icon != net.type){
                                if(net.type == "wireless")
                                    window.ifaceNet.icon = net.type;
                                else
                                   window.ifaceNet.icon = "wired";
                               
                                require("fs").writeFileSync(window.ifaceNetFile, JSON.stringify(window.ifaceNet, "", 4));
                            }

                        }    
                    });

            
            

            /*if (net.ip4 === "127.0.0.1") {
                offline = true;
            } else {*/
                //https://freegeoip.app/ para mostrar la ip en la seccion Try it yourself: solo precionando el boton
                /*if (this.lastconn.finished) {
                    this.lastconn = require("https").get({host: "freegeoip.app", port: 443, path: "/json/", localAddress: net.ip4, agent: this._httpsAgent}, res => {
                        let rawData = "";
                        res.on("data", chunk => {
                            rawData += chunk;
                        });
                        res.on("end", () => {
                            try {
                                let data = JSON.parse(rawData);
                                this.ipinfo = {
                                    ip: data.ip,
                                    geo: {
                                        latitude: data.latitude,
                                        longitude: data.longitude,
                                        metro_code: data.zip_code,
                                        time_zone: data.time_zone
                                    }
                                };

                                // if (!this.ipinfo.api_version.startsWith("3")) console.warn("Warning: ipinfo API version might not be compatible");

                                // delete this.ipinfo.api_version;
                                // delete this.ipinfo.time;
                                this.paren_t.setAttribute("title",this.paren_t.getAttribute("title") + "\nIP: "+window._escapeHtml(this.ipinfo.ip));*/

                                /*let ip = this.ipinfo.ip;
                                
                                if(document.querySelector("#mod_netstat_innercontainer > div:nth-child(2)").id == 'true')
                                    document.querySelector("#mod_netstat_innercontainer > div:nth-child(2) > h2").innerHTML = window._escapeHtml(ip);
                                else
                                    document.querySelector("#mod_netstat_innercontainer > div:nth-child(2) > h2").innerHTML = "XX.XX.XX.XX";*/

                            /*} catch(e) {
                                console.warn(e);
                                console.info(rawData.toString());
                                let electron = require("electron");
                                electron.ipcRenderer.send("log", "note", "NetStat: Error parsing data from ipinfo.now.sh");
                                electron.ipcRenderer.send("log", "debug", `Error: ${e}`);
                            }
                        });
                    }).on("error", e => {
                        // Drop it
                    });
                }*/

                //let p = await this.ping(window.settings.pingAddr || "1.1.1.1", 80, net.ip4).catch(() => { offline = true });
                


                /*this.offline = offline;
                if (offline) {*/
                    /*document.querySelector("#mod_netstat_innercontainer > div:first-child > h2").innerHTML = "OFFLINE";
                    document.querySelector("#mod_netstat_innercontainer > div:nth-child(2) > h2").innerHTML = "--.--.--.--";
                    document.querySelector("#mod_netstat_innercontainer > div:nth-child(3) > h2").innerHTML = "--ms";*/
                //    this.paren_t.setAttribute("style", "opacity: 0.5");
                    //sufre al renderizar los saltos d elinea la primera vez
                    //this.paren_t.setAttribute("title",this.paren_t.getAttribute("title") + "\nSatus: Offline");
               /*      this.paren_t.setAttribute("title",window.objRedTitle);
                    window.ssidWifi = '';
                } else {
                    this.paren_t.setAttribute("style", "");
                    //this.paren_t.setAttribute("title",this.paren_t.getAttribute("title") + "\nSatus: Online");

                    if(window.ssidWifi == '' && net.type == "wireless"){
                        const { exec } = require("child_process");
                        let cmd = "nmcli -t -f NAME connection show --active";//"nmcli -t -f active,ssid dev wifi | cut -d\\' -f2 | grep '^yes*\\|^si*'"; // funciona pero la nueva esta mejor
                        exec(cmd, (error, stdout, stderr) => {

                            if(stdout != ''){
                                //let out = stdout.split(':');
                                window.ssidWifi = stdout;//out[out.length - 1];
                                this.paren_t.setAttribute("title",this.paren_t.getAttribute("title") + ", " +  window.ssidWifi);
                            }else{
                                this.paren_t.setAttribute("style", "opacity: 0.5");                 
                                this.paren_t.setAttribute("title",window.objRedTitle);
                                window.ssidWifi = '';
                             //console.log(stderr); //contenido-->> /bin/sh: 1: nmcl: not found
                            }

                        });
                    }*/

                    /*document.querySelector("#mod_netstat_innercontainer > div:first-child > h2").innerHTML = "ONLINE";
                    document.querySelector("#mod_netstat_innercontainer > div:nth-child(3) > h2").innerHTML = ((p != -1)?Math.round(p)+"ms":"OFF");*/
                //}
            //}
        });
    }
    /*ping(target, port, local) {
        return new Promise((resolve, reject) => {
            
            if(window.settings.enablePing)
            {
                let s = new require("net").Socket();
                let start = process.hrtime();
                
                s.connect({
                port,
                host: target,
                localAddress: local,
                family: 4
                }, () => {
                    let time_arr = process.hrtime(start);
                    let time = (time_arr[0] * 1e9 + time_arr[1]) / 1e6;
                    resolve(time);
                    s.destroy();
                });
                s.on('error', e => {
                    s.destroy();
                    reject(e);
                });
                s.setTimeout(1900, function() {
                    s.destroy();
                    reject(new Error("Socket timeout"));
                });
            }
            else
            {
                resolve(-1);
            }
        });
    }*/
}

module.exports = {
    Netstat
};
