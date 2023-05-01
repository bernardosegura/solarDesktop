use std::env;
use std::fs;
use ansi_term::Color;
use ansi_term::Style;
use std::path::Path;
use std::os::unix::fs::symlink;

fn main() {
    let argv: Vec<String> = env::args().collect();
    let opc = ["list", "add", "rm"];
    let mut home: String = "".to_string();

    if let Some(home_dir) = env::var_os("HOME") {    
        home = String::from(home_dir.to_string_lossy()) + "/modulos";
    }else {
        println!("Undefined variable HOME ($HOME)");
    }

    if argv.len() > 1 {
        let argmt: &str = &argv[1];
        if opc.contains(&argmt){
             match argmt {
                "list" => list(home),//println!("Ejecutando Comando {}",&argmt),
                "add" => add(home),
                "rm" => rm(home),
                _ => como_usar(),
            } 
        }else{
            como_usar();
        }
        return;
    }else{
        como_usar();
        return;
    }
}

fn como_usar(){
    let bold = Style::new().bold();
    let underline = Style::new().underline();
    println!("");
    println!("      -------Config Group Apps v0.1 - Tools Solar-------");
    println!("     |                                                  |");
    println!("     |                        Used                      |");
    println!("     |                                                  |");
    println!("      --------------------------------------------------");
    println!("     | {} {} -> List to applications of grups      |",bold.paint("cgapp"),underline.paint("list"));
    println!("     |                                                  |");
    println!("     | {} {} -> Add to applications of grups        |",bold.paint("cgapp"),underline.paint("add"));
    println!("     |                                                  |");
    println!("     | {} {} -> Remove to applications of grups      |",bold.paint("cgapp"),underline.paint("rm"));
    println!("      --------------------------------------------------");
    println!("");
}

fn list(home: String){
    //let pwd = ".";
    let bold = Style::new().bold();
    let underline = Style::new().underline();
    let mut index: i32 = 0;
    println!("Config Group Apps v0.1 - Tools Solar");
    println!("");
    println!("      -----------------Format----------------");
    println!("     | index.- Application -> Path -> Active |");
    println!("      ---------------------------------------");
    println!("");
    
    if let Ok(entries) = fs::read_dir(home){
        for entry in entries{
            if let Ok(entry) = entry {
                 if let Ok(metadata) = fs::read_link(entry.path()) {
                    index += 1;
                    print!("{}.- {} -> {} -> ",index,bold.paint(entry.file_name().to_string_lossy()),underline.paint(metadata.as_path().to_string_lossy()));  
                    if metadata.as_path().exists() {
                        println!("{}",Color::Green.paint("TRUE"));
                    }
                    else{
                        println!("{}",Color::Red.paint("FALSE"));                        
                    }
                 }
            }
        }
    }
    println!("");
}

fn add(home: String){
    let pwd = ".";
    let mut index: i32 = 0;
    println!("Config Group Apps v0.1 - Tools Solar");
    println!("");
    println!("      -------------------add-----------------");
    println!("     |                Adding...              |");
    println!("      ---------------------------------------");
    println!("");
    
    if let Ok(entries) = fs::read_dir(pwd){
        for entry in entries{
            if let Ok(entry) = entry { 
                if let Some(ext) = entry.path().extension(){
                    if ext == "xobj" {
                        let hflnk = format!("{}/{}", home, entry.file_name().to_string_lossy());
                        let path_hflnk = Path::new(&hflnk);
                        if !path_hflnk.exists(){
                            if let Ok(current_dir) = env::current_dir() {
                               let target = format!("{}/{}", current_dir.display(), entry.file_name().to_string_lossy()); 
                               index += 1;
                               //print!("{} {} --> {}......",index, path_hflnk,target); 
                               print!("{}.- {} ...... ",index, path_hflnk.to_string_lossy()); 
                               if let Ok(_result) = symlink(target, path_hflnk){
                                  println!("Added");
                               }else {
                                  println!("--");
                               }
                            }  
                        }
                    }
                }
            }
        }
    }
    println!("");
}

fn rm(home: String){
    let mut index: i32 = 0;
    println!("Config Group Apps v0.1 - Tools Solar");
    println!("");
    println!("      -------------------rm------------------");
    println!("     |                Removing...            |");
    println!("      ---------------------------------------");
    println!("");
    
    if let Ok(entries) = fs::read_dir(home){
        for entry in entries{
            if let Ok(entry) = entry {
                 if let Ok(metadata) = fs::read_link(entry.path()) { 
                    if !metadata.as_path().exists() { 
                       index += 1; 
                       print!("{}.- {} ...... ",index,entry.file_name().to_string_lossy());
                       if let Ok(_result) = fs::remove_file(entry.path()){
                          println!("Removed");
                       }else {
                          println!("--");
                       }
                    }
                 }
            }
        }
    }
    println!("");
}