class Clock {
    constructor(parentId) {
        if (!parentId) throw "Missing parameters";

        // Create DOM
        this.parent = document.getElementById(parentId);
        let tAdmin = 'mate-time-admin';
        let execTime = `onclick="timeAdmin('${tAdmin}')" style="cursor: pointer;" title="Time Admin"`;
        let which = require("child_process").execSync("which " + tAdmin + ' | wc -l').toString();
        let time = (parseInt(which) != 0)? true:false;
        
        this.parent.innerHTML += `<div id="mod_clock">
            <h1 id="mod_clock_text" ${(time)?execTime:''}><span>?</span><span>?</span><span>:</span><span>?</span><span>?</span><span>:</span><span>?</span><span>?</span></h1>
        </div>`;

        this.lastTime = new Date();

        this.updateClock();
        this.updater = setInterval(() => {
            this.updateClock();
        }, 1000);
    }
    updateClock() {
        let time = new Date();
        let array = [time.getHours(), time.getMinutes(), time.getSeconds()];

        array.forEach((e, i) => {
            if (e.toString().length !== 2) {
                array[i] = "0"+e;
            }
        });

        if(window.settings.ampm){
            var hora = parseInt(array[0]);
             array[2] = (hora < 12)? "AM": "PM";
             array[0] = (hora <= 12)? array[0]: (hora - 12);
             if (array[0].toString().length !== 2) {
                array[0] = "0"+ array[0];
            }
        }

        let clockString = `${array[0]}:${array[1]}:${array[2]}`;
        array = clockString.match(/.{1}/g);
        clockString = "";
        array.forEach(e => {
            if (e === ":") clockString += "<em>"+e+"</em>";
            else clockString += "<span>"+e+"</span>";
        });
        document.getElementById("mod_clock_text").innerHTML = clockString;
        this.lastTime = time;
    }
}

module.exports = {
    Clock
};
