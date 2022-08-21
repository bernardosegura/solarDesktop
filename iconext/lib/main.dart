import 'package:flutter/material.dart';
import 'dart:io';

import 'package:path_provider/path_provider.dart';

import "package:path/path.dart" show /*dirname,*/ basename, basenameWithoutExtension;

import 'dart:convert'; // para el jsonDecode,jsonEncode...

import 'package:xml/xml.dart';
import 'package:file_picker_desktop/file_picker_desktop.dart'; 
import 'package:flutter_svg/flutter_svg.dart';
import 'package:flutter/services.dart'; //TextInputFormatter y FilteringTextInputFormatter
import 'package:flutter_window_close/flutter_window_close.dart';
//flutter pub add flutter_window_close

Future<void> main() async {
  final directory = await getApplicationDocumentsDirectory();// del usuario
  final fileSetting = new File(directory.path + "/.config/Solar_eDEX/settings.json");
  Map obj = {};
  Map _temaColor = {"r":0,"g":0,"b":0};
  try {
      obj = await jsonDecode(fileSetting.readAsStringSync());
      final fileTheme = new File(directory.path + "/.config/Solar_eDEX/themes/"+obj["theme"]+".json");
      obj = await jsonDecode(fileTheme.readAsStringSync());

      _temaColor['r'] = obj['colors']['r'];
      _temaColor['g'] = obj['colors']['g'];
      _temaColor['b'] = obj['colors']['b'];
      
      runApp(/*const*/ MyApp(temaColor:_temaColor));
   }catch(e) {
      print(e);
   }
}

class MyApp extends StatelessWidget {
  final Map temaColor;
  const MyApp({required this.temaColor, Key? key}) : super(key: key);
  

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context){

  final hslColor = HSLColor.fromColor(Color.fromRGBO(temaColor['r'], temaColor['g'], temaColor['b'], 1));
  int colorHexa = int.parse('FF' + (temaColor['r']).toRadixString(16).padLeft(2, '0') + (temaColor['g']).toRadixString(16).padLeft(2, '0') + (temaColor['b']).toRadixString(16).padLeft(2, '0'), radix: 16);
  final lightness = hslColor.lightness;

  /// if [500] is the default color, there are at LEAST five
  /// steps below [500]. (i.e. 400, 300, 200, 100, 50.) A
  /// divisor of 5 would mean [50] is a lightness of 1.0 or
  /// a color of #ffffff. A value of six would be near white
  /// but not quite.
  final lowDivisor = 6;

  /// if [500] is the default color, there are at LEAST four
  /// steps above [500]. A divisor of 4 would mean [900] is
  /// a lightness of 0.0 or color of #000000
  final highDivisor = 5;

  final lowStep = (1.0 - lightness) / lowDivisor;
  final highStep = lightness / highDivisor;

  Map<int, Color> colorCodes = {
     50: (hslColor.withLightness(lightness + (lowStep * 5))).toColor(),
    100: (hslColor.withLightness(lightness + (lowStep * 4))).toColor(),
    200: (hslColor.withLightness(lightness + (lowStep * 3))).toColor(),
    300: (hslColor.withLightness(lightness + (lowStep * 2))).toColor(),
    400: (hslColor.withLightness(lightness + lowStep)).toColor(),
    500: (hslColor.withLightness(lightness)).toColor(),
    600: (hslColor.withLightness(lightness - highStep)).toColor(),
    700: (hslColor.withLightness(lightness - (highStep * 2))).toColor(),
    800: (hslColor.withLightness(lightness - (highStep * 3))).toColor(),
    900: (hslColor.withLightness(lightness - (highStep * 4))).toColor(),
  };

  MaterialColor tema = new MaterialColor(colorHexa, colorCodes);  

  return MaterialApp(
      title: 'IconExt',
      theme: ThemeData(
        
        primarySwatch: tema, //Colors.blue,
      ),
      home: const MyHomePage(title: 'IconExt v0.1 - Icon set added'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({Key? key, required this.title}) : super(key: key);

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  int _counter = 0;
  final List<Widget> _painters = <Widget>[];
  final List<String> nameIcons = <String>[];
  final List<String> nameIconsBuff = <String>[];


Future<String> localPath() async {
  final directory = await getApplicationDocumentsDirectory();// del usuario
    return (directory.path + /*"\\"*/"/.config/Solar_eDEX/");
}

void abrirArchivo() async {
  final result = await pickFiles(
    //dialogTitle: 'Please select an input file:',
    type: FileType.custom,
    allowedExtensions: ['svg','json']);
  if (result == null) return;

  String fileExt = result.files.single.path.toString().substring(result.files.single.path.toString().lastIndexOf('.'));
  
  if(fileExt.toLowerCase() == '.svg')
     abrirSVG(result.files.single.path.toString());
  else
    setState(() {
      abrirJSON(result.files.single.path.toString());
    });
}

void abrirSVG(String svg) {
  
   final file = new File(svg); //(dirname(Platform.script.toFilePath()) + '\\flutter.svg');
   Map obj_svg = {};
   try {
      final document = XmlDocument.parse(file.readAsStringSync());
      final svg = document.findAllElements('svg');

      final filename = basenameWithoutExtension(file.path); //basename(file.path);
      /*svg.map((node) => node.text)
        .forEach(print);*/
       final tamanio = svg.map((node) => node.getAttribute('viewBox')).toString().replaceAll(")","").split(" "); 
       final path = document.findAllElements('path');//svg.map((node) => node.findAllElements('path'));//.map((node) => node.getAttribute('d')).toString(); 
       final ancho = tamanio[2], alto = tamanio[3];
       final d = path.map((node) => node.getAttribute('d')).toString().replaceAll("(","").replaceAll(")","");

       //obj_svg = jsonDecode('{"$filename":{"width": $ancho,"height": $alto,"svg": "<path d=\\"$d\\"/>"}}');
       obj_svg = jsonDecode('{"width": $ancho,"height": $alto,"svg": "<path d=\\"$d\\"/>"}');
       if(!obj.containsKey(filename)){
            obj[filename] = obj_svg;
            setState(() {
              changed = true; 
              var index = _painters.length == 0 ? 0 : _painters.length;
              var _controller = TextEditingController();
            _painters.add(SvgPicture.string('''<svg viewBox="0 0 $ancho $alto"
                  xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">
                  <path d="$d"/>
                </svg>'''));

            nameIcons.add(filename);
            nameIconsBuff.add(filename);

                _painters.add(Column( crossAxisAlignment: CrossAxisAlignment.start,
                children: <Widget>[
                  Text('$filename'),
                  TextField(
                    controller: _controller,
                    inputFormatters: <TextInputFormatter>[
                      FilteringTextInputFormatter.allow(RegExp(r'[A-Za-z0-9_-]')),
                      ],
                    decoration: InputDecoration(
                      //border: InputBorder.none,
                      hintText: '$filename',
                      suffixIcon: IconButton(
                        onPressed: _controller.clear,
                        icon: Icon(Icons.clear),
                      )
                    ),
                    onChanged: (value){
                      setState(() {
                          nameIcons[(index/2).toInt()] = (value != ''?value:filename);                  
                        });
                    },
                  ),            
                  const SizedBox(
                    height: 5,
                  ),
                  Row(
                    children: <Widget>[
                    IconButton(
                      icon: const Icon(Icons.save),
                      tooltip: 'New name',
                      onPressed: () {
                        cambiarNombre(index);
                      },
                    ),
                  IconButton(
                      icon: const Icon(Icons.delete),
                      tooltip: 'Delete',
                      onPressed: () => eliminarIcono(index),
                    ),]),            
                ]),
                );
          });
       }
    } catch (e) {
      print(e);
       showDialog<String>(
        context: context,
        builder: (BuildContext context) => AlertDialog(
          title: const Text('Error'),
          content: Text(basename(file.path)+': $e'),
          actions: <Widget>[
            TextButton(
              onPressed: () => Navigator.pop(context, 'OK'),
              child: const Text('OK'),
            )
          ],
        ),
      );
    }
   
  //print(jsonEncode(obj_svg));

}

 void jsonInit(){
  //https://pub.flutter-io.cn/packages/flutter_svg/example
    //flutter pub add flutter_svg
    abrirJSON(dir + 'iconext.json'); 
    //final file = new File(dir + 'iconext.json');
    //obj = jsonDecode(file.readAsStringSync());
    setState(() {
            changed = false;
      });
 }

 void guardar(){

  final file = File(dir + 'iconext.json');

  file.writeAsString(jsonEncode(obj));

  showDialog<String>(
        context: context,
        builder: (BuildContext context) => AlertDialog(
          title: const Text('Save'),
          content: Text('Successful!'),
          actions: <Widget>[
            TextButton(
              onPressed: () => Navigator.pop(context, 'OK'),
              child: const Text('OK'),
            )
          ],
        ),
      );      
  // print(obj.keys.toList());
 }

 void abrirJSON(String json){
  //https://pub.flutter-io.cn/packages/flutter_svg/example
    //flutter pub add flutter_svg
    
  final file = new File(json);
  Map obj_icon = {};
  try {
      obj_icon = jsonDecode(file.readAsStringSync());
      obj_icon..forEach((key, value){

         if(!obj.containsKey(key)){
            obj[key] = value;
            var width = value['width'];
            var height = value['height'];
            var svg = value['svg'];
            var index = _painters.length == 0 ? 0 : _painters.length;
            var _controller = TextEditingController();
  
            setState(() {
              changed = true;
            });
          
            _painters.add(SvgPicture.string('''<svg viewBox="0 0 $width $height"
              xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">
              $svg
            </svg>'''));

            nameIcons.add(key);
            nameIconsBuff.add(key);

            _painters.add(Column( crossAxisAlignment: CrossAxisAlignment.start,
            children: <Widget>[
              Text('$key'),
              TextField(
                controller: _controller,
                //keyboardType: TextInputType.number,
                inputFormatters: <TextInputFormatter>[
                  FilteringTextInputFormatter.allow(RegExp(r'[A-Za-z0-9_-]')),
                ],
                decoration: InputDecoration(
                  //border: InputBorder.none,
                  hintText: '$key',
                  suffixIcon: IconButton(
                    onPressed: _controller.clear,
                    icon: Icon(Icons.clear),
                  )
                ),
                onChanged: (value){
                  setState(() {
                      nameIcons[(index/2).toInt()] = (value != ''?value:key);                  
                    });
                }
              ),            
              const SizedBox(
                height: 5,
              ),
              Row(
                children: <Widget>[
                IconButton(
                  icon: const Icon(Icons.save),
                  tooltip: 'New name',
                  onPressed: () {cambiarNombre(index);},
                ),
              IconButton(
                  icon: const Icon(Icons.delete),
                  tooltip: 'Delete',
                  onPressed: () => eliminarIcono(index),
                ),]),            
            ]),
            );
         }
        });

    } catch (e) {
      print(e);
       showDialog<String>(
        context: context,
        builder: (BuildContext context) => AlertDialog(
          title: const Text('Error'),
          content: Text('$e'),
          actions: <Widget>[
            TextButton(
              onPressed: () => Navigator.pop(context, 'OK'),
              child: const Text('OK'),
            )
          ],
        ),
      );
    }
 }

void eliminarIcono(int index){
  //print(obj.keys.toList()[(index/2).toInt()]); 
  //print(obj.keys.elementAt((index/2).toInt())); 
  showDialog<String>(
        context: context,
        builder: (BuildContext context) => AlertDialog(
          title: const Text('Delete icon'),
          content: Text('Are you sure?'),
          actions: <Widget>[
            TextButton(
              onPressed: (){ 
                setState(() {
                   changed = true; 
                   _painters[index] = Column(crossAxisAlignment: CrossAxisAlignment.start,
                          children: <Widget>[
                            Text('')]);
                   _painters[(index + 1)] =Column(crossAxisAlignment: CrossAxisAlignment.start,
                          children: <Widget>[
                            Text('Delete...')]);
                     obj.remove(nameIconsBuff[(index/2).toInt()]);    
                   }); 
                Navigator.pop(context, 'YES');
                },
              child: const Text('YES'),
            ),
            TextButton(
              onPressed: () => Navigator.pop(context, 'NO'),
              child: const Text('NO'),
            )
          ],
        ),
      );
  
} 

void cambiarNombre(int index){
   if(!obj.containsKey(nameIcons[(index/2).toInt()])){
      setState(() {
            changed = true;  
            var dataOBJ = jsonEncode(obj);
            dataOBJ = dataOBJ.replaceAll(nameIconsBuff[(index/2).toInt()], nameIcons[(index/2).toInt()]);
            obj = jsonDecode(dataOBJ);
            String key = nameIcons[(index/2).toInt()]; 
            nameIconsBuff[(index/2).toInt()] = key;
            var _controller = TextEditingController();
            _painters[(index + 1)] = Column( crossAxisAlignment: CrossAxisAlignment.start,
                    children: <Widget>[
                      Text('$key'),
                      TextField(
                        controller: _controller,
                        inputFormatters: <TextInputFormatter>[
                          FilteringTextInputFormatter.allow(RegExp(r'[A-Za-z0-9_-]')),
                        ],
                        decoration: InputDecoration(
                          //border: InputBorder.none,
                          hintText: '$key',
                          suffixIcon: IconButton(
                            onPressed: _controller.clear,
                            icon: Icon(Icons.clear),
                          )
                        ),
                        onChanged: (value){
                          setState(() {
                              nameIcons[(index/2).toInt()] = (value != ''?value:key);                  
                            });
                        }
                      ),            
                      const SizedBox(
                        height: 5,
                      ),
                      Row(
                        children: <Widget>[
                        IconButton(
                          //key: const Key('holis'),
                          icon: const Icon(Icons.save),
                          tooltip: 'New name',
                          onPressed: () {cambiarNombre(index);},
                        ),
                      IconButton(
                          icon: const Icon(Icons.delete),
                          tooltip: 'Delete',
                          onPressed: () => eliminarIcono(index),
                        ),]),
                    ]);
                  _controller.clear();
          }); 
   }else{
      showDialog<String>(
              context: context,
              builder: (BuildContext context) => AlertDialog(
                title: const Text('Error'),
                content: Text(nameIcons[(index/2).toInt()] + ' already exists.'),
                actions: <Widget>[
                  TextButton(
                    onPressed: () => Navigator.pop(context, 'OK'),
                    child: const Text('OK'),
                  )
                ],
              ),
            );
   }
}

void backup() async{
  String? outputFile = await saveFile(
   // dialogTitle: 'Please select an backup file:',
    type: FileType.custom,
    allowedExtensions: ['json']
  );
  if (outputFile == null)return;

  final fileBack = File(outputFile);

  final file = new File(dir + 'iconext.json');

  fileBack.writeAsString(file.readAsStringSync());

}

void salir(){
  if(changed)
    showDialog<String>(
          context: context,
          builder: (BuildContext context) => AlertDialog(
            title: const Text('Unsaved changes'), 
            content: Text('Discard changes?'),
            actions: <Widget>[
              TextButton(
                onPressed: (){ 
                    exit(0);
                  },
                child: const Text('YES'),
              ),
              TextButton(
                onPressed: () => Navigator.pop(context, 'NO'),
                child: const Text('NO'),
              )
            ],
          ),
        );
    else
      if(save)
          showDialog<String>(
              context: context,
              builder: (BuildContext context) => AlertDialog(
                title: const Text('Warning'),
                content: Text('*You need to logout to see the changes.'),
                actions: <Widget>[
                  TextButton(
                    onPressed: (){ 
                        exit(0);
                      },
                    child: const Text('OK'),
                  )
                ],
              ),
            );
        else  
            exit(0);
}

@override
void initState() {
  //super.initState();
  FlutterWindowClose.setWindowShouldCloseHandler(() async {
    salir();
    return false;
  });
}

 String dir = ""; 
 bool changed = false;
 bool save = false;
 Map obj = {};

  @override
 Widget build(BuildContext context) {
    // This method is rerun every time setState is called, for instance as done
    // by the _incrementCounter method above.
    //
    // The Flutter framework has been optimized to make rerunning build methods
    // fast, so that you can just rebuild anything that needs updating rather
    // than having to individually change instances of widgets.
    
    //*Map obj = jsonDecode('{"Uno":"0","Dos":"2"}');
    //Map objDonas = {"Tes":"3","Cuatro":"4","Uno":"5"}; // de esta forma marca error al querer realizar la union
    //*Map objDonas = jsonDecode(jsonEncode({"Tes":"3","Cuatro":"4","Uno":"1"}));

    //*obj.addAll(objDonas);

    //*obj["Cinco"] = 5;
    
    //print(obj);

    //obj.remove('Cuatro');

    //print(jsonEncode(obj)); //-->muestra el obj codificado


    if( dir == '')
      localPath().then((String result){
         setState(() {
              dir = result;
              jsonInit();
              });
      });

    return Scaffold(
      appBar: AppBar(
        leading: IconButton(
        onPressed: () { changed = false; save = true; guardar();},
        tooltip: 'Save',
        icon: const Icon(Icons.save),
      ),
     // leadingWidth: 100,
        title: Text(widget.title),
        actions:[IconButton(
        onPressed: backup,
        tooltip: 'Backup',
        icon: const Icon(Icons.backup),
      ), 
        IconButton(
        onPressed: () => salir(),
        tooltip: 'Exit',
        icon: const Icon(Icons.logout),
      )],
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            //Text('*Al guardar los cambios, se veran reflejados en el proximo reinicio.'),
            /*Text(
              '$_counter',
              style: Theme.of(context).textTheme.headline4,
            ),
             ElevatedButton(
         onPressed: jsonInit,
         child: const Text('Leer JSON...'),
         ),
          const SizedBox(
          height: 5,
         ),    
         ElevatedButton(
         onPressed: () => exit(0),
         child: const Text('Salir...'),
         ),*/ 
         Expanded(
          child: GridView.extent(
            shrinkWrap: true,
            maxCrossAxisExtent: 140.0,//203.0,
            padding: const EdgeInsets.all(4.0),
            mainAxisSpacing: 4.0,
            crossAxisSpacing: 4.0,
            children: _painters.toList(),
          ),
        ),   
          ],
        ),
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: abrirArchivo, 
        //_incrementCounter,
        tooltip: 'Add',
        child: const Icon(Icons.add),
      ), // This trailing comma makes auto-formatting nicer for build methods.
    );
  }
}
