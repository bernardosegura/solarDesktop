#
# <img src="https://github.com/bernardosegura/solarDesktop/blob/master/solar.svg" height="50px" width="50px" /> Solar Desktop
Es un entorno de escritorio para Linux, con temática geek, el cual es un __Fork__ del proyecto [eDEX-UI](https://github.com/GitSquared/edex-ui).
#
<p align="justify"> Actualmente este entorno reutiliza componentes del entorno de escritorio mate, así que para tener una experiencia más agradable y compatible es necesario tener instalado este escritorio como un tema alternativo, sin embargo no es obligatorio para su funcionamiento. En un futuro puede ser adaptable a otros entornos de escritorio y poder reutilizar módulos que ya controlan ciertos aspectos del equipo o en su defecto crear estos componentes para que quede de manera autónoma.</p>

# 
## Sistema Operativo
El sistema operativo que se utilizó para la instalación es __Debian__ en una laptop, esto por su versatilidad y documentación que existe, sin embargo es posible instalarlo en otros sistemas operativos. De momento el componente del instalador fue probado e instalado en __debian 10__ con gestor de sesiones __LightDM__.

# 
## Información General
El entorno cuenta con 2 tipo de aplicaciones.

1. __Binarias (X Window)__
2. __Nativas del Entorno (modulos xobj)__

Las __Binarias__ son las nativas gráficas del sistema operativo las cuales son ejecutadas fuera de el entorno y se indica en la parte izquierda la etiqueta __WND__ las aplicaciones gráficas en ejecución, se accede a ellas con _Alt+Tab_ para avanzar entre ellas a la Derecha y _Ctrl+Alt+Tab_ para avanzar entre ellas a la Izquierda. La combinación _wind+Tab_ regresa a la pantalla principal (el entorno).
Los binarios que no utilizan interfaz gráfica (X Window) o que su ejecución es en terminal, pueden ser ejecutadas directamente en el entorno ya que se cuenta con 5 terminales tty para su utilización simultánea.

Las __Nativas del Entorno__ son módulos los cuales se encuentran en la parte inferior y estos pueden ser pequeñas aplicaciones en javascript o accesos directos a diferentes aplicaciones tanto internas como externas del entorno. Estas son alojadas en el directorio _raiz/modulos_ del usuario que está ejecutando el entorno.

# 
## Capturas de Pantalla
Pantalla con el tema por default (_espacial_) del entorno
<img src="https://github.com/bernardosegura/solarDesktop/blob/master/img/desktop.png" />
