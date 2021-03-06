#
# <img src="https://github.com/bernardosegura/solarDesktop/blob/master/solar.svg" height="50px" width="50px" /> Solar Desktop
Es un entorno de escritorio para Linux, con temática geek, el cual es un __Fork__ del proyecto [eDEX-UI](https://github.com/GitSquared/edex-ui).
<br>[v3.1.0-0606.22-beta](https://github.com/bernardosegura/solarDesktop/releases/tag/3.1.0-0606.22-beta)
#
<p align="justify"> Actualmente este entorno reutiliza componentes del entorno de escritorio mate, así que para tener una experiencia más agradable y compatible es necesario tener instalado este escritorio como un tema alternativo, sin embargo no es obligatorio para su funcionamiento. En un futuro puede ser adaptable a otros entornos de escritorio y poder reutilizar módulos que ya controlan ciertos aspectos del equipo o en su defecto crear estos componentes para que quede de manera autónoma.</p>

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
__Construcción__ en solar-edex
```bash
$ npm run build
```

El entorno cuenta con 2 tipo de aplicaciones.

1. __Binarias (X Window)__
2. __Nativas del Entorno (modulos xobj)__

<p align="justify"> Las <b>Binarias</b> son las nativas gráficas del sistema operativo las cuales son ejecutadas fuera de el entorno y se indica en la parte izquierda la etiqueta <b>X11</b> las aplicaciones gráficas en ejecución, al dar click se activa el panel lateral derecho y muestra las aplicaciones en ejecución, así como también usando la combinación de teclas <i>Alt+W</i>. Se accede a ellas dando click en el panel lateral derecho en la aplicación o con <i>Alt+Tab</i> para avanzar entre ellas a la Derecha y <i>Ctrl+Alt+Tab</i> para avanzar entre ellas a la Izquierda. La combinación <i>Win+Tab</i> regresa a la pantalla principal (el entorno).
Los binarios que no utilizan interfaz gráfica (X Window) o que su ejecución es en terminal, pueden ser ejecutadas directamente en el entorno ya que se cuenta con 5 terminales tty para su utilización simultánea.</p>

<p align="justify"> Las <b>Nativas del Entorno</b> son módulos los cuales se encuentran en la parte inferior y estos pueden ser pequeñas aplicaciones en javascript o accesos directos a diferentes aplicaciones tanto internas como externas del entorno. Estas son alojadas en el directorio <i>modulos</i> en la carpeta <i>home</i> del usuario que está ejecutando el entorno.</p>

# 
## Capturas de Pantalla
<table>
  <tr>
    <td>
      Pantalla con el tema por default (<b>espacial</b>)
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/desktop.png" />
    </td>
    <td>
      Pantalla con fondo personalizado <b>Alien</b>
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/confondo.png" />
    </td>
  </tr>
  <tr>
    <td>
      Pantallas con fondos personalizados <b>Anime Debian</b>
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/confondoanime.png" />
    </td>
    <td>
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/confondoanime2.png" />
    </td>
  </tr>
  <tr>
    <td>
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/confondoanime3.png" />
    </td>
    <td>
      Pantalla con fondo personalizado <b>Simulando SO</b>
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/confondoanimewin.png" />
    </td>
  </tr>
  <tr>
    <td>
      Pantalla personalizada <b>Terminal sin Fondo</b>
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/confondoanimewinsinfnd.png" />
    </td>
    <td>
      Pantalla con fondo personalizado <b>Anime Windows</b>
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/confondoanimewin2.png" />
    </td>
  </tr>
  <tr>
    <td>
      Pantalla de Administrador de Archivos <b>X Windows</b>
      <img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/fmanager.png" />
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
