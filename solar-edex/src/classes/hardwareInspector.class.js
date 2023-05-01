class HardwareInspector {
    constructor(parentId) {
        if (!parentId) throw "Missing parameters";

        this.produc_name = ["butterfly", "link", "lumpy", "parrot", "stout", "stumpy", "falco", "leon", "mccloud", "monroe", "panther", "peppy", "tricky", "wolf", "zako", "auron_paine", "auron_yuna", "buddy", "gandof", "guado", "lulu", "rikku", "samus", "tidus", "banjo", "candy", "clapper", "enguarde","glimmer", "gnawty", "heli", "kip", "ninja", "orco", "quawks", "squawks", "sumo","swanky","winky", "banon", "celes", "cyan", "edgar", "kefka", "reks","relm", "setzer", "terra", "terra13", "ultima", "wizpig", "asuka", "caroline", "cave","chell", "lars", "sentry",  "astronaut","babymega", "babytiger", "blacktip", "blue", "bruce", "electro", "epaulette", "lava", "nasher","nasher360","pyro","rabbid","reef", "robo", "robo360", "sand", "santa", "snappy", "whitetip",  "akali", "akali360","atlas","bard","ekko", "eve","excelsior","jax","karma", "kench","leona","nautilus","nocturne", "pantheon","shyvana","sion","sona", "soraka","syndra","teemo","vayne", "wukong", "ampton", "apel","bloog","blooglet","blooguard", "blorb","bluebird","bobba","bobba360", "casta","dood","dorp","droid", "fleex","foob","foob360","garg", "garg360","garfour","glk","glk360", "grabbiter","laser","laser14","lick", "meep","mimrock","nospike","orbatrix", "phaser","phaser360","phaser360s","sparky", "sparky360","vorticon","vortininja", "aleena", "barla","careena","kasumi","kasumi360", "liara","treeya","treeya360", "arcada", "sarien", "akemi","dragonair","drallion", "dratini","duffy","faffy","helios", "jinlon","kaisa","kindred","kled", "kohaku","nightfury","noibat","wyvern", "berknip","dirinboz","ezkinil","gumboz", "morphius","vilboz","vilboz14","vilboz360", "woomax", "chronicler","collis","copano","delbin", "drobit","eldrid","lillipup","lindar", "voema","chronicler","volet","volta","voxel",  "beetley","blipper","bookem","boten", "botenflex","bugzzy","cret","cret360", "drawcia","drawlat","drawman","drawper", "galith","galith360","gallop","galnat", "galnat360","galtic","galtic360","kracko","kracko360","landia","landrid", "lantis","madoo","magister","maglet", "maglia","maglith","magma","magneto", "magolor","magpie","metaknight","pasara", "pirette", "pirika","sasuke","storo","storo360", "anahera","banshee","crota","crota360", "felwinter","kano","mithrax","osiris", "primus","redrix","taniks","taeko", "volmar","zavala"];
        this.produc_name_pixel = ["atlas","eve","nocturne"];

        // Create DOM
        this.parent = document.getElementById(parentId);
        this._element = document.createElement("div");
        this._element.setAttribute("id", "mod_hardwareInspector");
        this._element.innerHTML = `<div id="mod_hardwareInspector_inner">
            <div>
                <h1>MANUFACTURER</h1>
                <h2 id="mod_hardwareInspector_manufacturer" >NONE</h2>
            </div>
            <div>
                <h1>MODEL</h1>
                <h2 id="mod_hardwareInspector_model" >NONE</h2>
            </div>
            <div>
                <h1>CHASSIS</h1>
                <h2 id="mod_hardwareInspector_chassis" >NONE</h2>
            </div>
        </div>`;

        this.parent.append(this._element);

        this.updateInfo();
        this.infoUpdater = setInterval(() => {
            this.updateInfo();
        }, 20000);
    }
    updateInfo() {
        window.si.system().then(d => {
            window.si.chassis().then(e => {
                document.getElementById("mod_hardwareInspector_manufacturer").innerText = this._trimDataString(d.manufacturer);
                document.getElementById("mod_hardwareInspector_model").innerText = this._trimDataString(d.model, d.manufacturer, e.type);
                document.getElementById("mod_hardwareInspector_chassis").innerText = e.type;
                if(window.isChromeOS == null){
                    if(this.produc_name.indexOf(this._trimDataString(d.model, d.manufacturer, e.type).toLowerCase()) !== -1){
                        window.isChromeOS = true;
                    }else
                       window.isChromeOS = false;                       
                   window.registerKeyboardShortcuts();
                   
                   if(this.produc_name_pixel.indexOf(this._trimDataString(d.model, d.manufacturer, e.type).toLowerCase()) !== -1){
                        require("child_process").exec('xmodmap -e "keycode 191 = Menu" &>/dev/null', (error, stdout, stderr) => {});//para usar la tecla menu como menu, no pasa por wm ya que no se pudo capturar el evento.
                    } 
                }    
            });
        });
    }
    _trimDataString(str, ...filters) {
        return str.trim().split(" ").filter(word => {
            if (typeof filters !== "object") return true;

            return !filters.includes(word);
        }).slice(0, 2).join(" ");
    }
}

module.exports = {
    HardwareInspector
};
