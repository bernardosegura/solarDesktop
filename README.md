#
# <img src="https://github.com/bernardosegura/solarDesktop/blob/master/solar.svg" height="50px" width="50px" /> Solar Desktop
Es un entorno de escritorio para Linux, con temática geek, el cual es un __Fork__ del proyecto [eDEX-UI](https://github.com/GitSquared/edex-ui).
<br>[v4.0.0-0601.23-beta](https://github.com/bernardosegura/solarDesktop/releases/tag/4.0.0-0601.23-beta)
#
<p align="justify">Actualmente este entorno reutiliza componentes del entorno de escritorio mate, así que para tener una experiencia más automática es necesario tener instalado este escritorio como un tema alternativo, sin embargo no es obligatorio para su funcionamiento. La reutilización de elementos de otro sistema de escritorio se tendría que realizar de manera manual. Posiblemente en un futuro se integren estas aplicaciones directamente en este entorno.</p>

# 
## Sistema Operativo
<p align="justify"> El sistema operativo que se utilizó para la instalación es <b>Debian</b> en una laptop, esto por su versatilidad y documentación que existe, sin embargo es posible instalarlo en otros sistemas operativos. De momento el componente del instalador fue probado e instalado en <b>Debian 11</b> con gestor de sesiones <b>LightDM</b>.</p>

# 
## Información General
__Compilación__ en solar-edex
```bash
$ npm start 
# si se cuenta con una instancia ya ejecutándose
$ npm start dev 
```
* __Nota:__ esto solo es para el panel principal, para cada uno de los módulos, estos deben de compilarse de forma independiente.

__Construcción__ en solar-edex
```bash
$ npm run build
```

El entorno cuenta con 2 tipo de aplicaciones.

1. __Binarias (X Window)__
2. __Nativas del Entorno (modulos xobj)__

<p align="justify"> Las <b>Binarias</b> son las nativas gráficas del sistema operativo las cuales son ejecutadas fuera de el entorno y se indica en la parte izquierda con la etiqueta <b>X11</b> las aplicaciones gráficas en ejecución, en la parte superior se coloca el icono de estas aplicaciones para facilitar su acceso como si de una barra de tareas se tratara. Se accede a ellas dando click en el icono de la aplicación o con <i>Alt+Tab</i> para avanzar entre ellas a la Derecha y <i>Ctrl+Alt+Tab</i> para avanzar entre ellas a la Izquierda. La combinación <i>Win+Tab</i> regresa a la pantalla principal (el entorno).
Los binarios que no utilizan interfaz gráfica (X Window) o que su ejecución es en terminal, pueden ser ejecutadas directamente en el entorno ya que se cuenta con 5 terminales tty para su utilización simultánea.</p>

<p align="justify"> Las <b>Nativas del Entorno</b> son módulos los cuales se encuentran en la parte inferior y estos pueden ser pequeñas aplicaciones en javascript o accesos directos a diferentes aplicaciones tanto internas como externas del entorno. Estas son alojadas en el directorio <i>modulos</i> en la carpeta <i>home</i> del usuario que está ejecutando el entorno.</p>

# 
## Capturas de Pantalla
<table>
  <tr>
    <td>
      Panel principal (tema por default <b>espacial</b>)
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/desktop.png" />
    </td>
    <td>
      Configurar módulos del panel (<b>App in Menu</b>)
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/configapps.png" />
    </td>
  </tr>
  <tr>
    <td>
      Aplicación de escritorio a modulo (<b>App Desktop to Module</b>)
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/desktopapps.png" />
    </td>
    <td>
       Enriquecer el entorno con más iconos (<b>Add Icon to System</b>)
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/addicons.png" />
    </td>
  </tr>
  <tr>
    <td>
      Ejecutar Modulo (<b>Run</b>)
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/runapps.png" />
    </td>
    <td>
      Aplicación web de chrome a módulo (<b>WebApp to Module</b>)
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/webtoapps.png" />
    </td>
  </tr>
  <tr>
    <td>
      Dispositivos USB (<b>USB Devices</b>)
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/usbdev.png" />
    </td>
    <td>
      Configuraciones (<b>Settings</b>)
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/settings.png" />
    </td>
  </tr>
  <tr>
    <td>
      Apagar sistema (<b>Power Off</b>)
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/poweroff.png" />
    </td>
    <td>
      Pantalla de Administrador de Archivos <b>Terminal</b>
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/fmanagerconsol.png" />
    </td>
  </tr>
  <tr>
    <td>
      Pantalla de Editor de Imagenes <b>Gimp</b>
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/gimp.png" />
    </td>
    <td>
      Pantalla de Interacción con Juegos <b>Emulación</b>
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/games.png" />
    </td>
  </tr>
  <tr>
    <td>
     <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/mario.png" />
    </td>
    <td>
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/mslug2.png" />
    </td>
  </tr>
</table>
